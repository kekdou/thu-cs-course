#include <iostream>
#include <vector>
#include <string>

using namespace std;

void print(vector<vector<int>> h) {
    for (auto& x : h) {
        for (auto& y : x) {
            cout << y << " ";
        }
        cout << endl;
    }
}

int main() {
    int N, M;
    cin >> N;
    vector<vector<int>> h(3);
    for (int i = 0; i < N; i++) {
        int x = 0;
        cin >> x;
        h[0].push_back(x);
    }
    cin >> M;
    vector<string> op;
    for (int i = 0; i < M; i++) {
        string s;
        cin >> s;
        op.push_back(s);
    }
    print(h);
    for (int i = 0; i < M; i++) {
        int op1 = op[i][0] - 'A';
        int op2 = op[i][1] - 'A';
        if (op1 < 0 || op1 > 2) {
            cout << "ERROR";
            return 0;
        }
        if (op2 < 0 || op2 > 2) {
            cout << "ERROR";
            return 0;
        }
        if (h[op1].empty()) {
            cout << "ERROR";
            return 0;
        }
        int top = __INT_MAX__;
        if (!h[op2].empty()) {
            top = *(h[op2].end() - 1);
        }
        int mov = *(h[op1].end() - 1);
        if (mov > top) {
            cout << "ERROR";
            return 0;
        }
        h[op1].pop_back();
        h[op2].push_back(mov);
        print(h);
        if (h[0].empty() && h[1].empty()) {
            return 0;
        }
    }
    return 0;
}