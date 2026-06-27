#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int R;
vector<vector<int>> pyramid;
vector<vector<int>> dp;

int main() {
    cin >> R;
    for (int i = 0; i < R; i++) {
        pyramid.push_back(vector<int>());
        dp.push_back(vector<int>());
        for (int j = 0; j <= i; j++) {
            int x;
            cin >> x;
            pyramid[i].push_back(x);
            dp[i].push_back(0);
        }
    }
    dp[0][0] = pyramid[0][0];
    for (int i = 1; i < R; i++) {
        for (int j = 0; j <= i; j++) {
            if (j == 0) {
                dp[i][j] = dp[i - 1][j] + pyramid[i][j];
            } else if (j == i) {
                dp[i][j] = dp[i - 1][i - 1] + pyramid[i][j];
            } else {
                dp[i][j] = max(dp[i - 1][j], dp[i - 1][j - 1]);
                dp[i][j] += pyramid[i][j];
            }
        }
    }
    int res = 0;
    for (int i = 0; i < R; i++) {
        res = max(res, dp[R - 1][i]);
    }
    cout << res << endl;
    return 0;
}