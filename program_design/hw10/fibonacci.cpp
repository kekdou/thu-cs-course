#include <iostream>
#include <cmath>

using namespace std;

#define ll long long

const int MOD = 10000;

struct Matrix {
    ll mat[2][2];
    Matrix() {
        mat[1][0] = mat[0][1] = 0;
        mat[0][0] = mat[1][1] = 1;
    }
};

Matrix Multiply(Matrix A, Matrix B) {
    Matrix C;
    C.mat[0][0] = C.mat[0][1] = C.mat[1][0] = C.mat[1][1] = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                C.mat[i][j] = (C.mat[i][j] + A.mat[i][k] * B.mat[k][j]) % MOD;
            }
        }
    }
    return C;
}

Matrix Power(Matrix A, ll p) {
    Matrix res;
    while (p > 0) {
        if (p & 1) {
            res = Multiply(res, A);
        }
        A = Multiply(A, A);
        p >>= 1;
    }
    return res;
}


int main() {
    ll n = 0;
    cin >> n;
    n %= 15000;
    Matrix T;
    T.mat[0][0] = T.mat[0][1] = T.mat[1][0] = 1;
    T.mat[1][1] = 0;
    Matrix res = Power(T, n - 1);
    cout << res.mat[0][0] << endl;
    return 0;
}