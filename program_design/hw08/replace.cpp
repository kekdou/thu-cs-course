#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>

using namespace std;

bool isWordPart(char c) {
    return isalnum(static_cast<unsigned char>(c));
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
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        size_t pos = 0;
        while ((pos = line.find("me", pos)) != string::npos) {
            bool l_check = (pos == 0) || !isWordPart(line[pos - 1]);
            size_t next = pos + 2;
            bool r_check = (next == line.length()) || !isWordPart(line[next]);
            if (l_check && r_check) {
               line.replace(pos, 2, "you");
                pos += 3; 
            } else {
                pos += 2;
            }
        }
        fout << line << "\n";
    }
    fin.close();
    fout.close();
    return 0;
}