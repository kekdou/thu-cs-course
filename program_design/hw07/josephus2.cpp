#include <iostream>

using namespace std;

struct Node {
    int index;
    bool live = 1;
    Node* next;
};

int main() {
    int n, k;
    cin >> n >> k;
    Node* head = new Node;
    head->index = 1;
    Node* temp = head;
    for (int i = 2; i <= n; i++) {
        Node* p = new Node;
        p->index = i;
        temp->next = p;
        temp = p;
    }
    temp->next = head;
    Node* cur_node = head;
    int clock = 0;
    int dead = 0;
    do {
        if (cur_node->live) {
            clock++;
        }
        if (clock == k + 1) {
            cur_node->live = 0;
            dead++;
            clock = 0;
        }
        cur_node = cur_node->next;
    } while (dead != n - 1);
    while (!cur_node->live) {
        cur_node = cur_node->next;
    }
    cout << cur_node->index << endl;
    return 0;
}