


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

	strategy_mode = 1;//0=tabu, 1=CC基础版, 2=CC+tabu混合

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
	

	 //init_color(); 
	 //localsearch(cutoff);


	 init_color_reduction();
	 localsearch_reduction(cutoff);


	// 将历史最优的合法解恢复到当前图中
    for (auto v : remaining_vertex){
        vertex_color[v] = best_solution[v];
    }
	best_score = compute_score_reduction(); // 计算最终得分

    if (!verify_solution()){//验证解的正确性
        cout << "solution error" << endl;
        return -1;
    }

	cout<<file_name << " " << best_score + remove_score << " " << final_time << " " << seed <<" "<<current_iter<<endl;

    return 0;
}