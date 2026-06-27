#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>

using namespace std;

typedef long long ll;

int main() {
    int n, k;
    cin >> n >> k;
    vector<ll> a(n + 1), S(n + 1, 0);
    for (int i = 1; i <= n; i++) {
        cin >> a[i];
        S[i] = S[i - 1] + a[i];
    }
    vector<ll> dp(n + 2, 0);
    deque<int> q;
    auto getValue = [&](int j) { return dp[j] - S[j]; };
    q.push_back(0);
    for (int i = 1; i <= n + 1; i++) {
        if (!q.empty() && q.front() < i - k - 1) {
            q.pop_front();
        }
        dp[i] = S[i - 1] + getValue(q.front());
        if (i <= n) {
            while (!q.empty() && getValue(q.back()) <= getValue(i)) {
                q.pop_back();
            }
            q.push_back(i);
        }
    }
    cout << dp[n + 1] << endl;
    return 0;
}

// int main() {
//     int n, k;
//     cin >> n >> k;
//     vector<ll> val(n + 1);
//     for (int i = 1; i <= n; i++) {
//         cin >> val[i];
//     }
//     vector<vector<ll>> dp(2, vector<ll>(k + 1));
//     int reverse = 0;
//     for (int i = 1; i <= n; i++) {
//         ll prev_max = 0;
//         for (int j = 0; j <= k ; j++) {
//             prev_max = max(prev_max, dp[1 - reverse][j]);
//         }
//         dp[reverse][0] = prev_max;
//         for (int j = 0; j < k; j++) {
//             dp[reverse][j + 1] = dp[1 - reverse][j] + val[i];
//         }
//         reverse = 1 - reverse;
//     }
//     ll final = 0;
//     for (int i = 0; i <= k; i++) {
//         final = max(final, dp[1 - (n % 2)][i]);
//     }
//     cout << final << endl;
//     return 0;
// }