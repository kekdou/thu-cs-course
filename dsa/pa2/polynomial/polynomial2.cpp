#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <stack>

#define ll long long

using namespace std;

const ll MOD = 1000000007;

int priority_map[128];

struct Poly {
    vector<ll> coeffs;
    Poly() {}
    Poly(int p, ll c) {
        c = ((c % MOD) + MOD) % MOD;
        if (c != 0) {
            coeffs.assign(p + 1, 0);
            coeffs[p] = c;
        }
    }
};

void Init() {
    priority_map['+'] = 1;
    priority_map['-'] = 1;
    priority_map['*'] = 2;
    priority_map['^'] = 3;
    priority_map['x'] = 0;
    priority_map['('] = 0;
    priority_map[')'] = 0;
}

ll Mol (ll val) {
    return ((val % MOD) + MOD) % MOD;
}


void Clean(Poly& p) {
    while (!p.coeffs.empty() && p.coeffs.back() == 0) {
        p.coeffs.pop_back();
    }
}

Poly Add(const Poly& a, const Poly& b) {
    Poly res;
    int n = max(a.coeffs.size(), b.coeffs.size());
    res.coeffs.assign(n, 0);
    for (int i = 0; i < a.coeffs.size(); i++) {
        res.coeffs[i] = a.coeffs[i];
    }
    for (int i = 0; i < b.coeffs.size(); i++) {
        res.coeffs[i] += b.coeffs[i] % MOD;
    }
    Clean(res);
    return res;
}

Poly Sub(const Poly& a, const Poly& b) {
    Poly res;
    int n = max(a.coeffs.size(), b.coeffs.size());
    res.coeffs.assign(n, 0);
    for (int i = 0; i < a.coeffs.size(); i++) {
        res.coeffs[i] = a.coeffs[i];
    }
    for (int i = 0; i < b.coeffs.size(); i++) {
        res.coeffs[i] = (res.coeffs[i] - b.coeffs[i]) % MOD;
    }
    Clean(res);
    return res;
}

Poly Mul(const Poly& a, const Poly& b) {
    if (a.coeffs.empty() || b.coeffs.empty()) {
        return Poly();
    }
    Poly res;
    res.coeffs.assign(a.coeffs.size() + b.coeffs.size() - 1, 0);
    for (int i = 0; i < a.coeffs.size(); i++) {
        if (a.coeffs[i] == 0) {
            continue;
        }
        for (int j = 0; j < b.coeffs.size(); j++) {
            res.coeffs[i + j] = (res.coeffs[i + j] + a.coeffs[i] * b.coeffs[j]) % MOD;
        }
    }
    Clean(res);
    return res;
}

Poly Exp(const Poly& a, int pow) {
    Poly res(0, 1);
    if (pow == 0) {
        return res;
    }
    Poly base(a);
    while (pow > 0) {
        if (pow & 1) {
            res = Mul(res, base);
        }
        base = Mul(base, base);
        pow >>= 1;
    }
    return res;
}

string PreProcess(const string& input) {
    string str;
    for (char c : input) {
        if (c == ' ') {
            continue;
        }
        if (!str.empty()) {
            char c1 = str.back();
            char c2 = c;
            if ((isdigit(c1) || c1 == ')' || c1 == 'x') && c2 == '(') {
                str += '*';
            } else if ((isdigit(c1) || c1 == ')') && c2 == 'x') {
                str += '*';
            }
        }
        str += c;
    }
    return str;
}

void Compute(stack<Poly>& nums, stack<char>& ops) {
    Poly right = nums.top();
    nums.pop();
    char op = ops.top();
    ops.pop();
    if (op == '-' && nums.empty()) {
        nums.push(Sub(Poly(0, 0), right));
        return;
    }
    if (op == '^') {
        int pow = right.coeffs.empty() ? 0 : right.coeffs[0];
        Poly left = nums.top();
        nums.pop();
        nums.push(Exp(left, pow));
    } else {
        Poly left = nums.top();
        nums.pop();
        if (op == '+') {
            nums.push(Add(left, right));
        } else if (op == '-') {
            nums.push(Sub(left, right));
        } else if (op == '*') {
            nums.push(Mul(left, right));
        }
    }
}

Poly Solve(const string& s) {
    stack<Poly> nums;
    stack<char> ops;
    for (int i = 0; i < s.size(); i++) {
        if (isdigit(s[i])) {
            ll val = 0;
            while (i < s.size() && isdigit(s[i])) {
                val = (val * 10 + (s[i++] - '0')) % MOD;
            }
            nums.push(Poly(0, val));
            i--;
        } else if (s[i] == 'x') {
            nums.push(Poly(1, 1));
        } else if (s[i] == '(') {
            ops.push('(');
        } else if (s[i] == ')') {
            while (ops.top() != '(') {
                Compute(nums, ops);
            }
            ops.pop();
        } else {
            while (!ops.empty() && ops.top() != '(' &&
                   (priority_map[ops.top()] >= priority_map[s[i]])) {
                Compute(nums, ops);
            }
            ops.push(s[i]);
        }
    }
    while (!ops.empty()) {
        Compute(nums, ops);
    }
    return nums.empty() ? Poly() : nums.top();
}

void Print(Poly& res) {
    Clean(res);
    if (res.coeffs.empty()) {
        printf("0 \n");
        return;
    }
    int highest = res.coeffs.size() - 1;
    for (int i = highest; i >= 0; i--) {
       printf("%lld ", Mol(res.coeffs[i]));
    }
    cout << "\n";
}

int main() {
    string express;
    cin >> express;
    Init();
    express = PreProcess(express);
    Poly res = Solve(express);
    Print(res);
    return 0;
}