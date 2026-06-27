#include <iostream>
#include <vector>

#define ll long long

using namespace std;

ll merge(vector<int>& num, int left, int mid, int right) {
    vector<int> temp(right - left + 1);
    int i = left, j = mid + 1, k = 0;
    ll count = 0;
    while (i <= mid && j <= right) {
        if (num[i] <= num[j]) {
            temp[k++] = num[i++];
        } else {
            temp[k++] = num[j++];
            count += (mid - i + 1);
        }
    }
    while (i <= mid) temp[k++] = num[i++];
    while (j <= right) temp[k++] = num[j++];
    for (int p = 0; p < temp.size(); p++) {
        num[left + p] = temp[p];
    }
    return count;
}

ll solve(vector<int>& num, int left, int right) {
    if (left >= right) {
        return 0;
    }
    int mid = left + (right - left) / 2;
    return solve(num, left, mid) + solve(num, mid + 1, right) + merge(num, left, mid, right);
}

int main() {
    int n = 0;
    cin >> n;
    vector<int> num(n, 0);
    for (int i = 0; i < n; i++) {
        cin >> num[i];
    }
    cout << solve(num, 0, n - 1) << endl;
    return 0;
}

// 归并排序可以计数逆序对
