#include <iostream>
#include <vector>

// 红黑树（Red-Black Tree）演示实现：
// - 本质是“带颜色约束的二叉搜索树”
// - 保证树高为 O(log n)，查找/插入/删除都能稳定在对数复杂度
class RedBlackTree {
private:
    enum Color { RED, BLACK };

    struct Node {
        int key;
        Color color;
        Node* left;
        Node* right;
        Node* parent;

        explicit Node(int k)
            : key(k), color(RED), left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Node* root;
    Node* NIL;  // 哨兵空节点（统一把所有空孩子看成这个黑色节点）

    // 左旋：把 x 的右孩子 y 旋到 x 的位置
    void leftRotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != NIL) {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == NIL) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    // 右旋：把 y 的左孩子 x 旋到 y 的位置
    void rightRotate(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        if (x->right != NIL) {
            x->right->parent = y;
        }
        x->parent = y->parent;
        if (y->parent == NIL) {
            root = x;
        } else if (y == y->parent->right) {
            y->parent->right = x;
        } else {
            y->parent->left = x;
        }
        x->right = y;
        y->parent = x;
    }

    // 插入修复：
    // 新节点默认红色，若破坏“红节点不能有红孩子”则通过变色+旋转修正
    void insertFixup(Node* z) {
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                Node* y = z->parent->parent->right;  // 叔叔节点
                if (y->color == RED) {
                    // Case 1：父红叔红 -> 父叔变黑，祖父变红，继续向上修
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        // Case 2：先转成直线形
                        z = z->parent;
                        leftRotate(z);
                    }
                    // Case 3：直线形后，旋转+变色
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            } else {
                Node* y = z->parent->parent->left;  // 叔叔节点（镜像情况）
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        // 红黑树性质要求根必须为黑
        root->color = BLACK;
    }

    // 用子树 v 替换子树 u（不改变 v 的左右孩子结构）
    void transplant(Node* u, Node* v) {
        if (u->parent == NIL) {
            root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }

    // 返回以 x 为根的最小节点（一路向左）
    Node* minimum(Node* x) const {
        while (x->left != NIL) {
            x = x->left;
        }
        return x;
    }

    // 删除修复：
    // 删除黑节点可能导致“黑高不平衡”，通过兄弟节点相关的 4 类情形修复
    void deleteFixup(Node* x) {
        while (x != root && x->color == BLACK) {
            if (x == x->parent->left) {
                Node* w = x->parent->right;  // 兄弟
                if (w->color == RED) {
                    // Case 1：兄弟红 -> 先旋转转成兄弟黑的情况
                    w->color = BLACK;
                    x->parent->color = RED;
                    leftRotate(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    // Case 2：兄弟黑且两个侄子都黑 -> 兄弟染红，问题上移
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->right->color == BLACK) {
                        // Case 3：兄弟黑，近侄红远侄黑 -> 先转成 Case 4
                        w->left->color = BLACK;
                        w->color = RED;
                        rightRotate(w);
                        w = x->parent->right;
                    }
                    // Case 4：兄弟黑且远侄红 -> 旋转并重新着色后结束
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    leftRotate(x->parent);
                    x = root;
                }
            } else {
                Node* w = x->parent->left;  // 兄弟
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rightRotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->left->color == BLACK) {
                        w->right->color = BLACK;
                        w->color = RED;
                        leftRotate(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = BLACK;
    }

    // 内部查找：返回节点指针，找不到返回 NIL
    Node* searchNode(int key) const {
        Node* cur = root;
        while (cur != NIL) {
            if (key == cur->key) {
                return cur;
            }
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return NIL;
    }

    // 中序遍历（有序输出）
    void inorder(Node* node) const {
        if (node == NIL) {
            return;
        }
        inorder(node->left);
        std::cout << node->key << (node->color == RED ? "(R) " : "(B) ");
        inorder(node->right);
    }

    // 释放整棵树
    void destroy(Node* node) {
        if (node == NIL) {
            return;
        }
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    RedBlackTree() {
        // 构建唯一哨兵 NIL，它既是空叶子，也是根的父亲
        NIL = new Node(0);
        NIL->color = BLACK;
        NIL->left = NIL->right = NIL->parent = NIL;
        root = NIL;
    }

    ~RedBlackTree() {
        destroy(root);
        delete NIL;
    }

    bool search(int key) const {
        return searchNode(key) != NIL;
    }

    // 插入（不允许重复）
    void insert(int key) {
        Node* z = new Node(key);
        z->left = z->right = NIL;

        Node* y = NIL;
        Node* x = root;
        while (x != NIL) {
            y = x;
            if (z->key < x->key) {
                x = x->left;
            } else if (z->key > x->key) {
                x = x->right;
            } else {
                // 演示实现里：遇到重复值直接忽略
                delete z;
                return;
            }
        }

        z->parent = y;
        if (y == NIL) {
            root = z;
        } else if (z->key < y->key) {
            y->left = z;
        } else {
            y->right = z;
        }

        z->color = RED;
        insertFixup(z);
    }

    // 删除 key，成功返回 true，未找到返回 false
    bool remove(int key) {
        Node* z = searchNode(key);
        if (z == NIL) {
            return false;
        }

        Node* y = z;
        Color yOriginalColor = y->color;
        Node* x = NIL;

        if (z->left == NIL) {
            // 只有右子树（或无孩子）
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == NIL) {
            // 只有左子树
            x = z->left;
            transplant(z, z->left);
        } else {
            // 两个孩子都在：用后继节点（右子树最小）替换 z
            y = minimum(z->right);
            yOriginalColor = y->color;
            x = y->right;
            if (y->parent == z) {
                x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        delete z;

        if (yOriginalColor == BLACK) {
            // 仅当删除了黑节点才需要修复
            deleteFixup(x);
        }
        return true;
    }

    void printInorder() const {
        inorder(root);
        std::cout << '\n';
    }
};

int main() {
    RedBlackTree tree;

    // 1) 插入演示
    std::vector<int> nums = {41, 38, 31, 12, 19, 8, 25, 50, 60, 55};
    std::cout << "=== Insert ===\n";
    for (int x : nums) {
        tree.insert(x);
        std::cout << "insert " << x << " -> ";
        tree.printInorder();
    }

    // 2) 查找演示
    std::cout << "\n=== Search ===\n";
    std::cout << "Search 19: " << (tree.search(19) ? "found" : "not found") << '\n';
    std::cout << "Search 100: " << (tree.search(100) ? "found" : "not found") << '\n';

    // 3) 删除演示
    std::cout << "\n=== Delete ===\n";
    tree.remove(31);
    std::cout << "remove 31 -> ";
    tree.printInorder();
    tree.remove(41);
    std::cout << "remove 41 -> ";
    tree.printInorder();
    tree.remove(8);
    std::cout << "remove 8  -> ";
    tree.printInorder();

    std::cout << "\nFinal inorder:\n";
    tree.printInorder();

    return 0;
}
