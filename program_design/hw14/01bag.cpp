#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    int n, m;
    cin >> n >> m;
    vector<int> val(n + 1, 0), weight(n + 1, 0);
    for (int i = 1; i <= n; i++) {
        cin >> weight[i];
        cin >> val[i];
    }
    int res = 0;
    vector<vector<int>> dp(n + 1, vector<int>(m + 1, 0));
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            if (weight[i] > j) {
                dp[i][j] = dp[i - 1][j];
            } else {
                dp[i][j] = max(dp[i - 1][j], dp[i - 1][j - weight[i]] + val[i]);
            }
        }
    }
    cout << dp[n][m] << endl;
    return 0;
}