#include <iostream>

using namespace std;

int String(int n, int k) {
    if (k == 1) {
        return 1;
    }
    if (n == k) {
        return 1;
    }
    return String(n - 1, k - 1) + k * String(n - 1, k);
}

int main() {
    int n = 0, k = 0;
    cin >> n >> k;
    cout << String(n, k);
    return 0;
}