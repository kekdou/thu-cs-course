#include <cstdio>

long long row_col_sum[2001][2001];
int matrix[2001][2001];

int main() {
    int n, m, q;
    scanf("%d %d", &n, &m);
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            scanf("%d", &matrix[i][j]);
        }
    }
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            row_col_sum[i][j] = row_col_sum[i - 1][j] + row_col_sum[i][j - 1] - row_col_sum[i - 1][j - 1] + matrix[i][j];
        }
    }
    scanf("%d", &q);
    for (int i = 0; i < q; i++) {
        int x, y, a, b;
        scanf("%d %d %d %d", &x, &y, &a, &b);
        long long sum = row_col_sum[x + a - 1][y + b - 1] + row_col_sum[x - 1][y - 1] - row_col_sum[x + a - 1][y - 1] - row_col_sum[x - 1][y + b - 1];
        printf("%lld \n", sum); 
    }
    return 0;
}
