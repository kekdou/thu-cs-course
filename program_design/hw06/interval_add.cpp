#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    int n, k;
    cin >> n >> k;
    vector<int> nums(n + 2, 0);
    for (int i = 0; i < k; i++) {
        int l, r;
        cin >> l >> r;
        nums[l]++;
        nums[r + 1]--;
    }
    int max = 0;
    int cur_sum = 0;
    for (int i = 1; i <= n; i++) {
        cur_sum += nums[i];
        if (cur_sum > max) {
            max = cur_sum;
        }
    }
    cout << max << endl;
    return 0;
}