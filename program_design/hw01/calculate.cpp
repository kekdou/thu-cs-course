#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    float result = 1 + 1 / (1 + 1 / (1 + 1.0 / 5));
    cout << fixed << setprecision(4) << result << endl;
    return 0;
}