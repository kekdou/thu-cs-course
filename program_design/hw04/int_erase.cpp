#include <iostream>
#include <vector> 
#include <cmath>

using namespace std;

void getDigit(vector<int>& digit, int num) {
    if (num == 0) {
        digit.push_back(0);
        return;
    }
    int temp = num;
    while (temp > 0) {
        int d = temp % 10;
        digit.push_back(d);
        temp /= 10;
    }
}

int getScore(vector<int>& digit, int l, int r) {
    if (l == r) {
        return 0;
    }
    if ((r - l) == 1) {
        if (digit[l] == digit[r]) {
            return 1;
        } else {
            return -1;
        }  
    }
    if (digit[l] == digit[r]) {
        return (1 + getScore(digit, l + 1, r - 1));
    } else {
        return (-1 + getScore(digit, l + 1, r - 1));
    }
}

int main() {
    int n;
    int sum = 0;
    cin >> n;
    for (int i = 0; i < n; i++) {
        int x;
        cin >> x;
        vector<int> digit;
        getDigit(digit, x);
        if (digit.size() % 2 == 0) {
            sum += 2 * getScore(digit, 0, digit.size() - 1);
        } else {
            sum += getScore(digit, 0, digit.size() - 1);
        }
    }
    cout << sum << endl;
    return 0;
}

// score 这类的值一定要初始化！！