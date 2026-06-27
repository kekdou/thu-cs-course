#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    int n;
    cin >> n;
    vector<string> str;
    for (int i = 0; i < n; i++) {
        string s;
        cin >> s;
        str.push_back(s);
    }
    sort(str.begin(), str.end(), [](const string& a, const string& b){return a + b > b + a;});
    if (str[0] == "0") {
        cout << "0" << endl;
    } else {
        for (const string& s : str) {
            cout << s;
        }
        cout << endl;
    }
    return 0;
}