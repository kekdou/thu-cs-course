#include <iostream>

using namespace std;

int main() {
    for (int i = 0; i < 6; i++) {
        char murder = 'A';
        int sum = 0;
        murder += i;
        sum = (murder != 'A') + ((murder == 'A' && murder != 'C') || (murder == 'C' && murder != 'A')) +
              0 + (murder != 'F') + ((murder != 'A') && (murder != 'F') && (murder != 'C')) + (murder == 'F');
        if (sum == 3) {
            cout << murder << endl;
        }
    }
    return 0;
}