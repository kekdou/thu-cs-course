#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    int m, n;
    cin >> m >> n;
    vector<vector<int>> num(m + 1, vector<int>(n + 1));
    vector<vector<int>> pos(m + 1, vector<int>(n + 1));
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            cin >> num[i][j];
            pos[i][num[i][j]] = j;
        }
    }
    int max_lcs = 0;
    vector<int> dp(n + 1);
    for (int i = 1; i <= n; i++) {
        dp[i] = 1;
        for (int j = 1; j < i; j++) {
            bool y = 1;
            for (int k = 2; k <= m; k++) {
                if (pos[k][num[1][j]] > pos[k][num[1][i]]) {
                    y = 0;
                    break;
                }
            }
            if (y) {
                dp[i] = max(dp[i], dp[j] + 1);
            }
        }
        max_lcs = max(max_lcs, dp[i]);
    }
    cout << max_lcs << endl;
    return 0;
}