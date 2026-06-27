#include <iostream>
#include <fstream>
#include <string>

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
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        size_t pos = line.find("//");
        if (pos != string::npos) {
            if (pos == 0) {
                line = "";
            }
            line.erase(pos);
        }
        fout << line << '\n';
    }
    fin.close();
    fout.close();
    return 0;
}