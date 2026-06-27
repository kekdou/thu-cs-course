#include <iostream>
#include <fstream>
#include <string>

using namespace std;

bool isNum(char c) {
    if (c >= '0' && c <= '9') {
        return 1;
    }
    return 0;
}

int main() {
    ifstream fin("input.txt");
    if (!fin.is_open()) {
        cerr << "Error";
        return 0;
    }
    ofstream fout("output.txt");
    if (!fout.is_open()) {
        cerr << "Error";
        return 0;
    }
    string num;
    int sum = 0;
    char ch;
    while (fin.get(ch)) {
        if (isNum(ch)) {
            num += ch;
        }
        if (!isNum(ch) && !num.empty()) {
            sum += stoi(num);
            num.clear();
        }
    }
    if (!num.empty()) {
        sum += stoi(num);
    }
    fout << sum;
    fin.close();
    fout.close();
    return 0;
}