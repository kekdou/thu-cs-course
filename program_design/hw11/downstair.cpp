#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

bool check1, check2;

int calReslove(int n, vector<int>& water) {
    if (n == 1) {
        return 1;
    } else if (n == 2) {
        if (check1) {
            return 2;
        } else {
            return 1;
        }
    } else if (n == 3) {
        if (check1 && check2) {
            return 4;
        } else if (!check1 && !check2) {
            return 1;
        } else {
            return 2;
        }
    }
    bool exist_1 = find(water.begin(), water.end(), n - 1) == water.end();
    bool exist_2 = find(water.begin(), water.end(), n - 2) == water.end();
    bool exist_3 = find(water.begin(), water.end(), n - 3) == water.end();
    int n1 = 0, n2 = 0, n3 = 0;
    if (exist_1) {
        n1 = calReslove(n - 1, water);
    }
    if (exist_2) {
        n2 = calReslove(n - 2, water);
    }
    if (exist_3) {
        n3 = calReslove(n - 3, water);
    }
    return n1 + n2 + n3;
}


int main() {
    int h, n;
    cin >> h >> n;
    vector<int> water;
    for (int i = 0; i < n; i++) {
        int x = 0;
        cin >> x;
        water.push_back(x);
    }
    check1 = find(water.begin(), water.end(), 1) == water.end();
    check2 = find(water.begin(), water.end(), 2) == water.end();
    cout << calReslove(h, water) << endl;
    return 0;
}