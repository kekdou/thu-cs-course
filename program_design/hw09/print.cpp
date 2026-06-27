#include <iostream>
#include <algorithm>

using namespace std;

int main() {
    int N;
    cin >> N;
    int** arr = new int*[N];
    for (int i = 0; i < N; i++) {
        arr[i] = new int[N];
    }
    int num = 1;
    for (int s = 0; s <= 2 * (N - 1); s++) {
        if (s % 2 == 0) {
            for (int j = min(s, N - 1); j >= 0 && s - j < N; j--) {
                arr[s - j][j] = num++;
            }
        } else {
            for (int i = min(s, N - 1); i >= 0 && s - i < N; i--) {
                arr[i][s - i] = num++;
            }
        }
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout << arr[i][j] << " ";
        }
        cout << endl;
    }
    for (int i = 0; i < N; i++) {
        delete[] arr[i];
    }
    delete arr;
    return 0;
}