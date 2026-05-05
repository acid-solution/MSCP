#pragma once
#include "basic.h"
#include "util.h"

void perturbation_reduction_aers(long bms, double conflict_weight);
void aers_log_expand_reduction(long added);

inline long aers_current_score_reduction() {
    return cost + (long)remaining_vertex.size();
}

inline long aers_color_choice_value(long node, long color) {
    if (node >= 0 && node < (long)color_choice.size() &&
        color >= 0 && color < (long)color_choice[node].size()) {
        return color_choice[node][color];
    }
    return 0;
}

inline void aers_prepare_new_region_stamp() {
    if (aers_region_stamp >= std::numeric_limits<int>::max() - 1) {
        std::fill(aers_region_mark.begin(), aers_region_mark.end(), 0);
        std::fill(aers_candidate_mark.begin(), aers_candidate_mark.end(), 0);
        std::fill(aers_scanned_mark.begin(), aers_scanned_mark.end(), 0);
        std::fill(aers_boundary_scan_mark.begin(), aers_boundary_scan_mark.end(), 0);
        std::fill(aers_good_pool_mark.begin(), aers_good_pool_mark.end(), 0);
        std::fill(aers_conflict_pool_mark.begin(), aers_conflict_pool_mark.end(), 0);
        aers_region_stamp = 1;
    }
    aers_region_stamp++;
}

inline bool aers_mark_region_vertex(long v) {
    if (v < 0 || v >= (long)aers_region_mark.size()) return false;
    if (!remaining_vertex.exist(v)) return false;
    if (aers_region_mark[v] == (int)aers_region_stamp) return false;
    aers_region_mark[v] = (int)aers_region_stamp;
    aers_region_vertices.push_back(v);
    return true;
}

inline bool aers_is_region_vertex(long v) {
    return v >= 0 &&
           v < (long)aers_region_mark.size() &&
           aers_region_mark[v] == (int)aers_region_stamp;
}

inline long aers_cooldown_length() {
    long cooldown = max_no_impr / 2;
    return cooldown > 0 ? cooldown : 1;
}

inline long aers_region_no_impr_threshold() {
    long limit = aers_cooldown_length() / 2;
    return limit > 0 ? limit : 1;
}

inline void aers_add_overhead_time(clock_t start_clock) {
    aers_overhead_time += (double)(clock() - start_clock) / CLOCKS_PER_SEC;
}

void aers_consider_active_vertex(long v) {
    if (!aers_is_region_vertex(v)) return;
    if (!remaining_vertex.exist(v)) return;

    if (v >= 0 &&
        v < (long)good_node_color.size() &&
        v < (long)aers_good_pool_mark.size() &&
        !good_node_color[v].empty() &&
        aers_good_pool_mark[v] != (int)aers_region_stamp) {
        aers_good_pool_mark[v] = (int)aers_region_stamp;
        aers_region_good_vertices.push_back(v);
    }
    if (v >= 0 &&
        v < (long)conflict_vertex_in_color.size() &&
        v < (long)aers_conflict_pool_mark.size() &&
        conflict_vertex_in_color[v] > 0 &&
        aers_conflict_pool_mark[v] != (int)aers_region_stamp) {
        aers_conflict_pool_mark[v] = (int)aers_region_stamp;
        aers_region_conflict_vertices.push_back(v);
    }
}

void aers_refresh_active_around_vertex(long v) {
    if (v < 0) return;

    aers_consider_active_vertex(v);
    if (v >= (long)temp_adjacency_list.size()) return;

    for (auto u : temp_adjacency_list[v]) {
        if (aers_is_region_vertex(u)) {
            aers_consider_active_vertex(u);
        }
    }
}

void aers_scan_region_vertex_for_candidates(long v) {
    if (v < 0 || v >= (long)aers_scanned_mark.size()) return;
    if (aers_scanned_mark[v] == (int)aers_region_stamp) return;
    if (v >= (long)temp_adjacency_list.size()) return;

    aers_scanned_mark[v] = (int)aers_region_stamp;
    for (auto u : temp_adjacency_list[v]) {
        aers_expand_scan_edges++;

        if (!remaining_vertex.exist(u)) {
            aers_expand_skip_not_remaining++;
            continue;
        }
        if (u < 0 || u >= (long)aers_region_mark.size()) continue;

        if (aers_region_mark[u] == (int)aers_region_stamp) {
            aers_expand_skip_marked++;
            aers_candidate_skip_marked++;
            continue;
        }
        if (aers_candidate_mark[u] == (int)aers_region_stamp) {
            aers_candidate_skip_duplicate++;
            continue;
        }

        aers_candidate_mark[u] = (int)aers_region_stamp;
        aers_outer_candidates.push_back(u);
        aers_candidate_added++;
    }
}

long aers_expand_from_boundary_vertex(long node, long limit) {
    clock_t overhead_start = clock();
    aers_boundary_expand_calls++;
    aers_expand_call_count++;

    if (limit <= 0) {
        aers_add_overhead_time(overhead_start);
        return 0;
    }
    if (!aers_is_region_vertex(node) ||
        node < 0 ||
        node >= (long)temp_adjacency_list.size()) {
        aers_add_overhead_time(overhead_start);
        return 0;
    }

    if (node >= (long)aers_boundary_scan_mark.size()) {
        aers_add_overhead_time(overhead_start);
        return 0;
    }
    if (aers_boundary_scan_mark[node] != (int)aers_region_stamp) {
        aers_boundary_scan_mark[node] = (int)aers_region_stamp;
        if (node < (long)aers_boundary_next_index.size()) {
            aers_boundary_next_index[node] = 0;
        }
    }

    long added = 0;
    long& next_index = aers_boundary_next_index[node];
    const long degree = (long)temp_adjacency_list[node].size();

    while (next_index < degree && added < limit) {
        long u = temp_adjacency_list[node][next_index++];
        aers_boundary_expand_scan_edges++;
        aers_expand_scan_edges++;

        if (!remaining_vertex.exist(u)) {
            aers_expand_skip_not_remaining++;
            continue;
        }
        if (u < 0 || u >= (long)aers_region_mark.size()) continue;
        if (aers_region_mark[u] == (int)aers_region_stamp) {
            aers_expand_skip_marked++;
            continue;
        }
        if (!aers_mark_region_vertex(u)) continue;

        added++;
        aers_boundary_expand_added++;
        aers_expand_added++;
        if (u >= 0 && u < (long)good_node_color.size() &&
            !good_node_color[u].empty()) {
            aers_expand_added_good++;
        }
        if (u >= 0 && u < (long)conflict_vertex_in_color.size() &&
            conflict_vertex_in_color[u] > 0) {
            aers_expand_added_conflict++;
        }
        aers_consider_active_vertex(u);
    }

    if (added > 0) {
        aers_expand_count++;
        aers_total_region_size += (long)aers_region_vertices.size();
        aers_log_expand_reduction(added);
    }

    aers_add_overhead_time(overhead_start);
    return added;
}

void aers_after_region_move_reduction(long node) {
    aers_region_no_impr++;
    aers_refresh_active_around_vertex(node);
    aers_expand_from_boundary_vertex(node, aers_boundary_expand_size);
}

void aers_clear_region_reduction() {
    aers_region_vertices.clear();
    aers_frontier.clear();
    aers_next_frontier.clear();
    aers_outer_candidates.clear();
    aers_outer_candidate_head = 0;
    aers_region_good_vertices.clear();
    aers_region_conflict_vertices.clear();
    aers_prepare_new_region_stamp();
}

void aers_build_initial_region_from_perturb_node(long node) {
    clock_t overhead_start = clock();

    aers_clear_region_reduction();
    aers_mark_region_vertex(node);

    if (node >= 0 && node < (long)temp_adjacency_list.size()) {
        for (auto u : temp_adjacency_list[node]) {
            aers_expand_scan_edges++;
            if (!remaining_vertex.exist(u)) {
                aers_expand_skip_not_remaining++;
                continue;
            }
            if (u < 0 || u >= (long)aers_region_mark.size()) continue;
            if (aers_region_mark[u] == (int)aers_region_stamp) {
                aers_expand_skip_marked++;
                continue;
            }
            aers_mark_region_vertex(u);
        }
    }

    for (long v : aers_region_vertices) {
        aers_consider_active_vertex(v);
    }

    aers_add_overhead_time(overhead_start);
}

long aers_expand_region_reduction() {
    clock_t overhead_start = clock();
    aers_expand_call_count++;

    long added = 0;
    while (added < aers_expand_batch_size &&
           aers_outer_candidate_head < (long)aers_outer_candidates.size()) {
        long u = aers_outer_candidates[aers_outer_candidate_head++];

        if (!remaining_vertex.exist(u)) {
            aers_expand_skip_not_remaining++;
            continue;
        }
        if (u < 0 || u >= (long)aers_region_mark.size()) continue;
        if (aers_region_mark[u] == (int)aers_region_stamp) {
            aers_expand_skip_marked++;
            continue;
        }

        if (!aers_mark_region_vertex(u)) continue;
        added++;
        aers_expand_added++;

        if (u >= 0 && u < (long)good_node_color.size() &&
            !good_node_color[u].empty()) {
            aers_expand_added_good++;
        }
        if (u >= 0 && u < (long)conflict_vertex_in_color.size() &&
            conflict_vertex_in_color[u] > 0) {
            aers_expand_added_conflict++;
        }

        aers_scan_region_vertex_for_candidates(u);
    }

    if (added == 0 && aers_outer_candidate_head >= (long)aers_outer_candidates.size()) {
        aers_candidate_empty_exit++;
    }

    aers_add_overhead_time(overhead_start);
    return added;
}

void aers_exit_region_reduction(const string& reason, bool apply_cooldown = true) {
    if (aers_event_log == 1) {
        cerr << "[AERS_EXIT] file=" << file_name
             << " iter=" << current_iter
             << " reason=" << reason
             << " size=" << aers_region_vertices.size()
             << " no_impr=" << no_impr
             << endl;
    }

    aers_active = 0;
    if (apply_cooldown) {
        aers_cooldown_until = aers_no_impr + aers_cooldown_length();
    }
    aers_region_vertices.clear();
    aers_frontier.clear();
    aers_next_frontier.clear();
    aers_outer_candidates.clear();
    aers_outer_candidate_head = 0;
    aers_region_good_vertices.clear();
    aers_region_conflict_vertices.clear();
    aers_seed = -1;
    aers_expand_round = 0;
    aers_region_iter = 0;
    aers_force_expand = 0;
    aers_region_no_impr = 0;
    aers_before_perturb_score = 0;
    aers_exit_count++;
}

double aers_perturb_score(long node, long new_color, double conflict_weight) {
    long current_color = vertex_color[node];
    long penalty_diff = get_penalty(node, current_color) - get_penalty(node, new_color);
    double choose_score = (current_color - new_color) + penalty_diff;

    for (auto v : temp_adjacency_list[node]) {
        if (aers_color_choice_value(v, current_color) == 2 &&
            vertex_color[v] > current_color) {
            long old_eff = vertex_color[v] + get_penalty(v, vertex_color[v]);
            long new_eff = current_color + get_penalty(v, current_color);
            long delta_color = (old_eff - new_eff) / 3;
            choose_score += delta_color;
        }
        if (aers_color_choice_value(v, current_color) == 1 &&
            vertex_color[v] > current_color) {
            long old_eff = vertex_color[v] + get_penalty(v, vertex_color[v]);
            long new_eff = current_color + get_penalty(v, current_color);
            long delta_color = old_eff - new_eff;
            choose_score += delta_color;
        }
        if (vertex_color[v] == new_color) choose_score -= conflict_weight;
    }

    return choose_score;
}

long aers_choose_global_perturb_move_reduction(long bms, double conflict_weight,
                                               long& BestNode, long& BestColor) {
    if (remaining_vertex.size() == 0) return 0;

    long best_node = -1;
    long best_color = -1;
    double best_choose_score = -vertex_count;

    for (long i = 0; i < bms; ++i) {
        long node = remaining_vertex[rand() % remaining_vertex.size()];
        long current_color = vertex_color[node];
        if (max_color < current_color) continue;

        long new_color = rand() % (max_color - current_color + 1) + current_color + 1;
        double choose_score = aers_perturb_score(node, new_color, conflict_weight);

        if (choose_score > best_choose_score) {
            best_node = node;
            best_color = new_color;
            best_choose_score = choose_score;
        }
    }

    if (best_node == -1) return 0;
    BestNode = best_node;
    BestColor = best_color;
    return 1;
}

long aers_choose_region_perturb_move_reduction(long bms, double conflict_weight,
                                               long& BestNode, long& BestColor) {
    if (aers_region_vertices.empty()) return 0;

    long best_node = -1;
    long best_color = -1;
    double best_choose_score = -vertex_count;

    for (long i = 0; i < bms; ++i) {
        long node = aers_region_vertices[rand() % aers_region_vertices.size()];
        long current_color = vertex_color[node];
        if (max_color < current_color) continue;

        long new_color = rand() % (max_color - current_color + 1) + current_color + 1;
        double choose_score = aers_perturb_score(node, new_color, conflict_weight);

        if (choose_score > best_choose_score) {
            best_node = node;
            best_color = new_color;
            best_choose_score = choose_score;
        }
    }

    if (best_node == -1) return 0;
    BestNode = best_node;
    BestColor = best_color;
    return 1;
}

void aers_submit_perturb_move_reduction(long node, long color) {
    color_node_reduction(node, color);
    current_iter++;
    no_impr++;
    aers_no_impr++;
    aers_region_iter++;
    aers_total_region_iter++;
    aers_after_region_move_reduction(node);
}

long aers_start_region_reduction(long perturb_node, long perturb_color) {
    if (perturb_node == -1) return 0;

    aers_before_perturb_score = aers_current_score_reduction();
    aers_build_initial_region_from_perturb_node(perturb_node);
    if (aers_region_vertices.empty()) return 0;

    aers_seed = perturb_node;
    aers_active = 1;
    aers_expand_round = 0;
    aers_region_iter = 0;
    aers_force_expand = 0;
    aers_region_no_impr = 0;

    aers_enter_count++;
    aers_total_region_size += (long)aers_region_vertices.size();

    if (aers_event_log == 1) {
        cerr << "[AERS_ENTER] file=" << file_name
             << " iter=" << current_iter
             << " seed=" << aers_seed
             << " size=" << aers_region_vertices.size()
             << " no_impr=" << no_impr
             << " conflict=" << edge_conflict
             << " score=" << aers_before_perturb_score
             << endl;
    }

    aers_submit_perturb_move_reduction(perturb_node, perturb_color);
    return 1;
}

void aers_log_expand_reduction(long added) {
    if (aers_event_log == 1) {
        cerr << "[AERS_EXPAND] file=" << file_name
             << " iter=" << current_iter
             << " round=" << aers_expand_round
             << " added=" << added
             << " size=" << aers_region_vertices.size()
             << endl;
    }
}

void aers_update_region_reduction() {
    if (aers_mode == 0) return;

    long trigger_threshold = aers_cooldown_length();
    aers_region_no_impr_limit = aers_region_no_impr_threshold();

    if (aers_active == 0) {
        if (edge_conflict == 0 &&
            aers_no_impr > trigger_threshold &&
            aers_no_impr >= aers_cooldown_until) {
            long perturb_node = -1;
            long perturb_color = -1;
            if (!aers_choose_global_perturb_move_reduction(
                    pertub_bms, conflict_weight, perturb_node, perturb_color)) {
                return;
            }
            aers_start_region_reduction(perturb_node, perturb_color);
        }
        return;
    }

    if (aers_region_vertices.empty()) {
        aers_exit_region_reduction("empty_region");
        return;
    }

    if (aers_region_no_impr >= aers_region_no_impr_limit) {
        aers_region_no_impr_exit++;
        aers_exit_region_reduction("region_no_impr", false);

        if (edge_conflict == 0) {
            long perturb_node = -1;
            long perturb_color = -1;
            if (aers_choose_global_perturb_move_reduction(
                    pertub_bms, conflict_weight, perturb_node, perturb_color)) {
                aers_start_region_reduction(perturb_node, perturb_color);
            }
        }
    }
}

long choose_good_node_reduction_aers(long bms, long& BestNode, long& BestColor) {
    if (aers_region_good_vertices.empty()) return 0;

    long best_node = -1;
    long best_color = -1;
    double best_color_score = -1;
    long fail_count = 0;
    long max_fail = bms * 3;
    if (max_fail <= 0) max_fail = 3;

    for (long i = 0; i < bms && fail_count < max_fail; ) {
        if (aers_region_good_vertices.empty()) break;
        long pool_index = rand() % aers_region_good_vertices.size();
        long node = aers_region_good_vertices[pool_index];
        aers_choose_sample_count++;
        aers_good_pool_sample_count++;

        if (!aers_is_region_vertex(node) ||
            node < 0 ||
            node >= (long)good_node_color.size() ||
            good_node_color[node].empty()) {
            aers_choose_skip_empty_good++;
            aers_good_pool_stale_count++;
            if (node >= 0 && node < (long)aers_good_pool_mark.size()) {
                aers_good_pool_mark[node] = 0;
            }
            aers_region_good_vertices[pool_index] = aers_region_good_vertices.back();
            aers_region_good_vertices.pop_back();
            fail_count++;
            continue;
        }

        long index = rand() % good_node_color[node].size();
        long new_color = good_node_color[node][index];

        if (is_lock(node, new_color)) {
            aers_choose_skip_locked++;
            fail_count++;
            continue;
        }

        i++;
        long current_color = vertex_color[node];
        long penalty_diff = get_penalty(node, current_color) - get_penalty(node, new_color);
        double score = (current_color - new_color) + penalty_diff
                     + conflict_weight * (aers_color_choice_value(node, current_color)
                     - aers_color_choice_value(node, new_color));

        if (score > best_color_score) {
            best_color_score = score;
            best_node = node;
            best_color = new_color;
        }
    }

    if (best_node == -1) return 0;

    BestNode = best_node;
    BestColor = best_color;
    return 1;
}

long remove_conflict_new4_reduction_aers() {
    if (edge_conflict <= 0) return 0;
    if (aers_region_conflict_vertices.empty()) return 0;

    long sample_count = choose_conflict_node_bms;
    if (sample_count <= 0) sample_count = 1;
    sample_count *= 3;

    long node = -1;
    for (long i = 0; i < sample_count; ++i) {
        if (aers_region_conflict_vertices.empty()) break;
        long pool_index = rand() % aers_region_conflict_vertices.size();
        long candidate = aers_region_conflict_vertices[pool_index];
        aers_remove_sample_count++;
        aers_conflict_pool_sample_count++;
        if (aers_is_region_vertex(candidate) &&
            candidate >= 0 &&
            candidate < (long)conflict_vertex_in_color.size() &&
            conflict_vertex_in_color[candidate] > 0) {
            node = candidate;
            break;
        }
        aers_remove_sample_miss++;
        aers_conflict_pool_stale_count++;
        if (candidate >= 0 && candidate < (long)aers_conflict_pool_mark.size()) {
            aers_conflict_pool_mark[candidate] = 0;
        }
        aers_region_conflict_vertices[pool_index] = aers_region_conflict_vertices.back();
        aers_region_conflict_vertices.pop_back();
    }

    if (node == -1) return 0;

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

    color_node_reduction(node, new_color);
    current_iter++;
    no_impr++;
    aers_no_impr++;
    aers_region_iter++;
    aers_total_region_iter++;
    aers_after_region_move_reduction(node);
    return 1;
}

void perturbation_reduction_aers(long bms, double conflict_weight) {
    if (aers_active == 0) {
        long perturb_node = -1;
        long perturb_color = -1;
        if (!aers_choose_global_perturb_move_reduction(
                bms, conflict_weight, perturb_node, perturb_color)) {
            return;
        }
        aers_start_region_reduction(perturb_node, perturb_color);
        return;
    }

    if (aers_region_vertices.empty()) {
        aers_exit_region_reduction("empty_region");
        return;
    }

    long best_node = -1;
    long best_color = -1;
    if (!aers_choose_region_perturb_move_reduction(
            bms, conflict_weight, best_node, best_color)) {
        aers_region_no_impr_exit++;
        aers_exit_region_reduction("region_perturb_fail", false);
        return;
    }

    aers_submit_perturb_move_reduction(best_node, best_color);
}
