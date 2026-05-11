# AERS 讨论与实验记录

本文件整理本次 Codex 会话中关于 AERS 的设计、实现、测试和结论。它不是逐字聊天记录，而是面向后续继续实验的决策日志。

## 背景

目标是在 MSCP reduction 局部搜索中实现 AERS：Adaptive Expanding Region Sampling。

AERS 的初始定位是：

- 不新增搜索算子。
- 不改 old 版局部搜索。
- 不替换全局结构。
- move 仍通过 `color_node_reduction()` 提交。
- 全局 `cost`、`color_choice`、`good_node_color`、`conflict_node_queue` 等结构继续共享。
- 默认不新增命令行参数，通过代码里的 `aers_mode` 手动开关。

当前源码中：

- `ReduceColor.cpp` 里 `aers_mode = 1`。
- 新增文件：`MSCP/aers.h`。
- `ReduceColor.h` 顶部包含 `#include "aers.h"`。
- summary 输出写到 `cerr`，不改变原始 `cout` 结果格式。

## 当前 AERS 总体流程

当前版本大致流程：

1. 只在合法解阶段触发 AERS。
2. 从全局 `remaining_vertex` 选择扰动 move。
3. 以扰动点和一跳邻居建立初始区域。
4. 提交扰动后进入区域搜索。
5. 区域内使用精确 active 池选择 good/conflict 点。
6. 找到新 best 后不退出 AERS，继续在当前区域深搜。
7. 当前区域长期没有新 best 后退出并换区域。
8. 区域扩张使用 boundary active 池，而不是从 move 点盲扩。

触发条件：

```text
edge_conflict == 0
aers_no_impr > max_no_impr / 2
aers_no_impr >= aers_cooldown_until
```

区域退出条件：

```text
aers_region_no_impr >= max_no_impr / 4
```

best 更新后：

```text
aers_success_count++
aers_region_no_impr = 0
不退出 AERS
```

## 当前核心数据结构

### 区域集合

```cpp
vector<long> aers_region_vertices;
vector<int> aers_region_mark;
long aers_region_stamp;
```

### 精确区域 active 池

当前不再从整个区域盲抽，而是维护两个精确池：

```cpp
vector<long> aers_region_good_vertices;
vector<long> aers_region_conflict_vertices;
vector<int> aers_good_pool_mark;
vector<int> aers_conflict_pool_mark;
vector<long> aers_good_pool_pos;
vector<long> aers_conflict_pool_pos;
```

含义：

```text
aers_region_good_vertices:
    区域内 good_node_color[v] 非空的点

aers_region_conflict_vertices:
    区域内 conflict_vertex_in_color[v] > 0 的点
```

相关函数：

```cpp
aers_add_good_active()
aers_remove_good_active()
aers_add_conflict_active()
aers_remove_conflict_active()
aers_sync_active_vertex()
aers_sync_active_around_vertex()
```

当前每次区域 move 后会同步 `node + node 的区域内邻居`。

这个改动有效降低了 miss：

```text
remove_sample_miss = 0
choose_skip_empty_good 很低
```

### boundary active 池

当前扩张不再从 move 点直接扩，而是维护 boundary 池：

```cpp
vector<long> aers_boundary_active_vertices;
vector<int> aers_boundary_active_mark;
vector<long> aers_boundary_active_pos;
vector<int> aers_boundary_scan_mark;
vector<int> aers_boundary_exhausted_mark;
vector<long> aers_boundary_next_index;
```

相关函数：

```cpp
aers_add_boundary_active()
aers_remove_boundary_active()
aers_mark_boundary_exhausted()
aers_advance_boundary_vertex()
aers_expand_from_boundary_pool()
```

当前扩张方式：

```text
从 boundary pool 随机抽一个点
推进它的 next_index，跳过不在 remaining 或已在区域的邻居
如果找到区域外邻居，就加入一个点
如果该 boundary 点还没扫完，则重新放回 boundary pool
每次区域 move 后最多扩张 aers_boundary_expand_size = 50 个点
```

注意：曾尝试“确认入池 + 单个 boundary 连续吸收邻居”的版本，质量和迭代都不理想，已回退。

## 已尝试方案与结论

### 1. 初始 BFS / 候选池扩张

最初 AERS 是停滞时建立 BFS 局部区域，choose/remove/perturb 从区域采样。

问题：

- BFS 和扩张扫描开销偏大。
- 区域里多数点没有 good/conflict，采样命中低。
- 后来转为“合法解阶段扰动建区域”。

### 2. 合法解触发 + 扰动影响区

改为：

```text
合法解停滞
-> 全局选择扰动点
-> 扰动点 + 一跳邻居建初始区域
-> 区域修复
```

这个方向更符合直觉：合法解阶段本来 good move 少，AERS 应该靠扰动制造局部修复结构。

### 3. best 后不退出 AERS

原先找到一次新 best 就退出 AERS。

后来改为：

```text
找到 best 不退出
继续压榨当前区域
直到区域 no-impr 达到阈值
```

这个方向在 `cnr-2000` 上明显有效。

### 4. 惰性区域 active 池

曾维护：

```cpp
aers_region_good_vertices
aers_region_conflict_vertices
```

但只是惰性加入，抽到过期点才删除。

问题：

```text
good_pool_stale 高
conflict_pool_stale 高
remove_sample_miss 高
```

### 5. 精确区域 active 池

加入 O(1) 删除位置表后，区域 active 池变精确。

结论：

- `remove_sample_miss` 可降到 0。
- `choose_skip_empty_good` 可降到很低。
- 这是有效方向，应保留。

### 6. 降频轻量版

曾尝试：

```text
boundary_expand_size = 20
boundary_expand_interval = 20
neighbor_sync_interval = 10
```

结论：

- 迭代数提高。
- 质量明显下降。
- 这是硬截断/固定间隔类方案，不符合后续方向，已回退。

### 7. soft_region_size

曾尝试：

```text
soft_region_size = 5000
```

结论：

- 质量明显下降。
- 相当于硬限制区域展开。
- 已删除。
- 后续不要再提或使用这类靠硬阈值省成本的方案。

### 8. exhausted 标记

对已扫完邻接表的 boundary 点标记 exhausted，后续不再从它扩。

结论：

- 有价值。
- 不新增调参参数。
- 可保留。

### 9. boundary active 池

将扩张来源从 move 点改为 boundary pool。

结论：

- 质量提升明显。
- 扩张调用次数下降。
- 但 `skip_marked / scan_edges` 仍然很高。
- 当前主要瓶颈转移到 boundary 池 stale / next_index 过期。

### 10. 确认入池 + 连续吸收

尝试：

```text
新点入 boundary pool 前先确认有外部邻居
选中一个 boundary 后连续吸收外部邻居
```

结果不佳：

```text
score = 690078
iter  = 3125380
avg_region_size = 119434
```

问题：

- 区域膨胀过大。
- 迭代下降。
- 没有真正降低 marked 扫描。

结论：已回退。

## 重要测试结果

### no-AERS 60 秒对照

当前记住的 no-AERS 对照：

```text
score = 699990
iter  = 5861741
```

### 精确 active 池 + exhausted，但无 boundary pool

```text
score = 693316
iter  = 4087412
迭代比例 ≈ 69.7%
```

### 当前 boundary-pool 版本

```text
score = 688930
iter  = 3793330
```

相对 no-AERS：

```text
score 改善 = 11060
迭代比例 ≈ 64.7%
```

关键 summary：

```text
enter=16
expand=75572
exit=15
success=1179
total_region_iter=1972036
avg_region_size=4688.77
overhead_time=1.927
overhead_ratio=0.0323849
expand_calls=75572
scan_edges=83860657
expand_added=3778175
skip_marked=80082129
boundary_expand_calls=75572
boundary_expand_added=3778175
boundary_expand_scan_edges=83860304
boundary_exhausted_count=3423678
boundary_pool_samples=7201853
boundary_pool_stale=3423678
boundary_pool_empty=15
choose_samples=18807554
choose_skip_empty_good=1896
choose_skip_locked=4913983
remove_samples=97435
remove_sample_miss=0
good_pool_stale=1896
conflict_pool_stale=0
```

### 大规模 result 文件对比

比较 `reduction.txt` 与 `dp约简.txt` 的共同实例/seed：

```text
共同可比结果：413 组
AERS 更好：125 组
AERS 更差：244 组
持平：44 组
```

按实例平均：

```text
共同实例数：43 个
AERS 平均更好：14 个
AERS 平均更差：27 个
基本持平：2 个
```

结论：

```text
当前 AERS 不是普适性提升。
在 rgg 系列图上更好。
在很多真实网络/网页/引用类图上变差。
```

特别差的实例包括：

```text
eu-2005
web-Stanford
coPapersDBLP
coPapersCiteseer
cnr-2000 在 result 文件里也变差
```

## 明确不要再提的方向

用户明确指出这些是硬截断，会影响效果，不要再作为优化建议：

```text
失败上限
扫描上限
固定扩张间隔
固定同步间隔
区域大小上限
move 数上限
soft_region_size
单点扫描 step limit
```

原则：

```text
不要靠人为阈值省时间。
不要掩耳盗铃式地提前判定区域不可用。
优化应减少重复工作、复用已有计算、改进数据结构。
```

## 当前主要问题

当前最大瓶颈：

```text
skip_marked / scan_edges ≈ 95.5%
```

解释：

```text
boundary 点当初有区域外邻居；
但随着区域扩张，这些邻居可能已经被其他 boundary 点加入区域；
再次抽到该 boundary 点时，需要跳过大量已在区域内的 marked 邻居。
```

当前 choose/remove miss 已基本不是主要问题：

```text
remove_sample_miss = 0
choose_skip_empty_good 很低
```

## 后续更合理的优化方向

不要再加硬阈值。更合理的方向是复用已有计算、减少重复扫描。

最重要的思路：

```text
color_node_reduction() 内部本来已经扫描 node 的邻居，
并维护 good_node_color / conflict_vertex_in_color / valid_node / conflict_node_queue。
当前 AERS 在 move 后又调用 aers_sync_active_around_vertex(node)，重复扫一遍邻居。
```

下一步可考虑：

```text
把 AERS active 池同步挂进 color_node_reduction() 的邻居更新循环里。
当 color_node_reduction 已经更新某个点 v 的 good/conflict 状态时，
顺手调用 aers_sync_active_vertex(v)。
```

这样不是硬截断，也不会漏同步，而是消除重复工作。

类似地，boundary 状态也应尽量在已有邻居更新过程中顺手更新，而不是后续抽出来才发现过期。

## 当前代码状态备注

当前已回退到上一版 boundary-pool 逻辑。

保留：

- 精确区域 active 池。
- boundary active 池。
- exhausted 标记。
- `aers_total_region_size` 改成 `long long`，防止 Windows 下 summary 溢出。

已删除/回退：

- `soft_region_size`。
- 轻量降频参数。
- 确认入池 helper。
- 单 boundary 连续吸收。

## 编译与测试命令

AERS 编译：

```bash
g++ -std=c++11 -O3 ReduceColor.cpp -o ReduceColor.exe
```

AERS 60 秒测试：

```bash
.\ReduceColor.exe ..\cnr-2000 60 1
```

no-AERS 临时编译：

```powershell
(Get-Content .\ReduceColor.cpp) -replace 'aers_mode = 1;', 'aers_mode = 0;' | g++ -std=c++11 -O3 -x c++ -I. -o ReduceColor_noaers.exe -
```

no-AERS 60 秒测试：

```bash
.\ReduceColor_noaers.exe ..\cnr-2000 60 1
```

