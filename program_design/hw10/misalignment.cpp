#include <iostream>

using namespace std;

int MisAlign(int n) {
    if (n == 1) {
        return 0;
    } else if (n == 2) {
        return 1;
    } else {
        return (n - 1) * (MisAlign(n - 2) + MisAlign(n - 1));
    }
}


int main() {
    int n = 0;
    cin >> n;
    cout << MisAlign(n) << endl;
    return 0;
}