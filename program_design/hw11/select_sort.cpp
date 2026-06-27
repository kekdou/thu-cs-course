#include <iostream>
#include <vector>

using namespace std;

void swap(int i, int j, vector<int>& arr) {
    int temp = arr[j];
    arr[j] = arr[i];
    arr[i] = temp;
    cout << "swap(a[" << i << "], a[" << j << "]):";
    for (int& x : arr) {
        cout << x << " ";
    }
    cout << endl;
}

void Select(vector<int>& arr, int bgn = 0) {
    int n = arr.size();
    if (bgn == n) {
        return;
    }
    int min_index = bgn;
    for (int i = bgn + 1; i < n; i++) {
        if (arr[i] < arr[min_index]) {
            min_index = i;
        }
    }
    swap(bgn, min_index, arr);
    Select(arr, bgn + 1);
}

int main() {
    int n = 0; 
    cin >> n;
    vector<int> arr(n);
    for (int i = 0; i < n; i++) {
        cin >> arr[i];
    }
    Select(arr);
    return 0;
}