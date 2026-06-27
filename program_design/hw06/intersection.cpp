#include <iostream>

using namespace std;

int main() {
    int n, m;
    cin >> n >> m;
    int nset[100000], mset[100000];
    for (int i = 0; i < n; i++) {
        int x;
        cin >> x;
        nset[i] = x;
    }
    for (int i = 0; i < m; i++) {
        int x;
        cin >> x;
        mset[i] = x;
    }
    int nptr = 0, mptr = 0;
    int k = 0;
    int res[100000];
    while (nptr < n && mptr < m) {
        if (nset[nptr] > mset[mptr]) {
            mptr++;
        } else if (nset[nptr] < mset[mptr]) {
            nptr++;
        } else {
            res[k++] = mset[mptr];
            mptr++;
            nptr++;
        }
    }
    cout << k;
    return 0;
}