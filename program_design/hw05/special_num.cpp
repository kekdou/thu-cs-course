#include <iostream>

using namespace std;

bool checkSpecial(int i, int n) {
    int nums[6];
    int temp = i;
    int sum = 0;
    int k = 0;
    while(temp > 0) {
        int num = temp % 10;
        nums[k++] = num;
        sum += num;
        temp /= 10;
    }
    if (sum != n) {
        return 0;
    }
    for (int j = 0; j < k / 2; j++) {
        if (nums[j] != nums[k - 1 - j]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    int n;
    cin >> n;
    for (int i = 10000; i < 1000000; i ++) {
        if (checkSpecial(i, n)) {
            cout << i << endl;
        }
    }
    return 0;
}