#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <map>
#include <algorithm>
#include <vector>

using namespace std;

bool myCompare(const pair<string, int>& a, const pair<string, int>& b) {
    if (a.second != b.second) {
        return a.second > b.second;
    }
    return a.first < b.first;
}

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
    map<string, int> freq;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        string word;
        for (char c : line) {
            if (isalpha(c)) {
                c = tolower(c);
                word += c;
            } else {
                if (!word.empty()) {
                    freq[word]++;
                    word.clear();
                }
            }
        }
        if (!word.empty()) {
            freq[word]++;
        }
    }
    vector<pair<string, int>> f(freq.begin(), freq.end());
    sort(f.begin(), f.end(), myCompare);
    for (auto& p : f) {
        fout << p.first << " " << p.second << '\n';
    }
    fin.close();
    fout.close();
    return 0;
}