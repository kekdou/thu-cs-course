#include <iostream>

using namespace std;

void to_Binary(int num) {
    bool neg = 0;
    int n = num;
    if (num == 0) {
        cout << "0";
        return;
    }
    if (num < 0) {
        n = -num;
        neg = 1;
    }
    if (neg) {
        cout << "-";
    }
    bool start = 0;
    for (int i = 31; i > -1; i--) {
        int k = n >> i;
        if (k & 1) {
            start = 1;
        }
        if (start) {
            cout << (k & 1);
        }
    }
}

int main() {
    int num;
    cin >> num;
    to_Binary(num);
    return 0;
}