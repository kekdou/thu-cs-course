#include <iostream>
#include <fstream>

using namespace std;

const int plen_target = 1 << 11;
const int plen_bound = 1 << 12;

int main() {
    ofstream fout("../code/06.in");
    for (int i = 0; i < plen_target; i++) {
        fout << "AB";
    }
    fout << "\n";
    fout << plen_target + 1 << "\n";
    for (int i = 0; i < plen_target + 1; i++) {
        char c = (i % 6) + 'C';
        fout << "0 " << c << "\n"; 
    }
    fout.close();
    return 0;
}