#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

int main() {
    int n, q;
    cin >> n >> q;
    vector<int> nums;
    for (int i = 0; i < n; i++) {
        int x;
        cin >> x;
        nums.push_back(x);
    }
    sort(nums.begin(), nums.end());
    for (int i = 0; i < q; i++) {
        int x;
        cin >> x;
        cout << nums[nums.size() - x] << " ";
    }
    return 0;
}