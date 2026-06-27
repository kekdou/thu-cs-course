#include <cstdio>

#define ll long long

int res = 0;
int n = 0;
ll P = 0ll;

void Solve(ll** g, int depth, ll value) {
    if (value > P) {
        return;
    }
    if (depth == n - 1) {
        res++;
        return;
    }
    Solve(g, depth + 1, value + g[depth + 1][0]);
    Solve(g, depth + 1, value + g[depth + 1][1]);
}

int main() {
    scanf("%d %lld", &n, &P);
    ll** gift = new ll*[n];
    for (int i = 0; i < n; i++) {
        gift[i] = new ll[2];
        scanf("%lld %lld", &gift[i][0], &gift[i][1]);
    }
    Solve(gift, -1, 0ll);
    printf("%d", res);
    for (int i = 0; i < n; i++) {
        delete[] gift[i];
    }
    delete[] gift;
    return 0;
}