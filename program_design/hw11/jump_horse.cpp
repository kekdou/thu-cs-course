#include <iostream>
#include <vector>
#include <set>

using namespace std;

int main() {
    int n, m, K;
    cin >> n >> m >> K;
    set<pair<int, int>> stone;
    for (int i = 0; i < K; i++) {
        int x, y;
        cin >> x >> y;
        stone.insert({x, y});
    }
    vector<vector<int>> dp(n + 1, vector<int>(m + 1, 0));
    dp[0][0] = 1;
    for (int i = 1; i <= n; i++) {
        for (int j = 0; j <= m; j++) {
            if (stone.count({i, j})) {
                dp[i][j] = 0;
                continue;
            }
            if (i >= 2 && j >= 1 && !stone.count({i - 1, j - 1})) {
                dp[i][j] += dp[i - 2][j - 1];
            }
            if (i >= 1 && j >= 2 && !stone.count({i - 1, j - 1})) {
                dp[i][j] += dp[i - 1][j - 2];
            }
            if (i >= 2 && j + 1 <= m && !stone.count({i - 1, j + 1})) {
                dp[i][j] += dp[i - 2][j + 1];
            }
            if (i >= 1 && j + 2 <= m && !stone.count({i - 1, j + 1})) {
                dp[i][j] += dp[i - 1][j + 2];
            }
        }
    }
    cout << dp[n][m] << endl;
    return 0;
}