#include <iostream>

using namespace std;

typedef struct Node {
    int value;
    Node* next;
} Node;

void _insert(Node*& head, int x, int value) {
    Node* p = head;
    Node* new_node = new Node;
    new_node->value = value;
    if (x == 0) {
        new_node->next = head;
        head = new_node;
        return;
    }
    for (int i = 1; i < x; i++) {
        p = p->next;
    }
    new_node->next = p->next;
    p->next = new_node;
}

void _delete(Node*& head, int x) {
    Node* p = head;
    if (x == 1) {
        head = p->next;
        delete p;
        return;
    }
    for (int i = 2; i < x; i++) {
        p = p->next;
    }
    p->next = p->next->next;
}

void _print(Node* head) {
    Node* p = head;
    while(p) {
        cout << p->value << " ";
        p = p->next;
    }
    cout << endl;
}

int main() {
    char ch;
    int x, v;
    Node *head = NULL;
    while (true) {
        cin >> ch;
        if (ch == 'N') {
            while (head != NULL)
                _delete(head, 1);
        } else if (ch == 'I') {
            cin >> x >> v;
            Node *node = head;
            _insert(head, x, v);
        } else if (ch == 'D') {
            cin >> x;
            _delete(head, x);
        } else if (ch == 'P')
            _print(head);
        else if (ch == 'E')
            break;
    }
}