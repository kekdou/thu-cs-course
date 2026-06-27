#include <ctime>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

// 跳跃表（Skip List）演示实现：
// - 由多层“有序链表”组成
// - 高层用于快速跳跃，底层保存完整有序序列
// - 平均时间复杂度：查找/插入/删除都是 O(log n)
class SkipList {
private:
    struct Node {
        int key;
        std::vector<Node*> forward;  // forward[i] 指向第 i 层的后继节点

        Node(int value, int level) : key(value), forward(level + 1, nullptr) {}
    };

    int maxLevel;          // 最大层数（例如 16）
    double probability;    // 升层概率（常用 0.5）
    int currentLevel;      // 当前跳表最高有效层
    Node* head;            // 头节点（哨兵，不存有效数据）

    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;

    // 随机层高生成：每次“抛硬币”成功则升一层
    int randomLevel() {
        int level = 0;
        while (dist(rng) < probability && level < maxLevel) {
            ++level;
        }
        return level;
    }

public:
    explicit SkipList(int maxLvl = 16, double p = 0.5)
        : maxLevel(maxLvl),
          probability(p),
          currentLevel(0),
          head(new Node(std::numeric_limits<int>::min(), maxLvl)),
          rng(static_cast<unsigned>(std::time(nullptr))),
          dist(0.0, 1.0) {}

    ~SkipList() {
        Node* cur = head;
        while (cur) {
            Node* next = cur->forward[0];
            delete cur;
            cur = next;
        }
    }

    // 查找 key 是否存在
    bool search(int key) const {
        Node* cur = head;

        // 从最高层往下“跳”
        for (int level = currentLevel; level >= 0; --level) {
            while (cur->forward[level] && cur->forward[level]->key < key) {
                cur = cur->forward[level];
            }
        }

        // 到达第 0 层后，检查下一个节点是否命中
        cur = cur->forward[0];
        return cur && cur->key == key;
    }

    // 插入 key（不插入重复值）
    void insert(int key) {
        std::vector<Node*> update(maxLevel + 1, nullptr);
        Node* cur = head;

        // 先记录每一层里“插入位置前驱”
        for (int level = currentLevel; level >= 0; --level) {
            while (cur->forward[level] && cur->forward[level]->key < key) {
                cur = cur->forward[level];
            }
            update[level] = cur;
        }

        cur = cur->forward[0];

        // 已存在则忽略
        if (cur && cur->key == key) {
            return;
        }

        int newLevel = randomLevel();
        if (newLevel > currentLevel) {
            // 新节点层高超过当前最高层时，新增层的前驱都设为 head
            for (int level = currentLevel + 1; level <= newLevel; ++level) {
                update[level] = head;
            }
            currentLevel = newLevel;
        }

        Node* node = new Node(key, newLevel);

        // 在每一层做“链表插入”
        for (int level = 0; level <= newLevel; ++level) {
            node->forward[level] = update[level]->forward[level];
            update[level]->forward[level] = node;
        }
    }

    // 删除 key，删除成功返回 true
    bool remove(int key) {
        std::vector<Node*> update(maxLevel + 1, nullptr);
        Node* cur = head;

        // 同样先定位每层前驱
        for (int level = currentLevel; level >= 0; --level) {
            while (cur->forward[level] && cur->forward[level]->key < key) {
                cur = cur->forward[level];
            }
            update[level] = cur;
        }

        cur = cur->forward[0];
        if (!cur || cur->key != key) {
            return false;
        }

        // 在节点出现的每一层做“链表删除”
        for (int level = 0; level <= currentLevel; ++level) {
            if (update[level]->forward[level] != cur) {
                break;
            }
            update[level]->forward[level] = cur->forward[level];
        }

        delete cur;

        // 若最高层已空，降低 currentLevel
        while (currentLevel > 0 && head->forward[currentLevel] == nullptr) {
            --currentLevel;
        }
        return true;
    }

    // 打印第 0 层（完整有序序列）
    void printLevel0() const {
        Node* cur = head->forward[0];
        while (cur) {
            std::cout << cur->key << ' ';
            cur = cur->forward[0];
        }
        std::cout << '\n';
    }

    // 打印所有层，便于观察结构
    void printAllLevels() const {
        for (int level = currentLevel; level >= 0; --level) {
            std::cout << "Level " << level << ": ";
            Node* cur = head->forward[level];
            while (cur) {
                std::cout << cur->key << ' ';
                cur = cur->forward[level];
            }
            std::cout << '\n';
        }
    }
};

int main() {
    SkipList skiplist(8, 0.5);  // 演示时层数不必太大

    std::vector<int> nums = {10, 20, 15, 30, 25, 5, 35, 40, 1};

    std::cout << "=== Insert ===\n";
    for (int x : nums) {
        skiplist.insert(x);
        std::cout << "insert " << x << " -> ";
        skiplist.printLevel0();
    }

    std::cout << "\nAll levels after insertion:\n";
    skiplist.printAllLevels();

    std::cout << "\n=== Search ===\n";
    std::cout << "search 25: " << (skiplist.search(25) ? "found" : "not found") << '\n';
    std::cout << "search 99: " << (skiplist.search(99) ? "found" : "not found") << '\n';

    std::cout << "\n=== Delete ===\n";
    std::vector<int> del = {15, 1, 30, 99};
    for (int x : del) {
        std::cout << "remove " << x << ": " << (skiplist.remove(x) ? "ok" : "not found") << " -> ";
        skiplist.printLevel0();
    }

    std::cout << "\nAll levels after deletion:\n";
    skiplist.printAllLevels();

    return 0;
}
