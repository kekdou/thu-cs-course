#include <cstdio>

#define ll long long
#define MAX_N 1000001

int children[MAX_N];
int idx[MAX_N + 1] = {0};
ll val[MAX_N + 1] = {0};
ll sum[MAX_N + 1] = {0};
int master[MAX_N + 1] = {0};

void FillSum(int node) {
    ll s = val[node];
    for (int i = idx[node - 1]; i < idx[node]; i++) {
        int child = children[i];
        FillSum(child);
        s += sum[child];
    }
    sum[node] = s;
}

void Solve(int u, int p) {
    if (p == 0) {
        master[u] = 0;
    } else {
        int cur = p;
        while (cur != 0 && val[cur] < val[u]) {
            cur = master[cur];
        }
        master[u] = cur;
    }
    int stu = 0;
    ll max_sum = 0;
    for (int i = idx[u - 1]; i < idx[u]; i++) {
        int child = children[i];
        if (sum[child] > max_sum) {
            max_sum = sum[child];
            stu = child;
        } else if (sum[child] == max_sum) {
            if (child < stu) {
                stu = child;
            }
        }
    }
    for (int i = idx[u - 1]; i < idx[u]; i++) {
        int child = children[i];
        if (child == stu) {
            Solve(child, u);
        } else {
            Solve(child, 0);
        }
    }
} 

int main() {
    int N;
    scanf("%d", &N);
    int util = 0;
    bool not_root[MAX_N + 1] = {0};
    for (int i = 1; i <= N; i++) {
        int num = 0;
        scanf("%d", &num);
        idx[i] = num + idx[i - 1];
        for (int j = 0; j < num; j++) {
            scanf("%d", &children[util++]);
            not_root[children[util - 1]] = 1;
        }
    }
    int root = -1;
    for (int i = 1; i <= N; i++) {
        if (!not_root[i]) {
            root = i;
            break;
        }
    }
    for (int i = 1; i <= N; i++) {
        scanf("%lld", &val[i]);
    }
    FillSum(root);
    Solve(root, 0);
    for (int i = 1; i <= N; i++) {
        printf("%d\n", master[i] == 0 ? -1 : master[i]);
    }
    return 0;
}