#include <iostream>
#include <vector>

using namespace std;

vector<vector<char>> Rotation_90(vector<vector<char>>& graph) {
    int n = graph.size();
    vector<vector<char>> res(n, vector<char>(n, ' '));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int res_i = j;
            int res_j = -i + n - 1;
            res[res_i][res_j] = graph[i][j];
        }
    }
    return res;
}

vector<vector<char>> Rotation_180(vector<vector<char>>& graph) {
    int n = graph.size();
    vector<vector<char>> res(n, vector<char>(n, ' '));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int res_i = -i + n - 1;
            int res_j = -j + n - 1;
            res[res_i][res_j] = graph[i][j];
        }
    }
    return res;
}

vector<vector<char>> Rotation_270(vector<vector<char>>& graph) {
    int n = graph.size();
    vector<vector<char>> res(n, vector<char>(n, ' '));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int res_i = -j + n - 1;
            int res_j = i;
            res[res_i][res_j] = graph[i][j];
        }
    }
    return res;
}

vector<vector<char>> Flip_level(vector<vector<char>>& graph) {
    int n = graph.size();
    vector<vector<char>> res(n, vector<char>(n, ' '));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int res_i = i;
            int res_j = -j + n - 1;
            res[res_i][res_j] = graph[i][j];
        }
    }
    return res;
}

bool isEqual(const vector<vector<char>>& a, const vector<vector<char>>& b, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (a[i][j] != b[i][j]) {
                return 0;
            }
        }
    }
    return 1;
}

int main() {
    int n;
    cin >> n;
    vector<vector<char>> graph;
    vector<vector<char>> other;
    for (int i = 0; i < n; i++) {
        vector<char> g;
        for (int j = 0; j < n; j++) {
            char c;
            cin >> c;
            g.push_back(c);
        }
        graph.push_back(g);
    }
    for (int i = 0; i < n; i++) {
        vector<char> g;
        for (int j = 0; j < n; j++) {
            char c;
            cin >> c;
            g.push_back(c);
        }
        other.push_back(g);
    }
    vector<vector<char>> t1 = Rotation_90(graph);
    vector<vector<char>> t2 = Rotation_180(graph);
    vector<vector<char>> t3 = Rotation_270(graph);
    vector<vector<char>> t4 = Flip_level(graph);
    if (isEqual(graph, other, n)) {
        cout << "6" << endl;
    } else if (isEqual(other, t1, n)) {
        cout << "1" << endl;
    } else if (isEqual(other, t2, n)) {
        cout << "2" << endl;
    } else if (isEqual(other, t3, n)) {
        cout << "3" << endl;
    } else if (isEqual(other, t4, n)) {
        cout << "4" << endl;
    } else if (isEqual(other, Rotation_90(t4), n) 
            || isEqual(other, Rotation_180(t4), n) 
            || isEqual(other, Rotation_270(t4), n)) {
        cout << "5" << endl;
    } else {
        cout << "7" << endl;
    }
    return 0;
}