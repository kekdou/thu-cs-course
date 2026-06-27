#include <iostream>
#include <vector>
#include <queue>

using namespace std;

struct Node {
    int x, y;
    Node(int a, int b): x(a), y(b) {}
};

int main() {
    int n, m;
    cin >> n >> m;
    vector<vector<char>> grid(n, vector<char>(m));
    vector<vector<bool>> visited(n, vector<bool>(m, 0));
    int water = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            cin >> grid[i][j];
        }
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (grid[i][j] != 'W') {
                visited[i][j] = 1;
                continue;
            }
            if (visited[i][j]) {
                continue;
            }
            queue<Node> q;
            q.push(Node(i, j));
            visited[i][j] = 1;
            while (!q.empty()) {
                Node cur = q.front();
                q.pop();
                vector<Node> temp;
                temp.push_back(Node(cur.x - 1, cur.y + 1));
                temp.push_back(Node(cur.x, cur.y + 1));
                temp.push_back(Node(cur.x + 1, cur.y + 1));
                temp.push_back(Node(cur.x - 1, cur.y));
                temp.push_back(Node(cur.x + 1, cur.y));
                temp.push_back(Node(cur.x - 1, cur.y - 1));
                temp.push_back(Node(cur.x, cur.y - 1));
                temp.push_back(Node(cur.x + 1, cur.y - 1));
                for (auto& v : temp) {
                    if (v.x < 0 || v.x >= n || v.y < 0 || v.y >= m) {
                        continue;
                    }
                    if (!visited[v.x][v.y] && grid[v.x][v.y] == 'W') {
                        visited[v.x][v.y] = 1;
                        q.push(Node(v.x, v.y));
                    }
                }
            }
            water++;
        }
    }
    cout << water << endl;
    return 0;
}