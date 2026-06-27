#include <iostream>

using namespace std;

int calLevel(int n) {
    if (n >= 95) {
        return 1;
    } else if (n >= 80 && n < 95) {
        return 2;
    } else if (n >= 60 && n < 80) {
        return 3;
    } else {
        return 4;
    }
}

int main() {
    int n;
    int sum = 0, min = 0, max = 0;
    cin >> n;
    for (int i = 0; i < n; i++) {
        int num;
        cin >> num;
        sum += num;
        if (i == 0) {
            min = num;
            max = num;
            cout << calLevel(num) << " ";
            continue;
        }
        if (min > num) {
            min = num;
        }
        if (max < num) {
            max = num;
        }
        cout << calLevel(num) << " ";
    }
    cout << endl;
    double average = double(sum) / n;
    cout << calLevel(max) << " " << calLevel(average) << " " << calLevel(min) << endl;
    return 0;
}