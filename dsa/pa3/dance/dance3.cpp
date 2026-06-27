#include <cstdio>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

using namespace std;

const int MAX_NODES = 500000 + 5;

struct Node {
    int left;
    int right;
    int sub_size;
    int label;
    uint32_t priority_value;
    bool reverse_tag;
};

Node tree[MAX_NODES];
int root = 0;
int node_count = 0;

uint32_t rng_state = 2463534242u;
uint32_t nextRandom() {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

void pushUp(int node) {
    tree[node].sub_size = tree[tree[node].left].sub_size + tree[tree[node].right].sub_size + 1;
}

void applyReverse(int node) {
    if (!node) {
        return;
    }
    swap(tree[node].left, tree[node].right);
    tree[node].reverse_tag = !tree[node].reverse_tag;
}

 void pushDown(int node) {
    if (!node || !tree[node].reverse_tag) {
        return;
    }
    applyReverse(tree[node].left);
    applyReverse(tree[node].right);
    tree[node].reverse_tag = false;
}

 int newNode(int value) {
    ++node_count;
    tree[node_count].left = 0;
    tree[node_count].right = 0;
    tree[node_count].sub_size = 1;
    tree[node_count].label = value;
    tree[node_count].priority_value = nextRandom();
    tree[node_count].reverse_tag = false;
    return node_count;
}

void split(int node, int k, int &left_tree, int &right_tree) {
    if (!node) {
        left_tree = 0;
        right_tree = 0;
        return;
    }
    pushDown(node);
    int left_size = tree[tree[node].left].sub_size;
    if (k <= left_size) {
        right_tree = node;
        split(tree[node].left, k, left_tree, tree[node].left);
        pushUp(right_tree);
    } else {
        left_tree = node;
        split(tree[node].right, k - left_size - 1, tree[node].right, right_tree);
        pushUp(left_tree);
    }
}

int merge(int left_tree, int right_tree) {
    if (!left_tree || !right_tree) {
        return left_tree ? left_tree : right_tree;
    }
    if (tree[left_tree].priority_value < tree[right_tree].priority_value) {
        pushDown(left_tree);
        tree[left_tree].right = merge(tree[left_tree].right, right_tree);
        pushUp(left_tree);
        return left_tree;
    }
    pushDown(right_tree);
    tree[right_tree].left = merge(left_tree, tree[right_tree].left);
    pushUp(right_tree);
    return right_tree;
}

int kthNode(int node, int k) {
    while (node) {
        pushDown(node);
        int left_size = tree[tree[node].left].sub_size;
        if (k < left_size) {
            node = tree[node].left;
        } else if (k == left_size) {
            return node;
        } else {
            k -= left_size + 1;
            node = tree[node].right;
        }
    }
    return 0;
}

void opI(int label, int pos) {
    int left_tree, right_tree;
    split(root, pos, left_tree, right_tree);
    int node = newNode(label);
    root = merge(merge(left_tree, node), right_tree);
}

void opS(int i, int j) {
    int node_i = kthNode(root, i);
    int node_j = kthNode(root, j);
    int tmp = tree[node_i].label;
    tree[node_i].label = tree[node_j].label;
    tree[node_j].label = tmp;
}

void opR(int r, int current_size) {
    if (current_size <= 1) return;
    r %= current_size;
    if (r < 0) r += current_size;
    if (r == 0) return;

    int left_tree, right_tree;
    split(root, current_size - r, left_tree, right_tree);
    root = merge(right_tree, left_tree);
}

void reverseRange(int left_index, int right_index) {
    if (left_index > right_index) return;
    int left_tree, mid_tree, right_tree;
    split(root, left_index, left_tree, mid_tree);
    split(mid_tree, right_index - left_index + 1, mid_tree, right_tree);
    applyReverse(mid_tree);
    root = merge(merge(left_tree, mid_tree), right_tree);
}

void opF(int i, int j, int current_size) {
    if (i <= j) {
        reverseRange(i, j);
    } else {
        int length = current_size - i + j + 1;
        opR(current_size - i, current_size);
        reverseRange(0, length - 1);
        opR(i, current_size);
    }
}

void inOrder(int node, vector<int> &result) {
    if (!node) {
        return;
    }
    pushDown(node);
    inOrder(tree[node].left, result);
    result.push_back(tree[node].label);
    inOrder(tree[node].right, result);
}

int main() {
    int n, m;
    scanf("%d %d", &n, &m);
    int current_size = 0;
    for (int i = 0; i < m; ++i) {
        char op;
        scanf(" %c", &op);
        if (op == 'I') {
            int x, p;
            scanf("%d %d", &x, &p);
            opI(x, p);
            current_size++;
        } else if (op == 'S') {
            int i_pos, j_pos;
            scanf("%d %d", &i_pos, &j_pos);
            opS(i_pos, j_pos);
        } else if (op == 'R') {
            int r;
            scanf("%d", &r);
            opR(r, current_size);
        } else if (op == 'F') {
            int i_pos, j_pos;
            scanf("%d %d", &i_pos, &j_pos);
            opF(i_pos, j_pos, current_size);
        }
    }
    vector<int> res;
    res.reserve(current_size);
    inOrder(root, res);
    for (auto& r : res) {
        printf("%d ", r);
    }
    return 0;
}