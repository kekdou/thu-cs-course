#include <iostream>
#include <cstring>

using namespace std;

int* buildNext(char* p) {
    int m = strlen(p);
    int j = 0;
    int* next = new int[m];
    int t = next[0] = -1;
    while (j < m -1) {
        if (0 > t || p[t] == p[j]) {
            t++;
            j++;
            next[j] = t;
        } else {
            t = next[t];
        }
    }
    return next;
}

int main() {
    char* P = "MAM";
    int* next = buildNext(P);
    for (int i = 0; i < 3; i++) {
        printf("%d ", next[i]);
    }
    return 0;
}