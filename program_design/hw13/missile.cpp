#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int dpLIS(const vector<int>& segment, int minH, int maxH) {
    if (segment.empty()) {
        return 0;
    }
    vector<int> filtered;
    for (int h : segment) {
        if (h >= minH && h <= maxH) {
            filtered.push_back(h);
        }
    }
    if (filtered.empty()) {
        return 0;
    }
    int n = filtered.size();
    vector<int> dp(n, 1);
    int res = 1;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (filtered[j] <= filtered[i]) {
                dp[i] = max(dp[j] + 1, dp[i]);
            }
        }
        res = max(res, dp[i]);
    }
    return res;
}


int getLIS(const vector<int>& segment, int minH, int maxH) {
    if (segment.empty()) {
        return 0;
    }
    vector<int> filtered;
    for (int h : segment) {
        if (h >= minH && h <= maxH) {
            filtered.push_back(h);
        }
    }
    if (filtered.empty()) {
        return 0;
    }
    vector<int> d;
    for (int h : filtered) {
        auto it = upper_bound(d.begin(), d.end(), h);
        if (it == d.end()) {
            d.push_back(h);
        } else {
            *it = h;
        }
    }
    return d.size();
}
int main() {
    int n, m;
    cin >> n;
    vector<int> height(n);
    for (int i = 0; i < n; i++) {
        cin >> height[i];
    }
    cin >> m;
    if (m == 0) {
        cout << dpLIS(height, 0, __INT_MAX__) << endl;
        return 0;
    }
    vector<int> goal(m);
    for (int i = 0; i < m; i++) {
        cin >> goal[i];
    }
    int total = m;
    vector<int> s1;
    for (int i = 0; i < goal[0]; i++) {
        s1.push_back(height[i]);
    }
    total += dpLIS(s1, -1, height[goal[0]]);
    for (int i = 0; i < m - 1; i++) {
        vector<int> si;
        for (int j = goal[i] + 1; j < goal[i + 1]; j++) {
            si.push_back(height[j]);
        }
        total += dpLIS(si, height[goal[i]], height[goal[i + 1]]);
    }
    vector<int> sn;
    for (int i = goal[m - 1] + 1; i < n; i++) {
        sn.push_back(height[i]);
    }
    total += dpLIS(sn, height[goal[m - 1]], __INT_MAX__);
    cout << total << endl;
    return 0;
}