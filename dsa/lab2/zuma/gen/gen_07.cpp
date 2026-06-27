#include <iostream>
#include <fstream>

using namespace std;

const int plen_target = 1 << 11;
const int plen_bound = 1 << 12;

int main() {
    ofstream fout("../code/07.in");
    for (int i = 0; i < plen_target; i++) {
        char c = (i % 2) + 'A';
        fout << c;
    }
    for (int i = 0; i < (plen_target >> 1); i++) {
        char c = (i % 2) + 'C';
        fout << c << c;
    }
    for (int i = 0; i < plen_target; i++) {
        char c = (i % 2) + 'B';
        fout << c;
    }
    fout << "\n";
    fout << plen_target / 2 + 1 << "\n";
    for (int i = 0; i < (plen_target >> 1); i++) {
        char c = (i % 2) + 'C';
        fout << plen_target << " " << c << "\n";
    }
    fout << plen_target << " " << 'B' << "\n";
    return 0;
}