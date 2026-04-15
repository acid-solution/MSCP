#pragma once
#include "basic.h"
#include <cmath>
 
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
    // 1. 计算初始化阶段的纯耗时
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    // 2. 区分不同约简模式下的得分计算
    long current_score = 0;
    if (reduction_mode == 2) {
        current_score = compute_score_reduction() + remove_score; 
    } else {
        current_score = compute_score() + remove_score;
    }

    // 3. 打印详细的测试面板
    cout << "\n================ [ 初始化阶段测试报告 ] ================" << endl;
    cout << "当前初始化策略 (init_mode) : " << init_mode << endl;
    cout << "初始化耗时                 : " << time_taken << " 秒" << endl;
    cout << "最大颜色编号 (max_color)   : " << max_color << " (共使用 " << max_color + 1 << " 种颜色)" << endl;
    cout << "基础花费 (cost)            : " << cost << endl;
    cout << "初始冲突边数 (edge_conflict): " << edge_conflict << endl;
    cout << "初始未解决冲突节点数       : " << conflict_node_queue.size() << endl;
    cout << "初始总得分 (Score)         : " << current_score << endl;
    cout << "========================================================\n" << endl;

    exit(0); // 测试完毕后退出程序
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

