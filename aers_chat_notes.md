# 第一版 AERS

## 1. 启用方式

启用条件：

```text
AERS_MODE != 0
```

有约简时走 reduction AERS。

无约简时走 old AERS。

更准确地说，程序先执行约简：

```text
reduction(reduction_mode)
```

如果最终：

```text
remove_num == 0
```

就走 old AERS。

如果确实有点被约简掉，就走 reduction AERS。

## 2. 进入方式

进入条件：

```text
当前不在 AERS
当前无冲突
AERS 停滞超过 max_no_impr / 2
冷却期已过
能用原版扰动逻辑选出 seed move
```

当前默认值：

```text
max_no_impr = 100000
进入阈值 = 50000
```

注意：如果 big perturbation 触发过，`max_no_impr` 会按 Luby 序列变化，所以进入阈值也会跟着变。

big perturbation 调整 `max_no_impr` 的条件是：

```text
vertex_count < 100000
no_impr > max_no_impr
```

seed 选择：

```text
用原版扰动的同一套逻辑选 seed move
```

AERS 的区别是：

```text
原版扰动：选完直接扰动
AERS：选完后用这个点建区域，再执行这次扰动
```

## 3. 初始区域

区域定义：

```text
seed + seed 的一跳 remaining 邻居
```

只加入仍在当前搜索图里的点。

## 4. 区域内候选集维护

AERS 维护 3 个区域候选池。

### 4.1 区域 good 池

保存：

```text
区域内 good_node_color 不为空的点
```

也就是区域内当前存在可改进颜色的点。

区域内选 good move 时，只从这个池采样。

采样规模：

```text
choose_conflict_node_bms = 90
```

大图 `vertex_count > 100000` 时：

```text
choose_conflict_node_bms = 10
```

补充：如果 BMS 采样没选出可用 move，会继续在区域 good 池里顺序扫一遍兜底，但不会回到全局 good 池。

### 4.2 区域 conflict 池

保存：

```text
区域内 conflict_vertex_in_color > 0 的点
```

也就是区域内当前有冲突的点。

区域内修冲突时，只从这个池随机选点。

### 4.3 区域 boundary 池

保存：

```text
区域内还没扫完邻居的点
```

每个 boundary 点记录：

```text
下次从第几个邻居继续扫
```

扫完所有邻居后，这个点标记为 exhausted，以后不再作为扩张源。

## 5. 候选集如何更新

每次区域内执行 move 时：

```text
用 AERS 包装版 color_node 提交 move
提交过程中同步被影响的邻居
最后同步 moved node 自己
如果点还在区域内，就根据当前状态更新它在 good 池、conflict 池里的归属
如果采样时发现池里有脏点，就现场重新同步或踢掉
```

所以候选集不是每轮全量重建，而是：

```text
区域建立时初始化一次
之后靠 move 的局部影响增量维护
采样遇到脏点再懒修正
```

更准确地说，同步点嵌在 `color_node()` / `color_node_reduction()` 的原更新逻辑里。

`color_node` 已经更新某个邻居或 moved node 的 good/conflict 状态后，会顺手触发 AERS active 池同步。

## 6. 区域内搜索逻辑

AERS active 后：

```text
有区域 good move：只从区域 good 池选
没有 good move 但有冲突：只从区域 conflict 池修
无冲突：只在区域内做扰动
```

区域扰动采样规模：

```text
pertub_bms = 90
```

大图 `vertex_count > 100000` 时：

```text
pertub_bms = 10
```

注意：当前没有 boundary-priority perturb。

区域扰动仍然从整个 `aers_region_vertices` 里采样。

## 7. 区域扩张

每次区域内 move 后：

```text
如果 moved node 是 active boundary，就从它继续向外扩
```

每次最多加入：

```text
aers_boundary_expand_size = 50
```

扩张方式：

```text
从 moved boundary 上次停的位置继续扫邻居
遇到区域外 remaining 点就加入区域
每次 move 最多加 50 个
扫完则标记 exhausted
```

如果 moved node 不是 active boundary，本轮不会随机从其他 boundary 补扩张。

## 8. 退出方式

退出条件：

```text
区域停滞达到 max_no_impr / 4
区域扰动失败
区域修冲突失败
seed move 执行失败
```

当前默认值：

```text
max_no_impr = 100000
区域退出阈值 = 25000
```

找到新 best 时：

```text
不退出
只清空区域停滞计数
继续在当前区域搜
```

补充：如果区域修冲突失败，AERS 会退出当前区域，然后 fallback 执行一次全局冲突修复。

## 9. 退出后冷却

退出后设置：

```text
cooldown_until = 当前 aers_no_impr + max_no_impr / 2
```

当前默认冷却长度：

```text
50000
```

也就是退出后至少还要再经历约 50000 次 AERS 视角下的无改进计数，才允许再次进入。

如果 `max_no_impr` 被 big perturbation 改过，冷却长度也会随之变化。

## 10. 诊断开关

新增诊断开关：

```text
AERS_DIAG
```

当前默认值：

```text
aers_diag = 0
```

也就是默认关闭诊断。

启用诊断：

```text
AERS_DIAG=1
```

关闭诊断时：

```text
不统计 AERS 诊断计数
不调用 AERS 诊断用 clock()
不输出 AERS_SUMMARY
```

开启诊断时：

```text
统计 AERS enter / exit / success
统计区域大小、扩张次数、扫描边数、skip_marked
统计 boundary / good 池 / conflict 池相关指标
统计 AERS wrapper 和 inline sync 调用次数
统计各类 AERS exclusive time
输出 AERS_SUMMARY 到 stderr
```

注意：`AERS_DIAG` 不影响搜索逻辑。

也就是说：

```text
aers_no_impr / aers_region_no_impr 照常更新
active 池同步照常执行
边界扩张照常执行
进入 / 退出条件照常判断
good move / conflict repair / region perturb 的选择逻辑不变
```

`AERS_DIAG=0` 只是减少诊断开销，不是算法开关。

算法开关仍然是：

```text
AERS_MODE
```

## 总结

全局无冲突且停滞超过 `max_no_impr / 2` 后，用原版扰动逻辑选 seed，以 `seed + 一跳 remaining 邻居` 建区域。

区域内维护 good 池、conflict 池、boundary 池，后续 good move、冲突修复、扰动都限制在区域内。

每次 move 后只从被移动的 boundary 点继续向外扩，最多扩 50 个。

候选池靠 move 的局部影响增量同步，采样遇到脏点再懒修正。

找到新 best 不退出，只重置区域停滞。

区域停滞到 `max_no_impr / 4` 或区域操作失败就退出，并冷却 `max_no_impr / 2` 后才能再次进入。

`AERS_DIAG` 只控制诊断统计和 `AERS_SUMMARY` 输出，不改变 AERS 搜索行为。
