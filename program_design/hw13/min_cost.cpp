#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

#define ll long long

int main() {
    ll n, L;
    cin >> n >> L;
    vector<ll> a(n + 1);
    vector<ll> S(n + 1, 0);
    for (ll i = 1; i <= n; i++) {
        cin >> a[i];
        S[i] = S[i - 1] + a[i];
    }
    vector<ll> dp(n + 1, 0);
    for (ll i = 1; i <= n; i++) {
        dp[i] = __LONG_LONG_MAX__;
        for (ll j = 0; j < i; j++) {
            ll segment_sum = S[i] - S[j];
            ll cost = (segment_sum - L) * (segment_sum - L);
            dp[i] = min(dp[i], dp[j] + cost);
        }
    }
    cout << dp[n] << endl;
    return 0;
}