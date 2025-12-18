#include"ReduceColor.h"

int main(int argc, char* argv[]){

	begin_time = clock(); 
	string file_name;
	int cutoff = atoi(argv[2]);
	long seed = atoi(argv[3]);
	srand(seed);
	file_name = argv[1];

	cout<<file_name<<endl;
    build(file_name);

	cout << "build done" << endl;
	init_color(); 
	cout << "init color done" << endl;
	if (vertex_count > 100000){
		bms = 10;
		choose_conflict_node_bms = bms;
	 	pertub_bms = bms;
		remove_conflict_bms = bms;
	}

	conflict_weight = density / 3;
	cout<<"localsearch begin"<<endl;
	localsearch_new(cutoff);
	cout<<"localsearch done"<<endl;
	for(auto v : remaining_vertex){
		if(good_node_color[v].size()> max_size){
			max_size = good_node_color[v].size();
		}
	}
	cout<<file_name << " " << best_score + remove_score << " " << final_time << " " << seed <<" "<<max_size<<endl;

    return 0;
}