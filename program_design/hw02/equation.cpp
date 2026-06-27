#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    double a, b , c, d;
    cin >> a >> b;
    cin >> c >> d;
    if (a == c) {
        if (b == d) {
            cout << "Infinite" << endl;
            return 0;
        }
        cout << "No Solution" << endl;
        return 0;
    }
    double x = (b - d) / (c - a);
    double y = a * x + b;
    cout << fixed << setprecision(2) << x << " " << y << endl;
    return 0;
}