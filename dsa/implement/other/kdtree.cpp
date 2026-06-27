#include <iostream>
#include <vector>
#include <algorithm>

// 1. 定义二维空间中的点
struct Point {
    int x, y;
    char name;
};

// 2. KD 树的节点结构（普通的二叉树）
struct KDNode {
    Point pt;            // 每一个节点都代表空间里真实存在的一个点
    int depth;           // 记录当前节点处于第几层，用来决定用 X 切还是 Y 切
    KDNode* left = nullptr;
    KDNode* right = nullptr;
    
    KDNode(Point p, int d) : pt(p), depth(d) {}
};

class KDTree {
public:
    // --- 核心一：建树（自顶向下切西瓜） ---
    KDNode* build(std::vector<Point>& pts, int start, int end, int depth) {
        if (start > end) return nullptr;
        
        // 关键规律：偶数层看 X 轴(0)，奇数层看 Y 轴(1)
        int axis = depth % 2; 
        
        // 找到当前区域里，按指定轴排序后的“中位数（正中间的点）”
        int mid = start + (end - start) / 2;
        std::nth_element(pts.begin() + start, pts.begin() + mid, pts.begin() + end + 1, 
            [axis](const Point& a, const Point& b) {
                return (axis == 0) ? (a.x < b.x) : (a.y < b.y);
            }
        );
        
        // 咔嚓！这个中间点成为当前子树的根节点（切线上的钉子）
        KDNode* node = new KDNode(pts[mid], depth);
        
        // 递归去切左半边（或下半边）和右半边（或上半边）
        node->left = build(pts, start, mid - 1, depth + 1);
        node->right = build(pts, mid + 1, end, depth + 1);
        
        return node;
    }

    // --- 核心二：查询（范围搜索 + 智能剪枝） ---
    void searchRange(KDNode* root, int x_min, int x_max, int y_min, int y_max, 
                     std::vector<Point>& results, int& check_counter) {
        if (!root) return;
        
        check_counter++; // 记录计算机一共检查了多少个点
        
        // 1. 检查当前节点（钉子本身）是否在查询矩形内
        if (root->pt.x >= x_min && root->pt.x <= x_max &&
            root->pt.y >= y_min && root->pt.y <= y_max) {
            results.push_back(root->pt);
        }
        
        // 2. 判断当前切线和查询矩形的位置关系，决定如何剪枝
        int axis = root->depth % 2;
        
        if (axis == 0) { // 当前是 X 轴切线（垂直线）
            // 如果查询矩形有一部分在切线左边，我们就必须进左子树
            if (x_min <= root->pt.x) {
                searchRange(root->left, x_min, x_max, y_min, y_max, results, check_counter);
            }
            // 如果查询矩形有一部分在切线右边，我们就必须进右子树
            if (x_max >= root->pt.x) {
                searchRange(root->right, x_min, x_max, y_min, y_max, results, check_counter);
            }
            // 💡 隐含的剪枝：如果 x_min > root->pt.x（矩形全在右边），
            // 上面第一个 if 就会失败，左子树整棵被无视，成功完成剪枝！
            
        } else { // 当前是 Y 轴切线（水平线）
            // 如果查询矩形有一部分在切线下边，进左子树
            if (y_min <= root->pt.y) {
                searchRange(root->left, x_min, x_max, y_min, y_max, results, check_counter);
            }
            // 如果查询矩形有一部分在切线上边，进右子树
            if (y_max >= root->pt.y) {
                searchRange(root->right, x_min, x_max, y_min, y_max, results, check_counter);
            }
        }
    }
};

int main() {
    // 准备和之前一模一样的 10 个空间点
    std::vector<Point> points = {
        {10, 80, 'A'}, {20, 10, 'B'}, {30, 70, 'C'}, {40, 20, 'D'}, {50, 90, 'E'},
        {60, 30, 'F'}, {70, 65, 'G'}, {80, 40, 'H'}, {90, 50, 'I'}, {100, 5, 'J'}
    };
    
    // 目标查询矩形：X 在 [35, 85]，Y 在 [25, 75]
    int x_min = 35, x_max = 85;
    int y_min = 25, y_max = 75;
    
    std::cout << "=== KD树（2D-Tree）实验场景 ===" << std::endl;
    std::cout << "查询矩形范围：X[" << x_min << ", " << x_max << "], Y[" << y_min << ", " << y_max << "]\n\n";

    KDTree tree;
    // 从第 0 层开始建树
    KDNode* root = tree.build(points, 0, points.size() - 1, 0);
    std::cout << ">> KD树构建成功！(总节点数: " << points.size() << ", 内存占用极小)\n";

    // 现场查询
    std::vector<Point> results;
    int check_counter = 0;
    tree.searchRange(root, x_min, x_max, y_min, y_max, results, check_counter);

    // 输出结果
    std::cout << ">> 找到矩形内的点有: ";
    for (const auto& p : results) std::cout << p.name << " ";
    
    std::cout << "\n>> 整个搜索过程中，计算机仅仅敲门检查了 " << check_counter << " 个节点。\n";
    std::cout << ">> 其余不沾边的空间分支在半路全部被【剪枝】淘汰了！\n";

    return 0;
}