#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

bool isValid(int a, int b, int c) {
    if (a > 0 && b > 0 && c > 0) {
        if ((a + b > c) && (a + c > b) && (b + c > a)) {
            return 1;
        }
    }
    return 0;
}

double calArea(int a, int b, int c) {
    double p = (a + b + c) / 2.0;
    double S = sqrt(p * (p - a) * (p - b) * (p - c));
    return S;
}

int main() {
    int a, b, c;
    cin >> a >> b >> c;
    if (!isValid(a, b, c)) {
        cout << "Invalid Input" << endl;
    } else {
        cout << fixed << setprecision(2) << calArea(a, b, c) << endl;
    }
    return 0;
}