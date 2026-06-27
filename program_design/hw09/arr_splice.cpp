#include <iostream>

using namespace std;

int** join(int **a, int **b, int m, int n, int x, int y){
    int** c = new int*[x];
    for (int i = 0; i < x; i++) {
        c[i] = new int[y];
    }
    int i = 0, j = 0, k = 0;
    while (k < x * y) {
        *(*(c + (k / y)) + k % y) = *(*(a + i / n) + i % n);
        k++;
        *(*(c + (k / y)) + k % y) = *(*(b + i / n) + i % n);
        k++;
        i++;
        j++;
    }
    return c;
}

int main(){
    int m, n, x, y;
    cin >> m >> n >> x >> y;
    int** a = new int*[m];
    for (int i = 0; i < m; i++) {
        a[i] = new int[n];
    }
    int** b = new int*[m];
    for (int i = 0; i < m; i++) {
        b[i] = new int[n];
    }
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            cin >> a[i][j];
        }
    }
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            cin >> b[i][j];
        }
    }
    int** c;
    c = join(a,b,m,n,x,y);
    for(int i=0; i<x; i++) {
        for(int j=0; j<y; j++) {
            cout << c[i][j] << " ";
        }
        cout << endl;
    }
    for (int i = 0; i < m; i++) {
        delete[] a[i];
        delete[] b[i];
    }
    delete[] a;
    delete[] b;
    for (int i = 0; i < x; i++) {
        delete[] c[i];
    }
    delete[] c;
    return 0;
}