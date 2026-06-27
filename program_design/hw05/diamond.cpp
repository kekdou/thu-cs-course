#include <iostream>
#include <cmath>

using namespace std;

void print(int row, int n) {
    for (int i = 0; i < (n - row); i++) {
        cout << " ";
    }
    for (int i = 0; i < (2 * row - 1); i++) {
        cout << "*";
    }
}

int main() {
    int n;
    cin >> n;
    for (int i = 1; i <= (2 * n - 1); i++) {
        int k = i;
        if (i > n) {
            k = 2 * n - i;
        }
        print(k, n);
        if (i != (2 * n - 1)) {
            cout << endl;
        }
    }
    return 0;
}