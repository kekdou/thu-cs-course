#include <cstdio>

#define ll long long

struct Node{
    ll val;
    int index;
    Node(int a = 0, int b = 0): val(a), index(b){}
};

ll temp[20001];

void MergeSort(ll* arr, int left, int right) {
    if (left >= right) {
        return;
    }
    int mid = (right - left) / 2 + left;
    MergeSort(arr, left, mid);
    MergeSort(arr, mid + 1, right);
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }
    while (i <= mid) {
        temp[k++] = arr[i++];
    }
    while (j <= right) {
        temp[k++] = arr[j++];
    }
    for (int i = left; i <= right; i++) {
        arr[i] = temp[i];
    }
}

int Unique(ll* arr, int size) {
    if (size == 0) {
        return 0;
    }
    ll* slow = arr;
    for (ll* fast = arr + 1; fast < arr + size; fast++) {
        if (*fast != *slow) {
            *(++slow) = *fast;
        }
    }
    return slow - arr + 1;
}

bool PushBack(ll* arr, ll val, int k) {
    int i = 0;
    while (arr[i] != 0) {
        i++;
    }
    if (i >= k) {
        return 0;
    }
    arr[i] = val;
    return 1;
}

void Solve(ll* arr, ll m, int n, int k) {
    Node queue[20001] = {Node()};
    int front = 0, back = 0;
    ll res[2000][10] = {{0}};
    int row = 0;
    for (int i = 0; i < n * k; i++) {
        ll cur = arr[i];
        ll expect = arr[i] + m;
        bool ok = 0;
        int r;
        if (queue[front].val == 0 || queue[front].val != cur) {
            if (row >= n) {
                return;
            }
            r = row++;
        } else {
            r = queue[front++].index;
        }
        ok = PushBack(res[r], cur, k);
        if (!ok) {
            return;
        }
        if (res[r][k - 1] == 0) {
            queue[back++] = Node(expect, r);
        }
    }
    printf("%lld\n", m);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k; j++) {
            printf("%lld ", res[i][j]);
        }
        printf("\n");
    }
}

int main() {
    int n, k;
    scanf("%d %d", &n, &k);
    ll arr[20001];
    for (int i = 0; i < n * k; i++) {
        scanf("%lld", &arr[i]);
    }
    MergeSort(arr, 0, n * k - 1);
    ll m[2001];
    for (int i = 0; i <= n; i++) {
        m[i] = arr[i + 1] - arr[0];
    }
    MergeSort(m, 0, n - 1);
    int size = Unique(m, n);
    for (int i = 0; i < size; i++) {
        Solve(arr, m[i], n, k);
    }
    return 0;
}
