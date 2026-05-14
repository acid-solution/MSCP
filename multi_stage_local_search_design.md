# MSCP 多阶段局部搜索设计备忘

## 背景理解

当前 MSCP 局部搜索程序的实验时间通常很长，例如 7200 秒；图规模也很大，有些实例文件甚至达到数 GB。因此，不能把搜索过程简单理解为“贪心下降慢慢改进”。

还需要特别注意：MSCP 的优化目标不是“颜色越少越好”。很多实例表明，使用更多颜色反而可能使最终得分 `score` 更小。因此后续设计第二、三阶段时，不能把搜索方向简单叙述成减少颜色数或优先降色；更准确的目标始终是降低 `best_score`，颜色数量只是影响得分结构的因素之一。

实际运行中，初期贪心改进通常很快完成，程序的大部分时间更接近下面这个长期循环：

```text
小扰动 -> 修复冲突 -> 局部搜索 -> 找合法解 -> 尝试刷新 best_score
```

观察到的输出节奏也支持这一点：如果每次刷新历史最优解都输出，输出往往不是均匀出现，而是呈现：

```text
长时间没有输出
-> 突然连续出现很多输出
-> 再次进入长时间没有输出
```

这说明搜索过程可能具有“平台期 -> 突破点 -> 连续改进 -> 新平台期”的结构。原版第一阶段一旦命中突破点，往往能够连续改进；真正需要解决的问题，是长时间平台期中命中突破点的概率偏低。这里的“连续改进”描述的是搜索现象，不应理解为必须由第一阶段完成；后续阶段也应当能够作为独立搜索分布，自行完成搜索、修复和刷新 `best_score` 的过程。

## 阶段 1：完全原版结构

阶段 1 暂时定义为完全保留原版局部搜索结构，不做全局重构，也不急着实现完整三阶段。

对应当前代码中的两条主路径：

- `localsearch_old`
- `localsearch_reduction`

阶段 1 的主体逻辑保持为：

```text
choose_good_node / choose_good_node_reduction
-> 如果有好移动，执行 color_node / color_node_reduction
-> 否则修复冲突 remove_conflict_new4 / remove_conflict_new4_reduction
-> 如果 edge_conflict == 0，则计算 score
-> 如果 score 刷新 best_score，则 update_best_solution / update_best_solution_reduction
-> 合法解时执行原版小扰动 perturbation / perturbation_reduction
```

这里的小扰动可以继续保留在阶段 1 中。它每次主要是单点扰动，力度不大，适合作为原版搜索分布的一部分。

当前的大扰动 `big_pertub` 更像粗暴重启，不太适合解释为有逻辑的多阶段搜索策略。后续可以考虑删除或屏蔽它，再用新的第二/第三阶段机制替代。

## 阶段切换依据

阶段切换不应表述为“长时间没找到最优解”，因为真实最优解未知。

代码层面更准确的表述是：

```text
连续一段时间没有刷新历史最优解 best_score
```

也就是使用当前已有的 `no_impr`：

- `no_impr` 表示连续多少次迭代没有刷新 `best_score`。
- 一旦刷新历史最优解，就将 `no_impr` 重置为 0。

阶段判断可以先抽象为：

```cpp
if (no_impr < T1) {
    // 阶段 1：完全原版结构
} else {
    // 阶段 2：平台期策略
}
```

其中 `T1` 是后续需要实验确定的阶段切换阈值。它不代表“最优解未找到”，只代表“当前搜索分布已经连续较久没有刷新历史最优”。

## 阶段之间的关系

三阶段不是流水线结构，也不是“上一阶段为下一阶段做准备”的调用关系。

主循环在每一次迭代中只根据当前 `no_impr` / 迭代状态判断当前所属阶段，并只执行该阶段对应的 step：

```text
当前状态 -> get_search_stage()
         -> stage_one_step / stage_two_step / stage_three_step
```

第一、第二、第三阶段应当被视为三套相对独立的搜索分布。它们可以共享全局搜索状态，例如 `vertex_color`、`color_choice`、`edge_conflict`、`best_score`、`no_impr` 等，但不应假设某一阶段执行后必须交由另一阶段收尾。

特别地，第二阶段不应被设计成“只做扰动，然后交给第一阶段修复/下降”。如果第二阶段允许产生冲突，那么第二阶段内部也应当有自己的冲突处理、合法性恢复或后续搜索逻辑；如果第二阶段暂时不想处理冲突，则它应只执行保持合法性的 move。

当前代码中的 fallback 到第一阶段只是因为第二、三阶段仍是占位函数，为避免空转而保留的临时工程保护。等第二、三阶段策略实现后，fallback 不应被视为算法设计的一部分。

## 阶段 2 的目标

阶段 2 不是继续做普通贪心，也不是简单加大随机扰动强度。

阶段 2 的目标应当是：

```text
当阶段 1 所代表的原版搜索分布长时间没有刷新 best_score 时，
阶段 2 切换到另一套独立的搜索分布。
阶段 2 的目标不是辅助阶段 1，也不是简单加大扰动，
而是在当前平台期内，用不同的候选选择、移动评价或局部结构调整方式，
独立尝试刷新 best_score 或进入更有潜力的搜索区域。
```

这里需要避免武断假设平台期一定来自某一种原因。例如，“高颜色点被邻居挡住”可以作为待验证假设，但不能直接当作结论写进算法叙事。

更稳妥的叙事是：

- 阶段 1 使用原版搜索分布，适合快速下降，也能在命中突破点后连续改进。
- 在大规模实例上，原版小扰动的随机采样可能长期命不中有效突破点。
- 阶段 2 在长时间未刷新 `best_score` 后启动。
- 阶段 2 不是大规模破坏当前解，而是改变采样分布。
- 阶段 2 应优先搜索更可能影响 MSCP 目标函数的区域。

## 当前不做的事情

这份设计备忘只记录方向，当前不做以下事情：

- 不做全局重构。
- 不急着实现完整三阶段。
- 不把阶段 2 写成某个未经验证的固定原因。
- 不用硬截断、扫描上限、失败次数上限来伪装优化。
- 不为了节省时间而跳过本来必须维护或搜索的有效工作。

## 后续推进建议

后续可以一步一步做：

1. 先屏蔽或移除 `big_pertub` 对主循环的影响，避免旧的大扰动和新阶段机制混在一起。
2. 在现有主循环外层增加最小化的阶段判断，不改动阶段 1 内部逻辑。
3. 给阶段 2 增加清晰的统计信息，例如触发次数、阶段 2 总耗时、平均单次成本、触发后多久刷新 `best_score`。
4. 先验证“改变采样分布”是否真的提高突破点命中率，再决定是否扩展到第三阶段。

重要约束：任何新策略都应尽量保持搜索覆盖和状态维护的完整性。优化方向应优先减少重复维护、复用已有状态、改善候选采样质量，而不是通过任意上限跳过可能有效的搜索。

## 2026-05-12 框架改造记录

本次已经先完成三阶段局部搜索框架的最小侵入式搭建，尚未实现第二、三阶段的具体策略。

已完成的代码结构调整：

- 在 `ReduceColor.h` 中新增 `SearchStage`，阶段值为 `STAGE_ONE`、`STAGE_TWO`、`STAGE_THREE`。
- 新增 `get_search_stage()`，暂时使用 `max_no_impr_basic` 作为阶段基础阈值，不使用会被 Luby 逻辑动态修改的 `max_no_impr`。
- 新增 `elapsed_time()`，统一用 `clock() - begin_time` 计算当前运行时间。
- 将 `localsearch_old()` 的原版 while 主体抽成 `stage_one_step_old()`。
- 将 `localsearch_reduction()` 的原版 while 主体抽成 `stage_one_step_reduction()`。
- 新增 `stage_two_step_old()`、`stage_three_step_old()`、`stage_two_step_reduction()`、`stage_three_step_reduction()` 作为占位函数，当前全部返回 `false`。
- 两个主循环现在只负责检查 cutoff、判断当前阶段，并调用对应阶段函数。
- 当前 fallback 仅因为第二、三阶段仍是空占位函数；等具体策略实现后，不应把 fallback 视为算法设计的一部分。
- 第一阶段继续保留原版小扰动 `perturbation()` / `perturbation_reduction()`。
- 第一阶段已经不再调用 `big_pertub(...)`；旧的大扰动函数和相关全局变量暂时保留，后续第三阶段再设计替代机制。

当前行为边界：

- `localsearch(mode, cutoff)` 外部接口未改变。
- 命令行参数未改变。
- `strategy_mode`、Tabu、CC、CICC 相关逻辑未改变。
- 未新增第二、三阶段策略参数。
- 未引入平台期、escape 等具体策略命名。

验证记录：

- 当前环境没有 `make` 命令，但有 `mingw32-make`。
- 已用 `mingw32-make` 按 Makefile 编译通过。
- 已执行 `git diff --check`，未发现空白格式问题。
- 新增测试样例文件 `as-22july06`，后续可以用它做功能和回归测试。

## 2026-05-12 第二阶段第一版策略尝试与回退

曾尝试实现的第二阶段策略命名为：合法吸收型颜色类重分布搜索（Legal Absorbing Color-Class Redistribution）。

这版策略的直觉是：第二阶段不做降色搜索，也不以减少颜色数为目标，而是在合法解附近改变 `color_use_number` 的分布。它优先尝试把“中等规模颜色类”中的顶点合法吸收到“较大规模颜色类”中，使颜色类大小分布更偏斜。这里的大/中等只指颜色类顶点数量，不指颜色编号。

实现后已用 `as-22july06`、60 秒、seed 1 做过一次对比测试：

```text
baseline: as-22july06 27605 59.382 1 7265333
stage2:   as-22july06 27610 13.133 1 5430662
stats:    triggers=300000 absorb=134207 repair=2 no_candidate=165791 improve=0 samples=197495976
```

结果说明这版策略没有刷新历史最优解，`stage2_improve_count = 0`，最终 `best_score` 还略差于 baseline。因此该实现已经用 Git 回退，不能作为当前第二阶段方案继续推进。

这次尝试暴露出的错误教训：

- “让颜色类大小更偏斜”不能直接等价为降低 `best_score`。它最多是一个待验证启发，不能作为第二阶段的核心判断依据。
- 合法 move 虽然不会破坏 `edge_conflict`，但大量合法重分布 move 可能只是消耗迭代，并不产生有效突破点。
- `no_candidate` 很高，说明“平均规模以上作为吸收类、规模大于 1 作为来源类”的候选定义过粗，采样很容易找不到真正有意义的 move。
- `samples` 接近 2 亿，说明即使不做全图枚举、不对每个候选计算完整 score，采样路径本身也可能变成显著成本。
- 第二阶段独立不等于必须在每次触发时强行执行一个弱 move。后续第二阶段需要有自己的有效性判断和统计口径，避免为了避免空转而把搜索分布拖入低质量随机游走。
- 无候选、无 move、无冲突修复、也没有改变搜索状态的 step，不应该增加 `no_impr`。如果为了避免 while 死循环必须推进 `current_iter`，也不能同时把它当成一次“没有改进的有效搜索”。否则会污染阶段切换，使第二阶段在没有实际动作时继续推高 `no_impr`，可能过早进入第三阶段；同时还会把大量未改变解的 step 计入正常迭代，浪费迭代预算并降低有效搜索比例。
- `no_candidate` 不应只靠调小采样参数缓解。v1 的根本问题是从 `remaining_vertex` 盲抽点，再碰运气判断是否属于来源类、是否能进入吸收类，导致候选生成效率很低。后续方向应优先维护候选空间，而不是每次临时随机搜索“有没有这样的 move”。
- 统计输出是必要的；没有 `trigger / move / no_candidate / improve / sample` 这类指标，很难判断一个阶段是在创造搜索价值，还是只是在吞掉迭代次数。

后续如果继续设计第二阶段，应保留这些约束：

- 目标仍然是刷新 `best_score`，不是降色、减少颜色数，或单纯改变颜色类大小分布。
- 第二阶段仍应是独立搜索分布，不依赖第一阶段修复或收尾。
- 允许使用采样作为搜索分布，但不能把“采样没找到”解释成全局没有有效 move。
- 只有真正执行 move、修复冲突、或改变了解状态时，才应该更新 `no_impr`。无候选只是第二阶段当前没有有效动作，应单独计入 `stage2_no_candidate_count`，不应污染 `no_impr` 的阶段切换语义。
- v2 不应继续从 `remaining_vertex` 盲抽候选，而应优先维护精确候选集。候选应明确到 move 级别，即 `(v, c_src -> c_dst)`，并至少满足：`vertex_color[v] == c_src`、`c_src != c_dst`、`color_use_number[c_src] > 1`、`color_use_number[c_dst] >= color_use_number[c_src]`、`color_choice[v][c_dst] == 0`，以及预计 `delta_score < 0`。
- 第二阶段候选维护的核心问题应变成：当前有哪些点可以合法进入哪些目标颜色类，以及这些 move 中哪些可能改善 `color_use_number` 排序后的 `score`。采样可以用于候选集内部选择，但不应替代候选集维护本身。
- 新逻辑必须继续保留独立性能统计，至少能看出触发次数、实际 move 次数、无候选次数、刷新次数和采样成本。

## 2026-05-12 第二阶段 BMS 2x 参数实验

本次实验只提高第二阶段内部使用的 BMS，不改变第一阶段全局参数：

- 默认 `bms` 保持 90。
- 大图覆盖分支中的 `bms` 保持 10。
- 新增 `stage2_bms()`，返回当前 `bms * 2`。
- `stage_two_step_old()` / `stage_two_step_reduction()` 使用 `stage2_bms()` 调用 `choose_good_node*` 和小扰动。
- 第二阶段不再 fallback 到第一阶段；第三阶段仍保留占位 fallback。

实验目的：观察“只扩大第二阶段搜索采样规模”是否比第一阶段原版分布更容易刷新 `best_score`，同时记录它对迭代数的影响。

测试使用 `as-22july06`、60 秒、seed 1：

```text
baseline:       as-22july06 27605 57.974 1 7396931
stage2 bms 2x:  as-22july06 27609 4.765 1 6942084
stage2 stats:   triggers=125128 moves=171955 improve=4

stage3 disabled + stage2 bms 2x:
                as-22july06 27582 53.742 1 4285591
stage2 stats:   triggers=2009090 moves=2771622 improve=13
```

观察：

- 第二阶段 BMS 2x 确实触发，并产生了 4 次阶段内刷新。
- 但最终 `best_score` 为 27609，略差于 baseline 的 27605。
- `current_iter` 从 7396931 降到 6942084，说明第二阶段更大 BMS 带来一定额外成本，但没有像全局 BMS 2x 那样大幅压低迭代数。
- 60 秒和 70 秒测试中，第二阶段统计完全相同，说明多出来的时间没有继续执行第二阶段。
- 临时禁用第三阶段后，`no_impr >= T` 的情况继续跑第二阶段，第二阶段触发次数从 125128 增加到 2009090，刷新次数从 4 增加到 13，`best_score` 也改善到 27582。

失败经验：

- 原先 `Stage1 -> Stage2 -> Stage3` 的固定区间切换会让第二阶段过早退出；当第三阶段仍是占位函数时，后续搜索实际变成第三阶段 fallback 到第一阶段，第二阶段无法继续发挥作用。
- 因此，单看“Stage2 BMS 2x 初版结果略差”会误判。真正暴露的问题是阶段边界设计不合理，以及 Stage3 未实现时不应该让搜索自然流入 Stage3。
- 第二阶段 BMS 2x 本身不能直接判定失败；在禁用 Stage3 的临时实验中，它继续执行后确实产生更多刷新。
- 后续不应继续只叠加 BMS 倍率调参，而应先重新设计阶段持续条件：例如在 Stage3 未实现前，让 Stage3 条件继续映射到 Stage2，或给 Stage3 明确独立逻辑和统计，避免占位阶段吞掉有效搜索时间。

## 2026-05-14 第二阶段 age / vertex_freq 增强版一阶段搜索

本次第二阶段不再设计新的颜色类吸收策略，也不再单纯放大 BMS。新的第一版实现遵循更简单的原则：第二阶段照搬第一阶段的 step 结构，只替换候选 move 的评分分布。

阶段二的 old 路径评分为：

```cpp
base_score =
    current_color - new_color
    + conflict_weight * (old_conflict - new_conflict);

stage2_score =
    base_score
    + age_weight * age(node)
    - freq_weight * vertex_freq[node];
```

阶段二的 reduction 路径在原版 reduction 评分基础上保留 `penalty_diff`：

```cpp
base_score =
    (current_color - new_color)
    + penalty_diff
    + conflict_weight * (old_conflict - new_conflict);

stage2_score =
    base_score
    + age_weight * age(node)
    - freq_weight * vertex_freq[node];
```

这里冲突控制完全沿用第一阶段已有的 `conflict_weight * (old_conflict - new_conflict)`，不额外加入 `max_stage2_conflict_delta`、`damage_penalty` 或其他硬预算。也就是说，会增加冲突的 move 仍然可以参与比较，但会自然被第一阶段原有冲突项扣分。

`age(node)` 使用 `current_iter - last_move_iter[node]` 懒计算，不做每轮全图递增。`vertex_freq[node]` 记录顶点真实 recolor 次数。每次 `color_node` / `color_node_reduction` 真正执行改色后都会更新这些历史状态，因此阶段二选择时看到的是阶段一、小扰动和修复动作共同留下的搜索历史。

阶段二执行流程仍然照搬第一阶段：

```text
choose_stage2_node / choose_stage2_node_reduction
-> 如果有候选 move，则执行 color_node / color_node_reduction
-> 否则执行 remove_conflict_new4 / remove_conflict_new4_reduction
-> 如果 edge_conflict == 0，则检查并刷新 best_score
-> 合法解时继续执行原版小扰动 perturbation / perturbation_reduction
```

候选池继续使用第一阶段的 `valid_node` / `good_node_color`，采样规模沿用 `choose_conflict_node_bms`。因此这版阶段二不是“随机乱染色”，而是在第一阶段原有好 move 候选池内，用 `age` 和 `vertex_freq` 改变采样偏好。

Stage3 当前仍未实现。为了避免占位 Stage3 吞掉第二阶段有效搜索时间，当前阶段判断在 `no_impr >= max_no_impr_basic` 后继续执行 Stage2；等第三阶段有真实策略后，再恢复 `STAGE_THREE` 的独立调度。

新增统计输出：

```text
[STAGE2_STATS] triggers=... moves=... no_candidate=... improve=... samples=... repairs=...
```

这些统计只用于观察第二阶段触发次数、实际 move 次数、无候选次数、刷新次数、采样成本和冲突修复次数，避免只看最终 `best_score` 而误判阶段二是否真正工作。

初次 360 秒测试使用 `as-22july06`、seed 1：

```text
baseline: as-22july06 27597 125.46 1 50780309
stage2:   as-22july06 27635 0.928 1 44954575
stats:    triggers=31010990 moves=25218415 no_candidate=5792575 improve=0 samples=2261103524 repairs=3174349
```

这个结果不理想：阶段二大量触发并执行了很多 move，但 `stage2_improve_count = 0`，最终 `best_score` 明显差于 baseline。当前实现可以作为 age / vertex_freq 偏置方案的第一版失败样本保留，用于后续分析；不应在这个结果上继续盲目追加参数调整。
