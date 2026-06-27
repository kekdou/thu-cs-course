#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <string>
#include <set>

using namespace std;

struct P {
    vector<int> project;
    int age;
    bool operator<(const P& o) const {
        if (age != o.age) return age < o.age;
        return project < o.project;
    }
};

vector<int> findIndex(const vector<int>& num, int n) {
    vector<int> res;
    for (int i = 0; i < (int)num.size(); i++) {
        if (num[i] == n) res.push_back(i);
    }
    if (res[0] > res[1]) reverse(res.begin(), res.end());
    return res;
}

char findName(const map<char, P>& m, int age) {
    for (int i = 0; i < 3; i++) {
        char N = 'A' + i;
        auto it = m.find(N);
        if (it != m.end() && it->second.age == age) {
            return N;
        }
    }
    return 'x';
}

string convert(int index) {
    switch (index) {
        case 0: {
            return "地理";
            break; 
        }
        case 1: {
            return "美术";
            break;
        }
        case 2: {
            return "算术";
            break;
        }
        case 3: {
            return "语文";
            break;
        }
        case 4: {
            return "音乐";
            break;
        }
        case 5: {
            return "政治";
            break;
        }
        default: {
            return "";
            break;
        }
    }
}
int main() {
    map<char, P> pf;
    vector<int> mask = {1, 1, 2, 2, 3, 3}; // index 表示课程，val 表示不同年龄老师
    // 地理，美术，算术，语文，音乐，政治
    set<map<char, P>> res;
    do {
        vector<int> ages = {1, 2, 3};
        do {
            pf['A'].age = ages[0];
            pf['A'].project = findIndex(mask, ages[0]);
            pf['B'].age = ages[1];
            pf['B'].project = findIndex(mask, ages[1]);
            pf['C'].age = ages[2];
            pf['C'].project = findIndex(mask, ages[2]);
            bool say1 = (mask[5] != mask[2]);
            bool say2 = (mask[0] > mask[3]);
            bool say3 = (pf['B'].age == 1);
            bool hasA0 = find(pf['A'].project.begin(), pf['A'].project.end(), 0) != pf['A'].project.end();
            bool hasA2 = find(pf['A'].project.begin(), pf['A'].project.end(), 2) != pf['A'].project.end();
            bool say4 = (!hasA0 && !hasA2) && (mask[0] != mask[2]);
            bool hasB3 = find(pf['B'].project.begin(), pf['B'].project.end(), 3) != pf['B'].project.end();
            bool hasB4 = find(pf['B'].project.begin(), pf['B'].project.end(), 4) != pf['B'].project.end();
            bool say5 = (!hasB3 && !hasB4) && (mask[3] != mask[4]);
            if (say1 && say2 && say3 && say4 && say5) {
                res.insert(pf);
            }
        } while (next_permutation(ages.begin(), ages.end()));
    } while (next_permutation(mask.begin(), mask.end()));
    for (const auto& m : res) {
        for (int i = 0; i < 3; i++) {
            char teacher = 'A' + i;
            const auto& proj = m.at(teacher).project;
            cout << convert(proj[0]) << " " << convert(proj[1]) << endl;
        }
    }
    return 0;
}