#include <iostream>

using namespace std;

int Poly(int x, int n) {
    if (n == 0) {
        return 1;
    } else if (n == 1) {
        return 2 * x;
    } else {
        return (2 * x * Poly(x, n - 1) - 2 * (n - 1) * Poly(x, n - 2));
    }
}

int main() {
    int n, x;
    cin >> n >> x;
    cout << Poly(x, n) << endl;
    return 0;
}