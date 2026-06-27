#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    int n, k;
    cin >> n >> k;
    vector<int> num;
    for (int i = 1; i <= n; i++) {
        num.push_back(i);
    }
    do {
        k--;
        if (k == 0) {
            for (auto& x : num) {
                cout << x << " ";
            }
            cout << endl;
            break;
        }
    } while (next_permutation(num.begin(), num.end()));
    return 0;
}