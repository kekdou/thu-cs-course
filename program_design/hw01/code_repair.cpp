#include <iostream>

using namespace std;

const int MAX_NUM = 100000 + 10;
int a[MAX_NUM];

int main() {
    int num, query, x;
    cin >> num >> query;
    for (int i = 0; i < num; i++) {
        cin >> a[i];
    }
    for (int i = 0; i < query; i++) {
        cin >> x;
        int l = 0, r = num - 1;
        while (l < r) {
            int mid = l + (r - l + 1) / 2;
            if (a[mid] <= x) l = mid;
            else r = mid - 1;
        }
        cout << a[l] << endl;
    }
    return 0;
}