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

void big_pertub(long big_pert_node_num, long big_pertub_bms, long conflict_weight){
    if(pertubation_mode == 0 && localsearch_mode == 0){ 
        if (vertex_count < 100000 && no_impr > max_no_impr){//如果顶点小于10万且10万次迭代没有改进
			big_pertub_old(big_pert_node_num, big_pertub_bms, conflict_weight);
			max_no_impr = luby(2,big_pert_num) * max_no_impr_basic; //调整最大无改进次数，2倍luby序列
			no_impr = 0;
			big_pert_num++;
		}
    } else if (pertubation_mode == 0 && localsearch_mode == 1) {
        if (vertex_count < 100000 && no_impr > max_no_impr){//如果顶点小于10万且10万次迭代没有改进
			big_pertub_reduction(big_pert_node_num, big_pertub_bms, conflict_weight);
			max_no_impr = luby(2,big_pert_num) * max_no_impr_basic; //调整最大无改进次数，2倍luby序列
			no_impr = 0;
			big_pert_num++;
		}
    } else {
        cout << "invalid pertubation mode" << endl;
        exit(1);
    }
}

void print_best_score(){
    // 将历史最优的合法解恢复到当前图中
    for (auto v : remaining_vertex){
        vertex_color[v] = best_solution[v];
    }
	best_score = compute_score_reduction(); // 计算最终得分
    if (!verify_solution()){//验证解的正确性
        cout << "solution error" << endl;
        exit(0);
    }
	
	cout<<file_name << " " << best_score + remove_score << " " << final_time << " " << seed <<" "<<current_iter<<endl;

    // // ===== push_down 测试汇总（测完可删） =====
    // cerr << "[PD_SUMMARY] " << file_name
    //      << " calls=" << pd_call_count
    //      << " success=" << pd_success_count
    //      << " success_rate=" << (pd_call_count ? (double)pd_success_count / pd_call_count : 0)
    //      << " total_gain=" << pd_total_gain
    //      << " avg_gain_per_success=" << (pd_success_count ? (double)pd_total_gain / pd_success_count : 0)
    //      << " nodes_moved=" << pd_nodes_moved
    //      << " total_time=" << pd_total_time << "s"
    //      << " time_ratio=" << (final_time > 0 ? pd_total_time / final_time : 0)
    //      << endl;
    // // ==========================================
}