#include <cstdio>

#define ll long long

int n = 0;
ll P = 0ll;

ll gift[40][2];

ll g1[1 << 20];
ll g2[1 << 20];
int size1 = 0, size2 = 0;

ll temp[1 << 20];

void Sum(ll* g, int start, int end, ll value, int& size) {
    if (value > P) {
        return;
    }
    if (start == end) {
        g[size++] = value;
        return;
    }
    Sum(g, start + 1, end, value + gift[start][0], size);
    Sum(g, start + 1, end, value + gift[start][1], size);
}

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

int BinarySearch(ll* arr, ll target, int size) {
    int left = 0;
    int right = size;
    while (left < right) {
        int mid = (right - left) / 2 + left;
        if (arr[mid] <= target) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return left;
}

int main() {
    scanf("%d %lld", &n, &P);
    for (int i = 0; i < n; i++) {
        scanf("%lld %lld", &gift[i][0], &gift[i][1]);
    }
    Sum(g1, 0, n / 2, 0ll, size1);
    Sum(g2, n / 2, n, 0ll, size2);
    MergeSort(g1, 0, size1 - 1);
    ll res = 0;
    for (int i = 0; i < size2; i++) {
        res += BinarySearch(g1, P - g2[i], size1);
    }
    printf("%lld", res);
    return 0;
}