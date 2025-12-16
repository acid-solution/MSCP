#include"ReduceColor.h"

int main(int argc, char* argv[]){

	begin_time = clock(); 
	string file_name;
	// long seed = 1;
	//seed = time(NULL) % 1000000;
	int cutoff = atoi(argv[2]);
	long seed = atoi(argv[3]);
	srand(seed);
	//cout << "seed:" << seed << endl;

	// int proportion = 100;
	// int size_threshold = 1000;
	file_name = argv[1];
	cout<<file_name<<endl;
    build(file_name);
	cout<<"init_color begin"<<endl;
	init_color(); 

	if (vertex_count > 100000){
		bms = 10;
		choose_conflict_node_bms = bms;
	 	pertub_bms = bms;
		remove_conflict_bms = bms;
	}

	conflict_weight = density / 3;

	localsearch_new(cutoff);
	cout<<file_name << " " << best_score + remove_score << " " << final_time << " " << seed << endl;

    return 0;
}