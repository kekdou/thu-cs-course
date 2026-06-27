#include <iostream>
#include <vector>
#include <queue>

using namespace std;


struct Node {
    int x, y;
};

int main() {
    int n, m;
    cin >> n >> m;
    vector<vector<int>> block(n + 1, vector<int>(m + 1, 0));
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            cin >> block[i][j];
        }
    }
    vector<vector<int>> dist(n + 1, vector<int>(m + 1, -1));
    int res = -1;
    queue<Node> q;
    q.push({1, 1});
    dist[1][1] = 0;
    while (!q.empty()) {
        Node cur = q.front();
        q.pop();
        if (cur.x == n && cur.y == m) {
            res = dist[cur.x][cur.y];
            break;
        }
        int type = block[cur.x][cur.y];
        vector<Node> next;
        if (type == 1) {
            next.push_back({2 * cur.x, cur.y});
            next.push_back({cur.x, 2 * cur.y});
        } else if (type == 2) {
            next.push_back({cur.x + 1, cur.y});
            next.push_back({cur.x, cur.y + 1});
            next.push_back({2 * cur.x, cur.y});
            next.push_back({cur.x, 2 * cur.y});
        }
        for (auto& v : next) {
            if (v.x >= 1 && v.x <= n && v.y >= 1 && v.y <= m) {
                if (dist[v.x][v.y] == -1) {
                    dist[v.x][v.y] = dist[cur.x][cur.y] + 1;
                    q.push(v);
                }
            }
        }
    }
    cout << res << endl;
    return 0;
}