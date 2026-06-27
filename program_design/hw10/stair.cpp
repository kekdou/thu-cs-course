#include <iostream>

using namespace std;

int sol[20] = {0};

int statistic(int n) {
    if (sol[n]) {
        return sol[n];
    }
    sol[n] = statistic(n - 1) + statistic(n - 2);
    return sol[n];
}

int main() {
    sol[0] = 0;
    sol[1] = 1;
    sol[2] = 2;
    int n = 0;
    cin >> n;
    cout << statistic(n) << endl;
    return 0;
}