#include <iostream>

using namespace std;

int Max(int a, int b) {
    return (a > b ? a : b);
}

int main() {
    int a, b, c;
    cin >> a >> b >> c;
    cout << Max(Max(a, b), c);
    return 0;
}