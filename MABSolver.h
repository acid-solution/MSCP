#pragma once
#include <vector>
#include <cmath>
#include <iostream>
#include <limits>

class MABSolver {
public:
    // 构造函数：传入我们要尝试的 alpha 候选值列表
    MABSolver(std::vector<double> candidate_alphas,double C = 1.0) {
        this->alphas = candidate_alphas;
        this->C = C;    
        int k = alphas.size();
        
        // 初始化统计数据
        counts.resize(k, 0);       // 每个臂被拉了多少次
        values.resize(k, 0.0);     // 每个臂的平均奖励 (Q值)
        total_counts = 0;          // 总共拉了多少次
    }

    // 核心功能1：根据 UCB1 算法选择下一个要使用的臂的索引
    int select_arm() {
        // 1. 如果有哪个臂一次都没试过，优先试一下（初始化）
        for (size_t i = 0; i < alphas.size(); ++i) {
            if (counts[i] == 0) { 
                return i;
            }
        }

        // 2. 计算每个臂的 UCB 值，选最大的
        int best_arm = 0;
        double max_ucb = -std::numeric_limits<double>::max();

        for (size_t i = 0; i < alphas.size(); ++i) {

            double n_i = static_cast<double>(counts[i]);
            double total_n = static_cast<double>(total_counts);
            // UCB 公式: Q(a) + C * sqrt(2 * ln(Total) / N(a))
            // C 是探索系数，通常取 1.0 或 2.0，这里取 0.5 到 1.0 之间调节
            double exploration = sqrt(2.0 * log(total_n) / n_i);
            double ucb_value = values[i] + C * exploration;

            if (ucb_value > max_ucb) {
                max_ucb = ucb_value;
                best_arm = i;
            }
        }
        return best_arm;
    }

    // 核心功能2：更新奖励
    // arm_index: 刚才选了哪个臂
    // reward: 那个臂表现得怎么样（例如：cost 降低了多少）
    void update(int arm_index, double reward) {
        
        if (arm_index < 0 || (size_t)arm_index >= alphas.size()) {
            std::cout<<"Invalid arm index: "<<arm_index<<std::endl;
            return;// 无效索引
        }        

        counts[arm_index]++;
        total_counts++;

        // 更新平均奖励 (增量更新公式)
        // NewValue = OldValue + (Reward - OldValue) / N
        double n = counts[arm_index];//该臂被选择的次数
        double old_value = values[arm_index];//该臂的旧平均奖励
        values[arm_index] = old_value + (reward - old_value) / n;//旧平均奖励 + (新奖励 - 旧平均奖励) / 该臂被选择的次数
    }

    // 获取当前选中的 alpha 值
    double get_alpha(int arm_index) {
        if (arm_index < 0 || (size_t)arm_index >= alphas.size()) return alphas[0];
        return alphas[arm_index];
    }

    // 调试用：打印当前状态
    void print_status() {
        std::cout << "MAB Status [Alpha | Count | AvgReward]: ";
        for(size_t i=0; i<alphas.size(); ++i){
            std::cout << "[" << alphas[i] << " | " << counts[i] << " | " << values[i] << "] ";
        }
        std::cout << std::endl;
    }

private:
    std::vector<double> alphas; // 候选参数集合
    std::vector<long> counts;    // 每个参数被使用的次数 N(a)
    std::vector<double> values; // 每个参数的平均奖励 Q(a)
    long total_counts;           // 总选择次数
    double C;                    // 探索系数
};