#include<cstdio>
#include<cstdlib>
#include<ctime>
int main(){
    srand(time(0));
    int n = 200;
    int m = 200;
    printf("%d %d\n",n,m);
    for(int i=1;i<=n;++i){
        for(int j=1;j<=m;++j){
            printf("%d ",rand()%1001);
        }printf("\n");
    }
    int q = 10;
    printf("%d\n",q);
    for(int i=1;i<=q;++i){
        int x = rand()%(n/3) + 1;
        int y = rand()%(m/3) + 1;
        int a = rand()%(n/3) + n/3;
        int b = rand()%(m/3) + m/3;
        a = (a > n - x + 1) ? (n - x + 1) : a; 
        b = (b > m - y + 1) ? (m - y + 1) : b;
        printf("%d %d %d %d\n",x,y,a,b);
    }
    return 0;
}