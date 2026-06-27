#include <iostream>
#include <vector>
#include <algorithm>

#define ll long long

using namespace std;

unsigned ll H[2001][2001];
unsigned ll P1[2001];
unsigned ll P2[2001];

unsigned ll B1 = 131;
unsigned ll B2 = 13331;

char T[2001][2001];
char P[101][101];

unsigned ll getHash(int r1, int c1, int r2, int c2) {
    return H[r2][c2] - H[r1 - 1][c2] * P2[r2 - r1 + 1] - H[r2][c1 - 1] * P1[c2 - c1 + 1]
           + H[r1 - 1][c1 - 1] * P1[c2 - c1 + 1] * P2[r2 - r1 + 1];
}

int main() {
    int M, N, m, n, k;
    scanf("%d %d %d %d %d", &M, &N, &m, &n, &k);
    for (int i = 1; i <= M; i++) {
        for (int j = 1; j <= N; j++) {
            scanf(" %c", &T[i][j]);
        }
    }
    P1[0] = 1;
    for (int i = 1; i <= N; i++) {
        P1[i] = P1[i - 1] * B1;
    }
    P2[0] = 1;
    for (int i = 1; i <= M; i++) {
        P2[i] = P2[i - 1] * B2;
    }
    for (int i = 1; i <= M; i++) {
        for (int j = 1; j <= N; j++) {
            H[i][j] = H[i - 1][j] * B2 + H[i][j - 1] * B1 - H[i - 1][j - 1] * B1 * B2 + T[i][j];
        }
    }
    vector<unsigned ll> hashes;
    for (int i = m; i <= M; i++) {
        for (int j = n; j <= N; j++) {
            hashes.push_back(getHash(i - m + 1, j - n + 1, i, j));
        }
    }
    sort(hashes.begin(), hashes.end());
    for (int i = 0; i < k; i++) {
        unsigned ll hash_P = 0;
        for (int r = 1; r <= m; r++) {
            unsigned ll row_hash = 0;
            for (int c = 1; c <= n; c++) {
                scanf(" %c", &P[r][c]);
                row_hash = row_hash * B1 + P[r][c];
            }
            hash_P = hash_P * B2 + row_hash;
        }

        auto range = equal_range(hashes.begin(), hashes.end(), hash_P);
        cout << distance(range.first, range.second) << "\n";
    }
    return 0;
}