#include <iostream>
#include <map>
#include <string>
#include <cctype>
#include <stack>

#define ll long long

using namespace std;

ll MOD = 1000000007;

map<char, int> priority;

ll Mol(ll val) {
    return ((val % MOD) + MOD) % MOD;
}

struct Poly{
    map<int, ll> coeffs;
    Poly() {}
    Poly(int p, ll c) {
        if (c != 0) {
            coeffs[p] = c;
        }
    }
    Poly(const Poly& other) {
        for (auto& p : other.coeffs) {
            coeffs[p.first] = p.second;
        }
    }
};

void Init() {
    priority['+'] = priority['-'] = 1;
    priority['*'] = 2;
    priority['^'] = 3;
    priority['x'] = priority['('] = priority[')'] = 0;
}

void Clean(Poly& p) {
    auto it = p.coeffs.begin();
    while (it != p.coeffs.end()) {
        if (it->second == 0) {
            it = p.coeffs.erase(it);
        } else {
            it++;
        }
    }
}

Poly Add(const Poly& a, const Poly& b) {
    Poly res;
    for (auto& p : a.coeffs) {
        res.coeffs[p.first] += p.second % MOD;
    }
    for (auto& p : b.coeffs) {
        res.coeffs[p.first] += p.second % MOD;
    }
    return res;
}

Poly Sub(const Poly& a, const Poly& b) {
    Poly res;
    for (auto& p : a.coeffs) {
        res.coeffs[p.first] = p.second % MOD;
    }
    for (auto& p : b.coeffs) {
        res.coeffs[p.first] -= p.second % MOD;
    }
    return res;
}

Poly Mul(const Poly& a, const Poly& b) {
    Poly res;
    for (auto& p1 : a.coeffs) {
        for (auto& p2 : b.coeffs) {
            int pow = p1.first + p2.first;
            ll coe = p1.second * p2.second;
            res.coeffs[pow] += coe;
            res.coeffs[pow] = res.coeffs[pow] % MOD;
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
    Clean(res);
    return res;
}

string PreProcess(string& input) {
    string str;
    for (char& c : input) {
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
        int pow = right.coeffs[0];
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

Poly Solve(string s) {
    stack<Poly> nums;
    stack<char> ops;
    for (int i = 0; i < s.size(); i++) {
        if (isdigit(s[i])) {
            ll val = 0;
            while (i < s.size() && isdigit(s[i])) {
                val = val * 10 + (s[i++] - '0');
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
                   (priority[ops.top()] >= priority[s[i]])) {
                Compute(nums, ops);
            }
            ops.push(s[i]);
        }
    }
    while (!ops.empty()) {
        Compute(nums, ops);
    }
    return nums.top();
}

void Print(Poly& res) {
    Clean(res);
    if (res.coeffs.empty()) {
        printf("0");
        return;
    }
    int highest = res.coeffs.rbegin()->first;
    for (int i = highest; i >= 0; i--) {
        ll output = res.coeffs.count(i) ? Mol(res.coeffs[i]) : 0;
        printf("%lld ", output);
    } 
}

int main() {
    string express;
    cin >> express;
    Init();
    express = PreProcess(express);
    Poly res = Solve(express);
    Print(res);
}