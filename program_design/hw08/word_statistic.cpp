#include <iostream>
#include <fstream>
#include <string>
#include <cctype>

using namespace std;

int main() {
    ifstream fin("input.txt");
    if (!fin.is_open()) {
        cerr << "Error" << endl;
        return 1;
    }
    ofstream fout("output.txt");
    if (!fout.is_open()) {
        cerr << "Error" << endl;
        return 1;
    }
    int count = 0;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        string word;
        for (char c : line) {
            if (isalpha(c)) {
                word += c;
            } else {
                if (!word.empty()) {
                    count++;
                    word.clear();
                }
            }
        }
        if (!word.empty()) {
            count++;
        }
    }
    fout << count;
    fin.close();
    fout.close();
    return 0;
}