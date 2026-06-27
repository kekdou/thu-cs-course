#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdio>

using namespace std;

void opI(vector<int>& dancer, vector<int>& object) {
    int label = object[0];
    int pos = object[1];
    dancer.insert(dancer.begin() + pos, label);
}

void opS(vector<int>& dancer, vector<int>& object) {
    swap(dancer[object[0]], dancer[object[1]]);
}

void opR(vector<int>& dancer, vector<int>& object) {
    int r = object[0];
    int n = dancer.size();
    r %= n;
    if (r < 0) {
        r += n;
    }
    vector<int> res(n, 0);
    for (int i = 0; i < n; i++) {
        int idx = (i + r) % n;
        res[idx] = dancer[i];
    }
    dancer.assign(res.begin(), res.end());
}

void opF(vector<int>& dancer, vector<int>& object) {
    int n = dancer.size();
    int num1 = object[0];
    int num2 = object[1];
    if (num1 <= num2) {
        reverse(dancer.begin() + num1, dancer.begin() + num2 + 1);
    } else {
        vector<int> res;
        int num = n - num1 + num2 + 1;
        for (int i = 0; i < num; i++) {
            res.push_back(dancer[(num1 + i) % n]);
        }
        reverse(res.begin(), res.end());
        for (int i = 0; i < num; i++) {
            dancer[(num1 + i) % n] = res[i];
        }
    }
}

void selectOP(char op, vector<int>& dancer, vector<int>& object) {
    switch(op) {
        case 'I':
            opI(dancer, object);
            break;
        case 'S':
            opS(dancer, object);
            break;
        case 'R':
            opR(dancer, object);
            break;
        case 'F':
            opF(dancer, object);
            break;
        default:
            break;
    }
}

int main() {
    int n, m;
    if (scanf("%d %d", &n, &m) != 2) return 0;
    vector<int> dancer;
    for (int i = 0; i < m; i++) {
        char op;
        if (scanf(" %c", &op) != 1) break;
        vector<int> object;
        if (op == 'R') {
            int num;
            if (scanf("%d", &num) != 1) break;
            object.push_back(num);
        } else {
            int num1, num2;
            if (scanf("%d %d", &num1, &num2) != 2) break;
            object.push_back(num1);
            object.push_back(num2);
        }
        selectOP(op, dancer, object);
    }
    for (auto& d : dancer) {
        printf("%d ", d);
    }
    return 0;
}
