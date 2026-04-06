#include "basic.h"

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

			if ((size_t)color_v >= color_choice[u].size()) {
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
	for (auto v : remaining_vertex){
		best_solution[v] = vertex_color[v];//保存当前解为最优解
	}
	if(!verify_solution()) {
		cout << "error init_color_mis" << endl;
		exit(0);
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

		if ((size_t)color >= color_choice[v].size()) {	//如果新颜色 超出 邻居颜色选择数组范围，扩展该数组
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
			for (size_t index = 0; index < good_node_color[v].size(); index++){
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

void perturbation_new(long bms, long conflict_weight){
    // 1. 判空保护，防止在空图或搜索末期报错
    if (remaining_vertex.size() == 0) return;

    // 2. 使用 static 避免频繁的内存分配销毁
    static vector<bool> visited;
    if (visited.size() <= vertex_count) {
        visited.assign(vertex_count + 1, false);
    }
    //通过 visited_history 记录本次修改的节点，在函数末尾统一重置，避免在循环中频繁访问 visited 数组导致的性能下降
    vector<long> visited_history; 

    // 3. 随机选择局部子图的中心锚点
    long center_index = rand() % remaining_vertex.size();
    long seed_node = remaining_vertex[center_index];
    vector<long> candidates;
    candidates.push_back(seed_node);
    visited[seed_node] = true;
    visited_history.push_back(seed_node); 
    //设定全局收集上限，严格控制局部搜索的规模，防止稠密图导致耗时激增
    long max_candidates = 500; 

    // 4. 收集一阶邻居
    vector<long> first_order;
    long deg1 = temp_adjacency_list[seed_node].size();
    if (deg1 > 0) {
        //随机生成起始索引
        long start_idx = rand() % deg1; 
        for (long i = 0; i < deg1; ++i) {
            if (candidates.size() >= max_candidates) break; 
            //取模实现环形遍历
            long v = temp_adjacency_list[seed_node][(start_idx + i) % deg1];
            if (!visited[v]) {
                first_order.push_back(v);
                candidates.push_back(v);
                visited[v] = true;
                visited_history.push_back(v); 
            }
        }
    }

    // 5. 对一阶邻居洗牌，增加随机性，防止后续的二阶收集总是从同一批节点开始，导致局部搜索的多样性不足
    for (long i = (long)first_order.size() - 1; i > 0; --i) {
        swap(first_order[i], first_order[rand() % (i + 1)]);
    }

    // 6. 收集二阶邻居（应用同等截断规则）
    for (auto u : first_order){
        if (candidates.size() >= max_candidates) break; 
        long deg2 = temp_adjacency_list[u].size();
        if (deg2 == 0) continue;

		//同上
        long start_idx2 = rand() % deg2; 
        for (long i = 0; i < deg2; ++i){
            if (candidates.size() >= max_candidates) break; 
            long w = temp_adjacency_list[u][(start_idx2 + i) % deg2];
            if (!visited[w]) {
                candidates.push_back(w);
                visited[w] = true;
                visited_history.push_back(w); 
            }
        }
    }

    // 7. 评估候选节点的提色收益
    //使用 tuple 存储评估结果，格式：<收益得分, 节点ID, 目标颜色>
    vector<tuple<long, long, long>> evaluated_nodes;
    //预分配内存
    evaluated_nodes.reserve(candidates.size()); 

    for (auto node : candidates) {
        long current_color = vertex_color[node];
        //随机分配一个更大的颜色编号
        long new_color = rand() % (max_color - current_color + 1) + current_color + 1;
        
        //基础得分：提色本身带来目标函数恶化，记为负收益
        long choose_score = (current_color - new_color); 

        //打分该节点
        for (auto v : temp_adjacency_list[node]){
            if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){
                choose_score += (vertex_color[v] - current_color) / 3;
            }
            if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){
                choose_score += (vertex_color[v] - current_color);
            }
            if (vertex_color[v] == new_color) {
                choose_score -= conflict_weight;
            }
        }
        evaluated_nodes.push_back(make_tuple(choose_score, node, new_color));
    }

    // 8. 执行局部最优扰动与禁忌锁定
    //依据 tuple 的第一个元素（choose_score）进行降序排列
    sort(evaluated_nodes.rbegin(), evaluated_nodes.rend());
    
    //选取前 bms 个得分最高的节点执行提色扰动
    long num_to_perturb = std::min((long)bms, (long)evaluated_nodes.size());

    for (long i = 0; i < num_to_perturb; ++i) {
        long target_node = get<1>(evaluated_nodes[i]);
        long target_color = get<2>(evaluated_nodes[i]);
        //改变节点颜色
        color_node(target_node, target_color);
        no_impr++;
        current_iter++;
        lock_node(target_node);
    }

    //仅重置本次被修改过的节点标记
    for (auto v : visited_history) {
        visited[v] = false; 
    }
}

void perturbation_new_reduction(long bms, long conflict_weight){
    // 1. 判空保护，防止在空图或搜索末期报错
    if (remaining_vertex.size() == 0) return;

    // 2. 使用 static 避免频繁的内存分配销毁
    static vector<bool> visited;
    if (visited.size() <= vertex_count) {
        visited.assign(vertex_count + 1, false);
    }
    //通过 visited_history 记录本次修改的节点，在函数末尾统一重置，避免在循环中频繁访问 visited 数组导致的性能下降
    vector<long> visited_history; 

    // 3. 随机选择局部子图的中心锚点
    long center_index = rand() % remaining_vertex.size();
    long seed_node = remaining_vertex[center_index];
    vector<long> candidates;
    candidates.push_back(seed_node);
    visited[seed_node] = true;
    visited_history.push_back(seed_node); 
    //设定全局收集上限，严格控制局部搜索的规模，防止稠密图导致耗时激增
    long max_candidates = 500; 

    // 4. 收集一阶邻居
    vector<long> first_order;
    long deg1 = temp_adjacency_list[seed_node].size();
    if (deg1 > 0) {
        //随机生成起始索引
        long start_idx = rand() % deg1; 
        for (long i = 0; i < deg1; ++i) {
            if (candidates.size() >= max_candidates) break; 
            //取模实现环形遍历
            long v = temp_adjacency_list[seed_node][(start_idx + i) % deg1];
            if (!visited[v]) {
                first_order.push_back(v);
                candidates.push_back(v);
                visited[v] = true;
                visited_history.push_back(v); 
            }
        }
    }

    // 5. 对一阶邻居洗牌，增加随机性，防止后续的二阶收集总是从同一批节点开始，导致局部搜索的多样性不足
    for (long i = (long)first_order.size() - 1; i > 0; --i) {
        swap(first_order[i], first_order[rand() % (i + 1)]);
    }

    // 6. 收集二阶邻居（应用同等截断规则）
    for (auto u : first_order){
        if (candidates.size() >= max_candidates) break; 
        long deg2 = temp_adjacency_list[u].size();
        if (deg2 == 0) continue;

		//同上
        long start_idx2 = rand() % deg2; 
        for (long i = 0; i < deg2; ++i){
            if (candidates.size() >= max_candidates) break; 
            long w = temp_adjacency_list[u][(start_idx2 + i) % deg2];
            if (!visited[w]) {
                candidates.push_back(w);
                visited[w] = true;
                visited_history.push_back(w); 
            }
        }
    }

    // 7. 评估候选节点的提色收益
    //使用 tuple 存储评估结果，格式：<收益得分, 节点ID, 目标颜色>
    vector<tuple<long, long, long>> evaluated_nodes;
    //预分配内存
    evaluated_nodes.reserve(candidates.size()); 

    for (auto node : candidates) {
        long current_color = vertex_color[node];
        //随机分配一个更大的颜色编号
        long new_color = rand() % (max_color - current_color + 1) + current_color + 1;
        
        //基础得分：提色本身带来目标函数恶化，记为负收益
        long penalty_diff = get_penalty(node, current_color) - get_penalty(node, new_color);
        long choose_score = (current_color - new_color) + penalty_diff;

        //打分该节点
        for (auto v : temp_adjacency_list[node]){
            if (color_choice[v][current_color] == 2 && vertex_color[v] > current_color){
                choose_score += (vertex_color[v] - current_color) / 3;
            }
            if (color_choice[v][current_color] == 1 && vertex_color[v] > current_color){
                choose_score += (vertex_color[v] - current_color);
            }
            if (vertex_color[v] == new_color) {
                choose_score -= conflict_weight;
            }
        }
        evaluated_nodes.push_back(make_tuple(choose_score, node, new_color));
    }

    // 8. 执行局部最优扰动与禁忌锁定
    //依据 tuple 的第一个元素（choose_score）进行降序排列
    sort(evaluated_nodes.rbegin(), evaluated_nodes.rend());
    
    //选取前 bms 个得分最高的节点执行提色扰动
    long num_to_perturb = std::min((long)bms, (long)evaluated_nodes.size());

    for (long i = 0; i < num_to_perturb; ++i) {
        long target_node = get<1>(evaluated_nodes[i]);
        long target_color = get<2>(evaluated_nodes[i]);
        //改变节点颜色
        color_node_reduction(target_node, target_color);
        no_impr++;
        current_iter++;
        lock_node(target_node);
    }

    //仅重置本次被修改过的节点标记
    for (auto v : visited_history) {
        visited[v] = false; 
    }
}
