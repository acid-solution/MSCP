#pragma once

// 把本次测得的 AERS 增量耗时累计到指定计数器。
inline void aers_add_ticks(clock_t& bucket, clock_t start_clock) {
    if (!aers_diag) return;
    bucket += clock() - start_clock;
}

inline clock_t aers_diag_clock() {
    return aers_diag ? clock() : 0;
}

inline void aers_diag_inc(long long& bucket) {
    if (aers_diag) bucket++;
}

inline void aers_diag_add(long long& bucket, long long delta) {
    if (aers_diag) bucket += delta;
}

// 汇总默认诊断中互不重叠的 AERS 增量耗时。
inline clock_t aers_measured_extra_exclusive_ticks() {
    return aers_build_region_exclusive_ticks
        + aers_expand_exclusive_ticks
        + aers_choose_good_exclusive_ticks
        + aers_remove_conflict_exclusive_ticks
        + aers_perturb_choose_exclusive_ticks
        + aers_after_move_exclusive_ticks
        + aers_stop_region_exclusive_ticks;
}

// 判断顶点是否仍在当前可搜索图中。
inline bool aers_remaining_vertex(long v) {
    return v >= 0
        && v < (long)vertex_color.size()
        && v < (long)remove_indicator.size()
        && !remove_indicator[v]
        && vertex_color[v] != -1
        && remaining_vertex.exist(v);
}

// 判断顶点是否属于当前 stamp 标记的 AERS 区域。
inline bool aers_in_region(long v) {
    return v >= 0
        && v < (long)aers_region_mark.size()
        && aers_region_mark[v] == aers_region_stamp;
}

// 安全读取 color_choice，颜色下标不存在时返回 0。
inline short aers_safe_color_choice(long node, long color) {
    if (node >= 0
        && node < (long)color_choice.size()
        && color >= 0
        && color < (long)color_choice[node].size()) {
        return color_choice[node][color];
    }
    return 0;
}

// 按当前图规模分配并重置全部 AERS 状态数组。
void aers_init_storage() {
    long n = vertex_count + 1;
    aers_region_vertices.clear();
    aers_region_mark.assign(n, 0);
    aers_region_stamp = 1;

    aers_region_good_vertices.clear();
    aers_region_conflict_vertices.clear();
    aers_good_pool_mark.assign(n, 0);
    aers_conflict_pool_mark.assign(n, 0);
    aers_good_pool_pos.assign(n, -1);
    aers_conflict_pool_pos.assign(n, -1);

    aers_boundary_active_vertices.clear();
    aers_boundary_active_mark.assign(n, 0);
    aers_boundary_active_pos.assign(n, -1);
    aers_boundary_exhausted_mark.assign(n, 0);
    aers_boundary_next_index.assign(n, 0);

    aers_active = 0;
    aers_inline_sync = 0;
    aers_search_used = 0;
    aers_no_impr = 0;
    aers_region_no_impr = 0;
    aers_cooldown_until = 0;
    aers_seed_node = -1;
}

// 推进区域 stamp，stamp 过大时清空相关标记数组。
void aers_next_region_stamp() {
    aers_region_stamp++;
    if (aers_region_stamp > 1000000000) {
        fill(aers_region_mark.begin(), aers_region_mark.end(), 0);
        fill(aers_good_pool_mark.begin(), aers_good_pool_mark.end(), 0);
        fill(aers_conflict_pool_mark.begin(), aers_conflict_pool_mark.end(), 0);
        fill(aers_boundary_active_mark.begin(), aers_boundary_active_mark.end(), 0);
        fill(aers_boundary_exhausted_mark.begin(), aers_boundary_exhausted_mark.end(), 0);
        aers_region_stamp = 1;
    }
}

// 如果顶点在区域 good move 池中，就用 O(1) 删除它。
void aers_remove_good_active(long v) {
    if (v < 0 || v >= (long)aers_good_pool_mark.size()) return;
    if (aers_good_pool_mark[v] != aers_region_stamp) return;

    long pos = aers_good_pool_pos[v];
    long last = aers_region_good_vertices.back();
    aers_region_good_vertices[pos] = last;
    aers_good_pool_pos[last] = pos;
    aers_region_good_vertices.pop_back();
    aers_good_pool_mark[v] = 0;
    aers_good_pool_pos[v] = -1;
}

// 如果区域顶点还不在 good move 池中，就用 O(1) 加入它。
void aers_add_good_active(long v) {
    if (!aers_in_region(v)) return;
    if (aers_good_pool_mark[v] == aers_region_stamp) return;
    aers_good_pool_mark[v] = aers_region_stamp;
    aers_good_pool_pos[v] = (long)aers_region_good_vertices.size();
    aers_region_good_vertices.push_back(v);
}

// 如果顶点在区域冲突池中，就用 O(1) 删除它。
void aers_remove_conflict_active(long v) {
    if (v < 0 || v >= (long)aers_conflict_pool_mark.size()) return;
    if (aers_conflict_pool_mark[v] != aers_region_stamp) return;

    long pos = aers_conflict_pool_pos[v];
    long last = aers_region_conflict_vertices.back();
    aers_region_conflict_vertices[pos] = last;
    aers_conflict_pool_pos[last] = pos;
    aers_region_conflict_vertices.pop_back();
    aers_conflict_pool_mark[v] = 0;
    aers_conflict_pool_pos[v] = -1;
}

// 如果区域顶点还不在冲突池中，就用 O(1) 加入它。
void aers_add_conflict_active(long v) {
    if (!aers_in_region(v)) return;
    if (aers_conflict_pool_mark[v] == aers_region_stamp) return;
    aers_conflict_pool_mark[v] = aers_region_stamp;
    aers_conflict_pool_pos[v] = (long)aers_region_conflict_vertices.size();
    aers_region_conflict_vertices.push_back(v);
}

// 按当前 good/conflict 状态同步一个区域顶点的 active 池归属。
void aers_sync_active_vertex(long v) {
    if (!aers_active || v < 0 || v >= (long)vertex_color.size()) return;
    if (!aers_in_region(v)) return;

    if (!aers_remaining_vertex(v)) {
        aers_remove_good_active(v);
        aers_remove_conflict_active(v);
        return;
    }

    if (good_node_color[v].empty()) {
        aers_remove_good_active(v);
    } else {
        aers_add_good_active(v);
    }

    if (conflict_vertex_in_color[v] > 0) {
        aers_add_conflict_active(v);
    } else {
        aers_remove_conflict_active(v);
    }
}

// color_node 更新邻居时调用的轻量 AERS 同步入口，只统计调用次数。
inline void aers_inline_sync_active_vertex(long v) {
    aers_diag_inc(aers_inline_sync_call_count);
    aers_sync_active_vertex(v);
}

// 如果顶点在 active boundary 池中，就用 O(1) 删除它。
void aers_remove_boundary_active(long v) {
    if (v < 0 || v >= (long)aers_boundary_active_mark.size()) return;
    if (aers_boundary_active_mark[v] != aers_region_stamp) return;

    long pos = aers_boundary_active_pos[v];
    long last = aers_boundary_active_vertices.back();
    aers_boundary_active_vertices[pos] = last;
    aers_boundary_active_pos[last] = pos;
    aers_boundary_active_vertices.pop_back();
    aers_boundary_active_mark[v] = 0;
    aers_boundary_active_pos[v] = -1;
}

// 把未耗尽的区域顶点加入 active boundary 池。
void aers_add_boundary_active(long v) {
    if (!aers_in_region(v)) return;
    if (aers_boundary_exhausted_mark[v] == aers_region_stamp) return;
    if (aers_boundary_active_mark[v] == aers_region_stamp) return;
    aers_boundary_active_mark[v] = aers_region_stamp;
    aers_boundary_active_pos[v] = (long)aers_boundary_active_vertices.size();
    aers_boundary_active_vertices.push_back(v);
}

// 标记某个 boundary 顶点在当前区域内已经扫描完。
void aers_mark_boundary_exhausted(long v) {
    if (v < 0 || v >= (long)aers_boundary_exhausted_mark.size()) return;
    if (aers_boundary_exhausted_mark[v] != aers_region_stamp) {
        aers_diag_inc(aers_boundary_exhausted_count);
    }
    aers_boundary_exhausted_mark[v] = aers_region_stamp;
    aers_remove_boundary_active(v);
}

// 把 remaining 顶点加入区域，并登记为 boundary 扩张来源。
void aers_add_region_vertex(long v) {
    if (!aers_remaining_vertex(v) || aers_in_region(v)) return;
    aers_region_mark[v] = aers_region_stamp;
    aers_region_vertices.push_back(v);
    aers_boundary_next_index[v] = 0;
    aers_boundary_exhausted_mark[v] = 0;
    aers_sync_active_vertex(v);
    aers_add_boundary_active(v);
}

// 推进一个 boundary 顶点，直到加入一个外部邻居或扫描耗尽。
bool aers_advance_boundary_vertex(long v) {
    if (!aers_in_region(v)) {
        aers_diag_inc(aers_boundary_advance_stale);
        return false;
    }
    if (aers_boundary_exhausted_mark[v] == aers_region_stamp) {
        aers_diag_inc(aers_boundary_advance_skip_exhausted);
        return false;
    }

    long& idx = aers_boundary_next_index[v];
    while (idx < (long)temp_adjacency_list[v].size()) {
        long u = temp_adjacency_list[v][idx++];
        aers_diag_inc(aers_scan_edges);

        if (!aers_remaining_vertex(u)) continue;
        if (aers_in_region(u)) {
            aers_diag_inc(aers_skip_marked);
            continue;
        }

        aers_add_region_vertex(u);
        aers_diag_inc(aers_expand_added);
        if (idx < (long)temp_adjacency_list[v].size()) {
            aers_add_boundary_active(v);
        } else {
            aers_mark_boundary_exhausted(v);
        }
        return true;
    }

    aers_mark_boundary_exhausted(v);
    return false;
}

// 清空所有区域级池，并切换到新的 region stamp。
void aers_clear_region() {
    aers_region_vertices.clear();
    aers_region_good_vertices.clear();
    aers_region_conflict_vertices.clear();
    aers_boundary_active_vertices.clear();
    aers_region_no_impr = 0;
    aers_seed_node = -1;
    aers_next_region_stamp();
}

// 将 AERS 指标汇总输出到 stderr，不改变原 cout 结果格式。
void print_aers_metrics() {
    if (aers_mode == 0 || !aers_diag || !aers_search_used) return;

    double elapsed = (double)(clock() - begin_time) / CLOCKS_PER_SEC;
    clock_t measured_extra_ticks = aers_measured_extra_exclusive_ticks();
    double overhead_time = (double)measured_extra_ticks / CLOCKS_PER_SEC;
    double overhead_ratio = elapsed > 0 ? overhead_time / elapsed : 0.0;
    double avg_region_size = aers_total_region_iter > 0
        ? (double)aers_total_region_size / (double)aers_total_region_iter
        : 0.0;

    cerr << "AERS_SUMMARY"
         << " enter=" << aers_enter_count
         << " expand=" << aers_expand_calls
         << " exit=" << aers_exit_count
         << " success=" << aers_success_count
         << " total_region_iter=" << aers_total_region_iter
         << " avg_region_size=" << avg_region_size
         << " overhead_time=" << overhead_time
         << " overhead_ratio=" << overhead_ratio
         << " expand_calls=" << aers_expand_calls
         << " scan_edges=" << aers_scan_edges
         << " expand_added=" << aers_expand_added
         << " skip_marked=" << aers_skip_marked
         << " boundary_expand_calls=" << aers_expand_calls
         << " boundary_expand_added=" << aers_expand_added
         << " boundary_expand_scan_edges=" << aers_scan_edges
         << " boundary_exhausted_count=" << aers_boundary_exhausted_count
         << " boundary_advance_stale=" << aers_boundary_advance_stale
         << " boundary_advance_skip_exhausted=" << aers_boundary_advance_skip_exhausted
         << " boundary_pool_samples=" << 0
         << " boundary_pool_stale=" << 0
         << " boundary_pool_empty=" << 0
         << " move_boundary_expand_calls=" << aers_move_boundary_expand_calls
         << " move_boundary_skip_nonboundary=" << aers_move_boundary_invariant_miss
         << " move_boundary_stale=" << aers_move_boundary_stale
         << " move_boundary_skip_exhausted=" << aers_move_boundary_skip_exhausted
         << " move_boundary_invariant_miss=" << aers_move_boundary_invariant_miss
         << " move_boundary_expand_added=" << aers_move_boundary_expand_added
         << " choose_samples=" << aers_choose_samples
         << " choose_skip_empty_good=" << aers_choose_skip_empty_good
         << " choose_skip_locked=" << aers_choose_skip_locked
         << " remove_samples=" << aers_remove_samples
         << " remove_sample_miss=" << aers_remove_sample_miss
         << " good_pool_stale=" << aers_good_pool_stale
         << " conflict_pool_stale=" << aers_conflict_pool_stale
         << " aers_build_region_exclusive_time=" << (double)aers_build_region_exclusive_ticks / CLOCKS_PER_SEC
         << " aers_expand_exclusive_time=" << (double)aers_expand_exclusive_ticks / CLOCKS_PER_SEC
         << " aers_choose_good_exclusive_time=" << (double)aers_choose_good_exclusive_ticks / CLOCKS_PER_SEC
         << " aers_remove_conflict_exclusive_time=" << (double)aers_remove_conflict_exclusive_ticks / CLOCKS_PER_SEC
         << " aers_perturb_choose_exclusive_time=" << (double)aers_perturb_choose_exclusive_ticks / CLOCKS_PER_SEC
         << " aers_after_move_exclusive_time=" << (double)aers_after_move_exclusive_ticks / CLOCKS_PER_SEC
         << " aers_stop_region_exclusive_time=" << (double)aers_stop_region_exclusive_ticks / CLOCKS_PER_SEC
         << " aers_measured_extra_exclusive_time=" << overhead_time
         << " aers_commit_wrapper_calls=" << aers_commit_wrapper_calls
         << " aers_inline_sync_call_count=" << aers_inline_sync_call_count
         << endl;
}

// 从带索引的顶点池中用 BMS 打分选择 reduction 扰动 move。
bool aers_choose_perturb_move(Vertex_vec_with_index& pool, long bms,double conflict_weight,long& BestNode, long& BestColor) {
    clock_t start_clock = aers_diag_clock();
    if (pool.empty()) {
        aers_add_ticks(aers_perturb_choose_exclusive_ticks, start_clock);
        return false;
    }

    long best_node = -1;
    long best_color = -1;
    double best_choose_score = -std::numeric_limits<double>::max();

    for (long i = 0; i < bms; ++i) {
        long node = pool[rand() % pool.size()];
        if (!aers_remaining_vertex(node)) continue;

        long current_color = vertex_color[node];
        long new_color = rand() % (max_color - current_color + 1) + current_color + 1;
        long penalty_diff = get_penalty(node, current_color) - get_penalty(node, new_color);
        double choose_score = (current_color - new_color) + penalty_diff;

        for (auto v : temp_adjacency_list[node]) {
            if (aers_safe_color_choice(v, current_color) == 2 && vertex_color[v] > current_color) {
                long old_eff = vertex_color[v] + get_penalty(v, vertex_color[v]);
                long new_eff = current_color + get_penalty(v, current_color);
                choose_score += (old_eff - new_eff) / 3;
            }
            if (aers_safe_color_choice(v, current_color) == 1 && vertex_color[v] > current_color) {
                long old_eff = vertex_color[v] + get_penalty(v, vertex_color[v]);
                long new_eff = current_color + get_penalty(v, current_color);
                choose_score += old_eff - new_eff;
            }
            if (vertex_color[v] == new_color) choose_score -= conflict_weight;
        }

        if (choose_score > best_choose_score) {
            best_choose_score = choose_score;
            best_node = node;
            best_color = new_color;
        }
    }

    if (best_node == -1) {
        aers_add_ticks(aers_perturb_choose_exclusive_ticks, start_clock);
        return false;
    }
    BestNode = best_node;
    BestColor = best_color;
    aers_add_ticks(aers_perturb_choose_exclusive_ticks, start_clock);
    return true;
}

// 从 vector 顶点池中用 BMS 打分选择 reduction 扰动 move。
bool aers_choose_perturb_move(vector<long>& pool, long bms,double conflict_weight,long& BestNode, long& BestColor) {
    clock_t start_clock = aers_diag_clock();
    if (pool.empty()) {
        aers_add_ticks(aers_perturb_choose_exclusive_ticks, start_clock);
        return false;
    }

    long best_node = -1;
    long best_color = -1;
    double best_choose_score = -std::numeric_limits<double>::max();

    for (long i = 0; i < bms; ++i) {
        long node = pool[rand() % pool.size()];
        if (!aers_remaining_vertex(node)) continue;

        long current_color = vertex_color[node];
        long new_color = rand() % (max_color - current_color + 1) + current_color + 1;
        long penalty_diff = get_penalty(node, current_color) - get_penalty(node, new_color);
        double choose_score = (current_color - new_color) + penalty_diff;

        for (auto v : temp_adjacency_list[node]) {
            if (aers_safe_color_choice(v, current_color) == 2 && vertex_color[v] > current_color) {
                long old_eff = vertex_color[v] + get_penalty(v, vertex_color[v]);
                long new_eff = current_color + get_penalty(v, current_color);
                choose_score += (old_eff - new_eff) / 3;
            }
            if (aers_safe_color_choice(v, current_color) == 1 && vertex_color[v] > current_color) {
                long old_eff = vertex_color[v] + get_penalty(v, vertex_color[v]);
                long new_eff = current_color + get_penalty(v, current_color);
                choose_score += old_eff - new_eff;
            }
            if (vertex_color[v] == new_color) choose_score -= conflict_weight;
        }

        if (choose_score > best_choose_score) {
            best_choose_score = choose_score;
            best_node = node;
            best_color = new_color;
        }
    }

    if (best_node == -1) {
        aers_add_ticks(aers_perturb_choose_exclusive_ticks, start_clock);
        return false;
    }
    BestNode = best_node;
    BestColor = best_color;
    aers_add_ticks(aers_perturb_choose_exclusive_ticks, start_clock);
    return true;
}

// 从区域 good move 池中选择最好的 reduction good move。
long aers_choose_good_node_reduction(long bms, long& BestNode, long& BestColor) {
    clock_t start_clock = aers_diag_clock();
    long best_node = -1;
    long best_color = -1;
    double best_color_score = -std::numeric_limits<double>::max();

    // 给一个区域 good move 候选打分，并更新当前最优 move。
    auto inspect_candidate = [&](long node) {
        aers_diag_inc(aers_choose_samples);
        if (!aers_in_region(node) || good_node_color[node].empty()) {
            aers_diag_inc(aers_choose_skip_empty_good);
            aers_diag_inc(aers_good_pool_stale);
            aers_sync_active_vertex(node);
            return;
        }

        long new_color = good_node_color[node][rand() % good_node_color[node].size()];
        if (is_lock(node, new_color)) {
            aers_diag_inc(aers_choose_skip_locked);
            return;
        }

        long current_color = vertex_color[node];
        long penalty_diff = get_penalty(node, current_color) - get_penalty(node, new_color);
        double score = (current_color - new_color) + penalty_diff
            + conflict_weight * (aers_safe_color_choice(node, current_color)
                               - aers_safe_color_choice(node, new_color));

        if (score > best_color_score) {
            best_color_score = score;
            best_node = node;
            best_color = new_color;
        }
    };

    if (!aers_region_good_vertices.empty()) {
        for (long i = 0; i < bms && !aers_region_good_vertices.empty(); ++i) {
            long node = aers_region_good_vertices[rand() % aers_region_good_vertices.size()];
            inspect_candidate(node);
        }

        if (best_node == -1) {
            for (long pos = 0; pos < (long)aers_region_good_vertices.size(); ) {
                long before_size = (long)aers_region_good_vertices.size();
                inspect_candidate(aers_region_good_vertices[pos]);
                if ((long)aers_region_good_vertices.size() == before_size) pos++;
            }
        }
    }

    if (best_node == -1) {
        aers_add_ticks(aers_choose_good_exclusive_ticks, start_clock);
        return 0;
    }
    BestNode = best_node;
    BestColor = best_color;
    aers_add_ticks(aers_choose_good_exclusive_ticks, start_clock);
    return 1;
}

// 在 reduction 路径中只用区域冲突池修复一个冲突。
bool aers_remove_conflict_reduction() {
    clock_t start_clock = aers_diag_clock();
    while (!aers_region_conflict_vertices.empty()) {
        long node = aers_region_conflict_vertices[rand() % aers_region_conflict_vertices.size()];
        aers_diag_inc(aers_remove_samples);

        if (!aers_in_region(node) || conflict_vertex_in_color[node] <= 0) {
            aers_diag_inc(aers_remove_sample_miss);
            aers_diag_inc(aers_conflict_pool_stale);
            aers_sync_active_vertex(node);
            continue;
        }

        long new_color = max_color + 1;
        if (new_color >= COLOR_NUM) {
            for (long i = 0; i < COLOR_NUM; i++) {
                if (aers_safe_color_choice(node, i) == 0) {
                    new_color = i;
                    break;
                }
            }
        }
        if (new_color >= COLOR_NUM) new_color = new_color % COLOR_NUM;

        aers_add_ticks(aers_remove_conflict_exclusive_ticks, start_clock);
        bool ok = aers_color_node_reduction(node, new_color);
        if (!ok) return false;

        current_iter++;
        no_impr++;
        aers_no_impr++;
        aers_after_region_move_reduction(node);
        return true;
    }

    aers_add_ticks(aers_remove_conflict_exclusive_ticks, start_clock);
    return false;
}

// 从当前 AERS 区域采样并执行一个 reduction 扰动 move。
bool aers_region_perturbation_reduction(long bms, double conflict_weight) {
    long node = -1;
    long color = -1;
    if (!aers_choose_perturb_move(aers_region_vertices, bms, conflict_weight, node, color)) {
        return false;
    }

    if (!aers_color_node_reduction(node, color)) return false;
    current_iter++;
    no_impr++;
    aers_no_impr++;
    aers_after_region_move_reduction(node);
    return true;
}
