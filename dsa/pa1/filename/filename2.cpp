#include <cstdio>

int main() {
    int n, m, K;
    scanf("%d %d %d", &n, &m, &K);
    char* s1 = new char[n + 1];
    char* s2 = new char[m + 1];
    scanf("%s %s", s1, s2);
    int MAX = m + n;
    int* V = new int[2 * MAX + 1]();
    for (int D = 0; D <= MAX; D++) {
        for (int k = -D; k <= D;k += 2) {
            int x = 0;
            if (k == -D || (k != D && V[k - 1 + MAX] < V[k + 1 + MAX])) {
                x = V[k + 1 + MAX];
            } else {
                x = V[k - 1 + MAX] + 1;
            }
            int y = x - k;
            while (x < n && y < m && s1[x] == s2[y]) {
                x++;
                y++;
            }
            V[k + MAX] = x;
            if (x >= n && y >= m) {
                if (D > K) {
                    printf("-1");
                } else {
                    printf("%d", D);
                }
                return 0;
            }
        }
    }
    printf("-1");
    delete[] s1;
    delete[] s2;
    return 0;
}