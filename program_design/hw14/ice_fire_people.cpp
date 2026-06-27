#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <cctype>

using namespace std;

struct State {
    int f, i, dist;
};

int R, C;
vector<string> grid;
int visited[625][625];

int getIndex(int r, int c) {
    return r * C + c;
}

bool isOnSwitch(int pos_idx, int door_type) {
    char target_switch = tolower(door_type);
    int r = pos_idx / C, c = pos_idx % C;
    return grid[r][c] == target_switch;
}

bool isVaild(int f1, int i1, int f2, int i2) {
    int fr2 = f2 / C, fc2 = f2 % C;
    int ir2 = i2 / C, ic2 = i2 % C;
    if (fr2 < 0 || fr2 >= R || fc2 < 0 || fc2 >= C || grid[fr2][fc2] == '#') {
        return 0;
    }
    if (ir2 < 0 || ir2 >= R || ic2 < 0 || ic2 >= C || grid[ir2][ic2] == '#') {
        return 0;
    }
    for (char door : {'A', 'B', 'C'}) {
        // 门通行规则
        bool f_involve = (grid[f1 / C][f1 % C] == door || grid[fr2][fc2] == door);
        bool i_involve = (grid[i1 / C][i1 % C] == door || grid[ir2][ic2] == door);
        if (f_involve || i_involve) {
            bool switch_active_S0 = (isOnSwitch(f1, door) || isOnSwitch(i1, door));
            bool switch_active_S1 = (isOnSwitch(f2, door) || isOnSwitch(i2, door));
            if (!switch_active_S0 || !switch_active_S1) {
                return 0;
            }
        }
        // 开关保护规则
        if (grid[f1 / C][f1 % C] == door && isOnSwitch(i1, door)) {
            if (i1 != i2) {
                return 0;
            }
        }
        if (grid[i1 / C][i1 % C] == door && isOnSwitch(f1, door)) {
            if (f1 != f2) {
                return 0;
            }
        }
    }
    return 1;
}

int main() {
    cin >> R >> C;
    int start_f = -1, start_i = -1, target = -1;
    grid.resize(R);
    for (int r = 0; r < R; r++) {
        cin >> grid[r];
        for (int c = 0; c < C; c++) {
            if (grid[r][c] == 'F') {
                start_f = getIndex(r, c);
            }
            if (grid[r][c] == 'I') {
                start_i = getIndex(r, c);
            }
            if (grid[r][c] == 'X') {
                target = getIndex(r, c);
            }
        }
    }
    for (int i = 0; i < 625; i++) {
        for (int j = 0; j < 625; j++) {
            visited[i][j] = 0;
        }
    }
    queue<State> q;
    q.push({start_f, start_i, 0});
    visited[start_f][start_i] = 1;
    int dr[] = {0, 0, 0, 1, -1};
    int dc[] = {0, 1, -1, 0, 0};
    while (!q.empty()) {
        State cur = q.front();
        q.pop();
        if (cur.f == target && cur.i == target) {
            cout << cur.dist << endl;
            return 0;
        }
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                int f_new_r = (cur.f / C) + dr[i], f_new_c = (cur.f % C) + dc[i];
                int i_new_r = (cur.i / C) + dr[j], i_new_c = (cur.i % C) + dc[j];
                if (f_new_r < 0 || f_new_r >= R || f_new_c < 0 || f_new_c >= C) {
                    continue;
                }
                if (i_new_r < 0 || i_new_r >= R || i_new_c < 0 || i_new_c >= C) {
                    continue;
                }
                int f_next = getIndex(f_new_r, f_new_c);
                int i_next = getIndex(i_new_r, i_new_c);
                if (!visited[f_next][i_next] && isVaild(cur.f, cur.i, f_next, i_next)) {
                    visited[f_next][i_next] = 1;
                    q.push({f_next, i_next, cur.dist + 1});
                }
            }
        }
    }
    cout << -1 << endl;
    return 0;
}