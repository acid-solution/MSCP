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
        init_color_degree_desc();
    } else if (mode == 2) {
        init_color_multi_random();
    } else if (mode == 3) {
        init_color_dp_aware();
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

void big_pertub(long big_pert_node_num, long big_pertub_bms, double conflict_weight){
    if(big_pertub_mode == 0 && localsearch_mode == 0){ 
        if (vertex_count < 100000 && no_impr > max_no_impr){//如果顶点小于10万且10万次迭代没有改进
			big_pertub_old(big_pert_node_num, big_pertub_bms, conflict_weight);
			max_no_impr = luby(2,big_pert_num) * max_no_impr_basic; //调整最大无改进次数，2倍luby序列
			no_impr = 0;
			big_pert_num++;
		}
    } else if (big_pertub_mode == 0 && localsearch_mode == 1) {
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

void perturbation(long pertub_bms, double conflict_weight){
    if(pertubation_mode == 0 && localsearch_mode == 0){ 
        perturbation_old(pertub_bms, conflict_weight);
    } else if (pertubation_mode == 0 && localsearch_mode == 1) {
        perturbation_reduction(pertub_bms, conflict_weight);
    } else if (pertubation_mode == 1) {
        chain_perturbation_reduction(pertub_bms, conflict_weight);
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
	long aers_region_samples = aers_enter_count + aers_expand_count;
	double aers_avg_region_size = aers_region_samples > 0
		? (double)aers_total_region_size / aers_region_samples
		: 0.0;
	double aers_overhead_ratio = final_time > 0
		? aers_overhead_time / final_time
		: 0.0;
	cerr << "[AERS_SUMMARY] file=" << file_name
	     << " enter=" << aers_enter_count
	     << " expand=" << aers_expand_count
	     << " exit=" << aers_exit_count
	     << " success=" << aers_success_count
	     << " total_region_iter=" << aers_total_region_iter
	     << " avg_region_size=" << aers_avg_region_size
	     << " aers_no_impr=" << aers_no_impr
	     << " region_no_impr=" << aers_region_no_impr
	     << " region_no_impr_limit=" << aers_region_no_impr_limit
	     << " cooldown_until=" << aers_cooldown_until
	     << " overhead_time=" << aers_overhead_time
	     << " overhead_ratio=" << aers_overhead_ratio
	     << " expand_calls=" << aers_expand_call_count
	     << " scan_edges=" << aers_expand_scan_edges
	     << " expand_added=" << aers_expand_added
	     << " skip_not_remaining=" << aers_expand_skip_not_remaining
	     << " skip_marked=" << aers_expand_skip_marked
	     << " added_good=" << aers_expand_added_good
	     << " added_conflict=" << aers_expand_added_conflict
	     << " choose_samples=" << aers_choose_sample_count
	     << " choose_skip_empty_good=" << aers_choose_skip_empty_good
	     << " choose_skip_locked=" << aers_choose_skip_locked
	     << " remove_samples=" << aers_remove_sample_count
	     << " remove_sample_miss=" << aers_remove_sample_miss
	     << " candidate_added=" << aers_candidate_added
	     << " candidate_skip_marked=" << aers_candidate_skip_marked
	     << " candidate_skip_duplicate=" << aers_candidate_skip_duplicate
	     << " candidate_empty_exit=" << aers_candidate_empty_exit
	     << " boundary_expand_calls=" << aers_boundary_expand_calls
	     << " boundary_expand_added=" << aers_boundary_expand_added
	     << " boundary_expand_scan_edges=" << aers_boundary_expand_scan_edges
	     << " region_no_impr_exit=" << aers_region_no_impr_exit
	     << " good_pool_size=" << aers_region_good_vertices.size()
	     << " conflict_pool_size=" << aers_region_conflict_vertices.size()
	     << " good_pool_samples=" << aers_good_pool_sample_count
	     << " good_pool_stale=" << aers_good_pool_stale_count
	     << " conflict_pool_samples=" << aers_conflict_pool_sample_count
	     << " conflict_pool_stale=" << aers_conflict_pool_stale_count
	     << endl;

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

    // cerr << "[PU_SUMMARY] " << file_name
    //      << " calls=" << pu_call_count
    //      << " success=" << pu_success_count
    //      << " success_rate=" << (pu_call_count ? (double)pu_success_count / pu_call_count : 0)
    //      << " total_gain=" << pu_total_gain
    //      << " avg_gain_per_success=" << (pu_success_count ? (double)pu_total_gain / pu_success_count : 0)
    //      << " nodes_moved=" << pu_nodes_moved
    //      << " total_time=" << pu_total_time << "s"
    //      << endl;
}
