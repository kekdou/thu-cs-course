#include <iostream>
#include <vector>
#include <queue>

using namespace std;

struct Node {
    int x, y;
    Node(int a = 0, int b = 0): x(a), y(b) {}
};

int main() {
    int n, m;
    cin >> n >> m;
    Node S, E;
    vector<vector<char>> grid(n, vector<char>(m, '.'));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            cin >> grid[i][j];
            if (grid[i][j] == 'S') {
                S = Node(i, j);
            } else if (grid[i][j] == 'E') {
                E = Node(i, j);
            }
        }
    }
    vector<vector<int>> dist(n, vector<int>(m, -1));
    queue<Node> q;
    q.push(S);
    dist[S.x][S.y] = 0;
    int res = -1;
    while (!q.empty()) {
        Node cur = q.front();
        q.pop();
        if (cur.x == E.x && cur.y == E.y) {
            res = dist[cur.x][cur.y];
            break;
        }
        vector<Node> temp;
        temp.push_back(Node(cur.x - 1, cur.y));
        temp.push_back(Node(cur.x + 1, cur.y));
        temp.push_back(Node(cur.x, cur.y - 1));
        temp.push_back(Node(cur.x, cur.y + 1));
        for (auto& v : temp) {
            if (v.x >= 0 && v.x < n && v.y >= 0 && v.y < m && grid[v.x][v.y] != '#') {
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