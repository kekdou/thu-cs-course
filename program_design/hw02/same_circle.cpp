#include <iostream>
#include <cmath>
#include <cstdlib>

using namespace std;

struct Point {
    double x;
    double y;
    Point(double a = 0, double b = 0): x(a), y(b) {}
};

double getDistance(Point a, Point b) {
    return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}

double* getVerticalLine(Point a, Point b) {
    double* l = new double[3];
    double delta_x = a.x - b.x;
    double delta_y = a.y - b.y;
    Point mid = Point((a.x + b.x) / 2, (a.y + b.y) / 2);
    l[0] = delta_x;
    l[1] = delta_y;
    l[2] = mid.x * delta_x + mid.y * delta_y;
    return l;
}

Point getCross(double* l1, double* l2) {
    double delta = l1[0] * l2[1] - l2[0] * l1[1];
    if (fabs(delta) < 1e-9) {
        cout << "No" << endl;
        exit(0);
    } else {
        double x = (l2[1] * l1[2] - l1[1] * l2[2]) / delta;
        double y = (l1[0] * l2[2] - l2[0] * l1[2]) / delta;
        return Point(x, y);
    }
}

int main() {
    Point P[4];
    for (int i = 0; i < 4; i++) {
        double x, y;
        cin >> x >> y;
        P[i].x = x;
        P[i].y = y;
    }
    double* l1 = getVerticalLine(P[0], P[1]);
    double* l2 = getVerticalLine(P[1], P[2]);
    Point center = getCross(l1, l2);
    double eps = 1e-9;
    double r = getDistance(center, P[0]);
    if (fabs(getDistance(center, P[3]) - r) < eps) {
        cout << "Yes" << endl;
    } else {
        cout << "No" << endl;
    }
    delete[] l1;
    delete[] l2;
    return 0;
}