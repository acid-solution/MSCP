#pragma once
#include "basic.h"
#include "util.h"


void read_file(string file_name){
	ifstream in_file(file_name);
	if (!in_file.is_open()) {
		cout << "in_file error" << endl;
		exit(1);
	}

	// 读取头部，得到顶点和边的数量（DIMACS: p edge V E）
	string line;
	istringstream is;
	string p, tmp;
	do {
		getline(in_file, line);
		is.clear();
		is.str(line);
		is >> p >> tmp >> vertex_count >> edge_count;
	} while (p != "p");

	// 读入所有边，做自环过滤与无向去重（排序+unique），再一次性建图
	vector<pair<long,long>> edges;
	if (edge_count > 0) edges.reserve(edge_count);
	long v1, v2;
	while (in_file >> tmp >> v1 >> v2) {
		if (v1 == v2) continue;              // 跳过自环
		v1--; v2--;                          // 转成 0-based
		if (v1 < 0 || v2 < 0 || v1 >= vertex_count || v2 >= vertex_count) continue; // 越界保护
		if (v1 > v2) std::swap(v1, v2);      // 无向边规范化到 (min,max)
		edges.emplace_back(v1, v2);
	}
	in_file.close();

	sort(edges.begin(), edges.end());
	edges.erase(unique(edges.begin(), edges.end()), edges.end());

	adjacency_list.clear();
	temp_adjacency_list.clear();
	adjacency_list.resize(vertex_count + 1);
	temp_adjacency_list.resize(vertex_count + 1);

	for (auto &e : edges) {
		long u = e.first, v = e.second;
		adjacency_list[u].push_back(v);
		adjacency_list[v].push_back(u);
		temp_adjacency_list[u].push_back(v);
		temp_adjacency_list[v].push_back(u);
	}

	// 以去重后的边数重新计算密度
	edge_count = (long)edges.size();
	density = (double)edge_count / (double)vertex_count;


	if (vertex_count > 100000){
		bms = 10;
		choose_conflict_node_bms = bms;
	 		pertub_bms = bms;
		bms_count = bms;
	}
	conflict_weight = density / 3;
}

void build(){
	cicc.resize(vertex_count + 1);
    dp_penalty.resize(vertex_count + 1); 
    color_penalty_sum.assign(COLOR_NUM + 10, vector<long>(COLOR_NUM + 10, 0));
	vertex_freq.resize(vertex_count + 1, 0);

    indicator.resize(vertex_count + 1, false);
	remove_indicator.resize(vertex_count + 1, false);
	conflict_vertex_in_color.resize(vertex_count + 1);
	vertex_color.resize(vertex_count + 1, -1);
	candidate_degree.resize(adjacency_list.size());					
	color_use_number.resize(vertex_count + 1, 0);
	tabu.resize(vertex_count + 1, 0);
	conf.resize(vertex_count + 1, 1);
	node_score.resize(vertex_count + 1, 0);
	best_solution.resize(vertex_count + 1, -1);
	good_node_color.resize( vertex_count + 1, vector<short>(0,0));
							//cout << "build done 1" << endl;

	remaining_vertex.init(adjacency_list.size());//初始化剩余节点
	for (vector<vector<short>>::size_type v = 0; v < adjacency_list.size() - 1; ++v) { //将所有节点加入剩余节点列表
		remaining_vertex.push_back(v);
	}
	working_vertex.init(adjacency_list.size());		//初始化工作节点列表
    conflict_node_queue.init(adjacency_list.size());//初始化冲突节点队列
	valid_node.init(vertex_count + 1);				//初始化有效节点列表
							//cout << "build done 2" << endl;

	color_choice.resize(vertex_count + 1);
	for(auto v : remaining_vertex){
		int max_deg = 0;
		for (auto u : adjacency_list[v]){ //遍历节点 v 的所有邻居 u
			if (adjacency_list[u].size() > (size_t)max_deg){ //找出邻居中度数最大的节点
				max_deg = adjacency_list[u].size();
			}
			if(max_deg > COLOR_NUM){
				max_deg = COLOR_NUM;
				break;
			}
		}
			color_choice[v].resize(max_deg + 1, 0);
	}
							//cout << "build done 3" << endl;
	//color_choice.resize(vertex_count + 1, vector<short>(COLOR_NUM+1, 0));

	//good_node_color_index.resize(vertex_count + 1, vector<short>(COLOR_NUM+1, -1));
}

bool find_clique(long vv){

	long add_v = vv; //从add_v也就是vv开始寻找团
	if (remove_indicator[add_v] == true) return false; //节点已经被移除，直接返回
	if (temp_adjacency_list[add_v].size() > density) return false; //节点度数过大，跳过

	if (temp_adjacency_list[add_v].size() == 0){ //孤立节点，直接移除
		remove_indicator[add_v] = true; //删除标记
		remove_score += 1; //得分
		remaining_vertex.remove(add_v); //剩余节点
		remove_num++; //移除数量
		return true;
	}

	vector<long> candidate; //候选节点列表
	vector<long> clique; //找到的团
	clique.push_back(add_v); //将起始节点加入团

	for (auto u : temp_adjacency_list[add_v]){ //把所有邻居节点(除了被移除的点）加入候选集
		if (remove_indicator[u] == true) continue; //跳过已经被移除的节点
		candidate.push_back(u); //邻居节点加入候选集
		candidate_degree[u] = 0; //初始化候选集的度数为0
		indicator[u] = true; //标记u在候选集中，方便O(1)判断
	}

	for (auto u : candidate) { //计算在候选集中的每个节点的度数
		for (auto w : temp_adjacency_list[u]) {
			if (indicator[w] == true) { //利用标记快速判断邻居w是否在候选集中
				candidate_degree[u]++;
			}
		}
	}
	for (auto u : temp_adjacency_list[add_v]) { //重置标记
		indicator[u] = false;
	}

	while (!candidate.empty()){ //扩展团
		if (candidate.size() <= bms_count) { //候选集较小时，选择度数最大的节点加入团
			add_v = candidate[0];
			for (vector<long>::size_type i = 1; i < candidate.size(); ++i) { //遍历候选集
				long v = candidate[i];
				if (candidate_degree[v] > candidate_degree[add_v]) { //选择度数最大的节点
					add_v = v;
				}
			}
		}
		else { //候选集较大时，使用BMS策略选择节点加入团
			add_v = candidate[rand() % candidate.size()]; //随机选择一个节点作为初始节点
			for (unsigned long i = 1; i < bms_count; ++i) { //进行bms_count次随机选择
				long v = candidate[rand() % candidate.size()]; //随机选择一个节点
				if (candidate_degree[v] > candidate_degree[add_v]) {//选择度数最大的节点 
					add_v = v;
				}
			}
		}

		for (auto v : candidate) { //在候选集里
			indicator[v] = true;
		}
		for (auto v : temp_adjacency_list[add_v]) { //add_v现在是将要加入团的节点，得是他的邻居
			indicator[v] = false;
		}
		for (vector<long>::size_type i = 0; i < candidate.size();){ 
			if (indicator[candidate[i]] == true){//candidate[i]在候选集里但不是add_v的邻居
				indicator[candidate[i]] = false;
				for (auto u : temp_adjacency_list[candidate[i]]){ //candidate[i]的所有邻居u的度数减1？？？会不会有没在candidate里的节点？
					candidate_degree[u]--; 
				}
				candidate[i] = *candidate.rbegin(); //用最后一个节点覆盖当前位置
				candidate.pop_back(); //删除最后一个节点
			}
			else {
				i++;
			}
		}
		clique.push_back(add_v);//将add_v加入团
	}

	vector<long> edge_out;
	for (auto v : clique){ //计算团内每个节点的出边数
		long node_edge_out = adjacency_list[v].size() - clique.size() + 1;//计算节点v的出边数=总度数-团内度数
		edge_out.push_back(node_edge_out); //记录每个节点的出边数
	}

	long edge_out_sum = 0;
	sort(edge_out.begin(), edge_out.end(), cmp_by_edgeout); //按出边数降序排序
	long i = 1;
	long clique_size = clique.size();
	bool valid_flag = true;
	for (auto v : edge_out){ //每个节点的出边数
		if (v > clique_size - i) { //阶梯状的规则
			valid_flag = false; 
		}
		++i;
		edge_out_sum += v; //计算出边数总和
	}

 	if (valid_flag){ //满足移除条件，移除该团内所有节点
		long i = 1;
 		for (auto v : clique){
 			remove_indicator[v] = true; //维护删除标记
			remove_clique(v); //移除顶点
			remove_num++; //维护移除数量
			remove_score += i; //维护移除得分
			i++; 
 		}
 	}

     return true;

}

void init_color_old(){
    long remainnign_size = remaining_vertex.size();
    long color_threshold = COLOR_NUM;
    if (remainnign_size < color_threshold) color_threshold = remainnign_size;

    for (auto v : remaining_vertex){
        vector<long> neig_color;
        neig_color.resize(color_threshold, 0);
        for (auto u : adjacency_list[v]){
            if (vertex_color[u] != -1){
                if (vertex_color[u] >= neig_color.size()) {
                    neig_color.resize(vertex_color[u] + 2, 0);
                }
                neig_color[vertex_color[u]] = 1;
            }
        }

        long color = 0;
        for (long i = 0; i < neig_color.size(); i++){
            if (neig_color[i] == 0) { color = i; break; }
        }
        if (neig_color[color] == 1) color = neig_color.size();

        if (color > max_color) max_color = color;
        vertex_color[v] = color;

        if ((size_t)color >= color_use_number.size()) {
            color_use_number.resize(color + 10, 0);
        }
        color_use_number[color]++;
    }

    finalize_init();  // 统一尾部
}

void init_color_degree_desc(){
    long remaining_size = remaining_vertex.size();
    long color_threshold = COLOR_NUM;
    if (remaining_size < color_threshold) color_threshold = remaining_size;

    // 1. 把所有 remaining_vertex 按度数降序排序
    vector<long> order;
    order.reserve(remaining_size);
    for (auto v : remaining_vertex) {
        order.push_back(v);
    }
    sort(order.begin(), order.end(), [](long a, long b){
        long da = temp_adjacency_list[a].size();
        long db = temp_adjacency_list[b].size();
        if (da != db) return da > db;  // 度数降序
        return a < b;                  // 度数相同时按编号，保证确定性
    });

    // 2. 按这个顺序贪心染最小可用色
    for (auto v : order){
        vector<long> neig_color;
        neig_color.resize(color_threshold, 0);

        // 注意：这里遍历 adjacency_list 而不是 temp_adjacency_list
        // 与 init_color_old 保持一致 —— 即使邻居在约简中被剥离，
        // 只要它已被赋色，就要避让（虽然实际上被剥离的节点 vertex_color 仍是 -1，所以无影响）
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
    }

    finalize_init();
}

void init_color_multi_random(){
    long runs = multi_init_runs;
    if (runs < 1) runs = 1;

    long best_pass_cost = std::numeric_limits<long>::max();
    vector<long> best_color_snapshot(vertex_count + 1, -1);
    long best_max_color = -1;
    vector<long> best_color_use_number;

    for (long r = 0; r < runs; ++r) {
        reset_color_assignment();
        long pc = single_random_greedy_pass();
        if (pc < best_pass_cost) {
            best_pass_cost = pc;
            // 保存这一轮的快照
            for (auto v : remaining_vertex) {
                best_color_snapshot[v] = vertex_color[v];
            }
            best_max_color = max_color;
            best_color_use_number = color_use_number;
        }
    }

    // 把最优快照恢复到全局状态
    reset_color_assignment();
    for (auto v : remaining_vertex) {
        vertex_color[v] = best_color_snapshot[v];
    }
    max_color = best_max_color;
    color_use_number = best_color_use_number;

    // 只对最优快照跑一次 finalize
    finalize_init();
}

void init_color_dp_aware(){
    long remaining_size = remaining_vertex.size();
    long color_threshold = COLOR_NUM;
    if (remaining_size < color_threshold) color_threshold = remaining_size;

    // 1. 预计算每个节点的 DP 包袱（带惩罚的颜色数）
    vector<long> dp_burden(vertex_count + 1, 0);
    for (auto v : remaining_vertex) {
        long cnt = 0;
        for (long p : dp_penalty[v]) {
            if (p > 0) cnt++;
        }
        dp_burden[v] = cnt;
    }

    // 2. 顶点排序：DP 包袱降序，相同则度数降序，再相同按编号
    vector<long> order;
    order.reserve(remaining_size);
    for (auto v : remaining_vertex) order.push_back(v);
    sort(order.begin(), order.end(), [&dp_burden](long a, long b){
        if (dp_burden[a] != dp_burden[b]) return dp_burden[a] > dp_burden[b];
        long da = temp_adjacency_list[a].size();
        long db = temp_adjacency_list[b].size();
        if (da != db) return da > db;
        return a < b;
    });

    // 3. 对每个节点：在所有"无冲突"颜色里选有效代价最小的
    for (auto v : order){
        // 标记已被邻居占用的颜色
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

        // 搜索范围必须覆盖到 dp_penalty[v] 的全长，
        // 否则可能错过"小色号有大惩罚、大色号惩罚为 0"的更优选择
        long penalty_size = (long)dp_penalty[v].size();
        long search_limit = std::max((long)neig_color.size(), penalty_size);

        long best_color = -1;
        long best_eff_cost = std::numeric_limits<long>::max();

        for (long c = 0; c < search_limit; ++c) {
            // 跳过被邻居占用的色
            if (c < (long)neig_color.size() && neig_color[c] == 1) continue;

            long eff_cost = c + get_penalty(v, c);
            if (eff_cost < best_eff_cost) {
                best_eff_cost = eff_cost;
                best_color = c;
                // 剪枝：c 单调递增、惩罚非负，
                // 一旦 c+1 已 >= 当前最优有效代价，后续不可能更好
                if ((long)(c + 1) >= best_eff_cost) break;
            }
        }

        // 兜底：search_limit 内全被占用（极罕见）
        if (best_color == -1) {
            best_color = (long)neig_color.size();
            while (best_color < (long)neig_color.size() && neig_color[best_color] == 1) {
                best_color++;
            }
        }

        long color = best_color;
        if (color > max_color) max_color = color;
        vertex_color[v] = color;

        if ((size_t)color >= color_use_number.size()) {
            color_use_number.resize(color + 10, 0);
        }
        color_use_number[color]++;
    }

    finalize_init();
}

long choose_good_node(long bms, long& BestNode, long& BestColor){//返回1表示找到合适节点，0表示没有
	//long bms = 100;
	//long iter = 0;

	long best_node = -1;
	long best_color = -1;
	long best_color_score = -1;

	if (!valid_node.empty()){
		long fail_count = 0;                          // 【改动】新增：记录跳过次数
		long max_fail = bms * 2;                      // 【改动】新增：防死循环上限
		for (long i = 0; i < bms; ){                  // 【改动】去掉 i++，改为手动递增
				if (fail_count >= max_fail) break;        // 【改动】新增：达到上限就退出
				//choose a rand node and rand color
				long index = rand() % valid_node.size();
				long node = valid_node[index];
				index = rand() % good_node_color[node].size();
				long new_color = good_node_color[node][index];
				
				// 统一策略跳过判断
				if (is_lock(node, new_color)) {
				    fail_count++;
				    continue;
				}
				i++;
		
				
			long current_color = vertex_color[node];
			double score = current_color - new_color + conflict_weight * (color_choice[node][current_color] - color_choice[node][new_color]);//打分函数

			if (score > best_color_score){
				best_color_score = score;
				best_node = node;
				best_color = new_color;
			}
		}
		BestNode = best_node;
		BestColor = best_color;
		return 1;
	}
	return 0;
}

long remove_conflict_new4(){//随机选择冲突节点，染色后tabu锁住
	if (edge_conflict > 0){
        long index = rand() % conflict_node_queue.size();
        long node = conflict_node_queue[index];
        long new_color = max_color + 1;

		if (new_color >= COLOR_NUM) 
			for (long i = 0; i < COLOR_NUM; i++) {
				if (i < color_choice[node].size() &&color_choice[node][i] == 0) { 
					new_color = i;
					break; 
				} 
			}

		if (new_color >= COLOR_NUM) 
			new_color = new_color % COLOR_NUM;
		
		color_node(node, new_color);
		current_iter++;
		no_impr++;
	}

	return 1;
}

long compute_score(){//计算实际染色的分数
	long sum_score = 0;
	for (auto v : remaining_vertex){
		sum_score += vertex_color[v] + 1;
	}
	return sum_score;
}

long compute_best_score(){//计算交换颜色后的分数
	long sum = 0;
	vector<long> color_num;
	for (long c = 0; c <= max_color; c++){
		color_num.push_back(color_use_number[c]);
	}
	sort(color_num.rbegin(),color_num.rend());
	for (long c = 0; c <= max_color; c++){
		sum = sum + color_num[c] * (c + 1);
	}
	return sum ;
}

void perturbation_old(long bms, long conflict_weight){

	long best_node = -1;
	long best_color = -1;
	long best_choose_score = -vertex_count;

	for (long i = 0; i < bms; ++i){ //随机采样bms次
		long index = rand() % remaining_vertex.size();//随机选择一个剩余节点
		long node = remaining_vertex[index];//获取该节点
		long current_color = vertex_color[node];//获取该节点的当前颜色
		long new_color = rand() % (max_color - current_color + 1) + current_color + 1;//随机选择一个比当前颜色大的新颜色
		long choose_score = (current_color - new_color) ;//计算选择得分，初始为颜色差值的负值


		//该节点变成新颜色后，邻居可以换成旧颜色，计算更换后的得分
		for (auto v : temp_adjacency_list[node]){//遍历该节点的所有邻居节点
			if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){//该邻居除了该节点为旧颜色外，还有其他邻居为旧颜色，但是该邻居的颜色比旧颜色大
					long delta_color = (vertex_color[v] - current_color) / 3;//有潜力，计算1/3
					choose_score += delta_color ;
			}
			if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){//该邻居只有该节点为旧颜色，且邻居的颜色比旧颜色大
					long delta_color = (vertex_color[v] - current_color);//绝对能降色，计算全部
					choose_score += delta_color;
			}
			if (vertex_color[v] == new_color) choose_score -= conflict_weight;//如果该邻居的颜色和新颜色相同，说明会产生冲突，扣分
		}
		if (choose_score > best_choose_score){//根据得分选择最佳节点和颜色
			best_node = node;
			best_color = new_color;
			best_choose_score = choose_score;
		}
	}
	no_impr++;
	color_node(best_node, best_color);
	current_iter++;
}

void big_pertub_old(long pertub_num, long bms, long conflict_weight){

	for (long i = 0; i < pertub_num; ++i){
		long best_node = -1;
		long best_color = -1;
		long best_choose_score = -vertex_count;
		
		long rand_color = rand() * 100 / RAND_MAX;
		if (rand_color <= 2){
			long index = rand() % remaining_vertex.size();
			long node = remaining_vertex[index];
			long current_color = vertex_color[node];
			long new_color = rand() % (max_color - current_color + 1) + current_color + 1;
			color_node(node, new_color);
			no_impr++;
			current_iter++;
			continue;
		}

			for (long i = 0; i < bms; ++i){
			long index = rand() % remaining_vertex.size();
			long node = remaining_vertex[index];
			long current_color = vertex_color[node];

			//#liyan 2 choice
			long new_color = rand() % (max_color + 2) ;
			//long new_color = rand() % (current_color + 1);
			long choose_score = (current_color - new_color) ;
			choose_score -= color_choice[node][new_color] * conflict_weight;
			for (auto v : temp_adjacency_list[node]){

				if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){
					long delta_color = (vertex_color[v] - current_color);
					choose_score += delta_color / 2;
				}
				if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){
					long delta_color = (vertex_color[v] - current_color);
					choose_score += delta_color;
				}

			}
			if (choose_score > best_choose_score){
				best_node = node;
				best_color = new_color;
				//best_node_old_color = vertex_color[node];
				best_choose_score = choose_score;
			}
		}

		color_node(best_node, best_color);
		current_iter++;
	}

	unlock_all_vertices(); // 解锁所有节点，准备进入下一轮迭代
}

void swap_two_color(long color_1, long color_2){
	//cout << "swap" << endl;
	if (color_1 == color_2) return;
	//#liyan debug
	//cout << "swap color: " << color_1 << " " << color_2 << endl;

	for (auto v : remaining_vertex){
		if (vertex_color[v] == color_1){
			color_node(v, color_2, false);
		}
		else if (vertex_color[v] == color_2){
			color_node(v, color_1, false);
		}
	}
}

void push_down_move() {
    long sz = remaining_vertex.size();
    if (sz == 0) return;

    if ((long)pd_tmp_cc_delta.size() < vertex_count + 1) {
        pd_tmp_cc_delta.assign(vertex_count + 1, 0);
    }

    // BMS 采样规模
    long sample = std::min(sz, 500L);

    for (long i = 0; i < sample; i++) {
        long v = remaining_vertex[rand() % sz];

        long c_v = vertex_color[v];
        if (c_v == 0) continue;

        // 选 v 的提升目标 c_new：从 c_v+1 开始找第一个不冲突的色号
        long c_new = -1;
        for (long c = c_v + 1; c <= max_color; c++) {
            long cv_at_c = (c < (long)color_choice[v].size()) ? color_choice[v][c] : 0;
            if (cv_at_c == 0) { c_new = c; break; }
        }
        if (c_new == -1) continue;

        // 原版无 DP 惩罚，delta 只看颜色差
        long delta = c_new - c_v;

        vector<long> accepted;
        pd_dirty_list.clear();

        for (auto u : temp_adjacency_list[v]) {
            long c_u = vertex_color[u];
            if (c_u <= c_v) continue;

            long real_cu_at_cv = (c_v < (long)color_choice[u].size()) ? color_choice[u][c_v] : 0;
            long effective_cc = real_cu_at_cv - 1 + pd_tmp_cc_delta[u];
            if (effective_cc != 0) continue;

            // 原版：c_u > c_v 必然带来 delta 减小，直接接受
            accepted.push_back(u);
            delta += c_v - c_u;

            for (auto w : temp_adjacency_list[u]) {
                if (pd_tmp_cc_delta[w] == 0) pd_dirty_list.push_back(w);
                pd_tmp_cc_delta[w]++;
            }
        }

        for (long w : pd_dirty_list) pd_tmp_cc_delta[w] = 0;
        pd_dirty_list.clear();

        if (delta < 0) {
            color_node(v, c_new, false);
            for (long u : accepted) {
                color_node(u, c_v, false);
            }
        }
    }
}

void update_best_solution(){
	long sz = remaining_vertex.size();
	long start_index = rand() % sz;
	long current_idx = start_index;
	for (long i = 0; i < sz; i++){//检查所有节点，尝试简单降色
		long node = remaining_vertex[current_idx];
		current_idx++;
		if (current_idx >= sz) {
			current_idx = 0;
		}
        long current_color = vertex_color[node];
        long best_color = current_color;
		for (long c = 0; c < current_color; c++){
			if (color_choice[node][c] == 0){
				best_color = c;
				break;
			}
		}
		if (best_color != current_color) color_node(node, best_color, false);
	}

	if (push_down_mode == 1) {
	push_down_move();
	}

	for (long i = 1; i <= max_color; i++){//颜色集合整体交换（大而顶点更多的颜色和小的交换）
		for (long j = i; j <= max_color; j++){
			if (color_use_number[j-1] < color_use_number[j]){
				swap_two_color(j,j-1);
			}
		}
	}

	long score = compute_score();//计算当前解的评分
	if (score < best_score){//更新最优解和最优评分
		best_score = score;//更新最优评分
		for (auto v : remaining_vertex){
			best_solution[v] = vertex_color[v];//保存当前解为最优解
		}
	}
}

bool color_node(long node, long color, bool lock_it){
	// 使用线性查找替代索引数组

    node_score[node] = 0; // 分数重置

    long old_conflict = 0;
    long new_conflict = 0;
    long old_color = vertex_color[node];
    cost = cost - old_color + color;


	if ((size_t)color >= color_use_number.size()) {
    color_use_number.resize(color + 10, 0); // 扩容并留点缓冲
	}
    color_use_number[old_color]--;
    color_use_number[color]++;
    
    // update max_color
    if (color > max_color) max_color = color;
    if (old_color == max_color){
        if (color_use_number[max_color] == 0){
            for (; max_color >= 0; max_color--){
                if (color_use_number[max_color] > 0){
                    break;
                }
            }
        }
    }
	
	if ((size_t)color >= color_choice[node].size()) { //如果新颜色 超出 当前节点颜色选择数组范围，扩展该数组
    	color_choice[node].resize(color + 1, 0);
    }

    // update info of neighborhood nodes
    for (auto v : temp_adjacency_list[node]){ //遍历node的邻居节点

		if ((size_t)color >= color_choice[v].size()) { //如果新颜色 超出 邻居颜色选择数组范围，扩展该数组
        	color_choice[v].resize(color + 1, 0);
    	}

        color_choice[v][old_color]--;
        color_choice[v][color]++;
        long current_neighbor_color = vertex_color[v];

        
		// 检查新颜色 color 是否导致邻居 v 的好颜色列表失效
        if (color_choice[v][color] > color_choice[v][current_neighbor_color]){
            // 遍历查找 color 并移除
            for (size_t i = 0; i < good_node_color[v].size(); ++i) {
                if (good_node_color[v][i] == color) {
                    // 将最后一个元素移到当前位置，然后弹出最后一个
                    good_node_color[v][i] = good_node_color[v].back();
                    good_node_color[v].pop_back();
                    
                    if (good_node_color[v].empty()) {
                        if (valid_node.exist(v)) valid_node.remove(v);
                    }
                    break; // 找到并处理后即可退出内层循环
                }
            }
        }

        // 检查旧颜色 old_color 是否应该加入邻居 v 的好颜色列表
        if (old_color < current_neighbor_color && color_choice[v][old_color] <= color_choice[v][current_neighbor_color]){
            bool exist = false;
            for(long c : good_node_color[v]) {
                if(c == old_color) {
                    exist = true;
                    break;
                }
            }
            if (!exist){
                good_node_color[v].push_back(old_color);
                if (good_node_color[v].size() == 1) valid_node.push_back(v);
            }
        }

        // old color has conflict with v
        if (vertex_color[v] == old_color){
            conflict_vertex_in_color[v]--;
            if (color_choice[v][old_color] == 0){
                if (conflict_node_queue.exist(v))
                    conflict_node_queue.remove(v);
            }

            // 邻居 v 现在的冲突减少了，需要检查 good_node_color[v] 里的候选颜色是否依然比当前颜色好
            for (long i = 0; i < (long)good_node_color[v].size(); ) {
                long neighbor_c = good_node_color[v][i];
                // 如果候选颜色的冲突数 比 当前颜色的冲突数还大（或者不再优），则移除
                if (color_choice[v][neighbor_c] > color_choice[v][current_neighbor_color]){
                    good_node_color[v][i] = good_node_color[v].back();
                    good_node_color[v].pop_back();
                    // 注意：这里不执行 i++，因为当前位置换来了新元素，需要再次检查
                } else {
                    i++;
                }
            }
            if (good_node_color[v].empty()) {
                if (valid_node.exist(v)) valid_node.remove(v);
            }
        }

        // new color has conflict with v
        if (vertex_color[v] == color){
            conflict_vertex_in_color[v]++;
            if (color_choice[v][color] == 1){
                conflict_node_queue.push_back(v);
            }

            // 邻居 v 的冲突增加了，可能有一些之前不是好颜色的颜色现在变成了好颜色
            for (long new_c = 0; new_c < current_neighbor_color; new_c++){
                if (color_choice[v][new_c] == color_choice[v][current_neighbor_color]){
                    // 检查 new_c 是否已存在
                    bool exist = false;
                    for (long c : good_node_color[v]) {
                        if (c == new_c) {
                            exist = true;
                            break;
                        }
                    }
                    
                    if (!exist){
                        good_node_color[v].push_back(new_c);
                        if (good_node_color[v].size() == 1) valid_node.push_back(v);
                    }
                }
            }
        }

        if (vertex_color[v] == old_color){
            ++old_conflict;
        } 
        if (vertex_color[v] == color) ++new_conflict;
    }

    // 修改 5: 更新当前节点 node 的 good_node_color
    good_node_color[node].clear(); 
    if (valid_node.exist(node)) valid_node.remove(node);
    
    for (long new_c = 0; new_c < color; new_c++){
        if (color_choice[node][new_c] <= color_choice[node][color]){
            // 这里不需要检查是否存在，因为是清空后重新添加，顺序从小到大，不会重复
            good_node_color[node].push_back(new_c);
            if (good_node_color[node].size() == 1) valid_node.push_back(node);
        }
    }

    // update conflict node queue
    if (old_conflict == 0 && new_conflict > 0){
        conflict_node_queue.push_back(node);
    }
    if (old_conflict > 0 && new_conflict == 0){
        if(conflict_node_queue.exist(node))
            conflict_node_queue.remove(node);
    }
    
    conflict_vertex_in_color[node] = new_conflict;
    edge_conflict = edge_conflict - 2*old_conflict + 2*new_conflict;
    
    vertex_color[node] = color;
    // 策略层回调：提交移动，统一处理 tabu/CC/CICC 的状态更新
    if (lock_it) {
        lock_unlock(node, old_color, color);
    }
    return true;
}

void localsearch_old(int cutoff){
	if (conflict_weight == 0) conflict_weight = 1; //避免冲突权重为0
	big_pert_node_num = vertex_count / big_pertub_num_k;//计算大扰动节点数
	if (big_pert_node_num > 500) big_pert_node_num = 500;//上限500 

	while (current_iter < max_iter)//迭代次数
	{
		long best_node = -1;
		long best_color = -1;
		long x = choose_good_node(choose_conflict_node_bms,best_node,best_color);//找到一个好的节点和颜色
		if (x == 1 && best_node != -1){ //如果能找到好的节点，进行贪心
			color_node(best_node,best_color);//对该节点进行染色
			current_iter++;
			no_impr++;
		}
		else{
			remove_conflict_new4();//贪心结束，进行冲突移除
		}
		
		long score = 0;
		if (edge_conflict == 0) score = compute_best_score();//如果没有冲突，就计算分数，计算时间
		best_time = clock();
		double run_time;
		run_time = (double) (best_time - begin_time) / CLOCKS_PER_SEC;
		if (edge_conflict == 0 && score < best_score) {//如果找到一个更好的解
			update_best_solution();//更新最优解
			final_time = run_time;//记录最终时间
			no_impr = 0;
		}
		if (run_time > cutoff) return;
		
		big_pertub(big_pert_node_num, big_pertub_bms, conflict_weight);

		if (edge_conflict == 0) perturbation(pertub_bms, conflict_weight);//普通扰动

	}
}



void tree_dp_reduction() {
    remove_score = 0; // 被剥离节点的总代价（分数）累加器
    
    vector<long> deg_queue; // 用于存放当前度数 <= 1 的“边缘叶子”节点
    
    // 遍历所有剩余节点，把所有末端叶子节点（只连着1条边）或孤立点（0条边）找出来入队
    for (auto v : remaining_vertex) {
        if (temp_adjacency_list[v].size() <= 1) {
            deg_queue.push_back(v);
            // 提前打上移除标记，防止在后续级联剥离时重复入队
            remove_indicator[v] = true; 
        }
    }

    long head = 0;
    // 2. 级联剥离与 DP 状态向上传递阶段
    while (head < deg_queue.size()) {
        long u = deg_queue[head++]; // u 就是当前要被剥离的叶子节点（你可以把它想象成基层员工）
        
        long parent = -1; // 寻找 u 唯一连着的那条边指向谁（即它的直属老板）
        for (auto w : temp_adjacency_list[u]) {
            parent = w; 
            break; 
        }
        long c1 = -1, c2 = -1;       // c1: 最优颜色，c2: 次优颜色
        long min1 = 1e9, min2 = 1e9; // min1: 最优总代价，min2: 次优总代价
        
        // u 只需要评估极小范围的颜色。假设它底下没有下属（没有任何惩罚），
        // 那么 dp_penalty[u].size() 为 0。它只需考察颜色 0、1、2 即可。
        long max_eval_color = dp_penalty[u].size() + 2; 
        
        for (long c = 0; c <= max_eval_color; ++c) {
            long cost = (c + 1) + get_penalty(u, c);
            
            // 维护最小值和次小值
            if (cost < min1) {
                min2 = min1; c2 = c1; // 原来的第一名变成第二名
                min1 = cost; c1 = c;  // 记录新的第一名
            } else if (cost < min2) {
                min2 = cost; c2 = c;  // 记录第二名
            }
        }
		//状态转移
        if (parent != -1) {
            long penalty = min2 - min1;
            
            if (penalty > 0) { 
                if (c1 >= dp_penalty[parent].size()) {
                    dp_penalty[parent].resize(c1 + 1, 0);
                }
                dp_penalty[parent][c1] += penalty;
            }
        }
        remove_score += min1;
        remove_clique(u);
        remove_num++;
        
		//级联反应
        if (parent != -1 && !remove_indicator[parent] && temp_adjacency_list[parent].size() <= 1) {
            remove_indicator[parent] = true;
            deg_queue.push_back(parent);
        }
    }
}

long compute_score_reduction(){//计算实际染色的分数
	long sum_score = 0;
	for (auto v : remaining_vertex){
		sum_score += (vertex_color[v] + 1) + get_penalty(v, vertex_color[v]);
	}
	return sum_score;
}

void swap_two_color_reduction(long c1, long c2){
    if (c1 == c2) return;

    long N1 = color_use_number[c1];
    long N2 = color_use_number[c2];
    long base_delta = (N1 - N2) * (c2 - c1); 

    long penalty_delta = 
          color_penalty_sum[c1][c2] + color_penalty_sum[c2][c1]   // 互换后的新惩罚
        - color_penalty_sum[c1][c1] - color_penalty_sum[c2][c2];  // 互换前的旧惩罚

    long total_delta = base_delta + penalty_delta;

    if (total_delta < 0){
        vector<long> nodes_to_swap;
        for (auto v : remaining_vertex){
            if (vertex_color[v] == c1 || vertex_color[v] == c2){
                nodes_to_swap.push_back(v);
            }
        }
        for (auto v : nodes_to_swap){
            if (vertex_color[v] == c1) {
                color_node_reduction(v, c2, false);
            } else {
                color_node_reduction(v, c1, false);
            }
        }
    }
}

void push_down_move_reduction() {
    long sz = remaining_vertex.size();
    if (sz == 0) return;

    if ((long)pd_tmp_cc_delta.size() < vertex_count + 1) {
        pd_tmp_cc_delta.assign(vertex_count + 1, 0);
    }

    // BMS 采样规模：可调参数
    // 小图（采样规模 >= 顶点数）退化为全遍历
    long sample = std::min(sz, 500L);

    for (long i = 0; i < sample; i++) {
        // 随机采样一个顶点（允许重复，简化实现）
        long v = remaining_vertex[rand() % sz];

        long c_v = vertex_color[v];
        if (c_v == 0) continue;

        // 选 v 的提升目标 c_new：从 c_v+1 开始找第一个不冲突的色号
        long c_new = -1;
        for (long c = c_v + 1; c <= max_color; c++) {
            long cv_at_c = (c < (long)color_choice[v].size()) ? color_choice[v][c] : 0;
            if (cv_at_c == 0) { c_new = c; break; }
        }
        if (c_new == -1) continue;

        long delta = (c_new + get_penalty(v, c_new)) - (c_v + get_penalty(v, c_v));

        vector<long> accepted;
        pd_dirty_list.clear();

        for (auto u : temp_adjacency_list[v]) {
            long c_u = vertex_color[u];
            if (c_u <= c_v) continue;

            long real_cu_at_cv = (c_v < (long)color_choice[u].size()) ? color_choice[u][c_v] : 0;
            long effective_cc = real_cu_at_cv - 1 + pd_tmp_cc_delta[u];
            if (effective_cc != 0) continue;

            long eff_old = c_u + get_penalty(u, c_u);
            long eff_new = c_v + get_penalty(u, c_v);
            if (eff_new >= eff_old) continue;

            accepted.push_back(u);
            delta += eff_new - eff_old;

            for (auto w : temp_adjacency_list[u]) {
                if (pd_tmp_cc_delta[w] == 0) pd_dirty_list.push_back(w);
                pd_tmp_cc_delta[w]++;
            }
        }

        for (long w : pd_dirty_list) pd_tmp_cc_delta[w] = 0;
        pd_dirty_list.clear();

        if (delta < 0) {
            color_node_reduction(v, c_new, false);
            for (long u : accepted) {
                color_node_reduction(u, c_v, false);
            }
        }
    }
}


// ===== push_down 测试专用 wrapper（测完可删） =====
void push_down_move_reduction_test() {
    clock_t pd_begin = clock();
    long cost_before = cost;
    long sz_before = remaining_vertex.size();

    // 记录调用前所有节点的颜色，用于统计被移动的节点数
    static vector<long> color_snapshot;
    if ((long)color_snapshot.size() < vertex_count + 1) {
        color_snapshot.assign(vertex_count + 1, 0);
    }
    for (auto v : remaining_vertex) {
        color_snapshot[v] = vertex_color[v];
    }

    // 调用真正的 push_down 函数
    push_down_move_reduction();

    // 统计
    long delta = cost_before - cost;  // 正值 = 改善
    long moved = 0;
    for (auto v : remaining_vertex) {
        if (color_snapshot[v] != vertex_color[v]) moved++;
    }

    pd_call_count++;
    if (delta > 0) {
        pd_success_count++;
        pd_total_gain += delta;
    }
    pd_nodes_moved += moved;
    pd_total_time += (double)(clock() - pd_begin) / CLOCKS_PER_SEC;

    cerr << "[PD_CALL] #" << pd_call_count
         << " gain=" << delta
         << " moved=" << moved
         << " time=" << (double)(clock() - pd_begin) / CLOCKS_PER_SEC << "s"
         << " cost_before=" << cost_before
         << " cost_after=" << cost
         << endl;
}
// =================================================

void update_best_solution_reduction(){
    long sz = remaining_vertex.size();
    long start_index = rand() % sz;
    
	long current_idx = start_index;
    // 1. 单点降色尝试
    for (long i = 0; i < sz; i++){

		long node = remaining_vertex[current_idx];
		current_idx++;
		if (current_idx >= sz) {
			current_idx = 0;
		}


        long current_color = vertex_color[node];
        long best_color = current_color;
        
        long eff_curr_cost = current_color + get_penalty(node, current_color);
        long min_eff_cost = eff_curr_cost;
        long limit_color = std::min((long)eff_curr_cost, max_color + 2);
        
        for (long c = 0; c < limit_color; c++){
            short conf_c = (c < (long)color_choice[node].size()) ? color_choice[node][c] : 0;
            if (conf_c == 0){
                long eff_new_cost = c + get_penalty(node, c);
                if (eff_new_cost < min_eff_cost){
                    min_eff_cost = eff_new_cost;
                    best_color = c;
                }
            }
        }
        if (best_color != current_color) {
            color_node_reduction(node, best_color, false);
        }
    }

	if (push_down_mode == 1) {
	push_down_move_reduction();
	}
	//push_down_move_reduction_test();

    // 2. 全局颜色集合交换
    for (long i = 1; i <= max_color; i++){
        for (long j = i; j <= max_color; j++){
                swap_two_color_reduction(j, i - 1);
        }
    }	

    // 3. 统计并更新最优解
    long score = cost + remaining_vertex.size();
    if (score < best_score){
        best_score = score;
        for (auto v : remaining_vertex){
            best_solution[v] = vertex_color[v];
        }
    }

	unlock_all_vertices(); // 解锁所有节点，准备进入下一轮迭代
}

bool color_node_reduction(long node, long color, bool lock_it){

	if (vertex_color[node] == -1) {
    cout << "Error: Trying to color an uninitialized or removed node!" << endl;
    return false;
	}
	vertex_freq[node]++;

    // 使用线性查找替代索引数组
    node_score[node] = 0; // 分数重置
    long old_conflict = 0;
    long new_conflict = 0;
    long old_color = vertex_color[node];

    //维护swap要用的数据结构
    long limit = dp_penalty[node].size(); 
	ensure_color_penalty_sum_size(std::max(old_color, color), limit);

    for (long target_c = 0; target_c < limit; target_c++) {
        long p = get_penalty(node, target_c);
        if (p > 0) {
            // node 离开了 old_color 阵营，old_color 阵营未来的账单里不再包含 node
            color_penalty_sum[old_color][target_c] -= p;
            // node 加入了新 color 阵营，新 color 阵营未来的账单必须算上 node
            color_penalty_sum[color][target_c] += p;
        }
    }


    // cost加入 DP 惩罚计算，维持全局代价的精确性
    cost = cost - (old_color + get_penalty(node, old_color)) + (color + get_penalty(node, color));

	if ((size_t)color >= color_use_number.size()) {
    color_use_number.resize(color + 10, 0); // 扩容并留点缓冲
	}
    color_use_number[old_color]--;
    color_use_number[color]++;
    
    // update max_color
    if (color > max_color) max_color = color;
    if (old_color == max_color){
        if (color_use_number[max_color] == 0){
            for (; max_color >= 0; max_color--){
                if (color_use_number[max_color] > 0){
                    break;
                }
            }
        }
    }
    
    if ((size_t)color >= color_choice[node].size()) {   //如果新颜色 超出 当前节点颜色选择数组范围，扩展该数组
        color_choice[node].resize(color + 1, 0);
    }

    auto get_safe_conflict = [&](long target_node, long target_color) -> short {
        if (target_color < color_choice[target_node].size()) {
            return color_choice[target_node][target_color];
        }
        return 0;
    };


    // update info of neighborhood nodes
    for (auto v : temp_adjacency_list[node]){   //遍历node的邻居节点

        if ((size_t)color >= color_choice[v].size()) {  //如果新颜色 超出 邻居颜色选择数组范围，扩展该数组
            color_choice[v].resize(color + 1, 0);
        }

        color_choice[v][old_color]--;
        color_choice[v][color]++;
        long current_neighbor_color = vertex_color[v];

        // 邻居当前颜色的代价 和 邻居染新颜色的代价
        long eff_curr_v = current_neighbor_color + get_penalty(v, current_neighbor_color);
        long eff_color = color + get_penalty(v, color);

		//邻居染新颜色的冲突 大于 邻居当前颜色的冲突 
		//说明新颜色 color 对邻居 v 来说不再是好颜色了，需要从 good_node_color[v] 中移除
        if (color_choice[v][color] > color_choice[v][current_neighbor_color]){
            for (size_t i = 0; i < good_node_color[v].size(); ++i) {
                if (good_node_color[v][i] == color) {
                    good_node_color[v][i] = good_node_color[v].back();
                    good_node_color[v].pop_back();
                    
                    if (good_node_color[v].empty()) {
                        if (valid_node.exist(v)) valid_node.remove(v);
                    }
                    break; 
                }
            }
        }

		//邻居染旧颜色的冲突数 小于等于 邻居当前颜色的冲突数
		//说明旧颜色 old_color 对邻居 v 来说可能又变成好颜色了，需要检查并加入 good_node_color[v]
        // 修复：计算 old_color 的实际代价，只有代价更小且冲突达标，才能算“好颜色”
        long eff_old_color = old_color + get_penalty(v, old_color);
        if (eff_old_color < eff_curr_v && color_choice[v][old_color] <= color_choice[v][current_neighbor_color]){
            bool exist = false;
            for(long c : good_node_color[v]) {
                if(c == old_color) {
                    exist = true;
                    break;
                }
            }
            if (!exist){
                good_node_color[v].push_back(old_color);
                if (good_node_color[v].size() == 1) valid_node.push_back(v);
            }
        }

        //如果邻居v旧颜色old_color恰好是它当前的颜色
		//说明v因为node的颜色改变而脱离了冲突，需要更新v的状态
        if (vertex_color[v] == old_color){
            conflict_vertex_in_color[v]--;
            if (color_choice[v][old_color] == 0){
                if (conflict_node_queue.exist(v))
                    conflict_node_queue.remove(v);
            }

            //邻居 v 因为脱离了冲突
			//需要检查 good_node_color[v] 里的候选颜色冲突数<= 当前减少后的冲突数
            for (long i = 0; i < (long)good_node_color[v].size(); ) {
                long neighbor_c = good_node_color[v][i];
                short conf_neighbor_c = get_safe_conflict(v, neighbor_c);

                // 如果候选颜色的冲突数更大，踢出
                if (conf_neighbor_c > color_choice[v][current_neighbor_color]){
                    good_node_color[v][i] = good_node_color[v].back();
                    good_node_color[v].pop_back();
                } else {
                    i++;
                }
            }
            if (good_node_color[v].empty()) {
                if (valid_node.exist(v)) valid_node.remove(v);
            }
        }

        //如果邻居v新颜色color恰好是它当前的颜色
		//说明v因为node的颜色改变而陷入了新的冲突，需要更新
        if (vertex_color[v] == color){
            conflict_vertex_in_color[v]++;
            if (color_choice[v][color] == 1){
                conflict_node_queue.push_back(v);
            }

			// 1. 瞬间建立 O(1) 的存在性哈希表 (利用局部 static vector 避免重复分配内存)
			static vector<bool> in_good_color;
			if (in_good_color.size() <= max_color + 2) {
				in_good_color.assign(max_color + 10, false); // 稍微多开一点缓冲
			}
			// 把当前邻居已有的好颜色打上 true 标记
			for (long c : good_node_color[v]) {
				in_good_color[c] = true;
			}

			// 2. 只需一层 O(C_max) 循环，内部是 O(1) 判断！
			long limit_v = std::min((long)eff_curr_v, max_color + 2);
			long t = 0;
			for (long new_c = 0; new_c < limit_v; new_c++){
				if (new_c == current_neighbor_color) continue;

				long eff_new_c = new_c + get_penalty(v, new_c);
				short conf_new_c = get_safe_conflict(v, new_c);

				if (eff_new_c < eff_curr_v && conf_new_c <= color_choice[v][current_neighbor_color]){
					// O(1) 瞬间判断是否存在！
					if (!in_good_color[new_c]){
						good_node_color[v].push_back(new_c);
						if (good_node_color[v].size() == 1) valid_node.push_back(v);
						in_good_color[new_c] = true; // 顺手更新标记

					}
				}
			}

			// 3. 用完后光速清空标记，留给下一次使用
			for (long c : good_node_color[v]) {
				in_good_color[c] = false;
			}
        }

        if (vertex_color[v] == old_color){
            ++old_conflict;
        } 
        if (vertex_color[v] == color) ++new_conflict;
    }

    // -------------------------------------------------------
    // 修改 5: 更新当前节点 node 自己的好颜色备选库
    // 同样扩大搜索圈，并使用有效代价作为金标准。
    // -------------------------------------------------------
    good_node_color[node].clear(); 
    if (valid_node.exist(node)) valid_node.remove(node);
    
    long eff_node_color = color + get_penalty(node, color);

	long limit_node = std::min((long)eff_node_color, max_color + 2);
	long t = 0;

    for (long new_c = 0; new_c < limit_node; new_c++){
        if (new_c == color) continue;

        long eff_new_c = new_c + get_penalty(node, new_c);
        short conf_new_c = get_safe_conflict(node, new_c);

        if (eff_new_c < eff_node_color && conf_new_c <= color_choice[node][color]){
            good_node_color[node].push_back(new_c);
            if (good_node_color[node].size() == 1) valid_node.push_back(node);

        }
    }

    // update conflict node queue
    if (old_conflict == 0 && new_conflict > 0){
        conflict_node_queue.push_back(node);
    }
    if (old_conflict > 0 && new_conflict == 0){
        if(conflict_node_queue.exist(node))
            conflict_node_queue.remove(node);
    }
    
    conflict_vertex_in_color[node] = new_conflict;
    edge_conflict = edge_conflict - 2*old_conflict + 2*new_conflict;
    
    vertex_color[node] = color;
    // 策略层回调：提交移动，统一处理 tabu/CC/CICC 的状态更新（可选）
    if (lock_it) {
        lock_unlock(node, old_color, color);
    }
    return true;
}

long choose_good_node_reduction(long bms, long& BestNode, long& BestColor){
    long best_node = -1;
    long best_color = -1;
    double best_color_score = -1; 

    if (!valid_node.empty()){
		long fail_count = 0;                          // 【改动】新增：记录跳过次数
		long max_fail = bms * 2;                      // 【改动】新增：防死循环上限
		for (long i = 0; i < bms; ){                  // 【改动】去掉 i++，改为手动递增
			if (fail_count >= max_fail) break;        // 【改动】新增：达到上限就退出
			//choose a rand node and rand color
			long index = rand() % valid_node.size();
			long node = valid_node[index];
			index = rand() % good_node_color[node].size();
			long new_color = good_node_color[node][index];
			
			// 统一策略跳过判断
			if (is_lock(node, new_color)) {
			    fail_count++;
			    continue;
			}
			i++;
		

            long current_color = vertex_color[node];
            long penalty_diff = get_penalty(node, current_color) - get_penalty(node, new_color);

            double score = (current_color - new_color) + penalty_diff + conflict_weight * (color_choice[node][current_color] - color_choice[node][new_color]);

            if (score > best_color_score){
                best_color_score = score;
                best_node = node;
                best_color = new_color;
            }
        }
        
        if (best_node != -1) {
            BestNode = best_node;
            BestColor = best_color;
            return 1;
        }
    }
    return 0;
}

void perturbation_reduction(long bms, long conflict_weight){

	long best_node = -1;
	long best_color = -1;
	long best_choose_score = -vertex_count;

	for (long i = 0; i < bms; ++i){ //随机采样bms次
		long index = rand() % remaining_vertex.size();//随机选择一个剩余节点
		long node = remaining_vertex[index];//获取该节点
		long current_color = vertex_color[node];//获取该节点的当前颜色
		long new_color = rand() % (max_color - current_color + 1) + current_color + 1;//随机选择一个比当前颜色大的新颜色
		long penalty_diff = get_penalty(node, current_color) - get_penalty(node, new_color);
        long choose_score = (current_color - new_color) + penalty_diff;


		//该节点变成新颜色后，邻居可以换成旧颜色，计算更换后的得分
		for (auto v : temp_adjacency_list[node]){//遍历该节点的所有邻居节点
			if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){//该邻居除了该节点为旧颜色外，还有其他邻居为旧颜色，但是该邻居的颜色比旧颜色大
					long old_eff = vertex_color[v] + get_penalty(v, vertex_color[v]);
    				long new_eff = current_color + get_penalty(v, current_color);
    				long delta_color = (old_eff - new_eff) / 3;
    				choose_score += delta_color;
			}
			if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){//该邻居只有该节点为旧颜色，且邻居的颜色比旧颜色大
					long old_eff = vertex_color[v] + get_penalty(v, vertex_color[v]);
    				long new_eff = current_color + get_penalty(v, current_color);
    				long delta_color = old_eff - new_eff; // 用真实有效代价的差值
    				choose_score += delta_color;
			}
			if (vertex_color[v] == new_color) choose_score -= conflict_weight;//如果该邻居的颜色和新颜色相同，说明会产生冲突，扣分
		}
		if (choose_score > best_choose_score){//根据得分选择最佳节点和颜色
			best_node = node;
			best_color = new_color;
			best_choose_score = choose_score;
		}
	}
	no_impr++;
	color_node_reduction(best_node, best_color);
	current_iter++;
}

long remove_conflict_new4_reduction(){//随机选择冲突节点，染色后tabu锁住
	if (edge_conflict > 0){
        long index = rand() % conflict_node_queue.size();
        long node = conflict_node_queue[index];
        long new_color = max_color + 1;

		if (new_color >= COLOR_NUM) {
			for (long i = 0; i < COLOR_NUM; i++) {
				// 加安全判断
				if (i < color_choice[node].size() && color_choice[node][i] == 0) { 
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
	}

	return 1;
}

void big_pertub_reduction(long pertub_num, long bms, long conflict_weight){

	for (long i = 0; i < pertub_num; ++i){
		long best_node = -1;
		long best_color = -1;
		//long best_node_old_color = -1;
		long best_choose_score = -vertex_count;
		
		long rand_color = rand() * 100 / RAND_MAX;
		if (rand_color <= 2){
			long index = rand() % remaining_vertex.size();
			long node = remaining_vertex[index];
			long current_color = vertex_color[node];
			long new_color = rand() % (max_color - current_color + 1) + current_color + 1;
			color_node_reduction(node, new_color);
			no_impr++;
			current_iter++;
			continue;
		}
			for (long i = 0; i < bms; ++i){
			long index = rand() % remaining_vertex.size();
			long node = remaining_vertex[index];
			long current_color = vertex_color[node];

			//#liyan 2 choice
			long new_color = rand() % (max_color + 2) ;
			//long new_color = rand() % (current_color + 1);
			long penalty_diff = get_penalty(node, current_color) - get_penalty(node, new_color);
			long choose_score = (current_color - new_color) + penalty_diff;

			choose_score -= color_choice[node][new_color] * conflict_weight;
			for (auto v : temp_adjacency_list[node]){

				if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){
					long old_eff = vertex_color[v] + get_penalty(v, vertex_color[v]);
					long new_eff = current_color + get_penalty(v, current_color);
					long delta_color = (old_eff - new_eff) / 2; 
					choose_score += delta_color;
				}
				if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){
					long old_eff = vertex_color[v] + get_penalty(v, vertex_color[v]);
					long new_eff = current_color + get_penalty(v, current_color);
					long delta_color = old_eff - new_eff; // 用真实有效代价的差值
					choose_score += delta_color;
				}

			}
			if (choose_score > best_choose_score){
				best_node = node;
				best_color = new_color;
				//best_node_old_color = vertex_color[node];
				best_choose_score = choose_score;
			}
		}

		color_node_reduction(best_node, best_color);
		current_iter++;
	}
	
	unlock_all_vertices(); // 解锁所有节点，准备进入下一轮迭代
}

void localsearch_reduction(int cutoff){
	if (conflict_weight == 0) conflict_weight = 1; //避免冲突权重为0
	big_pert_node_num = vertex_count / big_pertub_num_k;//计算大扰动节点数
	if (big_pert_node_num > 500) big_pert_node_num = 500;//上限500 

	while (current_iter < max_iter)//迭代次数
	{
		long best_node = -1;
		long best_color = -1;
		long x = choose_good_node_reduction(choose_conflict_node_bms,best_node,best_color);//找到一个好的节点和颜色
		if (x == 1 && best_node != -1){ //如果能找到好的节点，进行贪心
			color_node_reduction(best_node,best_color);//对该节点进行染色
			current_iter++;
			no_impr++;
		}
		else{
			remove_conflict_new4_reduction();//贪心结束，进行冲突移除
		}
		
		long score = 0;
        
		if (edge_conflict == 0) {score = cost + remaining_vertex.size();}//如果没有冲突，就计算分数，计算时间 
		//if (edge_conflict == 0) score = compute_best_score();

		best_time = clock();
		double run_time;
		run_time = (double) (best_time - begin_time) / CLOCKS_PER_SEC;
		if (edge_conflict == 0 && score < best_score) {//如果找到一个更好的解
			update_best_solution_reduction();//更新最优解
			final_time = run_time;//记录最终时间
			no_impr = 0;
		}

		if (run_time > cutoff) return;

		big_pertub(big_pert_node_num, big_pertub_bms, conflict_weight);

		if (edge_conflict == 0) perturbation(pertub_bms, conflict_weight);//普通扰动

	}
}



void chain_perturbation_reduction(long bms, long conflict_weight) {
    if (remaining_vertex.size() == 0) return;
 
    long best_net_gain = -vertex_count;  // 所有候选链中的最优净收益
    // 最优链的记录：seed降色信息 + 链上节点的提色信息
    long best_seed = -1;
    long best_seed_target = -1;
    vector<pair<long, long>> best_chain_moves; // <节点, 新颜色>
 
    for (long trial = 0; trial < bms; ++trial) {
        // ========== 第1步：随机选种子，寻找降色目标 ==========
        long idx = rand() % remaining_vertex.size();
        long seed = remaining_vertex[idx];
        long seed_color = vertex_color[seed];
        
        // 种子颜色已经是0，没有降色空间
        if (seed_color == 0) continue;
 
        // 寻找种子想降到的目标颜色：
        // 遍历比当前颜色小的颜色，找有效代价最小的那个
        long seed_eff = seed_color + get_penalty(seed, seed_color);
        long best_target = -1;
        long best_target_eff = seed_eff;
 
        for (long c = 0; c < seed_color; ++c) {
            long eff_c = c + get_penalty(seed, c);
            if (eff_c < best_target_eff) {
                best_target_eff = eff_c;
                best_target = c;
            }
        }
 
        // 没有比当前更好的颜色
        if (best_target == -1) continue;
 
        // ========== 第2步：找阻塞邻居，构造提色链 ==========
        // 阻塞邻居：颜色恰好等于 best_target 的邻居
        vector<long> blockers;
        for (auto v : temp_adjacency_list[seed]) {
            if (vertex_color[v] == best_target) {
                blockers.push_back(v);
            }
        }
 
        // 没有阻塞者，说明种子可以直接降色——这不应该出现在扰动阶段
        // （如果出现，说明 good_node_color 没包含它，可能是冲突数条件不满足）
        // 无论如何，这条链没有扰动意义，跳过
        if (blockers.empty()) continue;
 
        // 阻塞者太多（比如>5个），链的代价大概率很高，跳过
        if (blockers.size() > 5) continue;
 
        // 为每个阻塞者找最近可行色（比当前颜色大的最小无冲突颜色）
        vector<pair<long, long>> chain_moves; // <节点, 新颜色>
        long total_chain_cost = 0;
        bool chain_valid = true;
 
        for (auto blocker : blockers) {
            long blocker_color = vertex_color[blocker];
            long new_color = -1;
 
            // 找比 blocker_color 大的最小无冲突颜色
            // 搜索范围限制在 max_color+2 以内，避免跳到过大的颜色
            long search_limit = max_color + 2;
            if (search_limit >= COLOR_NUM) search_limit = COLOR_NUM - 1;
 
            for (long c = blocker_color + 1; c <= search_limit; ++c) {
                // 检查冲突数：该颜色在邻居中没有人用（或者少人用）
                short conf = 0;
                if (c < (long)color_choice[blocker].size()) {
                    conf = color_choice[blocker][c];
                }
                // 还要排除和种子的冲突：种子即将降到 best_target，不是 c，所以不影响
                // 但要注意链上其他 blocker 是否也选了同一个颜色
                if (conf == 0) {
                    new_color = c;
                    break;
                }
            }
 
            // 找不到无冲突色，退而求其次：选冲突最小的
            if (new_color == -1) {
                short min_conf = 32767;
                for (long c = blocker_color + 1; c <= search_limit; ++c) {
                    short conf = 0;
                    if (c < (long)color_choice[blocker].size()) {
                        conf = color_choice[blocker][c];
                    }
                    if (conf < min_conf) {
                        min_conf = conf;
                        new_color = c;
                    }
                }
            }
 
            if (new_color == -1) {
                // 极端情况，给一个 max_color+1
                new_color = max_color + 1;
            }
 
            // 计算该 blocker 提色的代价
            long cost_u = (new_color - blocker_color)
                        + (get_penalty(blocker, new_color) - get_penalty(blocker, blocker_color));
            total_chain_cost += cost_u;
 
            chain_moves.push_back({blocker, new_color});
        }
 
        // ========== 第3步：计算净收益 ==========
        // 种子降色的收益
        long seed_gain = (seed_color - best_target)
                       + (get_penalty(seed, seed_color) - get_penalty(seed, best_target));
 
        long net_gain = seed_gain - total_chain_cost;
 
        // ========== 第4步：和当前最优链比较 ==========
        if (net_gain > best_net_gain) {
            best_net_gain = net_gain;
            best_seed = seed;
            best_seed_target = best_target;
            best_chain_moves = chain_moves;
        }
    }
 
    // ========== 第5步：执行最优链 ==========
    if (best_seed == -1) {
        // 所有采样都没找到有效链，退化到原版单点扰动
        perturbation_reduction(bms, conflict_weight);
        return;
    }
 
    // 先提色链上的阻塞者（顺序：先提色，再降种子色，避免中间态冲突累积）
    for (auto& move : best_chain_moves) {
        color_node_reduction(move.first, move.second);
        current_iter++;
        no_impr++;
    }
 
    // 再降种子的颜色
    color_node_reduction(best_seed, best_seed_target);
    current_iter++;
    no_impr++;
}