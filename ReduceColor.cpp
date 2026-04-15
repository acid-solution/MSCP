#include"LS.h"
#include "util.h"

int main(int argc, char* argv[]){

	//0=tabu, 1=CC基础版, 2=CC+tabu混合, 3=CICC
	strategy_mode = 0;
	//0=不带约简，1=原版约简，2=dp约简
	reduction_mode = 0;
	//0=贪心初始化
	init_mode= 0;
	//0=原版局部搜索，1=reduction局部搜索
	localsearch_mode = 0;
	//0=不使用 push_down，1=使用 push_down
	push_down_mode = 1; 
	//0=原版扰动
	pertubation_mode = 0;

	file_name= argv[1];
	cutoff = atoi(argv[2]);
	seed = atoi(argv[3]);
	//max_iter = atoi(argv[4]);
	srand(seed);

	read_file(file_name);
	begin_time = clock();
	build();

	reduction(reduction_mode);
	init_color(init_mode);
	localsearch(localsearch_mode, cutoff);

	print_best_score();

    return 0;
}