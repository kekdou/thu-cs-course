#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
    ofstream fout("../code/03.in");
    for (int i = 0; i < 125000; i++) {
        fout << "AABB";
    }
    fout << "\n";
    fout << "500000" << "\n";
    for (int i = 0; i < 250000; i++) {
        fout << "0 A" << "\n";
        fout << "0 B" << "\n";
    }
    fout.close();
    return 0;
}