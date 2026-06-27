#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>

using namespace std;

struct State {
    int left_s, left_w;
    int boat;
    int dist;
};

bool Check(int s, int w, int n) {
    if (s == 0) {
        return 1;
    }
    if (s < w) {
        return 0;
    }
    int right_s = n - s;
    int right_w = n - w;
    if (right_s > 0 && right_s < right_w) {
        return 0;
    }
    return 1;
}

int Solve(int n, int m) {
    vector<vector<vector<bool>>> visited(n + 1, vector<vector<bool>>(n + 1, vector<bool>(2, 0)));
    queue<State> q;
    q.push({n, n, 0, 0});
    while (!q.empty()) {
        State cur = q.front();
        q.pop();
        if (cur.left_s == 0 && cur.left_w == 0) {
            return cur.dist;
        }
        for (int s = 0; s <= m; s++) {
            for (int w = 0; w <= m - s; w++) {
                if (s + w == 0) {
                    continue;
                }
                if (s > 0 && s < w) {
                    continue;
                }
                int next_left_s, next_left_w;
                if (cur.boat == 0) {
                    next_left_s = cur.left_s - s;
                    next_left_w = cur.left_w - w;
                } else {
                    next_left_s = cur.left_s + s;
                    next_left_w = cur.left_w + w;
                }
                if (next_left_s >= 0 && next_left_s <= n &&
                    next_left_w >= 0 && next_left_w <= n) {
                    if (!visited[next_left_s][next_left_w][1 - cur.boat]
                        && Check(next_left_s, next_left_w, n)) {
                        visited[next_left_s][next_left_w][1 - cur.boat] = 1;
                        q.push({next_left_s, next_left_w, 1 - cur.boat, cur.dist + 1});
                    }
                }
            }
        }
    }
    return -1;
}

int main() {
    int n, m;
    cin >> n >> m;
    int res = Solve(n, m);
    cout << res << endl;
    return 0;
}