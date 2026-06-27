#include <iostream>
#include <vector>
#include <algorithm>

#define ll long long

using namespace std;

struct Point {
    vector<ll> coords;
    Point(int d) : coords(d, 0) {}
};

ll dist2(const Point& p1, const Point& p2) {
    ll res = 0;
    for (int i = 0; i < p1.coords.size(); i++) {
        ll diff = p1.coords[i] - p2.coords[i];
        res += diff * diff;
    }
    return res;
}

struct Node {
    Point point;
    Node* left;
    Node* right;
    int axis;
    Node(const Point& p, int a): point(p), left(nullptr), right(nullptr), axis(a) {}
};

Node* build(vector<Point>& points, int depth, int d) {
    if (points.empty()) {
        return nullptr;
    }
    int axis = depth % d;
    sort(points.begin(), points.end(), [axis](const Point& a, const Point& b) {
        return a.coords[axis] < b.coords[axis];
    });
    int mid = points.size() / 2;
    Node* node = new Node(points[mid], axis);
    vector<Point> lc(points.begin(), points.begin() + mid);
    vector<Point> rc(points.begin() + mid + 1, points.end());
    node->left = build(lc, depth + 1, d);
    node->right = build(rc, depth + 1, d);
    return node;
}

struct Res {
    Node* best_node;
    ll best_dist2;
    Res(): best_node(nullptr), best_dist2(-1ll) {};
};

void query(Node* root, const Point& target, Res& res) {
    if (!root) {
        return;
    }
    ll d2 = dist2(root->point, target);
    if (!res.best_node || d2 < res.best_dist2) {
        res.best_node = root;
        res.best_dist2 = d2;
    }
    int axis = root->axis;
    ll diff = target.coords[axis] - root->point.coords[axis];
    Node* next_node = (diff < 0) ? root->left : root->right;
    Node* other_node = (diff < 0) ? root->right : root->left;
    query(next_node, target, res);
    if (diff * diff < res.best_dist2) {
        query(other_node, target, res);
    }
}

int main() {
    int d, n;
    scanf("%d %d", &d, &n);
    vector<Point> points;
    for (int i = 0; i < n; i++) {
        Point p(d);
        for (int j = 0; j < d; j++) {
            scanf("%lld", &p.coords[j]);
        }
        points.push_back(p);
    }
    Node* root = build(points, 0, d);
    int q;
    scanf("%d", &q);
    vector<Point> q_points;
    for (int i = 0; i < q; i++) {
        Point target(d);
        for (int j = 0; j < d; j++) {
            scanf("%lld", &target.coords[j]);
        }
        q_points.push_back(target);
    }
    for (int i = 0; i < q; i++) {
        Res res;
        query(root, q_points[i], res);
        printf("%lld\n", res.best_dist2);
    }
    return 0;
}