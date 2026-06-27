#include <iostream>
#include <queue>
#include <vector>

using namespace std;

int main() {
    int n, k;
    cin >> n >> k;
    priority_queue<int, vector<int>, greater<int>> pq;
    for (int i = 0; i < n; i++) {
        int x;
        cin >> x;
        pq.push(x);
    }
    for (int i = 1; i < k; i++) {
        pq.pop();
    }
    int res = pq.top();
    cout << res << endl;
    return 0;
}