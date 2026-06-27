#include <iostream>
#include <vector>

// 伸展树（Splay Tree）演示实现：
// - 支持：插入、查找、删除
// - 特点：每次访问后，会把相关节点旋转到根，提升后续局部访问效率
class SplayTree {
private:
    struct Node {
        int key;
        Node* left;
        Node* right;
        Node* parent;

        explicit Node(int value) : key(value), left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Node* root = nullptr;

    // 左旋：把 x 的右孩子 y 旋到 x 的位置
    void rotateLeft(Node* x) {
        Node* y = x->right;
        if (!y) return;

        x->right = y->left;
        if (y->left) {
            y->left->parent = x;
        }

        y->parent = x->parent;
        if (!x->parent) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }

        y->left = x;
        x->parent = y;
    }

    // 右旋：把 x 的左孩子 y 旋到 x 的位置
    void rotateRight(Node* x) {
        Node* y = x->left;
        if (!y) return;

        x->left = y->right;
        if (y->right) {
            y->right->parent = x;
        }

        y->parent = x->parent;
        if (!x->parent) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }

        y->right = x;
        x->parent = y;
    }

    // 伸展操作：把 x 一路旋转到根
    // 包含三类经典情况：
    // 1) Zig      : x 的父亲是根，单旋一次
    // 2) Zig-Zig  : x 与父亲同向（LL 或 RR），先旋祖父，再旋父亲
    // 3) Zig-Zag  : x 与父亲反向（LR 或 RL），先旋父亲，再旋祖父
    void splay(Node* x) {
        if (!x) return;

        while (x->parent) {
            Node* p = x->parent;
            Node* g = p->parent;

            // 情况 1：Zig（父亲是根）
            if (!g) {
                if (x == p->left) {
                    rotateRight(p);
                } else {
                    rotateLeft(p);
                }
            }
            // 情况 2：Zig-Zig（同向）
            else if ((x == p->left && p == g->left) || (x == p->right && p == g->right)) {
                if (x == p->left) {
                    rotateRight(g);
                    rotateRight(p);
                } else {
                    rotateLeft(g);
                    rotateLeft(p);
                }
            }
            // 情况 3：Zig-Zag（反向）
            else {
                if (x == p->left) {
                    rotateRight(p);
                    rotateLeft(g);
                } else {
                    rotateLeft(p);
                    rotateRight(g);
                }
            }
        }
    }

    // 标准 BST 查找，返回目标节点；若不存在返回最后访问到的节点（可用于后续 splay）
    Node* findNodeOrLast(int key) const {
        Node* cur = root;
        Node* last = nullptr;
        while (cur) {
            last = cur;
            if (key == cur->key) return cur;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return last;
    }

    Node* subtreeMin(Node* node) const {
        while (node && node->left) {
            node = node->left;
        }
        return node;
    }

    void inorder(Node* node) const {
        if (!node) return;
        inorder(node->left);
        std::cout << node->key << ' ';
        inorder(node->right);
    }

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    ~SplayTree() {
        destroy(root);
    }

    bool empty() const {
        return root == nullptr;
    }

    // 查找 key：
    // - 找到：返回 true，并把该节点伸展到根
    // - 未找到：返回 false，并把最后访问节点伸展到根（有助于后续访问）
    bool search(int key) {
        if (!root) return false;

        Node* node = findNodeOrLast(key);
        splay(node);
        return root && root->key == key;
    }

    // 插入 key（不插入重复值）
    void insert(int key) {
        if (!root) {
            root = new Node(key);
            return;
        }

        Node* node = findNodeOrLast(key);
        splay(node);  // 把最接近目标的位置提到根，便于后续处理

        if (root->key == key) {
            return;  // 已存在，不重复插入
        }

        Node* n = new Node(key);
        if (key < root->key) {
            // 新节点作为根，右子树接原根，左子树接原根的左子树
            n->left = root->left;
            if (n->left) n->left->parent = n;

            n->right = root;
            root->left = nullptr;
            root->parent = n;
            root = n;
        } else {
            n->right = root->right;
            if (n->right) n->right->parent = n;

            n->left = root;
            root->right = nullptr;
            root->parent = n;
            root = n;
        }
    }

    // 删除 key：
    // 1) 先搜索并把 key 旋到根
    // 2) 若根不是 key，说明不存在
    // 3) 否则删除根，并把左右子树重新拼接
    bool remove(int key) {
        if (!search(key)) {
            return false;
        }

        Node* toDelete = root;

        // 只有一侧子树为空，直接提升另一侧
        if (!root->left) {
            root = root->right;
            if (root) root->parent = nullptr;
            delete toDelete;
            return true;
        }
        if (!root->right) {
            root = root->left;
            if (root) root->parent = nullptr;
            delete toDelete;
            return true;
        }

        // 左右子树都存在：
        // - 取左子树的最大值（这里通过左子树先找最右节点）
        // - 将其伸展到左子树根，使其右孩子为空
        // - 再把原右子树接到其右侧
        Node* leftTree = root->left;
        Node* rightTree = root->right;
        leftTree->parent = nullptr;
        rightTree->parent = nullptr;
        delete toDelete;

        root = leftTree;
        Node* maxLeft = root;
        while (maxLeft->right) {
            maxLeft = maxLeft->right;
        }
        splay(maxLeft);  // 现在 maxLeft 成为根，且根无右孩子

        root->right = rightTree;
        rightTree->parent = root;
        return true;
    }

    void printInorder() const {
        inorder(root);
        std::cout << '\n';
    }

    void printRoot() const {
        if (!root) {
            std::cout << "Root: (null)\n";
        } else {
            std::cout << "Root: " << root->key << '\n';
        }
    }
};

int main() {
    SplayTree tree;
    std::vector<int> nums = {10, 20, 30, 40, 50, 25, 5, 1, 8};

    std::cout << "=== Insert ===\n";
    for (int x : nums) {
        tree.insert(x);
        std::cout << "insert " << x << " -> ";
        tree.printRoot();
    }

    std::cout << "\nInorder after insertion: ";
    tree.printInorder();

    std::cout << "\n=== Search ===\n";
    std::cout << "search 25: " << (tree.search(25) ? "found" : "not found") << ", ";
    tree.printRoot();
    std::cout << "search 100: " << (tree.search(100) ? "found" : "not found") << ", ";
    tree.printRoot();

    std::cout << "\n=== Delete ===\n";
    std::vector<int> del = {20, 10, 25};
    for (int x : del) {
        std::cout << "remove " << x << ": " << (tree.remove(x) ? "ok" : "not found") << ", ";
        tree.printRoot();
    }

    std::cout << "\nInorder after deletion: ";
    tree.printInorder();

    return 0;
}
