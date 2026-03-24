


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
		
	//cout<<file_name<<endl;
	begin_time = clock();
	read_file(file_name);

	build_reduction();
	init_color_reduction(); 
	localsearch_reduction(cutoff);

	// build();
	// init_color();
	// localsearch(cutoff);

	if (!verify_solution()){//验证解的正确性
		cout << "solution error" << endl;
		getchar();
	}

	cout<<file_name << " " << best_score + remove_score << " " << final_time << " " << seed <<" "<<current_iter<<endl;

    return 0;
}