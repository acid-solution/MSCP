#pragma once
#include "basic.h"

bool cmp_by_edgeout(long x, long y){
	return x > y;
}

void build(string file_name) {
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
	//cout << line << endl;
	//cout << edge_count <<endl;

	adjacency_list.resize(vertex_count + 1);
    temp_adjacency_list.resize(vertex_count + 1);

	long v1, v2;
	if (vertex_count < 2000) while (in_file >> tmp >> v1 >> v2) {
		if (v1 == v2) continue;
		v1 = v1 - 1;
		v2 = v2 - 1;
		//cout << v1 << " " << v2 << endl;
		//adjacency_list[v2].push_back(v1);
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
		//cout << v1 << " " << v2 << endl;
		//adjacency_list[v2].push_back(v1);
		/*
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
		*/
	adjacency_list[v1].push_back(v2);
	adjacency_list[v2].push_back(v1);
	temp_adjacency_list[v1].push_back(v2);
	temp_adjacency_list[v2].push_back(v1);
	}
	in_file.close();
						cout << "read file done" << endl;
	/*
	for (long i=0; i < vertex_count - 1; i++){
		cout << i << ": ";
		for (auto v : adjacency_list[i]){
			cout << v << " ";
		}
		cout << endl;
	}
	*/

    indicator.resize(vertex_count + 1, false);
	color_indicator.resize(vertex_count + 1, 0);
	remove_indicator.resize(vertex_count + 1, false);
	conflict_vertex_in_color.resize(vertex_count + 1);
	vertex_color.resize(vertex_count + 1, -1);
	remaining_vertex.init(adjacency_list.size());
						cout << "init done 1" << endl;
	for (vector<vector<long>>::size_type v = 0; v < adjacency_list.size() - 1; ++v) {
		remaining_vertex.push_back(v);
	}
	working_vertex.init(adjacency_list.size());
    conflict_node_queue.init(adjacency_list.size());
	valid_node.init(vertex_count + 1);
	candidate_degree.resize(adjacency_list.size());
						cout << "init done 2" << endl;
	//best_color_num = adjacency_list.size() - 1;

	//index_vertex_vec.resize(adjacency_list.size());
	color_choice.resize(vertex_count + 1, vector<short>(COLOR_NUM, 0));
						cout << "init done 3" << endl;
	color_use_number.resize(vertex_count + 1, 0);
	tabu.resize(vertex_count + 1, 0);
	conf.resize(vertex_count + 1, 1);
	node_score.resize(vertex_count + 1, 0);
	best_solution.resize(vertex_count + 1, -1);
	local_opt_solution.resize(vertex_count + 1, -1);
						cout << "init done 4" << endl;
	good_node_color.resize( vertex_count + 1, vector<long>(0,0));
						cout << "init done 5" << endl;
	good_node_color_index.resize(vertex_count + 1, vector<short>(COLOR_NUM + 1, -1));
						cout << "init done 6" << endl;

	//node_color_score.resize(vertex_count + 1, vector<long>(COLOR_NUM, -1));
	//node_color node_color1;
	//node_color1.color = -1; node_color1.score = -k_conflict_color * vertex_count ;
	//Best_node_color.resize(vertex_count + 1, node_color1);
	//cout << "beginreduc " << (double) (clock() - begin_time) / CLOCKS_PER_SEC << endl;
	if (vertex_count > 2000) for (auto v : remaining_vertex){
		if (v != 0)
		find_clique(v);
	}
	//cout << "finishreduc " << (double) (clock() - begin_time) / CLOCKS_PER_SEC << endl;
	//cout << "remain size" << remaining_vertex.size() << endl;
	//cout << "remove score " << remove_score << endl;
	//cout << "remove num: " << remove_num << endl;

}

void build_test(string file_name) {
	//创建文件并打开
	ifstream in_file(file_name);
	if (!in_file.is_open()) {
		cout <<file_name<< " in_file error" << endl;
		exit(1);
	}

	string line;
	istringstream is;//读取行内容的流
	string p, tmp;
	do {
		getline(in_file, line);//读一行
		is.clear();//清除上一次的流状态
		is.str(line);//读出流内容
		is >> p >> tmp >> vertex_count >> edge_count;
	} while (p != "p");//直到读到标准行

	density = edge_count / vertex_count; //计算图的平均度数

	adjacency_list.resize(vertex_count + 1);//存储原始图结构
    temp_adjacency_list.resize(vertex_count + 1);//存储约简后的图结构

	long v1, v2;
	if (vertex_count < 2000) while (in_file >> tmp >> v1 >> v2) {//去重
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
		v1 = v1 - 1;//顶点编号从0开始
		v2 = v2 - 1;
	adjacency_list[v1].push_back(v2);			//邻接表存储图结构
	adjacency_list[v2].push_back(v1);
	temp_adjacency_list[v1].push_back(v2);
	temp_adjacency_list[v2].push_back(v1);
	}
	in_file.close();//读取完毕关闭文件

    indicator.resize(vertex_count + 1, false);
	color_indicator.resize(vertex_count + 1, 0);
	remove_indicator.resize(vertex_count + 1, false);
	conflict_vertex_in_color.resize(vertex_count + 1);
	vertex_color.resize(vertex_count + 1, -1);
	remaining_vertex.init(adjacency_list.size());
	for (vector<vector<long>>::size_type v = 0; v < adjacency_list.size() - 1; ++v) {//初始化顶点集合
		remaining_vertex.push_back(v);
	}
	working_vertex.init(adjacency_list.size());
    conflict_node_queue.init(adjacency_list.size());
	valid_node.init(vertex_count + 1);
	candidate_degree.resize(adjacency_list.size());

	
	color_use_number.resize(vertex_count + 1, 0);
	tabu.resize(vertex_count + 1, 0);
	conf.resize(vertex_count + 1, 1);
	node_score.resize(vertex_count + 1, 0);
	best_solution.resize(vertex_count + 1, -1);
	local_opt_solution.resize(vertex_count + 1, -1);

	good_node_color.resize( vertex_count + 1, vector<long>(0,0));
	

	if (vertex_count > 2000) for (auto v : remaining_vertex){
		if (v != 0)
		find_clique(v);
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
	long remainnign_size = remaining_vertex.size() ;
	long color_threshold = COLOR_NUM;
	if (remainnign_size < color_threshold) color_threshold = remainnign_size;

	//new init color function 
	/**/
	for (auto v : remaining_vertex){
		vector<long> neig_color;
		neig_color.resize(color_threshold,0);
		for (auto u : adjacency_list[v]){
			if (vertex_color[u] != -1){
				neig_color[vertex_color[u]] = 1;
			}
		}

		long color = -1;
		for (long i = 0; i < 1000; i++){
			if (neig_color[i] == 0) {
				color = i;
				break;
			}
		}
		if(color == -1){
			printf("error init color!!!!!!!!\n");
			exit(0);
		}

		if (color > max_color) max_color = color;
		vertex_color[v] = color;
		cost += color;
		color_use_number[color]++;
		//if (v == 0) color_use_number[color]--;// why???
	}

	for (auto v : remaining_vertex){
		long color_v = vertex_color[v];
		for (auto u : temp_adjacency_list[v]){
			color_choice[u][color_v]++;

			if (vertex_color[u] == vertex_color[v]){
				conflict_vertex_in_color[v]++;
				edge_conflict++;
			}
		}
	}
    for (auto v : remaining_vertex){
        if (conflict_vertex_in_color[v] > 0){
            conflict_node_queue.push_back(v);
        }
		
		//初始化good_node_color 数组，其中是可以降低冲突并降低花费的选择，之后reduce conflict and color 要从这里面选择
		long current_color = vertex_color[v];
		for (long new_color = 0; new_color < current_color; new_color++){
			if (color_choice[v][new_color] <= color_choice[v][current_color]){
				good_node_color_index[v][new_color] = good_node_color[v].size();
				good_node_color[v].emplace_back(new_color);
			}
		}
    }

	// init good_node_color
	for (auto n : remaining_vertex){
		if (good_node_color[n].size() > 0){
			valid_node.push_back(n);
		}
	}

}

void init_color_test(){
	//初始化图的颜色
	vector<long> neig_color(remaining_vertex.size() + 1, 0);
	for (auto v : remaining_vertex){
		fill(neig_color.begin(), neig_color.end(), 0);
		//找出邻居颜色
		for (auto u : adjacency_list[v]){
			if (vertex_color[u] != -1){
				neig_color[vertex_color[u]] = 1;
			}
		}
		//分配一个最小的可用颜色
		long color = -1;
		for (long i = 0;; i++){
			if (neig_color[i] == 0) {
				color = i;
				break;
			}
		}
		//维护最大颜色，当前顶点染色，花费，颜色使用数量
		if (color > max_color) max_color = color;
		vertex_color[v] = color;
		cost += color;
		color_use_number[color]++;
	
	}
	
	color_choice.resize(vertex_count + 1, vector<short>(max_color, 0));
	good_node_color_index.resize(vertex_count + 1, vector<short>(max_color + 1, -1));

	//初始化好移动节点
    for (auto v : remaining_vertex){
		long current_color = vertex_color[v];
		for (long new_color = 0; new_color < current_color; new_color++){
			if (color_choice[v][new_color] <= color_choice[v][current_color]){
				good_node_color_index[v][new_color] = good_node_color[v].size();
				good_node_color[v].emplace_back(new_color);
			}
		}
    }
	for (auto n : remaining_vertex){
		if (good_node_color[n].size() > 0){
			valid_node.push_back(n);
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

//choose and return a var
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

//choose and return a var
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

bool color_node(long node, long color){

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
		color_choice[v][old_color]--;
		color_choice[v][color]++;
		long current_neighbor_color = vertex_color[v];//这个邻居的当前颜色

		// color在good_node_color[v]中但是由于更新使它冲突增加，不再是候选节点
		if (good_node_color_index[v][color] != -1){			//如果 v邻居中 
			if (color_choice[v][color] > color_choice[v][current_neighbor_color]){//重新评估该顶点
				long index_color = good_node_color_index[v][color];
				long end_color = *good_node_color[v].rbegin();		

				good_node_color[v][index_color] = *good_node_color[v].rbegin();
				good_node_color[v].pop_back();
				good_node_color_index[v][end_color] = index_color;//v邻居中 染 末尾颜色 的节点 下标更新
				good_node_color_index[v][color] = -1;
				//valid node update
				if (good_node_color[v].empty()) 
				if (valid_node.exist(v))
				valid_node.remove(v);
			}
		}
		//old_color 不在good_node_color[v]中， 但是更新使它的冲突下降，让它加入到其中。
		if (good_node_color_index[v][old_color] == -1){		//如果 v邻居中 没有染 旧颜色 的节点
			if (old_color < current_neighbor_color && color_choice[v][old_color] <= color_choice[v][current_neighbor_color]){
				good_node_color_index[v][old_color] = good_node_color[v].size();
				good_node_color[v].push_back(old_color);
				//valid node update
				if (good_node_color[v].size() == 1)valid_node.push_back(v);
			}
		}

		// old color has conflict with v
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
		//new color has conflict with v
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

		if (vertex_color[v] == old_color){
			++old_conflict;
		} 
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

bool color_node_old(long node, long color){

	//tabu[node] = current_iter + TABU_TIME;
	//cout << edge_conflict << " ";
	//update config checking vector
	node_score[node] = 0;
	//conf[node] = 0;
	for (auto v : temp_adjacency_list[node]){
		conf[v] = 1;
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
		color_choice[v][old_color]--;
		color_choice[v][color]++;
		long current_neighbor_color = vertex_color[v];

		// color在good_node_color[v]中但是由于更新使它冲突增加，不再是候选节点
		if (good_node_color_index[v][color] != -1){
			if (color_choice[v][color] > color_choice[v][current_neighbor_color]){
				long index_color = good_node_color_index[v][color];
				long end_color = *good_node_color[v].rbegin();

				good_node_color[v][index_color] = *good_node_color[v].rbegin();
				good_node_color[v].pop_back();
				good_node_color_index[v][end_color] = index_color;
				good_node_color_index[v][color] = -1;
				//valid node update
				if (good_node_color[v].empty()) valid_node.remove(v);
			}
		}
		//old_color 不在good_node_color[v]中， 但是更新使它的冲突下降，让它加入到其中。
		if (good_node_color_index[v][old_color] == -1){
			if (old_color < current_neighbor_color && color_choice[v][old_color] <= color_choice[v][current_neighbor_color]){
				good_node_color_index[v][old_color] = good_node_color[v].size();
				good_node_color[v].push_back(old_color);
				//valid node update
				if (good_node_color[v].size() == 1)valid_node.push_back(v);
			}
		}

		// old color has conflict with v
		if (vertex_color[v] == old_color){
			conflict_vertex_in_color[v]--;
			//if (node == 285 && v == 143){cout << "hhh" << old_color << " " << color << " " << vertex_color[143] << endl; 
			//cout << color_choice[v][old_color]; getchar();}
			if (color_choice[v][old_color] == 0){
				//cout << "a" << endl;
				conflict_node_queue.remove(v);
				//cout << "a" << endl;
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
					//cout << "b" << endl;
					if (good_node_color[v].empty())valid_node.remove(v);
					//cout << "b" << endl;
				}
			}
		}
		//new color has conflict with v
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

		if (vertex_color[v] == old_color){
			++old_conflict;
		} 
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
		//cout << "d" << endl;
		conflict_node_queue.remove(node);
		//cout << "d" << endl;
		//getchar();
	}
	
	conflict_vertex_in_color[node] = new_conflict;
	edge_conflict = edge_conflict - 2*old_conflict + 2*new_conflict;
	
	vertex_color[node] = color;
	//cout << vertex_color[1] << endl;
	//cout << color_choice[143][17] << " " << node << " " << color << endl;
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
