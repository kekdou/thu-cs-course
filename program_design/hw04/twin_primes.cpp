#include <iostream>
#include <cmath>

using namespace std;

bool is_Prime(int n) {
    for (int i = 2; i <= int(sqrt(n)); i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

int main() {
    int n;
    cin >> n;
    for (int i = n; i > 4; i--) {
        if (is_Prime(i)) {
            if (is_Prime(i - 2)) {
                cout << i - 2<< " " << i << endl;
                return 0;
            }
        }
    }
    return 0;
}