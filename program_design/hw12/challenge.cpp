#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

#define ll long long

struct Prop {
    int type, val;
    Prop(int t = 0, int v = 0): type(t), val(v) {}
};

struct Character {
    int H, A, D;
    vector<Prop> props;
    Character(int h = 0, int a = 0, int d = 0): H(h), A(a), D(d) {}
    void set(int h, int a, int d) {
        H = h;
        A = a;
        D = d;
    }
};

ll getLeftHP(ll hero_h, int hero_a, int hero_d, int mon_h, int mon_a, int mon_d) {
    if (hero_a <= mon_d) {
        return -1;
    }
    int damageToMon = hero_a - mon_d;
    int damageToHero = max(0, mon_a - hero_d);
    ll turns = (mon_h + damageToMon - 1) / damageToMon;
    ll total_damage = (turns - 1) * damageToHero;
    ll remain_hp = hero_h - total_damage;
    return (remain_hp > 0) ? remain_hp : -1;
}

int main() {
    // 勇者数据
    int H, A, D;
    cin >> H >> A >> D;

    // boss 数据
    int BH, BA, BD;
    cin >> BH >> BA >> BD;

    int n, m;
    cin >> n >> m;

    // 守卫编号, 掩码表示
    int k;
    cin >> k;
    int boss_guard_mask = 0;
    for (int i = 0; i < k; i++) {
        int id;
        cin >> id;
        boss_guard_mask |= (1 << (id - 1));
    }

    // 小怪数据
    vector<Character> monster(n);
    for (int i = 0; i < n; i++) {
        int h, a, d;
        cin >> h >> a >> d;
        monster[i] = Character(h, a, d);
    }

    // 道具数据
    for (int i = 0; i < m; i++) {
        int type, val, guard;
        cin >> type >> val >> guard;
        if (guard == 0) {
            if (type == 1) {
                H += val;
            } else if (type == 2) {
                A += val;
            } else if (type == 3) {
                D += val;
            }
        } else {
            monster[guard - 1].props.push_back(Prop(type, val));
        }
    }
    
    vector<ll> dp(1 << n, -1);
    dp[0] = H;
    ll max_final_hp = -1;
    for (int mask = 0; mask < (1 << n); mask++) {
        if (dp[mask] <= 0) {
            continue;
        }
        int cur_a = A, cur_d = D;
        for (int i = 0; i < n; i++) {
            if (mask & (1 << i)) {
                for (auto& p : monster[i].props) {
                    if (p.type == 2) {
                        cur_a += p.val;
                    } else if (p.type == 3) {
                        cur_d += p.val;
                    }
                }
            }
        }
        for (int i = 0; i < n; i++) {
            if (!(mask & (1 << i))) {
                ll next_hp = getLeftHP(dp[mask], cur_a, cur_d, monster[i].H, monster[i].A, monster[i].D);
                if (next_hp > 0) {
                    for (auto& p : monster[i].props) {
                        if (p.type == 1) {
                            next_hp += p.val;
                        }
                    }
                    int next_mask = mask | (1 << i);
                    dp[next_mask] = max(dp[next_mask], next_hp);
                }
            }
        }
        if ((mask & boss_guard_mask) == boss_guard_mask) {
            ll finalhp = getLeftHP(dp[mask], cur_a, cur_d, BH, BA, BD);
            if (finalhp > 0) {
                max_final_hp = max(max_final_hp, finalhp);
            }
        }
    }

    cout << max_final_hp << endl;
    return 0;
}