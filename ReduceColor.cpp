#include"ReduceColor.h"

int main(int argc, char* argv[]){

	string file_name;
	int cutoff = atoi(argv[2]);
	long seed = atoi(argv[3]);
	srand(seed);
	file_name = argv[1];

	//cout<<file_name<<endl;

	read_file(file_name);
	//cout << "read file done" << endl;

	begin_time = clock(); 
    build();
	//cout << "build done" << endl;
	
	clock_t init_time_start = clock();
	//init_color();
	init_color_mis(); 
	clock_t init_time_end = clock();

	double duration_seconds = (double)(init_time_end - init_time_start) / CLOCKS_PER_SEC;
	cout<<file_name<<" "<<cost<<" "<<max_color<<" "<<duration_seconds<<endl;

	exit(0);
	//cout << "init color done" << endl;

	if (vertex_count > 100000){
		bms = 10;
		choose_conflict_node_bms = bms;
	 	pertub_bms = bms;
		remove_conflict_bms = bms;
		bms_count = bms;
	}
	conflict_weight = density / 3;

	cout<<"localsearch begin"<<endl;
	localsearch_new(cutoff);
	cout<<"localsearch done"<<endl;

	cout<<file_name << " " << best_score + remove_score << " " << final_time << " " << seed <<endl;

    return 0;
}