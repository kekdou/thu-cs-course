#include <iostream>
#include <vector>

using namespace std;

const int MAX_RANGE = 1000000;
const int MOD = 10007;

vector<int> primes;
bool is_prime[MAX_RANGE];

void sieve() {
    fill(is_prime, is_prime + MAX_RANGE, true);
    is_prime[0] = is_prime[1] = false;
    for (int p = 2; p * p < MAX_RANGE; p++) {
        if (is_prime[p]) {
            for (int i = p * p; i < MAX_RANGE; i += p) {
                is_prime[i] = false;
            }
        }   
    }
    for (int p = 2; p < MAX_RANGE; p++) {
        if (is_prime[p]) {
            primes.push_back(p);
        }
    }
}

int main() {
    sieve();
    int q;
    cin >> q;
    for (int i = 0; i < q; i++) {
        int k;
        cin >> k;
        int sum = 0;
        for (int j = 0; j < k; j++) {
            sum = (sum + primes[j]) % MOD;
        }
        cout << sum << " ";
    }
    return 0;
}