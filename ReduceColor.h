#pragma once
#include "basic.h"
#include "MABSolver.h"

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
cout<<"reading file start"<<endl;
	do {
		getline(in_file, line);
		is.clear();
		is.str(line);
		is >> p >> tmp >> vertex_count >> edge_count;
	} while (p != "p");
cout<<"end reading file start"<<endl;
	density = edge_count / vertex_count; 

	adjacency_list.resize(vertex_count + 1);
    temp_adjacency_list.resize(vertex_count + 1);

	long v1, v2;
cout<<"begin reading edges"<<endl;
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

void build(){
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
						//cout << "build done 1" << endl;

	remaining_vertex.init(adjacency_list.size());//初始化剩余节点
	for (vector<vector<long>>::size_type v = 0; v < adjacency_list.size() - 1; ++v) {	//将所有节点加入剩余节点列表
		remaining_vertex.push_back(v);
	}
	working_vertex.init(adjacency_list.size());		//初始化工作节点列表
    conflict_node_queue.init(adjacency_list.size());//初始化冲突节点队列
	valid_node.init(vertex_count + 1);				//初始化有效节点列表
						//cout << "build done 2" << endl;

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
						//cout << "build done 3" << endl;
	//color_choice.resize(vertex_count + 1, vector<short>(COLOR_NUM+1, 0));

    //good_node_color_index.resize(vertex_count + 1, vector<short>(COLOR_NUM+1, -1));
	
						

						//exit(0);
	if (vertex_count > 2000) //顶点大于2000时才进行约简
	for (auto v : remaining_vertex){//从每个点开始寻找团
		//if (v != 0)
		//find_clique(v);
	}
}

bool find_clique(long vv){

	long add_v = vv;		//从add_v也就是vv开始寻找团
	if (remove_indicator[add_v] == true) return false;				//节点已经被移除，直接返回
	if (temp_adjacency_list[add_v].size() > density) return false;	//节点度数过大，跳过

	if (temp_adjacency_list[add_v].size() == 0){	//孤立节点，直接移除
		remove_indicator[add_v] = true;	//删除标记
		remove_score += 1;				//得分
		remaining_vertex.remove(add_v);	//剩余节点
		remove_num++;					//移除数量
		return true;
	}

    vector<long> candidate;		//候选节点列表
    vector<long> clique;		//找到的团
    clique.push_back(add_v);	//将起始节点加入团

    for (auto u : temp_adjacency_list[add_v]){		//把所有邻居节点(除了被移除的点）加入候选集
		if (remove_indicator[u] == true) continue;	//跳过已经被移除的节点
        candidate.push_back(u);		//邻居节点加入候选集
        candidate_degree[u] = 0;	//初始化候选集的度数为0
        indicator[u] = true;		//标记u在候选集中，方便O(1)判断
    }

	for (auto u : candidate) {		//计算在候选集中的每个节点的度数
		for (auto w : temp_adjacency_list[u]) {
			if (indicator[w] == true) {		//利用标记快速判断邻居w是否在候选集中
				candidate_degree[u]++;
			}
		}
	}
	for (auto u : temp_adjacency_list[add_v]) {	//重置标记
		indicator[u] = false;
	}

    while (!candidate.empty()){		//扩展团
		if (candidate.size() <= bms_count) {	//候选集较小时，选择度数最大的节点加入团
			add_v = candidate[0];
			for (vector<long>::size_type i = 1; i < candidate.size(); ++i) {//遍历候选集
				long v = candidate[i];
				if (candidate_degree[v] > candidate_degree[add_v]) {//选择度数最大的节点
					add_v = v;
				}
			}
		}
		else {									//候选集较大时，使用BMS策略选择节点加入团
			add_v = candidate[rand() % candidate.size()];				//随机选择一个节点作为初始节点
			for (unsigned long i = 1; i < bms_count; ++i) {				//进行bms_count次随机选择
				long v = candidate[rand() % candidate.size()];			//随机选择一个节点
				if (candidate_degree[v] > candidate_degree[add_v]) {//选择度数最大的节点	
					add_v = v;
				}
			}
		}

        for (auto v : candidate) {	//在候选集里
			indicator[v] = true;
		}
		for (auto v : temp_adjacency_list[add_v]) { //add_v现在是将要加入团的节点，得是他的邻居
			indicator[v] = false;
		}
        for (vector<long>::size_type i = 0; i < candidate.size();){	
            if (indicator[candidate[i]] == true){//candidate[i]在候选集里但不是add_v的邻居
                indicator[candidate[i]] = false;
                for (auto u : temp_adjacency_list[candidate[i]]){	//candidate[i]的所有邻居u的度数减1？？？会不会有没在candidate里的节点？
                    candidate_degree[u]--;		
                }
                candidate[i] = *candidate.rbegin();		//用最后一个节点覆盖当前位置
                candidate.pop_back();					//删除最后一个节点
            }
            else {
                i++;
            }
        }
        clique.push_back(add_v);//将add_v加入团
    }

	vector<long> edge_out;
	for (auto v : clique){	//计算团内每个节点的出边数
		long node_edge_out = adjacency_list[v].size() - clique.size() + 1;//计算节点v的出边数=总度数-团内度数
		edge_out.push_back(node_edge_out);	//记录每个节点的出边数
	}

	long edge_out_sum = 0;
	sort(edge_out.begin(), edge_out.end(), cmp_by_edgeout);	//按出边数降序排序
	long i = 1;
	long clique_size = clique.size();
	bool valid_flag = true;
	for (auto v : edge_out){	//每个节点的出边数
		if (v > clique_size - i) {	//阶梯状的规则
			valid_flag = false;		
		}
		++i;
		edge_out_sum += v;	//计算出边数总和
	}

 	if (valid_flag){	//满足移除条件，移除该团内所有节点
		long i = 1;
 		for (auto v : clique){
 			remove_indicator[v] = true;	//维护删除标记
			remove_clique(v);	//移除顶点
			remove_num++;		//维护移除数量
			remove_score += i;	//维护移除得分
			i++;		
 		}
 	}

     return true;

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

void init_color(){
	//init vertex with random color
	long remainnign_size = remaining_vertex.size();
	long color_threshold = COLOR_NUM;
	if (remainnign_size < color_threshold) color_threshold = remainnign_size;//调整最大颜色数目，避免颜色数目过大

	//new init color function 
	for (auto v : remaining_vertex){//给每个节点分配颜色
		vector<long> neig_color;
		neig_color.resize(color_threshold,0);//初始化邻居颜色记录数组
		for (auto u : adjacency_list[v]){	//遍历节点 v 的所有邻居 u
			if (vertex_color[u] != -1){		//如果邻居 u 已经被染色
				neig_color[vertex_color[u]] = 1;	//将该颜色标记为已被使用
			}
		}

		long color = 0;
		for (long i = 0; i < color_threshold; i++){	//寻找第一个未被使用的颜色
			if (neig_color[i] == 0) {
				color = i;
				break;
			}
		}

		if (color > max_color) max_color = color;	//更新最大颜色编号
		vertex_color[v] = color;					//为节点 v 分配颜色
		cost += color;								//更新当前解的花费
		color_use_number[color]++;					//维护颜色使用数量	
	}

	for (auto v : remaining_vertex){//初始化 color_choice 矩阵和冲突信息					
		long color_v = vertex_color[v];				//获取节点 v 的颜色编号
		for (auto u : temp_adjacency_list[v]){		//遍历节点 v 的所有邻居 u

			if (color_v >= color_choice[u].size()) {
                color_choice[u].resize(color_v + 1, 0);
            }

			color_choice[u][color_v]++;				//更新邻居 u 对应颜色 color_v 的选择计数

			if (vertex_color[u] == vertex_color[v]){	//如果邻居 u 与节点 v 颜色相同，表示冲突
				conflict_vertex_in_color[v]++;			//增加节点 v 的冲突计数
				edge_conflict++;						//增加总冲突边数	
			}
		}
	}

    for (auto v : remaining_vertex){//初始化冲突节点队列和 good_node_color 数组				
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
	for (auto n : remaining_vertex){//初始化有效节点列表	
		if (good_node_color[n].size() > 0){		//如果节点 n 存在好颜色
			valid_node.push_back(n);			//将节点 n 添加到有效节点列表中
		}
	}

	best_score = compute_score();
	if(!verify_solution()) {
		cout << "error init_color" << endl;
		exit(0);
	}

}

void init_color_mis(){

    vector<long> sorted_nodes; //方便排序的数组
    sorted_nodes.clear();
    sorted_nodes.reserve(remaining_vertex.size());
    for (auto v : remaining_vertex) {
        sorted_nodes.push_back(v);
    }

    //排序：度数从小到大
    sort(sorted_nodes.begin(), sorted_nodes.end(), [&](long a, long b) {
        if (temp_adjacency_list[a].size() != temp_adjacency_list[b].size())
            return temp_adjacency_list[a].size() < temp_adjacency_list[b].size();	
        return a < b;
    });

    //禁忌数组
    vector<short> forbidden_color_id; 
    forbidden_color_id.resize(adjacency_list.size() + 1, -1);

    long current_color = 0;//当前正在使用的颜色编号，从0号颜色开始
    long colored_count = 0;//已经染色的节点数量
    long total_nodes = sorted_nodes.size();

    //类似ksell和独立集染色
    while (colored_count < total_nodes) {
        bool used_this_color = false;

        for (auto v : sorted_nodes) {
            //已染色跳过
            if (vertex_color[v] != -1) continue;
            //如果 v 被标记为当前颜色禁忌，说明它的某个邻居已经染了 current_color
            if (forbidden_color_id[v] == current_color) continue; 

            //因为按颜色一层一层染色，所以只有同一层的颜色会冲突，之前层的颜色不会冲突
            
            vertex_color[v] = current_color;//为节点v分配颜色
            colored_count++;

            if (!used_this_color) {	//如果该颜色第一次被使用
                if (current_color > max_color) max_color = current_color;//更新最大颜色编号
                used_this_color = true;
            }
            cost += current_color;	//更新当前解的花费
            color_use_number[current_color]++;	//维护颜色使用数量

            //主动封锁邻居，既然 v 用了 current_color，那么 v 的所有邻居在这一轮都不能用了
            for (auto u : temp_adjacency_list[v]) {
                forbidden_color_id[u] = current_color;	//封锁邻居u
            }
        }
        current_color++;
    }

	for (auto v : remaining_vertex){//初始化 color_choice 矩阵和冲突信息					
		long color_v = vertex_color[v];				//获取节点 v 的颜色编号
		for (auto u : temp_adjacency_list[v]){		//遍历节点 v 的所有邻居 u

			if (color_v >= color_choice[u].size()) {
                color_choice[u].resize(color_v + 1, 0);
            }

			color_choice[u][color_v]++;				//更新邻居 u 对应颜色 color_v 的选择计数

			if (vertex_color[u] == vertex_color[v]){	//如果邻居 u 与节点 v 颜色相同，表示冲突
				conflict_vertex_in_color[v]++;			//增加节点 v 的冲突计数
				edge_conflict++;						//增加总冲突边数	
			}
		}
	}

    for (auto v : remaining_vertex){//初始化冲突节点队列和 good_node_color 数组				
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

	for (auto n : remaining_vertex){//初始化有效节点列表	
		if (good_node_color[n].size() > 0){		//如果节点 n 存在好颜色
			valid_node.push_back(n);			//将节点 n 添加到有效节点列表中
		}
	}

	best_score = compute_score();
	if(!verify_solution()) {
		cout << "error init_color_mis" << endl;
		exit(0);
	}

}

long choose_conflict_node(){
    long iter = 0;
    long last_impr = 0;
	long T_iter = 100;

	while (last_impr + T_iter > iter){
		
		no_impr++;
		iter++;
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

long choose_good_node(long bms, long& BestNode, long& BestColor){//返回1表示找到合适节点，0表示没有
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
			long score = current_color - new_color + conflict_weight * (color_choice[node][current_color] - color_choice[node][new_color]);//打分函数
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

long remove_conflict(){//随机贪心去除所有冲突

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

long remove_conflict_new2(long bms){//随机采样基于评分去除所有冲突
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

long remove_conflict_new4(){//随机选择冲突节点，染色后tabu锁住
	if (edge_conflict > 0){
		long index = rand() % conflict_node_queue.size();
		long node = conflict_node_queue[index];
		long current_color = vertex_color[node];
		long new_color = -1;
		current_iter++;
		no_impr++;

		if (conf[node] == 0) return 0;
		if (tabu[node] > current_iter) return 0;
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

long compute_score(){//计算实际染色的分数
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

void perturbation(long bms, long conflict_weight){

	long best_node = -1;
	long best_color = -1;
	long best_choose_score = -vertex_count;

	for (long i = 0; i < bms; ++i){				//随机采样bms次
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
	tabu[best_node] = current_iter + TABU_TIME;
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

void update_best_solution(){
	long sz = remaining_vertex.size();
	long start_index = rand() % sz;
	for (long i = 0; i < sz; i++){//检查所有节点，尝试简单降色
		long node = (start_index + i) % sz;
		long current_color = vertex_color[node];
		long best_color = current_color;
		for (long c = 0; c < current_color; c++){
			if (color_choice[node][c] == 0){
				best_color = c;
				break;
			}
		}
		if (best_color != current_color) color_node(node, best_color);
	}

	for (long i = 1; i <= max_color; i++){//颜色集合整体交换（大而顶点更多的颜色和小的交换）
		for (long j = i; j < max_color; j++){
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

void localsearch(int cutoff){
	if (conflict_weight == 0) conflict_weight = 1;		//避免冲突权重为0
	long big_pert_num = 0;
	big_pert_node_num = vertex_count / big_pertub_num_k;//计算大扰动节点数
	if (big_pert_node_num > 500) big_pert_node_num = 500;//上限500	

	while (current_iter < max_iter)//迭代次数
	{
		long best_node = -1;
		long best_color = -1;
		long x = choose_good_node(choose_conflict_node_bms,best_node,best_color);//找到一个好的节点和颜色
		if (x == 1 && best_node != -1){		//如果能找到好的节点，进行贪心
			color_node(best_node,best_color);//对该节点进行染色
			current_iter++;
			no_impr++;
			tabu[best_node] = current_iter + TABU_TIME;
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
			if (!verify_solution()){//验证解的正确性
				cout << "solution error" << endl;
				getchar();
			}
			final_time = run_time;//记录最终时间
			no_impr = 0;
		}

		if (run_time > cutoff) return;

		if (vertex_count < 100000 && no_impr > max_no_impr){//如果顶点小于10万且10万次迭代没有改进
			big_pertub(big_pert_node_num, big_pertub_bms, conflict_weight);
			max_no_impr = luby(2,big_pert_num) * max_no_impr_basic; //调整最大无改进次数，2倍luby序列
			no_impr = 0;
			big_pert_num++;
		}
		if (edge_conflict == 0) perturbation(pertub_bms, conflict_weight);//普通扰动

	}
}

void localsearch_MAB_1(int cutoff){
	if (conflict_weight == 0) conflict_weight = 1;		//避免冲突权重为0
	long big_pert_num = 0;
	big_pert_node_num = vertex_count / big_pertub_num_k;//计算大扰动节点数
	if (big_pert_node_num > 500) big_pert_node_num = 500;//上限500	


	std::vector<double> candidate_alphas = {0.5, 1.0, 3.0, 7.0, 10.0, 20.0}; 
    MABSolver mab(candidate_alphas);	//初始化MAB老虎机

	int current_arm = mab.select_arm(); // 初始选择
    double current_alpha_div = mab.get_alpha(current_arm);
    conflict_weight = (density / current_alpha_div);
    if (conflict_weight <= 0) cout<<"conflict weight error"<<endl;

	const int HISTORY_SIZE = 20;            // 记录最近20个周期
    const int CYCLE_LENGTH = vertex_count;          // 每个周期的迭代步数 (可根据图大小调整)
    int cycle_iter_count = 0;               // 当前周期已执行步数

	// [新增] 引入新的局部地形历史记录 (用 vector 替代旧的两个账本，保持轻量)
    std::vector<long> recent_scores;

    // 记录当前周期内见过的最好分数（初始化为最大值）
    long current_cycle_best_score = std::numeric_limits<long>::max();

    cout << "MAB Cyclic Started. Initial Alpha: " << current_alpha_div << endl;

	while (current_iter < max_iter)//迭代次数
	{
		long best_node = -1;
		long best_color = -1;
		long x = choose_good_node(choose_conflict_node_bms,best_node,best_color);//找到一个好的节点和颜色
		if (x == 1 && best_node != -1){		//如果能找到好的节点，进行贪心
			color_node(best_node,best_color);//对该节点进行染色
			current_iter++;
			no_impr++;
			tabu[best_node] = current_iter + TABU_TIME;
		}
		else{
			remove_conflict_new4();//贪心结束，进行冲突移除
		}
		
		long score = 0;
		if (edge_conflict == 0) {
			score = compute_best_score();//如果没有冲突，就计算分数，计算时间

			if (score < current_cycle_best_score) {
                current_cycle_best_score = score;
            }
		}
		best_time = clock();
		double run_time;
		run_time = (double) (best_time - begin_time) / CLOCKS_PER_SEC;
		if (edge_conflict == 0 && score < best_score) {//如果找到一个更好的解         
			update_best_solution();//更新最优解
			if (!verify_solution()){//验证解的正确性
				cout << "solution error" << endl;
				getchar();
			}
			final_time = run_time;//记录最终时间
			no_impr = 0;
		}

		// [新增] MAB 周期结算逻辑 
        cycle_iter_count++;
        if (cycle_iter_count >= CYCLE_LENGTH) {

			double reward = 0.0;

			if (current_cycle_best_score < std::numeric_limits<long>::max()) {
				if (recent_scores.empty()) {
                    // 历史记录为空（刚开始），既然拿到有效解，给满分鼓励
                    reward = 1.0; 
                } else {
                    // 1. 提取当前局部地形的最差和最好表现
                    long local_worst = current_cycle_best_score;
                    long local_best  = current_cycle_best_score;
                    for (long s : recent_scores) {
                        if (s > local_worst) local_worst = s;
                        if (s < local_best)  local_best  = s;
                    }
					// 2. 核心评价公式：你在近期的泥潭里排第几？
                    if (local_worst == local_best) {
                        // 防御机制：近期分数全部一样（完全卡死）
                        reward = (current_cycle_best_score < local_best) ? 1.0 : 0.5;
                    } else {
                        // 线性插值归一化 (越接近 local_best 分数越高)
                        reward = (double)(local_worst - current_cycle_best_score) / (local_worst - local_best);
                    }
                    
                    // 3. 双重保险 (防止破纪录导致 > 1.0，或极端情况 < 0.0)
                    if (reward > 1.0) reward = 1.0;
                    if (reward < 0.0) reward = 0.0;
                }
				// 4. 更新滑动窗口 (踢出最老的，放入最新的)
                recent_scores.push_back(current_cycle_best_score);
                if (recent_scores.size() > HISTORY_SIZE) {
                    recent_scores.erase(recent_scores.begin()); 
                }
            } else {
                // 如果本周期颗粒无收（完全在不可行区域乱转），给 0 分
                reward = 0.0; 
            }
            // 4. 更新 MAB 权重，选择下一个周期的参数
            mab.update(current_arm, reward);
             // [修改] 因为删除了 avg_history，稍微调整了终端打印信息，更关注当前得分和拿到的奖励
            cout << "Cycle End. Alpha: " << current_alpha_div 
                 << " score: " << current_cycle_best_score 
                 << " reward: " << reward;
            current_arm = mab.select_arm();
            current_alpha_div = mab.get_alpha(current_arm);
			 cout << " Next Alpha: " << current_alpha_div << endl;   
            // 6. 应用新参数
            conflict_weight = (density / current_alpha_div);
            if (conflict_weight <= 0) conflict_weight = 1;
            // 7. 重置周期计数器
            cycle_iter_count = 0;
            current_cycle_best_score = std::numeric_limits<long>::max();
        }


		if (vertex_count < 100000 && no_impr > max_no_impr){//如果顶点小于10万且10万次迭代没有改进
			big_pertub(big_pert_node_num, big_pertub_bms, conflict_weight);
			max_no_impr = luby(2,big_pert_num) * max_no_impr_basic; //调整最大无改进次数，2倍luby序列
			no_impr = 0;
			big_pert_num++;

			// [新增] 大扰动会彻底摧毁当前的地形结构！
            // 必须强制清空本周期的累计数据，防止大扰动造成的“分数变差”让当前的 Alpha 蒙冤背锅。
            cycle_iter_count = 0;
            current_cycle_best_score = std::numeric_limits<long>::max();
		}
		if (edge_conflict == 0) perturbation(pertub_bms, conflict_weight);//普通扰动

		if (run_time > cutoff) return;
	}
}
