#include <iostream>
#include <algorithm>

using namespace std;

struct Node {
    int val;
    int npl;
    Node* left;
    Node* right;
    Node() = default;
    Node(int v): val(v), npl(0), left(nullptr), right(nullptr) {}
};

Node* merge(Node* h1, Node* h2) {
    if (!h1) return h2;
    if (!h2) return h1;
    Node* min_heap = (h1->val < h2->val) ? h1 : h2;
    Node* max_heap = (h1->val < h2->val) ? h2 : h1;
    min_heap->right = merge(min_heap->right, max_heap);
    int left = (min_heap->left) ? min_heap->left->npl : -1;
    int right = min_heap->right->npl;
    if (left < right) {
        Node* tmp = min_heap->left;
        min_heap->left = min_heap->right;
        min_heap->right = tmp;
    }
    min_heap->npl = min(left, right) + 1;
    return min_heap;
}
