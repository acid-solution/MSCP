#pragma once
#include "basic.h"
#include <cmath>

static double luby(double y, int x){

    // Find the finite subsequence that contains index 'x', and the
    // size of that subsequence:
    int size, seq;
    for (size = 1, seq = 0; size < x+1; seq++, size = 2*size+1);

    while (size-1 != x){
        size = (size-1)>>1;
        seq--;
        x = x % size;
    }

    return pow(y, seq);
}

bool cmp_by_edgeout(long x, long y){
	return x > y;
}

bool remove_clique(long v){//删除节点 v 和其相关边
	//vector<long> &clique = *all_clique.rbegin();

        for(auto u : temp_adjacency_list[v]){	//删除节点v在temp_adjacency_list中所有邻居节点u与v的连接
			for (vector<long>::size_type i = 0; i < temp_adjacency_list[u].size(); ++i) {
				if (temp_adjacency_list[u][i] == v) {								//找到邻居节点 u 的邻接表中与 v 相连的边
					temp_adjacency_list[u][i] = *temp_adjacency_list[u].rbegin();	//将该边与邻接表的最后一条边交换
					temp_adjacency_list[u].pop_back();								//删除邻接表的最后一条边
					break;
				}
			}
        }
        remaining_vertex.remove(v);//从剩余节点列表中移除节点 v
    return true;
}

bool verify_solution(){
	for (auto v : remaining_vertex){
		for (auto u : temp_adjacency_list[v]){
			if (vertex_color[v] == vertex_color[u]){
				return false;
			}
		}
	}
	return true;
}

