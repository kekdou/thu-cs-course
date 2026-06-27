#include <iostream>
#include <vector>
#include <cstdio>
#include <algorithm>

using namespace std;

const int MAX_NODES = 500005;

struct Node {
    int ch[2];
    int parent;
    int size;
    int val;
    bool is_reverse;
};

Node tree[MAX_NODES];
int root = 0;
int used = 0;
int seq_n = 0;
int head_idx = 0;
int tail_idx = 0;

inline int newNode(int v) {
    used++;
    tree[used].val = v;
    tree[used].size = 1;
    tree[used].ch[0] = tree[used].ch[1] = tree[used].parent = 0;
    tree[used].is_reverse = false;
    return used;
}

inline void pushUp(int x) {
    tree[x].size = tree[tree[x].ch[0]].size + tree[tree[x].ch[1]].size + 1;
}

inline void applyReverse(int x) {
    if (!x) return;
    swap(tree[x].ch[0], tree[x].ch[1]);
    tree[x].is_reverse = !tree[x].is_reverse;
}

inline void pushDown(int x) {
    if (x && tree[x].is_reverse) {
        applyReverse(tree[x].ch[0]);
        applyReverse(tree[x].ch[1]);
        tree[x].is_reverse = false;
    }
}

inline int getChildIndex(int x) {
    return tree[tree[x].parent].ch[1] == x;
}

inline void rotate(int x) {
    int y = tree[x].parent;
    int z = tree[y].parent;
    int k = getChildIndex(x);
    
    tree[y].ch[k] = tree[x].ch[k ^ 1];
    if (tree[x].ch[k ^ 1]) {
        tree[tree[x].ch[k ^ 1]].parent = y;
    }
    
    tree[x].ch[k ^ 1] = y;
    tree[y].parent = x;
    
    tree[x].parent = z;
    if (z) {
        tree[z].ch[tree[z].ch[1] == y] = x;
    }
    
    pushUp(y);
    pushUp(x);
}

inline void splay(int x, int goal) {
    while (tree[x].parent != goal) {
        int y = tree[x].parent;
        int z = tree[y].parent;
        if (z != goal) {
            if (getChildIndex(x) == getChildIndex(y)) {
                rotate(y);
            } else {
                rotate(x);
            }
        }
        rotate(x);
    }
    if (goal == 0) {
        root = x;
    }
}

inline int kth(int k) {
    int x = root;
    while (1) {
        pushDown(x);
        int left_size = tree[tree[x].ch[0]].size;
        if (k <= left_size) {
            x = tree[x].ch[0];
        } else if (k == left_size + 1) {
            return x;
        } else {
            k -= left_size + 1;
            x = tree[x].ch[1];
        }
    }
}

void initSplay(int num_nodes) {
    root = used = seq_n = 0;
    head_idx = newNode(0);
    tail_idx = newNode(0);
    root = head_idx;
    tree[head_idx].ch[1] = tail_idx;
    tree[tail_idx].parent = head_idx;
    pushUp(head_idx);
}

void reverseRange(int l, int r) {
    if (seq_n == 0 || l > r) return;
    int left = kth(l + 1);
    int right = kth(r + 3);
    splay(left, 0);
    splay(right, left);
    int seg = tree[right].ch[0];
    if (seg) {
        applyReverse(seg);
    }
    pushUp(right);
    pushUp(left);
}

void opI(int pos, int value) {
    int left = kth(pos + 1);
    int right = kth(pos + 2);
    splay(left, 0);
    splay(right, left);
    int x = newNode(value);
    tree[right].ch[0] = x;
    tree[x].parent = right;
    pushUp(right);
    pushUp(left);
    seq_n++;
}

void opS(int i, int j) {
    int a = kth(i + 2);
    int b = kth(j + 2);
    swap(tree[a].val, tree[b].val);
}

void opR(int r) {
    if (seq_n <= 1) return;
    r %= seq_n;
    if (r < 0) {
        r += seq_n;
    }
    if (r == 0) return;
    reverseRange(0, seq_n - 1);
    reverseRange(0, r - 1);
    reverseRange(r, seq_n - 1);
}

void opF(int a, int b) {
    if (a <= b) {
        reverseRange(a, b);
    } else {
        int len = seq_n - a + b + 1;
        opR(seq_n - a);
        reverseRange(0, len - 1);
        opR(a);
    }
}

void in_order(int x) {
    if (!x) return;
    pushDown(x);
    in_order(tree[x].ch[0]);
    if (x != head_idx && x != tail_idx) {
        printf("%d ", tree[x].val);
    }
    in_order(tree[x].ch[1]);
}

int main() {
    int n, m;
    if (scanf("%d %d", &n, &m) != 2) return 0;
    
    initSplay(n + 2);
    
    for (int i = 0; i < m; i++) {
        char op;
        scanf(" %c", &op);
        if (op == 'R') {
            int r;
            scanf("%d", &r);
            opR(r);
        } else {
            int a, b;
            scanf("%d %d", &a, &b);
            if (op == 'I') {
                opI(b, a);
            } else if (op == 'S') {
                opS(a, b);
            } else if (op == 'F') {
                opF(a, b);
            }
        }
    }
    in_order(root);
    printf("\n");
    return 0;
}