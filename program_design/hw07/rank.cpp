#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

struct Info {
    string name;
    int grade;
    Info(string n, int g): name(n), grade(g){}
};

bool myCompare(const Info& a, const Info& b) {
    if (a.grade == b.grade) {
        return a.name < b.name;
    } else {
        return a.grade > b.grade;
    }
}

void Edit(vector<Info>& info, vector<string>& gf, vector<string>& bf) {
    sort(info.begin(), info.end(), [](const Info& a, const Info& b){return a.name < b.name;});
    sort(gf.begin(), gf.end());
    sort(bf.begin(), bf.end());
    int i = 0, j = 0, k = 0;
    while (k < info.size()) {
        Info& stu = info[k];
        if (i < gf.size() && stu.name == gf.at(i)) {
            if (stu.grade <= 95) {
                stu.grade += 5;
                if (stu.grade > 95) stu.grade = 95;
            }
            if (stu.grade < 60) stu.grade = 60;
            i++;
            k++;
        } else if (j < bf.size() && stu.name == bf.at(j)) {
            if (stu.grade >= 60) {
                stu.grade -= 5;
                if (stu.grade < 60) stu.grade = 60; 
            }
            j++;
            k++;
        } else {
            k++;
        }
    }
}

int main() {
    int N, X, Y;
    cin >> N >> X >> Y;
    vector<Info> info;
    for (int i = 0; i < N; i++) {
        string name;
        int grade;
        cin >> name >> grade;
        info.push_back(Info(name, grade));
    }
    vector<string> gf, bf;
    for (int i = 0; i < X; i++) {
        string name;
        cin >> name;
        gf.push_back(name);
    }
    for (int i = 0; i < Y; i++) {
        string name;
        cin >> name;
        bf.push_back(name);
    }
    Edit(info, gf, bf);
    sort(info.begin(), info.end(), myCompare);
    for (auto stu : info) {
        cout << stu.name << " " << stu.grade << endl;
    }
    return 0;
}