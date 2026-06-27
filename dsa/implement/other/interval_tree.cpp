#include <iostream>
#include <vector>
#include <algorithm>

// 定义区间结构
struct Interval {
    int left;
    int right;
    int id; // 用于标识区间
};

// 区间树节点定义
struct IntTreeNode {
    int x_mid;
    std::vector<Interval> L; // 按左端点升序排序的 S_mid
    std::vector<Interval> R; // 按右端点降序排序的 S_mid
    IntTreeNode* lc = nullptr;
    IntTreeNode* rc = nullptr;

    IntTreeNode(int mid) : x_mid(mid) {}
};

class IntervalTree {
public:
    IntTreeNode* root;

    IntervalTree(std::vector<Interval>& intervals) {
        root = buildTree(intervals);
    }

    // 外部查询接口
    void query(int q_x, std::vector<Interval>& result) {
        queryIntTree(root, q_x, result);
    }

private:
    // 递归建树
    IntTreeNode* buildTree(std::vector<Interval>& intervals) {
        if (intervals.empty()) return nullptr;

        // 1. 收集所有端点找中位数
        std::vector<int> endpoints;
        for (const auto& inv : intervals) {
            endpoints.push_back(inv.left);
            endpoints.push_back(inv.right);
        }
        std::sort(endpoints.begin(), endpoints.end());
        int mid_val = endpoints[endpoints.size() / 2];

        IntTreeNode* node = new IntTreeNode(mid_val);

        std::vector<Interval> S_left, S_right, S_mid;

        // 2. 将区间分流
        for (const auto& inv : intervals) {
            if (inv.right < mid_val) {
                S_left.push_back(inv);
            } else if (inv.left > mid_val) {
                S_right.push_back(inv);
            } else {
                S_mid.push_back(inv);
            }
        }

        // 3. 构建当前节点的 L 和 R 列表
        node->L = S_mid;
        std::sort(node->L.begin(), node->L.end(), [](const Interval& a, const Interval& b) {
            return a.left < b.left; // L: 左端点升序
        });

        node->R = S_mid;
        std::sort(node->R.begin(), node->R.end(), [](const Interval& a, const Interval& b) {
            return a.right > b.right; // R: 右端点降序
        });

        // 4. 递归构建左右子树
        node->lc = buildTree(S_left);
        node->rc = buildTree(S_right);

        return node;
    }

    // 核心查询算法（对应图中的伪代码）
    void queryIntTree(IntTreeNode* v, int q_x, std::vector<Interval>& result) {
        if (!v) return; // 递归基

        if (q_x < v->x_mid) {
            // 借助 L 列表，从左向右分捡包含 q_x 的区间
            for (const auto& inv : v->L) {
                if (inv.left <= q_x) {
                    result.push_back(inv);
                } else {
                    break; // 只要左端点大于 q_x，后面的一定不包含，直接截断
                }
            }
            // rc(v) 被剪枝，只递归左子树
            queryIntTree(v->lc, q_x, result);
        } 
        else if (v->x_mid < q_x) {
            // 借助 R 列表，从右向左分捡包含 q_x 的区间
            for (const auto& inv : v->R) {
                if (inv.right >= q_x) {
                    result.push_back(inv);
                } else {
                    break; // 只要右端点小于 q_x，后面的一定不包含，直接截断
                }
            }
            // lc(v) 被剪枝，只递归右子树
            queryIntTree(v->rc, q_x, result);
        } 
        else {
            // q_x == x_mid，S_mid 中的所有区间全部命中
            for (const auto& inv : v->L) {
                result.push_back(inv);
            }
            // 此时无需向下递归
        }
    }
};

int main() {
    // 测试数据：创建一些区间
    std::vector<Interval> intervals = {
        {10, 30, 1},
        {15, 20, 2},
        {17, 19, 3},
        {5,  12, 4},
        {1,  8,  5}
    };

    IntervalTree tree(intervals);
    std::vector<Interval> matched_intervals;
    
    int q_x = 11; // 查询点
    tree.query(q_x, matched_intervals);

    std::cout << "包含点 " << q_x << " 的区间有:\n";
    for (const auto& inv : matched_intervals) {
        std::cout << "区间 " << inv.id << ": [" << inv.left << ", " << inv.right << "]\n";
    }

    return 0;
}