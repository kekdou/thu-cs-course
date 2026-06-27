#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cmath>

using namespace std;

int main() {
    int m, p, q;
    cin >> m >> p >> q;
    vector<int> a, b;
    for (int i = 0; i < p; i++) {
        int x = 0;
        cin >> x;
        a.push_back(x);
    } 
    for (int i = 0; i < q; i++) {
        int x = 0;
        cin >> x;
        b.push_back(x);
    }
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    int min_dif = abs(a[0] - b[0]);
    int i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        int dif = a[i] - b[j];
        min_dif = min_dif < abs(dif) ? min_dif : abs(dif);
        if (dif < 0) {
            i++;
        } else if (dif > 0) {
            j++;
        } else {
            min_dif = 0;
            break;
        }
    }
    cout << fixed << setprecision(2) << sqrt(min_dif * min_dif + 1) << endl;
    return 0;
}