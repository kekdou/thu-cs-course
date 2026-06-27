#include <iostream>
#include <vector>

// B-Tree（B树）演示实现（面向初学者）：
// - 这里采用“最小度数 t”定义
// - 每个节点最多 2t-1 个键，最少 t-1 个键（根节点可例外）
// - 所有叶子在同一层，适合磁盘/分页场景
class BTree {
private:
    struct Node {
        bool leaf;
        std::vector<int> keys;
        std::vector<Node*> children;

        explicit Node(bool isLeaf) : leaf(isLeaf) {}
    };

    Node* root = nullptr;
    int t;  // 最小度数（建议 >= 2）

    void destroy(Node* x) {
        if (!x) return;
        for (Node* c : x->children) destroy(c);
        delete x;
    }

    // 在子节点已满时，分裂 parent->children[i]
    void splitChild(Node* parent, int i) {
        Node* y = parent->children[i];
        Node* z = new Node(y->leaf);

        int midKey = y->keys[t - 1];

        // z 接收 y 的后半部分 key
        for (int j = t; j < (int)y->keys.size(); ++j) z->keys.push_back(y->keys[j]);
        y->keys.resize(t - 1);

        // 若非叶子，还要切分 children
        if (!y->leaf) {
            for (int j = t; j < (int)y->children.size(); ++j) z->children.push_back(y->children[j]);
            y->children.resize(t);
        }

        parent->children.insert(parent->children.begin() + i + 1, z);
        parent->keys.insert(parent->keys.begin() + i, midKey);
    }

    // 在“保证 x 未满”的前提下插入 key
    void insertNonFull(Node* x, int key) {
        int i = (int)x->keys.size() - 1;

        if (x->leaf) {
            x->keys.push_back(0);  // 先扩容一个位置
            while (i >= 0 && key < x->keys[i]) {
                x->keys[i + 1] = x->keys[i];
                --i;
            }
            x->keys[i + 1] = key;
            return;
        }

        while (i >= 0 && key < x->keys[i]) --i;
        ++i;

        // 若目标孩子满了，先分裂，再决定走左还是右
        if ((int)x->children[i]->keys.size() == 2 * t - 1) {
            splitChild(x, i);
            if (key > x->keys[i]) ++i;
            else if (key == x->keys[i]) return;  // 防止重复
        }
        insertNonFull(x->children[i], key);
    }

    bool searchNode(Node* x, int key) const {
        if (!x) return false;
        int i = 0;
        while (i < (int)x->keys.size() && key > x->keys[i]) ++i;
        if (i < (int)x->keys.size() && key == x->keys[i]) return true;
        if (x->leaf) return false;
        return searchNode(x->children[i], key);
    }

    int findKey(Node* x, int key) const {
        int idx = 0;
        while (idx < (int)x->keys.size() && x->keys[idx] < key) ++idx;
        return idx;
    }

    int getPred(Node* x, int idx) const {
        Node* cur = x->children[idx];
        while (!cur->leaf) cur = cur->children.back();
        return cur->keys.back();
    }

    int getSucc(Node* x, int idx) const {
        Node* cur = x->children[idx + 1];
        while (!cur->leaf) cur = cur->children.front();
        return cur->keys.front();
    }

    void borrowFromPrev(Node* x, int idx) {
        Node* child = x->children[idx];
        Node* sibling = x->children[idx - 1];

        child->keys.insert(child->keys.begin(), x->keys[idx - 1]);
        if (!child->leaf) {
            child->children.insert(child->children.begin(), sibling->children.back());
            sibling->children.pop_back();
        }

        x->keys[idx - 1] = sibling->keys.back();
        sibling->keys.pop_back();
    }

    void borrowFromNext(Node* x, int idx) {
        Node* child = x->children[idx];
        Node* sibling = x->children[idx + 1];

        child->keys.push_back(x->keys[idx]);
        if (!child->leaf) {
            child->children.push_back(sibling->children.front());
            sibling->children.erase(sibling->children.begin());
        }

        x->keys[idx] = sibling->keys.front();
        sibling->keys.erase(sibling->keys.begin());
    }

    // 合并 children[idx] + key[idx] + children[idx+1] 到左孩子
    void merge(Node* x, int idx) {
        Node* left = x->children[idx];
        Node* right = x->children[idx + 1];

        left->keys.push_back(x->keys[idx]);
        for (int k : right->keys) left->keys.push_back(k);

        if (!left->leaf) {
            for (Node* c : right->children) left->children.push_back(c);
        }

        x->keys.erase(x->keys.begin() + idx);
        x->children.erase(x->children.begin() + idx + 1);
        delete right;
    }

    // 确保 children[idx] 至少有 t 个键（递归下降前调用）
    void fill(Node* x, int idx) {
        if (idx > 0 && (int)x->children[idx - 1]->keys.size() >= t) {
            borrowFromPrev(x, idx);
        } else if (idx < (int)x->children.size() - 1 && (int)x->children[idx + 1]->keys.size() >= t) {
            borrowFromNext(x, idx);
        } else {
            if (idx < (int)x->children.size() - 1) merge(x, idx);
            else merge(x, idx - 1);
        }
    }

    void removeFromNonLeaf(Node* x, int idx) {
        int key = x->keys[idx];

        // 情况1：左孩子够“胖”，用前驱替代
        if ((int)x->children[idx]->keys.size() >= t) {
            int pred = getPred(x, idx);
            x->keys[idx] = pred;
            removeNode(x->children[idx], pred);
        }
        // 情况2：右孩子够“胖”，用后继替代
        else if ((int)x->children[idx + 1]->keys.size() >= t) {
            int succ = getSucc(x, idx);
            x->keys[idx] = succ;
            removeNode(x->children[idx + 1], succ);
        }
        // 情况3：左右都瘦，先合并再删
        else {
            merge(x, idx);
            removeNode(x->children[idx], key);
        }
    }

    void removeNode(Node* x, int key) {
        int idx = findKey(x, key);

        // key 在当前节点
        if (idx < (int)x->keys.size() && x->keys[idx] == key) {
            if (x->leaf) {
                x->keys.erase(x->keys.begin() + idx);
            } else {
                removeFromNonLeaf(x, idx);
            }
            return;
        }

        // key 不在当前节点，且当前节点是叶子：不存在
        if (x->leaf) return;

        bool atLastChild = (idx == (int)x->keys.size());

        // 递归前保证目标孩子不少于 t 个键
        if ((int)x->children[idx]->keys.size() < t) fill(x, idx);

        // fill 可能触发合并，导致 children 数组变化
        if (atLastChild && idx > (int)x->keys.size()) removeNode(x->children[idx - 1], key);
        else removeNode(x->children[idx], key);
    }

    void printInorder(Node* x) const {
        if (!x) return;
        for (int i = 0; i < (int)x->keys.size(); ++i) {
            if (!x->leaf) printInorder(x->children[i]);
            std::cout << x->keys[i] << ' ';
        }
        if (!x->leaf) printInorder(x->children.back());
    }

    void printLevels(Node* x, int depth) const {
        if (!x) return;
        std::cout << "Depth " << depth << ": ";
        for (int k : x->keys) std::cout << k << ' ';
        std::cout << '\n';
        for (Node* c : x->children) printLevels(c, depth + 1);
    }

public:
    explicit BTree(int degree = 3) : t(degree < 2 ? 2 : degree) {
        root = new Node(true);
    }

    ~BTree() { destroy(root); }

    bool search(int key) const { return searchNode(root, key); }

    // 插入（忽略重复值）
    void insert(int key) {
        if (search(key)) return;

        // 根满了，先长高一层再分裂旧根
        if ((int)root->keys.size() == 2 * t - 1) {
            Node* s = new Node(false);
            s->children.push_back(root);
            splitChild(s, 0);
            root = s;
        }
        insertNonFull(root, key);
    }

    // 删除 key，存在则删除并返回 true
    bool remove(int key) {
        if (!search(key)) return false;

        removeNode(root, key);

        // 若根被删空且不是叶子，把高度降一层
        if (root->keys.empty() && !root->leaf) {
            Node* oldRoot = root;
            root = root->children[0];
            delete oldRoot;
        }
        return true;
    }

    void printInorder() const {
        printInorder(root);
        std::cout << '\n';
    }

    void printStructure() const {
        printLevels(root, 0);
    }
};

int main() {
    BTree tree(3);  // t=3：每个节点 key 数范围通常为 [2, 5]

    std::vector<int> nums = {10, 20, 5, 6, 12, 30, 7, 17, 3, 25, 40, 50, 1, 8};

    std::cout << "=== Insert ===\n";
    for (int x : nums) {
        tree.insert(x);
        std::cout << "insert " << x << " -> ";
        tree.printInorder();
    }

    std::cout << "\nStructure after insertion:\n";
    tree.printStructure();

    std::cout << "\n=== Search ===\n";
    std::cout << "search 17: " << (tree.search(17) ? "found" : "not found") << '\n';
    std::cout << "search 99: " << (tree.search(99) ? "found" : "not found") << '\n';

    std::cout << "\n=== Delete ===\n";
    std::vector<int> del = {6, 20, 10, 3, 99};
    for (int x : del) {
        std::cout << "remove " << x << ": " << (tree.remove(x) ? "ok" : "not found") << " -> ";
        tree.printInorder();
    }

    std::cout << "\nStructure after deletion:\n";
    tree.printStructure();

    return 0;
}
