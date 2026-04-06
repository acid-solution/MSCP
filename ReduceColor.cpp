#include"LS.h"

int main(int argc, char* argv[]){

	if (vertex_count > 100000){
		bms = 10;
		choose_conflict_node_bms = bms;
	 	pertub_bms = bms;
		remove_conflict_bms = bms;
		bms_count = bms;
	}
	conflict_weight = density / 3;

	//0=tabu, 1=CC基础版, 2=CC+tabu混合, 3=CICC
	strategy_mode = 0;
	//0=不带约简，1=dp约简，2=原版约简
	reduction_mode = 0;
	//0=原版初始化，1=reduction初始化
	init_mode= 0;
	//0=原版局部搜索，1=reduction局部搜索
	localsearch_mode = 0;

	string file_name= argv[1];
	int cutoff = atoi(argv[2]);
	long seed = atoi(argv[3]);
	//max_iter = atoi(argv[4]);

	srand(seed);

	begin_time = clock();
	read_file(file_name);
	build();

	reduction(reduction_mode);
	init_color(init_mode);
	localsearch(localsearch_mode, cutoff);

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