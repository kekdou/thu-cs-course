#include <iostream>
#include <vector>
#include <iomanip>

using namespace std;

int main() {
    int n, k;
    cin >> n >> k;
    vector<double> nums;
    for (int i = 0; i < n; i++) {
        double x;
        cin >> x;
        nums.push_back(x);
    }
    vector<double> sums(n + 1, 0);
    vector<double> sums_pow(n + 1, 0);
    for (int i = 0; i < n; i++) {
        sums[i + 1] = sums[i] + nums[i];
        sums_pow[i + 1] = sums_pow[i] + nums[i] * nums[i];
    }
    double min = __DBL_MAX__;
    for (int i = 0; i < n + 1 - k; i++) {
       double average = (sums[i + k] - sums[i]) / k;
       double average_pow = (sums_pow[i + k] - sums_pow[i]) / k;
       double var = average_pow - average * average;
       min = min < var ? min : var;
    }
    cout << fixed << setprecision(6) << min << endl;
    return 0;
}