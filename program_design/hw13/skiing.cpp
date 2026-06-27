#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int R, C;
int grid[101][101];
int memo[101][101];

int dx[] = {0, 0, 1, -1};
int dy[] = {1, -1, 0, 0};

int dfs(int x, int y) {
    if (memo[x][y] != 0) return memo[x][y];
    int max_len = 1;
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < R && ny >= 0 && ny < C && grid[nx][ny] < grid[x][y]) {
            max_len = max(max_len, dfs(nx, ny) + 1);
        }
    }
    return memo[x][y] = max_len;
}

int main() {
    cin >> R >> C;
    for (int i = 0; i < R; i++) {
        for (int j = 0; j < C; j++) {
            cin >> grid[i][j];
        }
    }
    int ans = 0;
    for (int i = 0; i < R; i++) {
        for (int j = 0; j < C; j++) {
            ans = max(ans, dfs(i, j));
        }
    }
    cout << ans << endl;
    return 0;
}