#include <cstdio>

int main() {
    int n, m, k;
    scanf("%d %d %d", &n, &m, &k);
    char* s1 = new char[n + 1];
    char* s2 = new char[m + 1];
    scanf("%s %s", s1, s2);
    int row = m + 1;
    int col = n + 1;
    int** dp = new int*[2];
    for (int i = 0; i < 2; i++) {
        dp[i] = new int[col];
    }
    for (int i = 0; i < col; i++) {
        dp[0][i] = i;
    }
    int reverse = 0;
    for (int r = 1; r < row; r++) {
        reverse = 1 - reverse;
        dp[reverse][0] = r;
        for (int c = 1; c < col; c++) {
            if (s1[c - 1] == s2[r - 1]) {
                dp[reverse][c] = dp[1 - reverse][c - 1];
            } else {
                if (dp[reverse][c - 1] < dp[1 - reverse][c]) {
                    dp[reverse][c] = dp[reverse][c - 1] + 1;
                } else {
                    dp[reverse][c] = dp[1 - reverse][c] + 1;
                }
            }
        }
    }
    int res = dp[1 - (row & 1)][n];
    if (res > k) {
        printf("-1");
    } else {
        printf("%d", res);
    }
    for (int i = 0; i < 2; i++) {
        delete[] dp[i];
    }
    delete[] dp;
    delete[] s1;
    delete[] s2;
    return 0;
}