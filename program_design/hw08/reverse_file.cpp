#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main() {
    ifstream fin("input.txt");
    ofstream fout("output.txt");
    if (!fin.is_open()) {
        cerr << "Error" << endl;
        return 0;
    }
    char ch;
    vector<char> c;
    while (fin.get(ch)) {
        c.push_back(ch);
    }
    for (auto it = c.rbegin(); it != c.rend(); it++) {
        fout << *it;
    }
    fin.close();
    fout.close();
    return 0;
}