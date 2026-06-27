#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

#define ll long long

int main() {
    int N, M;
    cin >> N >> M;
    vector<int> length(N), force(N);
    for (int i = 0; i < N; i++) {
        cin >> length[i];
    }
    for (int i = 0; i < N; i++) {
        cin >> force[i];
    }
    vector<ll> dp(M + 1, -1);
    dp[0] = 0;
    for (int i = 0; i < N; i++) {
        for (int j = length[i]; j <= M; j++) {
            if (dp[j - length[i]] != -1) {
                dp[j] = max(dp[j], dp[j - length[i]] + force[i]);
            }
        }
    }
    for (int i = 1; i <= M; i++) {
        if (i % 2 == 0) {
            int half = i / 2;
            if (dp[half] != -1) {
                dp[i] = max(dp[i], dp[half] * 2 + 233);
            }
        }
        for (int j = 1; j <= i / 2; j++) {
            if (dp[j] != -1 && dp[i - j] != -1) {
                ll bonus = (j == i - j) ? 233 : 0;
                dp[i] = max(dp[i], dp[j] + dp[i - j] + bonus);
            }
        }
    }
    cout << dp[M] << endl;
    return 0;
}