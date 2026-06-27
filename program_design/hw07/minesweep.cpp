#include <iostream>
#include <vector>

using namespace std;

int main() {
    int n;
    cin >> n;
    vector<int> nums;
    for (int i = 0; i < n; i++) {
        int x;
        cin >> x;
        nums.push_back(x);
    }
    vector<int> mines(n + 1, 0);
    int count = 0;
    for (int m = 0; m < 2; m++) {
        mines[1] = m;
        bool possible = 1;
        mines[2] = nums[0] - mines[1];
        if (mines[2] != 0 && mines[2] != 1) {
            continue;
        }
        for (int i = 2; i < n; i++) {
            mines[i + 1] = nums[i - 1] - mines[i] - mines[i - 1];
            if (mines[i + 1] != 0 && mines[i + 1] != 1) {
                possible = 0;
                break;
            }
        }
        if (possible && (nums[n - 1] == mines[n] + mines[n - 1])) {
            count++;
        }
    }
    cout << count << endl;
    return 0;
}