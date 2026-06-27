#include <iostream>
#include <vector>

using namespace std;

int quickPower(int base, int pow) {
    int res = 1;
    while (pow > 0) {
        if (pow & 1) {
            res *= base;
        }
        base *= base;
        pow >>= 1;
    }
    return res;
} 

vector<int> getDigit(int num) {
    vector<int> res;
    int n = num;
    while (n > 0) {
        res.push_back(n % 10);
        n /= 10;
    }
    return res;
}

bool isArmstrong(int num) {
    vector<int> digit = getDigit(num);
    int n = digit.size();
    int sum = 0;
    for (int x : digit) {
        sum += quickPower(x, n);
    }
    return sum == num;
}

int main() {
    for (int i = 1; i < 1e6; i++) {
        if (isArmstrong(i)) {
            cout << i << endl;
        }
    }
    return 0;
}