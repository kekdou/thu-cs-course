#include <iostream>
#include <vector>
#include <string>

using namespace std;

vector<int> calBigNum(vector<int>& n1, vector<int>& n2) {
    vector<int> res;
    int size = max(n1.size(), n2.size());
    int carry = 0;
    for (int i = 0; i < size; i++) {
        int a = (i < n1.size()) ? n1[i] : 0;
        int b = (i < n2.size()) ? n2[i] : 0;
        int sum = a + b + carry;
        res.push_back(sum % 10);
        carry = sum / 10;
    }
    if (carry) res.push_back(carry);
    return res;
}

int main() {
    string n1, n2;
    cin >> n1 >> n2;
    vector<int> num1, num2;
    for (int i = 0; i < n1.size(); i++) {
        num1.push_back(n1[n1.size() - 1 - i] - '0');
    }
    for (int i = 0; i < n2.size(); i++) {
        num2.push_back(n2[n2.size() - 1 - i] - '0');
    }
    vector<int> res = calBigNum(num1, num2);
    for (int i = res.size(); i > 0; i--) {
        cout << res[i - 1];
    }
    return 0;
}