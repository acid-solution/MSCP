#include"LS.h"
#include "util.h"

int main(int argc, char* argv[]){

	//0=tabu, 1=CC基础版, 2=CC+tabu混合, 3=CICC
	strategy_mode = 0;
	//0=不带约简，1=原版约简，2=dp约简
	reduction_mode = 2;
	//0=贪心初始化，1=度数降序初始化，2=多次随机初始化，3=DP感知初始化
	init_mode= 0;
	//0=原版局部搜索，1=reduction局部搜索
	localsearch_mode = 1;
	//0=不使用 push_down，1=使用 push_down
	push_down_mode = 1; 
	//0=不使用 pull_up，1=使用 pull_up
	pull_up_mode = 1;
	//0=原版大扰动
	big_pertub_mode = 0; 
	//0=原版扰动,1=chain扰动
	pertubation_mode = 0;

	aers_mode = 1;
	aers_diag = 0;
	small_graph_opt_mode = 1;
	small_graph_opt_diag = 0;

	if (getenv("AERS_MODE") != NULL) aers_mode = atoi(getenv("AERS_MODE"));
	if (getenv("AERS_DIAG") != NULL) aers_diag = atoi(getenv("AERS_DIAG"));
	if (getenv("INIT_MODE") != NULL) init_mode = atoi(getenv("INIT_MODE"));
	if (getenv("REDUCTION_MODE") != NULL) reduction_mode = atoi(getenv("REDUCTION_MODE"));
	if (getenv("PUSH_DOWN_MODE") != NULL) push_down_mode = atoi(getenv("PUSH_DOWN_MODE"));
	if (getenv("PULL_UP_MODE") != NULL) pull_up_mode = atoi(getenv("PULL_UP_MODE"));
	if (getenv("SMALL_GRAPH_OPT_MODE") != NULL) small_graph_opt_mode = atoi(getenv("SMALL_GRAPH_OPT_MODE"));
	if (getenv("SMALL_GRAPH_OPT_DIAG") != NULL) small_graph_opt_diag = atoi(getenv("SMALL_GRAPH_OPT_DIAG"));


	file_name= argv[1];
	cutoff = atoi(argv[2]);
	seed = atoi(argv[3]);
	//max_iter = atoi(argv[4]);
	srand(seed);

	read_file(file_name);
	small_graph_opt_active = (small_graph_opt_mode != 0 && vertex_count < small_graph_opt_threshold) ? 1 : 0;
	if (small_graph_opt_active) {
		init_mode = 4;
		push_down_mode = 2;
		pull_up_mode = 2;
	}
	begin_time = clock();
	build();

	reduction(reduction_mode);

	//clock_t init_time = clock();
	init_color(init_mode);

	if (getenv("INIT_ONLY") != NULL && atoi(getenv("INIT_ONLY")) != 0) {
		long init_score = compute_score_reduction() + remove_score;
		bool init_valid = verify_solution();
		cout << "INIT_ONLY"
			 << " score=" << init_score
			 << " seed=" << seed
			 << " init_mode=" << init_mode
			 << " colors=" << max_color + 1
			 << " remaining=" << remaining_vertex.size()
			 << " valid=" << (init_valid ? "PASS" : "FAIL")
			 << endl;
		return init_valid ? 0 : 1;
	}
	//test_init_effect(init_time, clock());

	if(remove_num == 0) localsearch_mode = 0;//如果没有进行约简，使用原版局部搜索
	
	localsearch(localsearch_mode, cutoff);

	print_best_score();

    return 0;
}
