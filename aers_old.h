
// 提交 old-search move，并临时打开 AERS active 池内联同步。
bool aers_color_node(long node, long color, bool lock_it) {
    aers_commit_wrapper_calls++;
    aers_inline_sync = 1;
    bool ok = color_node(node, color, lock_it);
    aers_inline_sync = 0;
    return ok;
}

// 从带索引的顶点池中用 BMS 打分选择 old-search 扰动 move。
bool aers_choose_perturb_move_old(Vertex_vec_with_index& pool, long bms,double conflict_weight,long& BestNode, long& BestColor) {
    clock_t start_clock = clock();
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
        double choose_score = current_color - new_color;

        for (auto v : temp_adjacency_list[node]) {
            if (aers_safe_color_choice(v, current_color) == 2 && vertex_color[v] > current_color) {
                choose_score += (vertex_color[v] - current_color) / 3;
            }
            if (aers_safe_color_choice(v, current_color) == 1 && vertex_color[v] > current_color) {
                choose_score += vertex_color[v] - current_color;
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

// 从 vector 顶点池中用 BMS 打分选择 old-search 扰动 move。
bool aers_choose_perturb_move_old(vector<long>& pool, long bms,double conflict_weight,long& BestNode, long& BestColor) {
    clock_t start_clock = clock();
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
        double choose_score = current_color - new_color;

        for (auto v : temp_adjacency_list[node]) {
            if (aers_safe_color_choice(v, current_color) == 2 && vertex_color[v] > current_color) {
                choose_score += (vertex_color[v] - current_color) / 3;
            }
            if (aers_safe_color_choice(v, current_color) == 1 && vertex_color[v] > current_color) {
                choose_score += vertex_color[v] - current_color;
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

// 从区域 good move 池中选择最好的 old-search good move。
long aers_choose_good_node(long bms, long& BestNode, long& BestColor) {
    clock_t start_clock = clock();
    long best_node = -1;
    long best_color = -1;
    double best_color_score = -std::numeric_limits<double>::max();

    // 给一个 old-search 区域 good move 候选打分，并更新当前最优 move。
    auto inspect_candidate = [&](long node) {
        aers_choose_samples++;
        if (!aers_in_region(node) || good_node_color[node].empty()) {
            aers_choose_skip_empty_good++;
            aers_good_pool_stale++;
            aers_sync_active_vertex(node);
            return;
        }

        long new_color = good_node_color[node][rand() % good_node_color[node].size()];
        if (is_lock(node, new_color)) {
            aers_choose_skip_locked++;
            return;
        }

        long current_color = vertex_color[node];
        double score = current_color - new_color
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

// 在 old-search 路径中只用区域冲突池修复一个冲突。
bool aers_remove_conflict() {
    clock_t start_clock = clock();
    while (!aers_region_conflict_vertices.empty()) {
        long node = aers_region_conflict_vertices[rand() % aers_region_conflict_vertices.size()];
        aers_remove_samples++;

        if (!aers_in_region(node) || conflict_vertex_in_color[node] <= 0) {
            aers_remove_sample_miss++;
            aers_conflict_pool_stale++;
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
        if (!aers_color_node(node, new_color)) return false;

        current_iter++;
        no_impr++;
        aers_no_impr++;
        aers_after_region_move_reduction(node);
        return true;
    }

    aers_add_ticks(aers_remove_conflict_exclusive_ticks, start_clock);
    return false;
}

// 从当前 AERS 区域采样并执行一个 old-search 扰动 move。
bool aers_region_perturbation(long bms, double conflict_weight) {
    long node = -1;
    long color = -1;
    if (!aers_choose_perturb_move_old(aers_region_vertices, bms, conflict_weight, node, color)) {
        return false;
    }

    if (!aers_color_node(node, color)) return false;
    current_iter++;
    no_impr++;
    aers_no_impr++;
    aers_after_region_move_reduction(node);
    return true;
}

// 通过一次全局 seed 扰动启动 old-search AERS 区域。
bool aers_start_region(long bms, double conflict_weight) {
    long seed = -1;
    long color = -1;
    if (!aers_choose_perturb_move_old(remaining_vertex, bms, conflict_weight, seed, color)) {
        return false;
    }

    aers_build_region(seed);
    if (!aers_color_node(seed, color)) {
        aers_stop_region_reduction();
        return false;
    }

    current_iter++;
    no_impr++;
    aers_no_impr++;
    aers_after_region_move_reduction(seed);
    return true;
}

// 检查 old-search 进入条件，搜索停滞时启动 AERS。
bool aers_update_region() {
    if (aers_mode == 0 || aers_active) return false;
    if (edge_conflict != 0) return false;
    if (aers_no_impr <= max_no_impr / 2) return false;
    if (aers_no_impr < aers_cooldown_until) return false;
    return aers_start_region(pertub_bms, conflict_weight);
}

void localsearch_aers(int cutoff) {
    aers_search_used = 1;
    if (conflict_weight == 0) conflict_weight = 1;
    big_pert_node_num = vertex_count / big_pertub_num_k;
    if (big_pert_node_num > 500) big_pert_node_num = 500;

    while (current_iter < max_iter) {
        long best_node = -1;
        long best_color = -1;

        if (aers_active) {
            long x = aers_choose_good_node(choose_conflict_node_bms, best_node, best_color);
            if (x == 1 && best_node != -1) {
                aers_color_node(best_node, best_color);
                current_iter++;
                no_impr++;
                aers_no_impr++;
                aers_after_region_move_reduction(best_node);
            } else if (edge_conflict > 0) {
                if (!aers_remove_conflict()) {
                    aers_stop_region_reduction();
                    remove_conflict_new4();
                    aers_no_impr++;
                }
            }
        } else {
            long x = choose_good_node(choose_conflict_node_bms, best_node, best_color);
            if (x == 1 && best_node != -1) {
                color_node(best_node, best_color);
                current_iter++;
                no_impr++;
                aers_no_impr++;
            } else {
                remove_conflict_new4();
                aers_no_impr++;
            }
        }

        long score = 0;
        if (edge_conflict == 0) score = compute_best_score();

        best_time = clock();
        double run_time = (double)(best_time - begin_time) / CLOCKS_PER_SEC;
        if (edge_conflict == 0 && score < best_score) {
            update_best_solution();
            final_time = run_time;
            no_impr = 0;
            aers_no_impr = 0;
            aers_record_success();
        }

        if (run_time > cutoff) return;

        bool aers_started = false;
        if (!aers_active) {
            aers_started = aers_update_region();
        }

        if (aers_active) {
            if (edge_conflict == 0) {
                if (!aers_region_perturbation(pertub_bms, conflict_weight)) {
                    aers_stop_region_reduction();
                }
            }
        } else if (!aers_started) {
            big_pertub(big_pert_node_num, big_pertub_bms, conflict_weight);

            if (edge_conflict == 0) {
                perturbation(pertub_bms, conflict_weight);
                aers_no_impr++;
            }
        }
    }
}

