#pragma once
#include "basic.h"

bool cmp_by_edgeout(long x, long y){
	return x > y;
}

void read_file(string file_name){
	ifstream in_file(file_name);
	if (!in_file.is_open()) {
		cout << "in_file error" << endl;
		exit(1);
	}

	//get vertex_count
	string line;
	istringstream is;
	string p, tmp;
	do {
		getline(in_file, line);
		is.clear();
		is.str(line);
		is >> p >> tmp >> vertex_count >> edge_count;
	} while (p != "p");

	density = edge_count / vertex_count; 

	adjacency_list.resize(vertex_count + 1);
    temp_adjacency_list.resize(vertex_count + 1);

	long v1, v2;
	if (vertex_count < 2000) while (in_file >> tmp >> v1 >> v2) {
		if (v1 == v2) continue;
		v1 = v1 - 1;
		v2 = v2 - 1;
		if (connect[v1][v2] == 0){
			connect[v1][v2] = 1;
			adjacency_list[v1].push_back(v2);
        	temp_adjacency_list[v1].push_back(v2);
		}
		if (connect[v2][v1] == 0){
			connect[v2][v1] = 1;
			adjacency_list[v2].push_back(v1);
			temp_adjacency_list[v2].push_back(v1);
		}
	}
	else while (in_file >> tmp >> v1 >> v2) {
		if (v1 == v2) continue;
		v1 = v1 - 1;
		v2 = v2 - 1;

	adjacency_list[v1].push_back(v2);
	adjacency_list[v2].push_back(v1);
	temp_adjacency_list[v1].push_back(v2);
	temp_adjacency_list[v2].push_back(v1);
	}
	in_file.close();
}

void build() {
    indicator.resize(vertex_count + 1, false);
	color_indicator.resize(vertex_count + 1, 0);
	remove_indicator.resize(vertex_count + 1, false);
	conflict_vertex_in_color.resize(vertex_count + 1);
	vertex_color.resize(vertex_count + 1, -1);
	candidate_degree.resize(adjacency_list.size());					
	color_use_number.resize(vertex_count + 1, 0);
	tabu.resize(vertex_count + 1, 0);
	conf.resize(vertex_count + 1, 1);
	node_score.resize(vertex_count + 1, 0);
	best_solution.resize(vertex_count + 1, -1);
	local_opt_solution.resize(vertex_count + 1, -1);
	good_node_color.resize( vertex_count + 1, vector<long>(0,0));
						cout << "build done 1" << endl;

	remaining_vertex.init(adjacency_list.size());//初始化剩余节点
	for (vector<vector<long>>::size_type v = 0; v < adjacency_list.size() - 1; ++v) {	//将所有节点加入剩余节点列表
		remaining_vertex.push_back(v);
	}
	working_vertex.init(adjacency_list.size());		//初始化工作节点列表
    conflict_node_queue.init(adjacency_list.size());//初始化冲突节点队列
	valid_node.init(vertex_count + 1);				//初始化有效节点列表
						cout << "build done 2" << endl;

	color_choice.resize(vertex_count + 1);
	for(auto v : remaining_vertex){
		int max_deg = 0;
		for (auto u : adjacency_list[v]){	//遍历节点 v 的所有邻居 u
			if (adjacency_list[u].size() > max_deg){	//找出邻居中度数最大的节点
				max_deg = adjacency_list[u].size();
			}
			if(max_deg > COLOR_NUM){
				max_deg = COLOR_NUM;
				break;
			}
		}
			color_choice[v].resize(max_deg + 1, 0);
	}
						cout << "build done 3" << endl;
	//color_choice.resize(vertex_count + 1, vector<short>(COLOR_NUM+1, 0));
    //good_node_color_index.resize(vertex_count + 1, vector<short>(COLOR_NUM+1, -1));
	
						

						//exit(0);
	if (vertex_count > 2000) for (auto v : remaining_vertex){
		if (v != 0);
		//find_clique(v);
	}
}

bool find_clique(long vv){

    //long add_v = rand() % remaining_vertex.size();
 	//while (remove_indicator[add_v] == true) add_v = rand() % remaining_vertex.size();
	//if (vv == 0) return false;
	long add_v = vv;
	if (remove_indicator[add_v] == true) return false;
	if (temp_adjacency_list[add_v].size() > density) return false;
	if (temp_adjacency_list[add_v].size() == 0){
		remove_indicator[add_v] = true;
		//cout << "valid " ;
		//cout << add_v << endl;
		remove_score += 1;
		remaining_vertex.remove(add_v);
		remove_num++;
		return true;
	}

    vector<long> candidate;
    vector<long> clique;
    clique.push_back(add_v);

    for (auto u : temp_adjacency_list[add_v]){
		if (remove_indicator[u] == true) continue;
        candidate.push_back(u);
        candidate_degree[u] = 0;
        indicator[u] = true;
    }

	for (auto u : candidate) {
		for (auto w : temp_adjacency_list[u]) {
			if (indicator[w] == true) {
				candidate_degree[u]++;
			}
		}
	}

	for (auto u : temp_adjacency_list[add_v]) {
		indicator[u] = false;
	}
    while (!candidate.empty()){
		if (candidate.size() <= bms_count) {
			add_v = candidate[0];
			for (vector<long>::size_type i = 1; i < candidate.size(); ++i) {
				long v = candidate[i];
				if (candidate_degree[v] > candidate_degree[add_v]) {
					add_v = v;
				}
			}
		}
		else {
			add_v = candidate[rand() % candidate.size()];
			for (unsigned long i = 1; i < bms_count; ++i) {
				long v = candidate[rand() % candidate.size()];
				if (candidate_degree[v] > candidate_degree[add_v]) {
					add_v = v;
				}
			}
		}

        for (auto v : candidate) {
			indicator[v] = true;
		}
		for (auto v : temp_adjacency_list[add_v]) {
			indicator[v] = false;
		}
        for (vector<long>::size_type i = 0; i < candidate.size();){
            if (indicator[candidate[i]] == true){
                indicator[candidate[i]] = false;
                for (auto u : temp_adjacency_list[candidate[i]]){
                    candidate_degree[u]--;
                }
                candidate[i] = *candidate.rbegin();
                candidate.pop_back();
            }
            else {
                i++;
            }
        }
        clique.push_back(add_v);
    }
	vector<long> edge_out;
	for (auto v : clique){
		long node_edge_out = adjacency_list[v].size() - clique.size() + 1;
		edge_out.push_back(node_edge_out);
	}

	long edge_out_sum = 0;
	sort(edge_out.begin(), edge_out.end(), cmp_by_edgeout);
	long i = 1;
	long clique_size = clique.size();
	bool valid_flag = true;
	for (auto v : edge_out){
		if (v > clique_size - i) {
			valid_flag = false;
			//break;
		}
		++i;
		edge_out_sum += v;
	}
 	//cout << "clique size:" << clique_size << "EdgeOut:" << edge_out_sum << endl;

     //record all the clique
 	if (valid_flag){
 		//all_clique.push_back(clique);
 		//cout << "valid_clique ";
		long i = 1;
 		for (auto v : clique){
 			remove_indicator[v] = true;
			//remaining_vertex.remove(v);
 			//cout << v << " ";
			remove_clique(v);
			remove_num++;
			remove_score += i;
			i++;
 		}
 		//cout << endl;
 	}

     return true;

}

bool remove_clique(long v){
	vector<long> &clique = *all_clique.rbegin();

        for(auto u : temp_adjacency_list[v]){
			for (vector<long>::size_type i = 0; i < temp_adjacency_list[u].size(); ++i) {
				if (temp_adjacency_list[u][i] == v) {
					temp_adjacency_list[u][i] = *temp_adjacency_list[u].rbegin();
					temp_adjacency_list[u].pop_back();
					break;
				}
			}
        }
        remaining_vertex.remove(v);
    return true;
}

void init_color(){
	//init vertex with random color
	long remainnign_size = remaining_vertex.size();
	long color_threshold = COLOR_NUM;
	if (remainnign_size < color_threshold) color_threshold = remainnign_size;//调整最大颜色数目，避免颜色数目过大

	//new init color function 
	/**/
	for (auto v : remaining_vertex){
		vector<long> neig_color;
		neig_color.resize(color_threshold,0);//初始化邻居颜色记录数组
		for (auto u : adjacency_list[v]){	//遍历节点 v 的所有邻居 u
			if (vertex_color[u] != -1){		//如果邻居 u 已经被染色
				neig_color[vertex_color[u]] = 1;	//将该颜色标记为已被使用
			}
		}

		long color = -1;
		for (long i = 0; i < color_threshold; i++){	//寻找第一个未被使用的颜色
			if (neig_color[i] == 0) {
				color = i;
				break;
			}
		}
		if(color == -1){
			printf("error init color!!!!!!!!\n");
			exit(0);
		}

		if (color > max_color) max_color = color;	//更新最大颜色编号
		vertex_color[v] = color;					//为节点 v 分配颜色
		cost += color;								//更新当前解的花费
		color_use_number[color]++;					//维护颜色使用数量	
	}

	for (auto v : remaining_vertex){					
		long color_v = vertex_color[v];				//获取节点 v 的颜色编号
		for (auto u : temp_adjacency_list[v]){		//遍历节点 v 的所有邻居 u
			color_choice[u][color_v]++;				//更新邻居 u 对应颜色 color_v 的选择计数

			if (vertex_color[u] == vertex_color[v]){	//如果邻居 u 与节点 v 颜色相同，表示冲突
				conflict_vertex_in_color[v]++;			//增加节点 v 的冲突计数
				edge_conflict++;						//增加总冲突边数	
			}
		}
	}
    for (auto v : remaining_vertex){				
        if (conflict_vertex_in_color[v] > 0){		//如果节点 v 存在冲突
            conflict_node_queue.push_back(v);		//将节点 v 添加到冲突节点队列中
        }
		
		//初始化good_node_color 数组，其中是可以降低冲突并降低花费的选择，之后reduce conflict and color 要从这里面选择
		long current_color = vertex_color[v];										//获取节点 v 的当前颜色编号
		for (long new_color = 0; new_color < current_color; new_color++){			//遍历所有比当前颜色编号小的颜色
			if (color_choice[v][new_color] <= color_choice[v][current_color]){		//如果新颜色的冲突数小于当前颜色的冲突数
				//good_node_color_index[v][new_color] = good_node_color[v].size();
				good_node_color[v].emplace_back(new_color);							//将该颜色添加到节点 v 的好颜色列表中
			}
		}
    }

	// init good_node_color
	for (auto n : remaining_vertex){			
		if (good_node_color[n].size() > 0){		//如果节点 n 存在好颜色
			valid_node.push_back(n);			//将节点 n 添加到有效节点列表中
		}
	}

}

void init_color_new(){
	//init vertex with random color
	long remainnign_size = remaining_vertex.size();
	long color_threshold = COLOR_NUM;
	if (remainnign_size < color_threshold) color_threshold = remainnign_size;//调整最大颜色数目，避免颜色数目过大

	//new init color function 
	/**/
	for (auto v : remaining_vertex){
		vector<long> neig_color;
		neig_color.resize(color_threshold,0);//初始化邻居颜色记录数组
		for (auto u : adjacency_list[v]){	//遍历节点 v 的所有邻居 u
			if (vertex_color[u] != -1){		//如果邻居 u 已经被染色
				neig_color[vertex_color[u]] = 1;	//将该颜色标记为已被使用
			}
		}

		long color = -1;
		for (long i = 0; i < color_threshold; i++){	//寻找第一个未被使用的颜色
			if (neig_color[i] == 0) {
				color = i;
				break;
			}
		}
		if(color == -1){
			printf("error init color!!!!!!!!\n");
			exit(0);
		}

		if (color > max_color) max_color = color;	//更新最大颜色编号
		vertex_color[v] = color;					//为节点 v 分配颜色
		cost += color;								//更新当前解的花费
		color_use_number[color]++;					//维护颜色使用数量	
	}

	for (auto v : remaining_vertex){					
		long color_v = vertex_color[v];				//获取节点 v 的颜色编号
		for (auto u : temp_adjacency_list[v]){		//遍历节点 v 的所有邻居 u
			color_choice[u][color_v]++;				//更新邻居 u 对应颜色 color_v 的选择计数

			if (vertex_color[u] == vertex_color[v]){	//如果邻居 u 与节点 v 颜色相同，表示冲突
				conflict_vertex_in_color[v]++;			//增加节点 v 的冲突计数
				edge_conflict++;						//增加总冲突边数	
			}
		}
	}
    for (auto v : remaining_vertex){				
        if (conflict_vertex_in_color[v] > 0){		//如果节点 v 存在冲突
            conflict_node_queue.push_back(v);		//将节点 v 添加到冲突节点队列中
        }
		
		//初始化good_node_color 数组，其中是可以降低冲突并降低花费的选择，之后reduce conflict and color 要从这里面选择
		long current_color = vertex_color[v];										//获取节点 v 的当前颜色编号
		for (long new_color = 0; new_color < current_color; new_color++){			//遍历所有比当前颜色编号小的颜色
			if (color_choice[v][new_color] <= color_choice[v][current_color]){		//如果新颜色的冲突数小于当前颜色的冲突数
				//good_node_color_index[v][new_color] = good_node_color[v].size();
				good_node_color[v].emplace_back(new_color);							//将该颜色添加到节点 v 的好颜色列表中
			}
		}
    }

	// init good_node_color
	for (auto n : remaining_vertex){			
		if (good_node_color[n].size() > 0){		//如果节点 n 存在好颜色
			valid_node.push_back(n);			//将节点 n 添加到有效节点列表中
		}
	}

}

long choose_conflict_node(){
    long iter = 0;
    long last_impr = 0;
	long T_iter = 100;

	while (last_impr + T_iter > iter){
		
		no_impr++;
		iter++;
		// #liyan debug
 #ifndef NDEBUG
		//cout << "conflict queue size: " << conflict_node_queue.size() << endl;
 #endif
		if (conflict_node_queue.empty()) return 0;

		long index = rand() % conflict_node_queue.size();
		long node = conflict_node_queue[index];
        long current_color = vertex_color[node];
		vector<long> valid_color;

		if (conf[node] == 0) continue;
		if (tabu[node] > current_iter) continue;

		for (long c = 0; c < current_color; c++){
			if (color_choice[node][c] <= conflict_vertex_in_color[node]) valid_color.push_back(c);
		}

        if (!valid_color.empty()) {
			index = rand() % valid_color.size();
			long new_color = valid_color[index];
            last_impr = iter;
            color_node(node,new_color);
			current_iter++;
        }
	}
    return 0;
}

void choose_conflict_node_new(long bms){
	//long bms = 100;
	long iter = 0;

	//add all valid color to the vector
	while (true)
	{
		vector<long> valid_node;
		for (auto n : remaining_vertex){
			if (good_node_color[n].size() > 0){
				valid_node.emplace_back(n);
			}
		}
		if (valid_node.empty())break;

		long best_node = -1;
		long best_color = -1;
		long best_color_score = -1;
		for (long i = 0; i < bms; i++){
			//with low prop ranom choose
			long prop = rand() * 100 / RAND_MAX;
			/*
			if (prop < 2){
				long index = rand() % valid_node.size();
				best_node = valid_node[index];
				index = rand() % good_node_color[best_node].size();
				best_color = good_node_color[best_node][index];
				best_color_score = 0;
				//break;
			}
			*/
			//choose a rand node and rand color
			long index = rand() % valid_node.size();
			long node = valid_node[index];
			index = rand() % good_node_color[node].size();
			long new_color = good_node_color[node][index];
			if (conf[node] == 0) continue;
			if (tabu[node] > current_iter) continue;

			long current_color = vertex_color[node];
			//long score = current_color - new_color + k_conflict_color * (color_choice[node][current_color] - color_choice[node][new_color]);
			long score = (current_color - new_color) * (color_choice[node][current_color] - color_choice[node][new_color] + 1);

			// #liyan debug
			//cout << node << " color " << new_color << " score " << score << endl;
			//getchar();

			if (score > best_color_score){
				best_color_score = score;
				best_node = node;
				best_color = new_color;
			}
		}
		// #liyan debug
		//cout << best_node << " " << vertex_color[best_node] << " " << color_choice[best_node][vertex_color[best_node]] << endl;
		//cout << best_color << " " <<  color_choice[best_node][best_color] << endl;
		if (best_color_score < 0)break;
		color_node(best_node, best_color);
		tabu[best_node] = current_iter + TABU_TIME;
		current_iter++;
		no_impr++;
	}
	
}

long choose_good_node(long bms, long& BestNode, long& BestColor){
	//long bms = 100;
	long iter = 0;

	long best_node = -1;
	long best_color = -1;
	long best_color_score = -1;

	if (!valid_node.empty()){
		for (long i = 0; i < bms; i++){
			//choose a rand node and rand color
			long index = rand() % valid_node.size();
			long node = valid_node[index];
			index = rand() % good_node_color[node].size();
			long new_color = good_node_color[node][index];
			if (conf[node] == 0) continue;
			if (tabu[node] > current_iter) continue;

			long current_color = vertex_color[node];
			long score = current_color - new_color + conflict_weight * (color_choice[node][current_color] - color_choice[node][new_color]);
			// long score = current_color - new_color + k_conflict_color * (color_choice[node][current_color] - color_choice[node][new_color]);
			//long score = (current_color - new_color) * (color_choice[node][current_color] - color_choice[node][new_color] + 1);

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

long choose_good_node_new(long bms, long& BestNode, long& BestColor){
	//long bms = 100;
	long iter = 0;

	long best_node = -1;
	long best_color = -1;
	long best_color_score = -1;

	if (!valid_node.empty()){
		for (long i = 0; i < bms; i++){
			//choose a rand node and rand color
			long index = rand() % valid_node.size();
			long node = valid_node[index];
			index = rand() % good_node_color[node].size();
			long new_color = good_node_color[node][index];
			if (conf[node] == 0) continue;
			if (tabu[node] > current_iter) continue;

			long current_color = vertex_color[node];
			long score = current_color - new_color + conflict_weight * (color_choice[node][current_color] - color_choice[node][new_color]);
			// long score = current_color - new_color + k_conflict_color * (color_choice[node][current_color] - color_choice[node][new_color]);
			//long score = (current_color - new_color) * (color_choice[node][current_color] - color_choice[node][new_color] + 1);

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

long remove_conflict(){

	long max_iter_rc = vertex_count;
	long iter_rc = 0;
	while (edge_conflict > 0 && iter_rc < max_iter_rc)
	{
		iter_rc++;
		no_impr++;

		long index = rand() % conflict_node_queue.size();
		long node = conflict_node_queue[index];
		long current_color = vertex_color[node];

		if (conf[node] == 0) continue;
		if (tabu[node] > current_iter) continue;

		vector<long> valid_color;
		for (long c = 0; c < current_color; c++){
			if (color_choice[node][c] == 0) valid_color.push_back(c);
		}
		if (valid_color.empty()){
			for (long c = current_color + 1; c <= max_color + 1; c++){
				if (color_choice[node][c] == 0) valid_color.push_back(c);
			}
		}
		index = rand() % valid_color.size();
		long new_color = valid_color[index];
		color_node(node,new_color);
		current_iter++;
		
	}
	return edge_conflict;
}

long remove_conflict_new2(long bms){
	long max_iter_rc = vertex_count;
	long iter_rc = 0;
	//long bms = 10;
	long problem = 0;
	
	//if (problem > 0) cout << "problem:" << problem << endl;
	/*
	vector<long> least_pointer;
	least_pointer.resize(vertex_count + 1, -1);
	for (auto v : conflict_node_queue){
		for (long c = 0; c <= max_color + 1; c++){
			if (color_choice[v][c] == 0){
				least_pointer[v] = c;
				break;
			}
		}
	}
	*/

	while (edge_conflict > 0 && iter_rc < max_iter_rc)
	{
		iter_rc++;
		no_impr++;
		long best_rm_score = - vertex_count * max_color;
		long best_node = -1;
		long best_color = -1;
		for (long i = 0; i < bms; i++){
			long index = rand() % conflict_node_queue.size();
			long node = conflict_node_queue[index];
			long current_color = vertex_color[node];
			long new_color = -1;
			double score = 0;

			if (conf[node] == 0) continue;
			if (tabu[node] > current_iter) continue;
			//find the least conflictless color
			//cout << "max color" << max_color << endl;
			for (long c = 0; c <= max_color + 1; c++){
				if (color_choice[node][c] == 0){
					new_color = c;
					break;
				}
			}
			if (new_color < 0){
				cout << node << " " << new_color << "error" << endl;
				getchar(); 
			}
			//降低花费 同时 降低冲突，好节点，优先选择
			if (new_color < current_color){
				best_color = new_color;
				best_node = node;
				break;
			}
			if (new_color == current_color){
				continue;
			}
			score = color_choice[node][current_color] * 100 / (new_color - current_color);
			//cout << score << " ";
			for (auto v : temp_adjacency_list[node]){
				long current_neighbor_color = vertex_color[v];
				if (color_choice[v][current_neighbor_color] > 0 && color_choice[v][new_color] == 0){
					//score--;//	更改了remove color 中的选择标准
					score -= color_choice[v][current_neighbor_color] * 100 / (new_color - current_neighbor_color); 
				}
			}
			if (score > best_rm_score){
				best_rm_score = score;
				best_node = node;
				best_color = new_color;
			}
		}
		if (best_node == -1)break;
		color_node(best_node, best_color);
		current_iter++;
		no_impr++;
	}
	return edge_conflict;
}

long remove_conflict_new4(){
	if (edge_conflict > 0){
		long index = rand() % conflict_node_queue.size();
		long node = conflict_node_queue[index];
		long current_color = vertex_color[node];
		long new_color = -1;
		current_iter++;
		no_impr++;

		if (conf[node] == 0) return 0;
		if (tabu[node] > current_iter) return 0;
		//new_color = current_color + 1 + rand() % (max_color - current_color + 1);
		new_color = max_color + 1;

		if (new_color >= COLOR_NUM) 
			for (long i = 0; i < COLOR_NUM; i++) {
				if (color_choice[node][i] == 0) { 
					new_color = i; break; } }

		if (new_color >= COLOR_NUM) 
			new_color = new_color % COLOR_NUM;
		
		color_node(node, new_color);
		current_iter++;
		no_impr++;
		tabu[node] = current_iter + TABU_TIME;
	}

	return 1;
}

long compute_score(){
	long sum_score = 0;
	for (auto v : remaining_vertex){
		sum_score += vertex_color[v] + 1;
	}

	for (long c = 1; c <= max_color; c++){
		if (color_use_number[c] > color_use_number[c-1]){
			swap_two_color(c-1, c);
			return sum_score;
		}
	}
	return sum_score;
}

long compute_best_score(){
	long sum = 0;
	vector<long> color_num;
	for (long c = 0; c <= max_color; c++){
		color_num.push_back(color_use_number[c]);
		//cout << color_use_number[c] << " ";
	}
	sort(color_num.rbegin(),color_num.rend());
	for (long c = 0; c <= max_color; c++){
		sum = sum + color_num[c] * (c + 1);
		//cout << color_num[c] * (c+1) << " ";
	}
	//cout << sum << endl;
	//cout << sum << endl;
	//每次交换两个颜色，逐步使它达到递减排序
	for (long c = 1; c <= max_color; c++){
		if (color_use_number[c] > color_use_number[c-1]){
			swap_two_color(c-1,c);
			//while(color_use_number[c] > color_use_number[c-1] && c >= 1){
			//	swap_two_color(c-1,c);
			//	c--;
			//}
			return sum ;
		}
	}

	return sum ;
}

void perturbation(long bms, long conflict_weight){

	long best_node = -1;
	long best_color = -1;
	long best_node_old_color = -1;
	long best_choose_score = -vertex_count;
	/*
	long rand_prop = rand() % 100;
	if(rand_prop < 0){
		long index = rand() % remaining_vertex.size();
		long node = remaining_vertex[index];
		long current_color = vertex_color[node];
		long new_color = rand() % (max_color - current_color + 1) + current_color + 1;
		color_node(node,new_color);
		return;
	}
	*/

	for (long i = 0; i < bms; ++i){
		long index = rand() % remaining_vertex.size();
		long node = remaining_vertex[index];
		long current_color = vertex_color[node];

		//#liyan 2 choice
		long new_color = rand() % (max_color - current_color + 1) + current_color + 1;
		//if(rand() % 2 == 0) new_color++;
		//long new_color = rand() % (current_color + 1);
		long choose_score = (current_color - new_color) ;
		for (auto v : temp_adjacency_list[node]){
			/*
			//basic score function
			if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){
				choose_score++;
			}
			if (vertex_color[v] == new_color) choose_score--;
			*/

			/*
			//consider color-choice[node][color] == 1 or color_choice[node][color] == 2
			vector<long> valid_node;
			if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){
				bool flag = true;
				for (auto u : valid_node){
					if (connect[v][u] == 1) flag = false;
				}
				if (flag == true){
					choose_score++;
				}
			}
			if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){
				bool flag = true;
				for (auto u : valid_node){
					if (connect[v][u] == 1) flag = false;
				}
				if (flag == true){
					valid_node.push_back(v);
					choose_score += 3;
				}
			}
			if (vertex_color[v] == new_color) choose_score -= 3;
			*/
			
			// color_choice and the cost change
			vector<long> added_node;
			/**/
			if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){
				bool flag = true;
				for (auto u : added_node){
					if (connect[v][u] == 1) flag = false;
				}
				if (flag == true){
					long delta_color = (vertex_color[v] - current_color) / 3;
					choose_score += delta_color ;
					//added_node.push_back(v);
				}
			}
			if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){
				bool flag = true;
				for (auto u : added_node){
					if (connect[v][u] == 1) flag = false;
				}
				if (flag == true){
					long delta_color = (vertex_color[v] - current_color);
					choose_score += delta_color;
					//added_node.push_back(v);
				}
			}
			if (vertex_color[v] == new_color) choose_score -= conflict_weight;
			

			/*
			vector<long> valid_node;
			if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){
				bool flag = true;
				for (auto u : valid_node){
					if (connect[v][u] == 1) flag = false;
				}
				if (flag == true){
					choose_score++;
				}
			}
			if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){
				bool flag = true;
				for (auto u : valid_node){
					if (connect[v][u] == 1) flag = false;
				}
				if (flag == true){
					valid_node.push_back(v);
					choose_score += 3;
				}
			}
			if (vertex_color[v] == new_color) choose_score -= 3;
			*/
			
		}
		if (choose_score > best_choose_score){
			best_node = node;
			best_color = new_color;
			best_node_old_color = vertex_color[node];
			best_choose_score = choose_score;
		}
	}
	no_impr++;
	color_node(best_node, best_color);
	current_iter++;
	tabu[best_node] = current_iter + TABU_TIME;



	long pert_num = 0;
	for (long i = 0; i < pert_num; i++){
		long best_node = -1;
		long best_color = -1;
		long best_node_old_color = -1;
		long best_choose_score = -vertex_count;
		for (long i = 0; i < bms; ++i){
			long index = rand() % remaining_vertex.size();
			long node = remaining_vertex[index];
			long current_color = vertex_color[node];
			long new_color = rand() % (max_color - current_color + 1) + current_color ;
			long choose_score = current_color - new_color;
			for (auto v : temp_adjacency_list[node]){
				/*
				if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){
					choose_score++;
				}
				*/
				if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){
					long delta_color = (vertex_color[v] - current_color);
					choose_score += delta_color / 3;
				}
				if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){
					long delta_color = (vertex_color[v] - current_color);
					choose_score += delta_color;
				}
			}
			if (choose_score > best_choose_score){
				best_node = node;
				best_color = new_color;
				best_node_old_color = vertex_color[node];
				best_choose_score = choose_score;
			}
		}
		color_node(best_node, best_color);
		tabu[best_node] = current_iter + TABU_TIME;
		current_iter++;
	}
}

void big_pertub(long pertub_num, long bms, long conflict_weight){

	for (long i = 0; i < pertub_num; ++i){
		long best_node = -1;
		long best_color = -1;
		long best_node_old_color = -1;
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
			tabu[node] = current_iter + TABU_TIME;
			continue;
		}
		/*
		for (long i = 0; i < bms; ++i){
			long index = rand() % remaining_vertex.size();
			long node = remaining_vertex[index];
			long current_color = vertex_color[node];

			//#liyan 2 choice
			long new_color = rand() % (max_color - current_color + 1) + current_color ;
			//long new_color = rand() % (current_color + 1);
			long choose_score = 0;
			for (auto v : temp_adjacency_list[node]){

				if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){
					choose_score++;
				}
				if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){
					choose_score += 3;
				}
				if (vertex_color[v] == new_color) choose_score -= 3;

			}
			if (choose_score > best_choose_score){
				best_node = node;
				best_color = new_color;
				best_node_old_color = vertex_color[node];
				best_choose_score = choose_score;
			}
		}
		*/

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
				best_node_old_color = vertex_color[node];
				best_choose_score = choose_score;
			}
		}

		color_node(best_node, best_color);
		tabu[best_node] = current_iter + TABU_TIME;
		current_iter++;
	}
}

void swap_two_color(long color_1, long color_2){
	//cout << "swap" << endl;
	if (color_1 == color_2) return;
	//#liyan debug
	//cout << "swap color: " << color_1 << " " << color_2 << endl;

	for (auto v : remaining_vertex){
		if (vertex_color[v] == color_1){
			color_node(v, color_2);
		}
		else if (vertex_color[v] == color_2){
			color_node(v, color_1);
		}
	}
}

void sort_color(){
		
	//cout << "color use num" << endl;
	for (long i = 0; i <= max_color; i++){
		//cout << color_use_number[i] << " ";
		if (i == 0)continue;
		long j = i;
		if (color_use_number[i] > color_use_number[i-1]){
			for (; j > 0; j--){
				if (color_use_number[j] >= color_use_number[j - 1]) swap_two_color(j, j-1);
			}
		}
	}
	//cout << endl;

	//cout << "color use number after swap " << endl;
	for (long i = 0; i <= max_color; i++){
		long sum = 0;
		for (auto v : remaining_vertex){
			if (vertex_color[v] == i)sum++;
		}
		//cout << sum << " ";
	}
	//cout << endl;
	
}

void update_best_solution(){
	//find the best local optimal, and then update the best solution
	long sz = remaining_vertex.size();
	long start_index = rand() % sz;
	for (long i = 0; i < sz; i++){
		long node = (start_index + i) % sz;
		long current_color = vertex_color[node];
		long best_color = current_color;
		for (long c = 0; c < current_color; c++){
			if (color_choice[node][c] == 0){
				best_color = c;
				break;
			}
		}
		//cout << node << " " << current_color << " ";
		//cout << vertex_color[0] << " ";
		
		if (best_color != current_color) color_node(node, best_color);
	}

	//update the best solution
	long score = 0;
	//score = compute_score();
	score = compute_best_score();
	if (score < best_score){
		best_score = score;
		for (auto v : remaining_vertex){
			best_solution[v] = vertex_color[v];
		}
	}
	//sort color
	for (long i = 1; i <= max_color; i++){
		for (long j = i; j < max_color; j++){
			if (color_use_number[j-1] < color_use_number[j]){
				swap_two_color(j,j-1);
			}
		}
	}
}

bool color_node_old(long node, long color){
	//使用数组

	node_score[node] = 0;//分数重置
	for (auto v : temp_adjacency_list[node]){
		conf[v] = 1;//格局检测
	}
	

	long old_conflict = 0;
	long new_conflict = 0;
	long old_color = vertex_color[node];
	cost = cost - old_color + color;

	color_use_number[old_color]--;
	color_use_number[color]++;
	
	//update max_color
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

	//update info of neighborhood nodes
	for (auto v : temp_adjacency_list[node]){

		if (color >= color_choice[v].size()) {	//如果新颜色 超出 邻居颜色选择数组范围，扩展该数组
            color_choice[v].resize(color + 1, 0);
        }

		color_choice[v][old_color]--;
		color_choice[v][color]++;
		long current_neighbor_color = vertex_color[v];//这个邻居的当前颜色

		// 对邻居的影响：A桌变“香”了
		if (good_node_color_index[v][color] != -1){			//邻居的列表里没有A桌？
			if (color_choice[v][color] > color_choice[v][current_neighbor_color]){//重新评估该颜色的好坏
				long index_color = good_node_color_index[v][color];//获取该颜色在邻居好颜色列表中的下标
				long end_color = *good_node_color[v].rbegin();//获取邻居好颜色列表的末尾颜色		

				good_node_color[v][index_color] = *good_node_color[v].rbegin();//用末尾颜色替换该颜色
				good_node_color[v].pop_back();
				good_node_color_index[v][end_color] = index_color;//v邻居中 染 末尾颜色 的节点 下标更新
				good_node_color_index[v][color] = -1;
				//valid node update
				if (good_node_color[v].empty()) 
				if (valid_node.exist(v))
				valid_node.remove(v);
			}
		}
		//对邻居的影响：B桌变“坑”了
		if (good_node_color_index[v][old_color] == -1){		//邻居的列表里有B桌？
			if (old_color < current_neighbor_color && color_choice[v][old_color] <= color_choice[v][current_neighbor_color]){
				good_node_color_index[v][old_color] = good_node_color[v].size();
				good_node_color[v].push_back(old_color);
				//valid node update
				if (good_node_color[v].size() == 1)valid_node.push_back(v);
			}
		}

		// 特殊情况：邻居原本就坐在 A桌
		if (vertex_color[v] == old_color){
			conflict_vertex_in_color[v]--;
			if (color_choice[v][old_color] == 0){
				if (conflict_node_queue.exist(v))
				conflict_node_queue.remove(v);
			}

			//update good_node_color  
			for (long index = 0; index < good_node_color[v].size(); index++){
				long neighbor_c = good_node_color[v][index];
				if (color_choice[v][neighbor_c] > color_choice[v][current_neighbor_color]){
					long end_color = *good_node_color[v].rbegin();
					long index_c = good_node_color_index[v][neighbor_c];

					good_node_color[v][index_c] = *good_node_color[v].rbegin();
					good_node_color[v].pop_back();
					good_node_color_index[v][end_color] = index_c;
					good_node_color_index[v][neighbor_c] = -1;			
					//update valid node
					if (good_node_color[v].empty())
					if (valid_node.exist(v))
					valid_node.remove(v);
				}
			}
		}
		// 特殊情况：邻居原本就坐在 B桌
		if (vertex_color[v] == color){
			conflict_vertex_in_color[v]++;
			if (color_choice[v][color] == 1){
				conflict_node_queue.push_back(v);
			}

			// update good_node_color 	
			for (long new_c = 0; new_c < current_neighbor_color; new_c++){
				if (color_choice[v][new_c] == color_choice[v][current_neighbor_color] && good_node_color_index[v][new_c] == -1){
					good_node_color_index[v][new_c] = good_node_color[v].size();
					good_node_color[v].push_back(new_c);
					//update valid node
					if (good_node_color[v].size() == 1)valid_node.push_back(v);
				}
			}
		}

		if (vertex_color[v] == old_color)++old_conflict;
		if (vertex_color[v] == color) ++new_conflict;
	}

	//update good_node_color
	for(auto c : good_node_color[node]){
		good_node_color_index[node][c] = -1;
	}
	good_node_color[node].clear(); 
	//cout << "c" << endl;
	if (valid_node.exist(node))valid_node.remove(node);
	//cout << "c" << endl;
	
	 for (long new_c = 0; new_c < color; new_c++){
		if (color_choice[node][new_c] <= color_choice[node][color]){
			good_node_color_index[node][new_c] = good_node_color[node].size();
			good_node_color[node].push_back(new_c);
			if (good_node_color[node].size() == 1)valid_node.push_back(node);
		}
	}

	//update conflict node queue
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
	return true;
}

bool color_node(long node, long color){
	// 使用线性查找替代索引数组

    node_score[node] = 0; // 分数重置
    for (auto v : temp_adjacency_list[node]){
        conf[v] = 1;
    }

    long old_conflict = 0;
    long new_conflict = 0;
    long old_color = vertex_color[node];
    cost = cost - old_color + color;

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
	
	if (color >= color_choice[node].size()) {	//如果新颜色 超出 当前节点颜色选择数组范围，扩展该数组
    	color_choice[node].resize(color + 1, 0);
    }

    // update info of neighborhood nodes
    for (auto v : temp_adjacency_list[node]){	//遍历node的邻居节点

		if (color >= color_choice[v].size()) {	//如果新颜色 超出 邻居颜色选择数组范围，扩展该数组
        	color_choice[v].resize(color + 1, 0);
    	}

        color_choice[v][old_color]--;
        color_choice[v][color]++;
        long current_neighbor_color = vertex_color[v];

        // -------------------------------------------------------
        // 修改 1: 检查新颜色 color 是否导致邻居 v 的好颜色列表失效
        // 原逻辑：使用 index 数组快速查找并删除
        // 新逻辑：线性查找 color 是否存在于 good_node_color[v] 中，若存在且冲突增加则移除
        // -------------------------------------------------------
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

        // -------------------------------------------------------
        // 修改 2: 检查旧颜色 old_color 是否应该加入邻居 v 的好颜色列表
        // 原逻辑：通过 index == -1 判断不存在
        // 新逻辑：线性扫描判断不存在
        // -------------------------------------------------------
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

            // -------------------------------------------------------
            // 修改 3: 邻居 v 现在的冲突减少了（因为移除了冲突源 old_color），
            // 需要检查 good_node_color[v] 里的候选颜色是否依然比当前颜色好。
            // 原逻辑：for 循环配合 index 数组删除
            // 新逻辑：使用迭代器或下标遍历，遇到不满足条件的就用 swap-remove 删除
            // -------------------------------------------------------
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

            // -------------------------------------------------------
            // 修改 4: 邻居 v 的冲突增加了，可能有一些之前不是好颜色的颜色现在变成了好颜色
            // 原逻辑：index == -1 检查
            // 新逻辑：线性扫描检查是否存在
            // -------------------------------------------------------
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

    // -------------------------------------------------------
    // 修改 5: 更新当前节点 node 的 good_node_color
    // 原逻辑：利用 index 数组置 -1 然后 clear
    // 新逻辑：直接 clear 重新计算
    // -------------------------------------------------------
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

void localsearch_new(int cutoff){
	if (conflict_weight == 0) conflict_weight = 1;
	//conflict_weight = 25;

	best_score = vertex_count * vertex_count;
	long big_pert_num = 0;
	while (current_iter < max_iter)
	{
		long best_node = -1;
		long best_color = -1;
		long x = choose_good_node(choose_conflict_node_bms,best_node,best_color);
		if (x == 1 && best_node != -1){//good_var 不为空
			color_node(best_node,best_color);
			current_iter++;
			no_impr++;
			tabu[best_node] = current_iter + TABU_TIME;
		}
		else{
			//remove_conflict_new_3(remove_conflict_bms);
			remove_conflict_new4();
		}
		//cout << edge_conflict << " " << max_color << endl;
		//remove_conflict_new2(remove_conflict_bms);
		//cout << "cycle begin 3" << endl;
		//decrease_color();
		//decrease_color_new();
		
		long score = 0;
		if (edge_conflict == 0) score = compute_best_score();
		//if (edge_conflict == 0) score = compute_score();
		//cout << score << endl;

		best_time = clock();
		double run_time;
		run_time = (double) (best_time - begin_time) / CLOCKS_PER_SEC;

		if (run_time > cutoff) return ;

		if (edge_conflict == 0 && score < best_score) {
			//cout << "score: " << score << endl;
			//cout << score << endl;
			//for (auto i : remaining_vertex) cout << i << " ";
			//cout << endl;
			//for (int i = 0; i < adjacency_list.size(); i++) cout << color_choice[i][vertex_color[i]] << " ";
			//cout << endl;
			//for (int i = 0; i < adjacency_list.size(); i++) cout << i << ":" << vertex_color[i] << " ";
			update_best_solution();
			if (!verify_solution()){
				cout << "solution error" << endl;
				getchar();
			}
			//cout << score << "  "  << best_score << "  " << remove_score << endl;

			//cout << "best score: " << best_score + remove_score << " " << current_iter << " " << no_impr << " " << run_time << endl;
			final_time = run_time;
			no_impr = 0;
			big_pert_node_num = vertex_count / big_pertub_num_k;
			if (big_pert_node_num > 500) big_pert_node_num = 500;
			
			//cout << "color use num";
			//big_pert_node_num = 5;
			for (long c = 0; c <= max_color; c++){
				//cout << color_use_number[c] << " ";
			}
			//cout <<endl;
		}
		if (run_time > cutoff) return;

		if (vertex_count < 100000 && no_impr > max_no_impr){
			big_pertub(big_pert_node_num, big_pertub_bms, conflict_weight);
			//if (max_no_impr < max_no_impr_basic * 10) max_no_impr = max_no_impr * max_no_impr_increase_coefficient;
			max_no_impr = luby(2,big_pert_num) * max_no_impr_basic; //using luby restart
			no_impr = 0;
			big_pert_num++;
		}
		if (edge_conflict == 0) perturbation(pertub_bms, conflict_weight);

	}
}
