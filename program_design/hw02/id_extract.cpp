#include <iostream>

using namespace std;

int main() {
    int id;
    cin >> id;
    int extracted_id = (id / 100) % 10000;
    cout << extracted_id << endl;
    return 0;
}