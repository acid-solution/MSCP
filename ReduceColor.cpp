


#include"ReduceColor.h"

int main(int argc, char* argv[]){

	if (vertex_count > 100000){
		bms = 10;
		choose_conflict_node_bms = bms;
	 	pertub_bms = bms;
		remove_conflict_bms = bms;
		bms_count = bms;
	}
	conflict_weight = density / 3;
	//conflict_weight = density / atof(argv[4]);

	string file_name;
	int cutoff = atoi(argv[2]);
	long seed = atoi(argv[3]);
	srand(seed);
	file_name = argv[1];

	//max_iter = atoi(argv[4]);
		
	//cout<<file_name<<endl;
	begin_time = clock();
	read_file(file_name);



	build();

	// if (vertex_count > 2000) //顶点大于2000时才进行约简
	// for (auto v : remaining_vertex){//从每个点开始寻找团
	// 	if (v != 0)
	// 	find_clique(v);
	// }

	tree_dp_reduction();
	

	init_color(); 
	localsearch(cutoff);


	// init_color_reduction();
	// localsearch_reduction(cutoff);


	// 将历史最优的合法解恢复到当前图中
    for (auto v : remaining_vertex){
        vertex_color[v] = best_solution[v];
    }
	best_score = compute_score_reduction(); // 计算最终得分

    if (!verify_solution()){//验证解的正确性
        cout << "solution error" << endl;
        return -1;
    }

	// ================= 统计打表代码 =================
    // long penalty_node_count = 0;
    // long normal_node_count = 0;
    // long total_freq_penalty = 0;
    // long total_freq_normal = 0;

    // // 只统计 remaining_vertex（存活在约简图中的节点）
    // for (auto v : remaining_vertex) {
    //     // 如果 dp_penalty[v] 数组有大小，说明它吸收了被剥离节点的惩罚，就是带悬挂的节点
    //     if (dp_penalty[v].size() > 0) {
    //         penalty_node_count++;
    //         total_freq_penalty += vertex_freq[v];
    //     } else {
    //         normal_node_count++;
    //         total_freq_normal += vertex_freq[v];
    //     }
    // }

    // double avg_freq_penalty = penalty_node_count > 0 ? (double)total_freq_penalty / penalty_node_count : 0.0;
    // double avg_freq_normal = normal_node_count > 0 ? (double)total_freq_normal / normal_node_count : 0.0;

// 多线程安全单行输出
    // 数据列依次为: [文件名] [最终得分] [耗时] [随机种子] [迭代次数] [带惩罚点平均频率] [带惩罚点数量] [无惩罚点平均频率] [无惩罚点数量]
    // cout << file_name << " " 
    //      << best_score + remove_score << " " 
    //      << final_time << " " 
    //      << seed << " " 
    //      << current_iter << " " 
    //      << avg_freq_penalty << " " 
    //      << penalty_node_count << " " 
    //      << avg_freq_normal << " " 
    //      << normal_node_count << endl;
    // =================================================================



	cout<<file_name << " " << best_score + remove_score << " " << final_time << " " << seed <<" "<<current_iter<<endl;

    return 0;
}