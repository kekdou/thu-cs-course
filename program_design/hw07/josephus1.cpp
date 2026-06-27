#include <iostream>
#include <algorithm>

using namespace std;

int main() {
    int n, k;
    cin >> n >> k;
    bool p[1001] = {0};
    int t = 0, dead = 0, clock = 0;
    do {
        t++;
        if (t > n) {
            t = 1;
        }
        if (!p[t]) {
            clock++;
        }
        if (clock == k + 1) {
            clock = 0;
            p[t] = 1;
            dead++;
        }
    } while (dead != n - 1);
    auto pos = find(p + 1, p + 1000, 0);
    int index = distance(p, pos);
    cout << index << endl;
    return 0;
}