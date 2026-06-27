#include <iostream>
#include <cmath>

using namespace std;

#define ll long long
#define MOD 10007

ll calPower(ll base, ll exp) {
    ll res = 1;
    base %= MOD;
    while (exp > 0) {
        if (exp % 2 == 1) {
            res = (res * base) % MOD;
        }
        base = (base * base) % MOD;
        exp /= 2;
    }
    return res;
}

int main() {
    ll n, k;
    cin >> n >> k;
    ll sum = 0;
    k %= MOD - 1;
    for (int i = 1; i <= n; i++) {
        sum = (sum + calPower(i, k)) % MOD;
    }
    cout << sum << endl;
    return 0;
}