#include <iostream>
#include <vector>
#include <algorithm>
#include <map>

using namespace std;

int findIndex(vector<char> arr, char c) {
    auto it = find(arr.begin(), arr.end(), c);
    if (it != arr.end()) {
        int index = distance(arr.begin(), it);
        return index;
    } else {
        return -1;
    }
}

int main() {
    vector<char> order = {'A', 'B', 'C'};
    do {
        vector<char> o;
        map<char, int> say;
        for (char c : order) {
            o.push_back(c);
        }
        say['A'] = (findIndex(order, 'B') > findIndex(order, 'A')) + (findIndex(order, 'C') == findIndex(order, 'A'));
        say['B'] = (findIndex(order, 'A') > findIndex(order, 'B')) + (findIndex(order, 'A') > findIndex(order, 'C'));
        say['C'] = (findIndex(order, 'C') > findIndex(order, 'B')) + (findIndex(order, 'B') > findIndex(order, 'A'));
        if(say[o[0]] > say[o[1]] && say[o[1]] > say[o[2]]) {
            for (char ch : o) {
                cout << ch << " ";
            }
            cout << endl;
            break;
        }
    } while (next_permutation(order.begin(), order.end()));
    return 0;
}
