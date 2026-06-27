#include <iostream>

#define ll long long

using namespace std;

struct Process {
    int id;
    int priority;
    int cre_time;
    int run_time;
    Process() = default;
    Process(int i, int p, int c, int r): id(i), priority(p), cre_time(c), run_time(r) {}
};

Process heap[1000001];
int heap_size = 0;

void swap(Process& p1, Process& p2) {
    Process tmp = p1;
    p1 = p2;
    p2 = tmp;
}

bool compare(const Process p1, const Process p2) {
    if (p1.priority != p2.priority) return p1.priority > p2.priority;
    if (p1.cre_time != p2.cre_time) return p1.cre_time < p2.cre_time;
    return p1.id < p2.id;
}

void siftUp(int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (compare(heap[i], heap[parent])) {
            swap(heap[i], heap[parent]);
            i = parent;
        } else {
            break;
        }
    }
}

void siftDown(int i) {
    while (2 * i + 1 < heap_size) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int best = i;
        if (left <= heap_size && compare(heap[left], heap[best])) best = left;
        if (right <= heap_size && compare(heap[right], heap[best])) best = right;
        if (best != i) {
            swap(heap[i], heap[best]);
            i = best;
        } else {
            break;
        }

    }
}

void push(Process& p) {
    heap[heap_size] = p;
    siftUp(heap_size);
    heap_size++;
}

void pop() {
    if (heap_size == 0) return;
    heap_size--;
    heap[0] = heap[heap_size];
    siftDown(0);
}

Process top() {
    return heap[0];
}

bool empty() {
    return heap_size == 0;
}

int main() {
    int n;
    scanf("%d", &n);
    ll cur_time = 0;
    for (int i = 0; i < n; i++) {
        Process next_p;
        scanf(" %d %d %d %d", &next_p.id, &next_p.priority, &next_p.cre_time, &next_p.run_time);
        while (!empty() && cur_time < next_p.cre_time) {
            Process cur = top();
            pop();
            ll time_limit = next_p.cre_time - cur_time;
            if (cur.run_time <= time_limit) {
                cur_time += cur.run_time;
                printf("%d %lld\n", cur.id, cur_time);
            } else {
                cur.run_time -= time_limit;
                cur_time = next_p.cre_time;
                push(cur);
            }
        }
        if (cur_time < next_p.cre_time) cur_time = next_p.cre_time;
        push(next_p);
    }
    while (!empty()) {
        Process cur = top();
        pop();
        cur_time += cur.run_time;
        printf("%d %lld\n", cur.id, cur_time);
    }
    return 0;
}