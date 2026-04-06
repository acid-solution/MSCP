#include "basic.h"
#include "ReduceColor.h"

void reduction(long mode){
    if (mode == 0) {
        // no reduction
    } else if (mode == 1) {
        if (vertex_count > 2000) //顶点大于2000时才进行约简
	    for (auto v : remaining_vertex){//从每个点开始寻找团
	    	if (v != 0)
	    	find_clique(v);
	    }         
    } else if(mode == 2) {
        tree_dp_reduction();
    } else {
        cout << "invalid reduction mode" << endl;
        exit(1);
    }
}

void init_color(long mode){
    if (mode == 0) {
        init_color_old();
    } else if (mode == 1) {
        init_color_reduction();
    } else {
        cout << "invalid init mode" << endl;
        exit(1);
    }
}

void localsearch(long mode, long cutoff){
    if (mode == 0) {
        localsearch_old(cutoff);
    } else if (mode == 1) {
        localsearch_reduction(cutoff);
    } else {
        cout << "invalid localsearch mode" << endl;
        exit(1);
    }
}