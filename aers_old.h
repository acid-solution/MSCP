#pragma once

bool aers_color_node(long node, long color, bool lock_it) {
    aers_diag_inc(aers_commit_wrapper_calls);
    return color_node(node, color, lock_it);
}

bool aers_choose_perturb_move_old(Vertex_vec_with_index& pool, long bms, double conflict_weight, long& BestNode, long& BestColor) {
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

bool aers_choose_perturb_move_old(vector<long>& pool, long bms, double conflict_weight, long& BestNode, long& BestColor) {
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

bool aers_global_good_move(long bms) {
    long best_node = -1;
    long best_color = -1;

    long x = choose_good_node(bms, best_node, best_color);
    if (x != 1 || best_node == -1) return false;

    bool in_region = aers_in_region(best_node);
    bool expand_boundary = aers_move_can_expand_boundary(best_node);
    if (!aers_color_node(best_node, best_color)) return false;

    current_iter++;
    no_impr++;
    aers_no_impr++;
    aers_note_good_move(in_region);
    aers_after_region_move_reduction(best_node, expand_boundary);
    return true;
}

bool aers_global_remove_conflict() {
    if (edge_conflict <= 0 || conflict_node_queue.empty()) return false;

    long index = rand() % conflict_node_queue.size();
    long node = conflict_node_queue[index];
    bool in_region = aers_in_region(node);
    bool expand_boundary = aers_move_can_expand_boundary(node);

    long new_color = max_color + 1;
    if (new_color >= COLOR_NUM) {
        for (long i = 0; i < COLOR_NUM; i++) {
            if (i < (long)color_choice[node].size() && color_choice[node][i] == 0) {
                new_color = i;
                break;
            }
        }
    }
    if (new_color >= COLOR_NUM) {
        new_color = new_color % COLOR_NUM;
    }

    if (!aers_color_node(node, new_color)) return false;

    current_iter++;
    no_impr++;
    aers_no_impr++;
    aers_note_conflict_repair(in_region);
    aers_after_region_move_reduction(node, expand_boundary);
    return true;
}

bool aers_region_perturbation(long bms, double conflict_weight) {
    long node = -1;
    long color = -1;
    if (!aers_choose_perturb_move_old(aers_region_vertices, bms, conflict_weight, node, color)) {
        return false;
    }

    bool in_region = aers_in_region(node);
    bool expand_boundary = aers_move_can_expand_boundary(node);
    if (!aers_color_node(node, color)) return false;

    current_iter++;
    no_impr++;
    aers_no_impr++;
    aers_note_perturb_move(in_region);
    aers_after_region_move_reduction(node, expand_boundary);
    return true;
}

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
    aers_note_perturb_move(aers_in_region(seed));
    aers_after_region_move_reduction(seed, aers_move_can_expand_boundary(seed));
    return true;
}

bool aers_update_region() {
    if (aers_mode == 0 || aers_active) return false;
    if (edge_conflict != 0) return false;
    if (aers_no_impr <= aers_max_no_impr / 2) return false;
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
            if (!aers_global_good_move(choose_conflict_node_bms) && edge_conflict > 0) {
                if (!aers_global_remove_conflict()) {
                    aers_stop_region_reduction();
                    remove_conflict_new4();
                    aers_no_impr++;
                    aers_last_move_in_region = -1;
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
