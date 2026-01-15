#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <utility>
#include <unistd.h>
#include <algorithm>
#include <cmath>

#include "util.h"

#define COLOR_NUM 1000

#define TABU_TIME 3

using namespace std;

bool remove_clique(long v);
bool color_node(long node, long color);
long compute_score();
bool swap_clique_color();
void color_graph();
void init_color();
long choose_conflict_node();
void swap_two_color(long color1, long color2);
bool find_clique(long v);


vector<vector<long>> adjacency_list;//存储原始的、完整的图结构
vector<vector<long>> temp_adjacency_list;//存储约简后的图结构
vector<vector<short>> color_choice;	//color_choice[u][c] 表示节点 u 的邻居中有多少个节点被染成了颜色 c
vector<long> tabu;

vector<bool> indicator; //在 find_clique 中使用，用于O(1)查询
vector<long> color_indicator; //没用到
vector<long> remove_indicator; //在 find_clique 中使用，标记节点是否被移除
vector<long> candidate_degree;
vector<vector<long>> all_clique;
vector<long> node_score;
vector<long> conf;
vector<long> vertex_color;// vertex_color[v] 是节点 v 的颜色编号
vector<long> color_use_number; //color_use_number[c] 是颜色 c 被使用的节点数量
vector<long> conflict_vertex_in_color; //all the conflict node
vector<long> best_solution;
vector<long> local_opt_solution;

vector<vector<long>> good_node_color; //对于每个节点 v，good_node_color[v] 维护了一个候选颜色列表。这个列表里的颜色通常是“好移动”的目标颜色
vector<vector<short>> good_node_color_index; //在color_node函数中使用，用于O(1)查询，已删除

long colored_vertex_num = 0; //the number of vertex colored of all graph
long max_color = -1;
long edge_conflict = 0;
long layer_edge_conflict; //edge conflict for each layer
long current_iter = 0;
long no_impr = 0;
long big_pert_node_num = 0;

long local_opt_cost;
long best_score;
double final_time;
unsigned long bms_count;
long vertex_count;
long edge_count;
long cost = 0;//当前解的花费
long remove_score;
long remove_num;
long density;

long connect[5000][5000];

clock_t begin_time, best_time;
long max_iter = 500000000000;

long max_no_impr_basic = 100000;
long bms = 90;
long big_pertub_num_k = 10;
long conflict_weight;
long choose_conflict_node_bms = bms;
long remove_conflict_bms = bms;
long pertub_bms = bms;
long big_pertub_bms = bms;
long max_no_impr = max_no_impr_basic;

class Vertex_vec_with_index {
public:
	Vertex_vec_with_index() {}
	Vertex_vec_with_index(vector<long>::size_type max_sz) : vertex_index(max_sz, -1) {}
	void init(vector<long>::size_type max_sz) {
#ifndef NDEBUG
		//cout << "initializing Vertex_vec_with_index" << endl;
		//cout << max_sz << "max_sz" <<endl;
#endif
		vertex_index.clear();
		vertex_index.resize(max_sz, -1);
		vertex_vec.clear();
	}
	void push_back(long v) {
		if (vertex_index[v] != -1) return;
		vertex_index[v] = vertex_vec.size();
		vertex_vec.push_back(v);
	}
	void remove(long v) {
#ifndef NDEBUG
		if (vertex_index[v] == -1){
			cout << "remove error: " << v << " not exist" << endl;
			getchar();
		}
		//cout << "remove func debug " << vertex_index[*vertex_vec.rbegin()] << " " << vertex_vec[vertex_index[v]] << endl;
#endif
		vertex_index[*vertex_vec.rbegin()] = vertex_index[v];
		vertex_vec[vertex_index[v]] = *vertex_vec.rbegin();
		vertex_vec.pop_back();
		vertex_index[v] = -1;
	}
	bool exist(long v) {
		return vertex_index[v] != -1;
	}
	bool empty() {
		return vertex_vec.empty();
	}
	vector<long>::size_type size() {
		return vertex_vec.size();
	}
	vector<long>::size_type index(long v) {
		return vertex_index[v];
	}
	long & operator [] (vector<long>::size_type i) {
		return vertex_vec[i];
	}
	vector<long>::iterator begin() {
		return vertex_vec.begin();
	}
	vector<long>::iterator end() {
		return vertex_vec.end();
	}

private:
	vector<long> vertex_vec;
	vector<long> vertex_index;
};
Vertex_vec_with_index remaining_vertex;
Vertex_vec_with_index working_vertex;
Vertex_vec_with_index conflict_node_queue;
Vertex_vec_with_index valid_node;



long max_size = 0;