#include <iostream>
#include <vector>

using namespace std;

bool checkMianzi(vector<int>& count, int total) {
    if (total == 0) {
        return 1;
    }
    int i = 1;
    while (i <= 9 && count[i] == 0) {
        i++;
    }
    if (count[i] >= 3) {
        count[i] -= 3;
        if (checkMianzi(count, total - 3)) {
            return 1;
            count[i] += 3;
        }
    }
    if (i <= 7 && count[i + 1] > 0 && count[i + 2] > 0) {
        count[i]--;
        count[i + 1]--;
        count[i + 2]--;
        if (checkMianzi(count, total - 3)) {
            return 1;
        }
        count[i]++;
        count[i + 1]++;
        count[i + 2]++;
    }
    return 0;
}

bool checkDuizi(vector<int>& count) {
    for (int i = 0; i <= 9; i++) {
        if (count[i] >= 2) {
            count[i] -= 2;
            if (checkMianzi(count, 12)) {
                return 1;
            }
            count[i] += 2;
        }
    }
    return 0;
}

int main() {
    vector<int> maj(14);
    for (int i = 0; i < 14; i++) {
        cin >> maj[i];
    }
    vector<int> count(10, 0);
    for (int& x : maj) {
        count[x]++;
    }
    if (checkDuizi(count)) {
        cout << "Yes" << endl;
    } else {
        cout << "No" << endl;
    }
    return 0;
}