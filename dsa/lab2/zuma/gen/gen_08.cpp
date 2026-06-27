#include <iostream>
#include <fstream>

using namespace std;

const int plen_target = 1 << 11;
const int plen_bound = 1 << 12;

int main() {
    fstream fout("../code/08.in");
    for (int i = 0; i < (plen_target >> 2); i++) {
        char c = (i % 2) + 'A';
        fout << c << c;
    }
    for (int i = 0; i < (plen_target >> 1); i++) {
        char c = (i % 2) + 'C';
        fout << c << c;
    }
    for (int i = 0; i < (plen_target >> 2); i++) {
        char c =  (1- (i % 2)) + 'A';
        fout << c << c;
    }
    fout << "\n";
    fout << plen_target / 2 << "\n";
    for (int i = 0; i < (plen_target >> 1); i++) {
        char c = (i % 2) + 'C';
        fout << plen_target << " " << c << "\n";
    }
    return 0;
}