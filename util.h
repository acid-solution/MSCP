#pragma once
#include "basic.h"
#include <cmath>
 
inline long get_penalty(long u, long c) {
    if (c < dp_penalty[u].size()) {
        return dp_penalty[u][c];
    }
    return 0;
}

inline void ensure_color_penalty_sum_size(long max_color_needed, long max_target_c) {
    long required_size = std::max(max_color_needed, max_target_c) + 10;
    if (required_size > color_penalty_sum.size()) {
        long old_size = color_penalty_sum.size();
        color_penalty_sum.resize(required_size);
        for (long i = 0; i < required_size; ++i) {
            color_penalty_sum[i].resize(required_size, 0);
        }
    }
} 



static double luby(double y, int x){
 
    // Find the finite subsequence that contains index 'x', and the
    // size of that subsequence:
    int size, seq;
    for (size = 1, seq = 0; size < x+1; seq++, size = 2*size+1);
 
    while (size-1 != x){
        size = (size-1)>>1;
        seq--;
        x = x % size;
    }
 
    return pow(y, seq);
}
 
bool cmp_by_edgeout(long x, long y){
	return x > y;
}
 
bool remove_clique(long v){//删除节点 v 和其相关边
	//vector<long> &clique = *all_clique.rbegin();
 
        for(auto u : temp_adjacency_list[v]){	//删除节点v在temp_adjacency_list中所有邻居节点u与v的连接
			for (vector<long>::size_type i = 0; i < temp_adjacency_list[u].size(); ++i) {
				if (temp_adjacency_list[u][i] == v) {								//找到邻居节点 u 的邻接表中与 v 相连的边
					temp_adjacency_list[u][i] = *temp_adjacency_list[u].rbegin();	//将该边与邻接表的最后一条边交换
					temp_adjacency_list[u].pop_back();								//删除邻接表的最后一条边
					break;
				}
			}
        }
        remaining_vertex.remove(v);//从剩余节点列表中移除节点 v
    return true;
}
 
bool verify_solution(){
	for (auto v : remaining_vertex){
		for (auto u : temp_adjacency_list[v]){
			if (vertex_color[v] == vertex_color[u]){
				return false;
			}
		}
	}
	return true;
}

// 在 LS.h 中添加此函数
void test_init_effect(clock_t start_time, clock_t end_time) {
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    long current_score = 0;
    if (reduction_mode == 2) {
        current_score = compute_score_reduction();
    } else {
        current_score = compute_score();
    }
    
    double avg_color = (double)cost / std::max((size_t)1, remaining_vertex.size());
    
    cout << "\n================ [ 初始化阶段测试报告 ] ================" << endl;
    cout << "文件: " << file_name << " | seed=" << seed << endl;
    cout << "init_mode                  : " << init_mode << endl;
    cout << "初始化耗时                 : " << time_taken << " 秒" << endl;
    cout << "--- 染色结果 ---" << endl;
    cout << "参与染色节点数             : " << remaining_vertex.size() << endl;
    cout << "最大颜色编号 (max_color)   : " << max_color << " (共 " << max_color + 1 << " 色)" << endl;
    cout << "平均颜色编号               : " << avg_color << endl;
    cout << "基础花费 (cost)            : " << cost << endl;
    cout << "初始冲突边数               : " << edge_conflict << endl;
    cout << "初始冲突节点数             : " << conflict_node_queue.size() << endl;
    cout << "可降色节点数 (valid_node)  : " << valid_node.size() << " / " 
         << remaining_vertex.size() << " (" 
         << 100.0 * valid_node.size() / std::max((size_t)1, remaining_vertex.size()) << "%)" << endl;
    cout << "解合法性                   : " << (verify_solution() ? "PASS" : "FAIL!") << endl;
    cout << "初始化得分 (Score)         : " << current_score << endl;
    cout << "========================================================\n" << endl;
}

void finalize_init(){
    bool use_reduction = (localsearch_mode == 1);

    // ============ 1. 计算 cost ============
    cost = 0;
    for (auto v : remaining_vertex){
        if (use_reduction) {
            cost += vertex_color[v] + get_penalty(v, vertex_color[v]);
        } else {
            cost += vertex_color[v];
        }
    }

    // ============ 2. 初始化 color_penalty_sum（仅 reduction 需要）============
    if (use_reduction) {
        for (auto v : remaining_vertex) {
            long c_src = vertex_color[v];
            long limit = dp_penalty[v].size();
            ensure_color_penalty_sum_size(c_src, limit);
            for (long target_c = 0; target_c < (long)dp_penalty[v].size(); target_c++) {
                long p = get_penalty(v, target_c);
                if (p > 0) {
                    color_penalty_sum[c_src][target_c] += p;
                }
            }
        }
    }

    // ============ 3. 初始化 color_choice 和冲突信息 ============
    for (auto v : remaining_vertex){
        long color_v = vertex_color[v];
        for (auto u : temp_adjacency_list[v]){
            if ((size_t)color_v >= color_choice[u].size()) {
                color_choice[u].resize(color_v + 1, 0);
            }
            color_choice[u][color_v]++;
            if (vertex_color[u] == vertex_color[v]){
                conflict_vertex_in_color[v]++;
                edge_conflict++;
            }
        }
    }

    // ============ 4. 初始化 conflict_node_queue 和 good_node_color ============
    for (auto v : remaining_vertex){
        if (conflict_vertex_in_color[v] > 0){
            conflict_node_queue.push_back(v);
        }

        long current_color = vertex_color[v];

        if (use_reduction) {
            // reduction 版：要求有效代价更小且冲突不增加
            long eff_curr_v = current_color + get_penalty(v, current_color);
            long limit_v = std::min((long)eff_curr_v, max_color + 2);
            for (long new_color = 0; new_color < limit_v; new_color++) {
                if (new_color == current_color) continue;
                long eff_new_color = new_color + get_penalty(v, new_color);
                short conf_new_color = (new_color < (long)color_choice[v].size())
                                       ? color_choice[v][new_color] : 0;
                if (eff_new_color < eff_curr_v &&
                    conf_new_color <= color_choice[v][current_color]) {
                    good_node_color[v].emplace_back(new_color);
                }
            }
        } else {
            // 原版：只要颜色编号更小且冲突不增加
            for (long new_color = 0; new_color < current_color; new_color++){
                if (color_choice[v][new_color] <= color_choice[v][current_color]){
                    good_node_color[v].emplace_back(new_color);
                }
            }
        }
    }

    // ============ 5. 初始化 valid_node ============
    for (auto n : remaining_vertex){
        if (good_node_color[n].size() > 0){
            valid_node.push_back(n);
        }
    }

    // ============ 6. 初始化 best_score 和 best_solution ============
    if (use_reduction) {
        best_score = cost + remaining_vertex.size();
    } else {
        best_score = compute_score();
    }
    for (auto v : remaining_vertex){
        best_solution[v] = vertex_color[v];
    }

    if(!verify_solution()) {
        cout << "error init_color" << endl;
        exit(0);
    }

    // ============ 7. CICC 初始化 ============
    if (strategy_mode == 3) {
        for (auto v : remaining_vertex) {
            cicc[v].assign(max_color + 2, 0);
        }
    }
}

long single_random_greedy_pass(){
    long remaining_size = remaining_vertex.size();
    long color_threshold = COLOR_NUM;
    if (remaining_size < color_threshold) color_threshold = remaining_size;

    // 1. 生成随机顺序
    vector<long> order;
    order.reserve(remaining_size);
    for (auto v : remaining_vertex) order.push_back(v);
    // Fisher-Yates 洗牌
    for (long i = order.size() - 1; i > 0; --i) {
        long j = rand() % (i + 1);
        std::swap(order[i], order[j]);
    }

    // 2. 按随机顺序贪心染最小可用色
    long pass_cost = 0;
    for (auto v : order){
        vector<long> neig_color;
        neig_color.resize(color_threshold, 0);
        for (auto u : adjacency_list[v]){
            if (vertex_color[u] != -1){
                if (vertex_color[u] >= (long)neig_color.size()) {
                    neig_color.resize(vertex_color[u] + 2, 0);
                }
                neig_color[vertex_color[u]] = 1;
            }
        }

        long color = 0;
        for (long i = 0; i < (long)neig_color.size(); i++){
            if (neig_color[i] == 0) { color = i; break; }
        }
        if (neig_color[color] == 1) color = neig_color.size();

        if (color > max_color) max_color = color;
        vertex_color[v] = color;

        if ((size_t)color >= color_use_number.size()) {
            color_use_number.resize(color + 10, 0);
        }
        color_use_number[color]++;

        // 累加选色阶段代价（用于多轮比较）
        if (localsearch_mode == 1) {
            pass_cost += color + get_penalty(v, color);
        } else {
            pass_cost += color;
        }
    }
    return pass_cost;
}

void reset_color_assignment(){
    for (auto v : remaining_vertex) {
        vertex_color[v] = -1;
    }
    max_color = -1;
    std::fill(color_use_number.begin(), color_use_number.end(), 0);
}



inline bool is_lock(long node, long target_color){
    switch (strategy_mode){
        case 0: // Tabu-only
            return tabu[node] > current_iter;
        case 1: // CC-only
            return (conf[node] == 0);
        case 2: // CC + Tabu
            return (tabu[node] > current_iter) || (conf[node] == 0);
        case 3: // Pure CICC
            return (target_color < (long)cicc[node].size()) && (cicc[node][target_color] > 0);
        default:
            return false;
    }
}

inline void lock_unlock(long node, long old_color, long target_color){
    // Tabu：节点级短期记忆（在模式 0 或 2 生效）
    if (strategy_mode == 0 || strategy_mode == 2){
        tabu[node] = current_iter + TABU_TIME;
    }

    // CC：锁定自身，解禁邻居（简化版）并设置冷却（在模式 1 或 2 生效）
    if (strategy_mode == 1 || strategy_mode == 2){
        conf[node] = 0;
        for (auto v : temp_adjacency_list[node]){
            conf[v] = 1;
        }
    }

    // CICC：颜色对门槛（在模式 3 生效）
    if (strategy_mode == 3) {
        long need = std::max(old_color, target_color) + 1;
        
        // 只在真的不够时扩容，且一次多留 50% 缓冲
        if ((long)cicc[node].size() < need) {
            cicc[node].resize(need * 3 / 2, 0);
        }
        
        cicc[node][old_color] = color_choice[node][old_color];
        
        for (auto v : temp_adjacency_list[node]) {
            if ((long)cicc[v].size() < need)
                cicc[v].resize(need * 3 / 2, 0);
            if (cicc[v][old_color] > 0)
                cicc[v][old_color]--;
        }
    }
}

inline void unlock_all_vertices() {
    switch (strategy_mode) {
        case 0: // Tabu-only
            for (size_t v = 0; v < tabu.size(); ++v) {
                tabu[v] = 0;
            }
            break;
        case 1: // CC-only
            for (size_t v = 0; v < conf.size(); ++v) {
                conf[v] = 1;
            }
            break;
        case 2: // CC + Tabu
            for (size_t v = 0; v < tabu.size(); ++v) {
                tabu[v] = 0;
            }
            for (size_t v = 0; v < conf.size(); ++v) {
                conf[v] = 1;
            }
            break;
        case 3: // Pure CICC
            for (auto& vec : cicc) {
                vec.assign(vec.size(), 0);
            }
            break;
    }
}


