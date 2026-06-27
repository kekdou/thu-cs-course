#include <iostream>

using namespace std;

int main() {
    for (int i = 0; i < 5; i++) {
        char bg = 'A' + i;
        for (int j = 0; j < 5; j++) {
            char w = 'A' + j;
            if (i == j) {
                continue;
            }
            int say1 = (bg != 'A') + (w == 'C');
            int say2 = (bg == 'D') + (w != 'E');
            int say3 = (bg == 'B') + (w != 'A');
            int say4 = (bg != 'A' && bg != 'B' && w != 'A' && w != 'B') + (bg != 'C');
            int say5 = (w == 'E' || bg == 'E') + (bg != 'A');
            if (say1 == 1 && say2 == 1 && say3 == 1 && say4 == 1 && say5 == 1) {
                cout << bg << endl;
                cout << w << endl;
                return 0;
            }
        }
    }
    return 0;
}