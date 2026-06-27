#include <iostream>
#include <vector>
#include <algorithm>

#define ll long long

using namespace std;

struct Point {
    ll x, y;
    Point(ll a = 0, ll b = 0): x(a), y(b) {}
};

ll dist2(Point& p1, Point& p2) {
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

struct Node {
    Point point;
    Node* left;
    Node* right;
    int axis;
    Node(Point p, int a): point(p), left(nullptr), right(nullptr), axis(a) {}
};

Node* build(vector<Point>& points, int depth) {
    if (points.empty()) {
        return nullptr;
    }
    int axis = depth % 2;
    sort(points.begin(), points.end(), [axis](const Point& a, const Point& b) {
        return (axis == 0) ? a.x < b.x : a.y < b.y;
    });
    int mid = points.size() / 2;
    Node* node = new Node(points[mid], axis);
    vector<Point> lc(points.begin(), points.begin() + mid);
    vector<Point> rc(points.begin() + mid + 1, points.end());
    node->left = build(lc, depth + 1);
    node->right = build(rc, depth + 1);
    return node;
}

struct Res {
    Node* best_node;
    ll best_dist2;
    Res(): best_node(nullptr), best_dist2(-1ll) {};
};

void query(Node* root, Point& target, Res& res) {
    if (!root) {
        return;
    }
    ll d2 = dist2(root->point, target);
    if (!res.best_node || d2 < res.best_dist2) {
        res.best_node = root;
        res.best_dist2 = d2;
    }
    int axis = root->axis;
    ll diff = (axis == 0) ? (target.x - root->point.x) : (target.y - root->point.y);
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
        ll x, y;
        scanf("%lld %lld", &x, &y);
        points.push_back(Point(x, y));
    }
    Node* root = build(points, 0);
    int q;
    scanf("%d", &q);
    vector<Point> q_points;
    for (int i = 0; i < q; i++) {
        ll x, y;
        scanf("%lld %lld", &x, &y);
        q_points.push_back(Point(x, y));
    }
    for (int i = 0; i < q; i++) {
        Res res;
        query(root, q_points[i], res);
        printf("%lld\n", res.best_dist2);
    }
    return 0;
}
