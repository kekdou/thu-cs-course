#include <iostream>
#include <vector>

using namespace std;

void Sieve(vector<bool>& is_prime) {
    is_prime[0] = is_prime[1] = 0;
    for (int p = 2; p * p <= 200; p++) {
        if (is_prime[p]) {
            for (int i = p * p; i <= 200; i += p) {
                is_prime[i] = 0;
            }
        }
    }
}

bool CanPlace(int r, int c, int num, vector<vector<int>>& board, vector<bool>& is_prime) {
    if (r > 0 && !is_prime[num + board[r - 1][c]]) {
        return 0;
    }
    if (c > 0 && !is_prime[num + board[r][c - 1]]) {
        return 0;
    }
    if ((r + c) % 2 == 0) {
        if (num % 2 == 0) {
            return 0;
        }
    } else {
        if (num % 2 != 0) {
            return 0;
        }
    }
    return 1;
}

bool Solve(int index, int N, int M, vector<vector<int>>& board, vector<bool>& used, vector<bool>& is_prime) {
    if (index == N * M) {
        return 1;
    }
    int r = index / M;
    int c = index % M;
    for (int i = 1; i <= N * M; i++) {
        if (!used[i]) {
            if (CanPlace(r, c, i, board, is_prime)) {
                board[r][c] = i;
                used[i] = 1;
                if (Solve(index + 1, N, M, board, used, is_prime)) {
                    return 1;
                }
                used[i] = 0;
            }
        }
    }
    return 0;
}

int main() {
    int N, M;
    cin >> N >> M;
    vector<bool> is_prime(201, 1); 
    Sieve(is_prime);
    vector<vector<int>> board(N, vector<int>(M));
    vector<bool> used(101, 0);
    if (Solve(0, N, M, board, used, is_prime)) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                cout << board[i][j] << " ";
            }
            cout << endl;
        }
    }
    return 0;
}