#pragma once

#include <algorithm>
#include <ctime>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

// 区域 move 后更新 AERS 计数，并按 moved node 做边界扩张。
void aers_after_region_move_reduction(long moved_node, bool expand_boundary);
// 用 seed 顶点和它的一跳 remaining 邻居建立新 AERS 区域。
void aers_build_region(long seed);
// 记录 active 区域内找到新 best，并重置区域停滞计数。
void aers_record_success();

long aers_mode = 0;
long aers_diag = 0;
long aers_active = 0;
long aers_search_used = 0;
long aers_no_impr = 0;
long aers_region_no_impr = 0;
long aers_cooldown_until = 0;
long aers_max_no_impr = max_no_impr_basic;
long aers_boundary_expand_size = 50;
long aers_seed_node = -1;
long aers_last_move_in_region = -1;

vector<long> aers_region_vertices;
vector<int> aers_region_mark;
int aers_region_stamp = 1;

vector<long> aers_boundary_active_vertices;
vector<int> aers_boundary_active_mark;
vector<long> aers_boundary_active_pos;
vector<int> aers_boundary_exhausted_mark;
vector<long> aers_boundary_next_index;

long long aers_enter_count = 0;
long long aers_exit_count = 0;
long long aers_success_count = 0;
long long aers_total_region_iter = 0;
long long aers_total_region_size = 0;
long long aers_expand_calls = 0;
long long aers_expand_added = 0;
long long aers_scan_edges = 0;
long long aers_skip_marked = 0;
long long aers_boundary_exhausted_count = 0;
long long aers_boundary_advance_stale = 0;
long long aers_boundary_advance_skip_exhausted = 0;
long long aers_move_boundary_expand_calls = 0;
long long aers_move_boundary_stale = 0;
long long aers_move_boundary_skip_exhausted = 0;
long long aers_move_boundary_invariant_miss = 0;
long long aers_move_boundary_expand_added = 0;
long long aers_commit_wrapper_calls = 0;
long long aers_good_move_total = 0;
long long aers_good_move_in_region = 0;
long long aers_good_move_out_region = 0;
long long aers_conflict_repair_total = 0;
long long aers_conflict_repair_in_region = 0;
long long aers_conflict_repair_out_region = 0;
long long aers_perturb_total = 0;
long long aers_perturb_in_region = 0;
long long aers_perturb_out_region = 0;
long long aers_enter_valid_node_size_sum = 0;
long long aers_enter_conflict_queue_size_sum = 0;
long long aers_enter_region_size_sum = 0;
long long aers_success_after_in_region_move = 0;
long long aers_success_after_out_region_move = 0;
clock_t aers_build_region_exclusive_ticks = 0;
clock_t aers_expand_exclusive_ticks = 0;
clock_t aers_choose_good_exclusive_ticks = 0;
clock_t aers_remove_conflict_exclusive_ticks = 0;
clock_t aers_perturb_choose_exclusive_ticks = 0;
clock_t aers_after_move_exclusive_ticks = 0;
clock_t aers_stop_region_exclusive_ticks = 0;

#include "aers_support.h"
#include "aers_old.h"

// 如果本次 moved node 是 active boundary，就从它开始增量扩张当前区域。
void aers_expand_from_moved_boundary(long moved_node) {
    clock_t start_clock = aers_diag_clock();
    if (moved_node < 0 || moved_node >= (long)aers_boundary_active_mark.size()) {
        aers_diag_inc(aers_move_boundary_stale);
        aers_add_ticks(aers_expand_exclusive_ticks, start_clock);
        return;
    }
    if (!aers_in_region(moved_node)) {
        aers_diag_inc(aers_move_boundary_stale);
        aers_remove_boundary_active(moved_node);
        aers_add_ticks(aers_expand_exclusive_ticks, start_clock);
        return;
    }
    if (aers_boundary_exhausted_mark[moved_node] == aers_region_stamp) {
        aers_diag_inc(aers_move_boundary_skip_exhausted);
        aers_remove_boundary_active(moved_node);
        aers_add_ticks(aers_expand_exclusive_ticks, start_clock);
        return;
    }
    if (aers_boundary_active_mark[moved_node] != aers_region_stamp) {
        aers_diag_inc(aers_move_boundary_invariant_miss);
        aers_add_ticks(aers_expand_exclusive_ticks, start_clock);
        return;
    }

    aers_diag_inc(aers_expand_calls);
    aers_diag_inc(aers_move_boundary_expand_calls);

    long added = 0;
    while (added < aers_boundary_expand_size) {
        if (aers_boundary_active_mark[moved_node] != aers_region_stamp) {
            break;
        }
        if (aers_boundary_exhausted_mark[moved_node] == aers_region_stamp) {
            aers_remove_boundary_active(moved_node);
            break;
        }

        long before = (long)aers_region_vertices.size();
        if (!aers_advance_boundary_vertex(moved_node)) {
            aers_remove_boundary_active(moved_node);
            break;
        }
        if ((long)aers_region_vertices.size() > before) {
            added++;
            aers_diag_inc(aers_move_boundary_expand_added);
        }
    }

    aers_add_ticks(aers_expand_exclusive_ticks, start_clock);
}

// 停止当前 AERS 区域，并设置下一次进入前的冷却。
void aers_stop_region_reduction() {
    clock_t start_clock = aers_diag_clock();
    if (aers_active) {
        aers_diag_inc(aers_exit_count);
        aers_cooldown_until = aers_no_impr + aers_max_no_impr / 2;
    }
    aers_active = 0;
    aers_clear_region();
    aers_add_ticks(aers_stop_region_exclusive_ticks, start_clock);
}

// 用 seed 顶点和它的一跳 remaining 邻居建立新 AERS 区域。
void aers_build_region(long seed) {
    clock_t start_clock = aers_diag_clock();
    aers_clear_region();
    aers_active = 1;
    aers_seed_node = seed;
    aers_diag_inc(aers_enter_count);

    aers_add_region_vertex(seed);
    for (long v : temp_adjacency_list[seed]) {
        if (aers_remaining_vertex(v)) aers_add_region_vertex(v);
    }
    aers_diag_add(aers_enter_valid_node_size_sum, (long long)valid_node.size());
    aers_diag_add(aers_enter_conflict_queue_size_sum, (long long)conflict_node_queue.size());
    aers_diag_add(aers_enter_region_size_sum, (long long)aers_region_vertices.size());

    aers_add_ticks(aers_build_region_exclusive_ticks, start_clock);
}

// 提交 reduction move；good/conflict 候选池复用全局状态，不再做 AERS 局部同步。
bool aers_color_node_reduction(long node, long color, bool lock_it) {
    aers_diag_inc(aers_commit_wrapper_calls);
    return color_node_reduction(node, color, lock_it);
}

// 通过一次全局 seed 扰动启动 reduction AERS 区域。
bool aers_start_region_reduction(long bms, double conflict_weight) {
    long seed = -1;
    long color = -1;
    if (!aers_choose_perturb_move(remaining_vertex, bms, conflict_weight, seed, color)) {
        return false;
    }

    aers_build_region(seed);
    if (!aers_color_node_reduction(seed, color)) {
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

// 检查 reduction 进入条件，搜索停滞时启动 AERS。
bool aers_update_region_reduction() {
    if (aers_mode == 0 || aers_active) return false;
    if (edge_conflict != 0) return false;
    if (aers_no_impr <= aers_max_no_impr / 2) return false;
    if (aers_no_impr < aers_cooldown_until) return false;
    return aers_start_region_reduction(pertub_bms, conflict_weight);
}

// 区域 move 后更新进展，区域长期无改进时退出。
void aers_after_region_move_reduction(long moved_node, bool expand_boundary) {
    if (!aers_active) return;

    clock_t start_clock = aers_diag_clock();
    aers_region_no_impr++;
    aers_diag_inc(aers_total_region_iter);
    aers_diag_add(aers_total_region_size, (long long)aers_region_vertices.size());
    aers_add_ticks(aers_after_move_exclusive_ticks, start_clock);

    if (expand_boundary) {
        aers_expand_from_moved_boundary(moved_node);
    }

    start_clock = aers_diag_clock();
    bool should_stop_region = aers_region_no_impr >= aers_max_no_impr / 4;
    aers_add_ticks(aers_after_move_exclusive_ticks, start_clock);

    if (should_stop_region) {
        aers_stop_region_reduction();
    }
}

// 记录 active 区域内找到新 best，并重置区域停滞计数。
void aers_record_success() {
    if (!aers_active) return;
    aers_diag_inc(aers_success_count);
    if (aers_last_move_in_region == 1) {
        aers_diag_inc(aers_success_after_in_region_move);
    } else if (aers_last_move_in_region == 0) {
        aers_diag_inc(aers_success_after_out_region_move);
    }
    aers_region_no_impr = 0;
}

void localsearch_reduction_aers(int cutoff) {
    aers_search_used = 1;
    if (conflict_weight == 0) conflict_weight = 1;
    big_pert_node_num = vertex_count / big_pertub_num_k;
    if (big_pert_node_num > 500) big_pert_node_num = 500;

    while (current_iter < max_iter) {
        long best_node = -1;
        long best_color = -1;

        if (aers_active) {
            if (!aers_global_good_move_reduction(choose_conflict_node_bms) && edge_conflict > 0) {
                if (!aers_global_remove_conflict_reduction()) {
                    aers_stop_region_reduction();
                    remove_conflict_new4_reduction();
                    aers_no_impr++;
                    aers_last_move_in_region = -1;
                }
            }
        } else {
            long x = choose_good_node_reduction(choose_conflict_node_bms, best_node, best_color);
            if (x == 1 && best_node != -1) {
                color_node_reduction(best_node, best_color);
                current_iter++;
                no_impr++;
                aers_no_impr++;
            } else {
                remove_conflict_new4_reduction();
                aers_no_impr++;
            }
        }

        long score = 0;
        if (edge_conflict == 0) score = cost + remaining_vertex.size();

        best_time = clock();
        double run_time = (double)(best_time - begin_time) / CLOCKS_PER_SEC;
        if (edge_conflict == 0 && score < best_score) {
            update_best_solution_reduction();
            final_time = run_time;
            no_impr = 0;
            aers_no_impr = 0;
            aers_record_success();
        }

        if (run_time > cutoff) return;

        bool aers_started = false;
        if (!aers_active) {
            aers_started = aers_update_region_reduction();
        }

        if (aers_active) {
            if (edge_conflict == 0) {
                if (!aers_region_perturbation_reduction(pertub_bms, conflict_weight)) {
                    aers_stop_region_reduction();
                }
            }
        } else if (!aers_started) {
            big_pertub(big_pert_node_num, big_pertub_bms, conflict_weight);

            if (edge_conflict == 0) {
                perturbation_reduction(pertub_bms, conflict_weight);
                aers_no_impr++;
            }
        }
    }
}
