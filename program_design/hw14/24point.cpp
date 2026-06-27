#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int final_res;

void Solve24(vector<int>& num) {
    if (num.size() == 1) {
        if (num[0] <= 24) {
            final_res = max(final_res, num[0]);
        }
        return;
    }

    for (int i = 0; i < num.size(); i++) {
        for (int j = 0; j < num.size(); j++) {
            if (i == j) {
                continue;
            }
            vector<int> next;
            for (int k = 0; k < num.size(); k++) {
                if (k != i && k != j) {
                    next.push_back(num[k]);
                }
            }
            int a = num[i], b = num[j];

            next.push_back(a + b);
            Solve24(next);
            next.pop_back();

            next.push_back(a - b);
            Solve24(next);
            next.pop_back();

            next.push_back(a * b);
            Solve24(next);
            next.pop_back();

            if (b != 0 && a % b == 0) {
                next.push_back(a / b);
                Solve24(next);
                next.pop_back();
            }
        }
    }
}

int main() {
    int N;
    cin >> N;
    vector<vector<int>> num(N, vector<int>(4));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 4; j++) {
            cin >> num[i][j];
        }
    }
    for (auto& v : num) {
        final_res = 0;
        Solve24(v);
        cout << final_res << endl;
    }
    return 0;
}