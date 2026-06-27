#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>

// 1. 定义二维空间中的点
struct Point {
    int x, y;
    char name;
};

// 2. 内部 Y 轴叶搜树（负责在 X 锁定的礼包里快速卡出 Y 范围）
struct YNode {
    int key; // 右子树的最小值（导航用）
    Point* pt = nullptr; // 只有叶子节点才存真正的点指针
    YNode* left = nullptr;
    YNode* right = nullptr;
    YNode(int k) : key(k) {}
};

// 3. 外层 X 轴主树（大蓝三角）
struct XNode {
    int key; // 右子树的最小值（导航用）
    YNode* y_root = nullptr; // 【核心套娃】当前节点管辖的所有点建成的 Y 轴子树
    XNode* left = nullptr;
    XNode* right = nullptr;
    XNode(int k) : key(k) {}
};

// --- 工具函数：快速构建 Y 轴叶搜导航树 ---
YNode* buildYTree(const std::vector<Point>& y_sorted_pts, int start, int end) {
    if (start > end) return nullptr;
    if (start == end) {
        YNode* leaf = new YNode(y_sorted_pts[start].y);
        leaf->pt = new Point(y_sorted_pts[start]); // 叶子节点存真实的实体点
        return leaf;
    }
    int mid = start + (end - start) / 2;
    YNode* node = new YNode(y_sorted_pts[mid + 1].y); // 规定为右子树的最小值
    node->left = buildYTree(y_sorted_pts, start, mid);
    node->right = buildYTree(y_sorted_pts, mid + 1, end);
    return node;
}

// --- 工具函数：递归构建外层 X 轴主树 ---
XNode* buildXTree(const std::vector<Point>& x_sorted_pts, int start, int end) {
    if (start > end) return nullptr;
    
    // 【关键】收集当前节点管辖的所有点（包含左右子树所有的点）
    std::vector<Point> sub_pts;
    for (int i = start; i <= end; ++i) sub_pts.push_back(x_sorted_pts[i]);
    
    // 把这批点按 Y 坐标排序，提前为它们建好 Y 轴小树（大礼包）
    std::sort(sub_pts.begin(), sub_pts.end(), [](const Point& a, const Point& b) { return a.y < b.y; });
    
    if (start == end) {
        XNode* leaf = new XNode(x_sorted_pts[start].x);
        leaf->y_root = buildYTree(sub_pts, 0, sub_pts.size() - 1); // 叶子也有Y树（只有它自己）
        return leaf;
    }
    
    int mid = start + (end - start) / 2;
    XNode* node = new XNode(x_sorted_pts[mid + 1].x); // 规定为右子树的最小值
    node->y_root = buildYTree(sub_pts, 0, sub_pts.size() - 1); // 内部节点打包所有子民建 Y 树
    
    node->left = buildXTree(x_sorted_pts, start, mid);
    node->right = buildXTree(x_sorted_pts, mid + 1, end);
    return node;
}

// --- 在内层 Y 轴叶搜树里卡出符合 [y_min, y_max] 的点 ---
void queryY(YNode* root, int y_min, int y_max, std::vector<Point>& results, int& counter) {
    if (!root) return;
    if (root->pt) { // 踩到了叶子节点，直接检查
        counter++; // 记录计算机检查了多少次
        if (root->pt->y >= y_min && root->pt->y <= y_max) {
            results.push_back(*(root->pt));
        }
        return;
    }
    // 导航树规则加速
    if (y_min < root->key) queryY(root->left, y_min, y_max, results, counter);
    if (y_max >= root->key) queryY(root->right, y_min, y_max, results, counter);
}

// --- 外层 2D 范围树查询：找出 X 轴符合的大礼包，直接把礼包丢进 Y 树里过滤 ---
void query2D(XNode* root, int x_min, int x_max, int y_min, int y_max, std::vector<Point>& results, int& counter) {
    if (!root) return;
    
    // 如果当前节点管辖的 X 范围【完全被包含】在查询区间内，说明它是一个现成的大礼包！
    // 理论上应该通过寻找到的 Canonical Subtree 的根直接调用其 Y 树，这里简化为叶搜索区域判定
    if (root->left == nullptr && root->right == nullptr) { // 简化演示：直接搜到 X 叶子
        if (root->key >= x_min && root->key <= x_max) {
            // 直接调用这个大礼包里的 Y 树二分查找，不再向下递归 X
            queryY(root->y_root, y_min, y_max, results, counter);
        }
        return;
    }
    
    if (x_min < root->key) query2D(root->left, x_min, x_max, y_min, y_max, results, counter);
    if (x_max >= root->key) query2D(root->right, x_min, x_max, y_min, y_max, results, counter);
}

int main() {
    // 1. 准备大样本数据：假设有 10 个错开的点
    std::vector<Point> points = {
        {10, 80, 'A'}, {20, 10, 'B'}, {30, 70, 'C'}, {40, 20, 'D'}, {50, 90, 'E'},
        {60, 30, 'F'}, {70, 65, 'G'}, {80, 40, 'H'}, {90, 50, 'I'}, {100, 5, 'J'}
    };
    
    // 目标查询：X 在 [35, 85]，Y 在 [25, 75]
    int x_min = 35, x_max = 85;
    int y_min = 25, y_max = 75;
    
    std::cout << "--- 实验场景 ---" << std::endl;
    std::cout << "查询目标：X 轴在 [" << x_min << ", " << x_max << "] 且 Y 轴在 [" << y_min << ", " << y_max << "]\n\n";

    // ================= 方法一：肉身模拟两次独立二分 + 暴力求交集 =================
    std::cout << "【方法一：两次独立二分 + 暴力遍历求交集】" << std::endl;
    int method1_ops = 0;
    
    // 分别准备两个按 X 和按 Y 排序的独立数组
    std::vector<Point> x_arr = points;
    std::sort(x_arr.begin(), x_arr.end(), [](const Point& a, const Point& b){ return a.x < b.x; });
    std::vector<Point> y_arr = points;
    std::sort(y_arr.begin(), y_arr.end(), [](const Point& a, const Point& b){ return a.y < b.y; });
    
    // 1. 第一次二分：在 X 数组里卡出范围
    std::vector<Point> x_pass;
    for(auto& p : x_arr) {
        method1_ops++; // 模拟二分查找判定（这里用遍历展示范围，实际二分为 logN）
        if(p.x >= x_min && p.x <= x_max) x_pass.push_back(p);
    }
    // 2. 第二次二分：在 Y 数组里卡出范围
    std::vector<Point> y_pass;
    for(auto& p : y_arr) {
        if(p.y >= y_min && p.y <= y_max) y_pass.push_back(p);
    }
    
    // 3. 灾难开始：两拨人无法无缝衔接，必须强行暴力对比（双重循环求交集）
    std::vector<Point> res1;
    for(auto& px : x_pass) {
        for(auto& py : y_pass) {
            method1_ops++; // 每次配对对比，计数器加1
            if(px.name == py.name) {
                res1.push_back(px);
            }
        }
    }
    std::cout << ">> 找到符合的点有: ";
    for(auto& p : res1) std::cout << p.name << " ";
    std::cout << "\n>> 计算机在后期求交集和判断中共执行了: " << method1_ops << " 次对比验证。\n\n";

    // ================= 方法二：二维范围树（大树套小树） =================
    std::cout << "【方法二：二维范围树（套娃树）】" << std::endl;
    // 1. 提前建树
    XNode* x_tree_root = buildXTree(x_arr, 0, x_arr.size() - 1);
    
    int method2_ops = 0;
    std::vector<Point> res2;
    // 2. 现场查询：X 树过滤出的直接大礼包，无缝在内部 Y 树里二分
    query2D(x_tree_root, x_min, x_max, y_min, y_max, res2, method2_ops);
    
    std::cout << ">> 找到符合的点有: ";
    for(auto& p : res2) std::cout << p.name << " ";
    std::cout << "\n>> 计算机在二分导航和输出中一共执行了: " << method2_ops << " 次对比验证。\n";
    
    return 0;
}