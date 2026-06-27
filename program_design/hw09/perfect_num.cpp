#include <iostream>
#include <vector>
#include <cmath>
#include <string>

using namespace std;

int calPal(int left, bool isOdd) {
    int res = left;
    if (isOdd) left /= 10;
    while (left > 0) {
        res = res * 10 + (left % 10);
        left /= 10;
    }
    return res;
}


int main() {
    int N;
    cin >> N;
    int pal_num = 0;
    int per_num = 0;
    string num = to_string(N);
    int len = num.length();
    for (int i = 1; i < len; i++) {
        pal_num += 9 * pow(10, (i - 1) / 2);
    }
    int mid = (len + 1) / 2;
    int left_max = stoi(num.substr(0, mid));
    int left_sta = pow(10, mid - 1);
    for (int i = left_sta; i <= left_max; i++) {
        if (calPal(i, len % 2 != 0) <= N) {
            pal_num++;
        }
    }
    vector<int> list = {6, 28, 496, 8128, 33550336};
    for (int p : list) {
        if (p <= N) {
            per_num++;
        } else {
            break;
        }
    }
    cout << per_num << " " << pal_num;
    return 0;
}