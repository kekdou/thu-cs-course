#include <iostream>
#include <cmath>
#include <vector>

using namespace std;


int main() {
    int A, B;
    cin >> A >> B;
    vector<vector<int>> result;
    for (int i = 0; i < 24; i++) {
        for (int j = 0; j < 60; j++) {
            int bin = A;
            int odd = B;
            int time[4];
            time[0] = i / 10;
            time[1] = i % 10;
            time[2] = j / 10;
            time[3] = j % 10;
            bool say1 = ((time[0] + time[1]) == (time[2] + time[3]));
            bool say2 = 0;
            for (int k = 0; k < 4; k++) {
                if ((time[k] % 2) == 0) {
                    bin--;
                } else {
                    odd--;
                }
            }
            if (bin == 0 && odd == 0) {
                say2 = 1;
            }
            if (say1 && say2){
                result.push_back({time[0], time[1], time[2], time[3]});
            }
        }
    }
    cout << result.size() << endl;
    for (vector<int> v : result) {
        cout << v.at(0) << v.at(1) << ":" << v.at(2) << v.at(3) << endl;
    }
    return 0;
}