#include <iostream>
#include <cmath>


using namespace std;

bool isPrime(int num) {
    for (int i = 2; i <= int(sqrt(num)); i++) {
        if (num % i == 0) {
            return 0;
        }
    }
    return 1;
}

bool isSpecial(int num) {
    int digit[7];
    int k = 0;
    int n = num;
    while (n > 0) {
        digit[k++] = n % 10;
        n /= 10;
    }
    for (int i = 0; i < k / 2; i++) {
        if (digit[i] != digit[k - 1 - i]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    int N;
    cin >> N;
    int m = 0;
    for (int i = 2; i <= N; i++) {
        if (isPrime(i) && isSpecial(i)) {
            m++;
        }
    }
    cout << m << endl;
    return 0;
}