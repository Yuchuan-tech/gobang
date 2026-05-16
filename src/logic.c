#include "logic.h"

static int CountDirection(Board* board, int x, int y, int dx, int dy, int piece) {
    int count = 0;
    for (int step = 1; step <= 5; step++) {
        int nx = x + dx * step;
        int ny = y + dy * step;
        if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) break;
        if (board->grid[nx][ny] == piece) count++;
        else break;
    }
    return count;
}//静态函数,检测某方向棋子数目,不包括自己

int CheckWin(Board* board, int lastX, int lastY, int piece) {
    int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

    for (int i = 0; i < 4; i++) {
        int dx = directions[i][0];
        int dy = directions[i][1];

        int total = 1;
        total += CountDirection(board, lastX, lastY, dx, dy, piece);
        total += CountDirection(board, lastX, lastY, -dx, -dy, piece);

        if (total >= 5) return 1;
    }
    return 0;
}//检测八方四面个数,到达五个即胜利