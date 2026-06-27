#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

double eps = 1e-9;

double ReLU(double x) {
    if (x > eps) {
        return x;
    } else {
        return 0;
    }
}

int main() {
    int n;
    cin >> n;
    double S = 0;
    for (int i = 0; i < n; i++) {
        double x;
        cin >> x;
        S += ReLU(x);
    }
    cout << fixed << setprecision(2) << S << endl;
    return 0;
}