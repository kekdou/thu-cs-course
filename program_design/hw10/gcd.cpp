#include <iostream>
#include <algorithm>

using namespace std;

int gcd(int a, int b) {
    int x = max(a, b);
    int y = min(a, b);
    if (y == 0) {
        return x;
    } else {
        return gcd(x % y, y);
    }
}

int main() {
    int a, b;
    cin >> a >> b;
    cout << gcd(a, b) << endl;
    return 0;
}