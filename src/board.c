#include "board.h"

void InitBoard(Board* board) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board->grid[i][j] = EMPTY;
        }
    }
    board->historyTop = -1;  // 空栈
}

int IsValidMove(Board* board, int x, int y) {
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) return 0;
    return board->grid[x][y] == EMPTY;
}

int PlacePiece(Board* board, int x, int y, int piece) {
    if (!IsValidMove(board, x, y)) return 0;

    // 记录历史
    board->historyTop++;
    board->historyX[board->historyTop] = x;
    board->historyY[board->historyTop] = y;
    board->historyPiece[board->historyTop] = piece;

    board->grid[x][y] = piece;
    return 1;
}

int UndoMove(Board* board) {
    if (board->historyTop < 0) return 0;  // 无历史可悔棋

    // 取出上一步
    int x = board->historyX[board->historyTop];
    int y = board->historyY[board->historyTop];

    // 清空格子
    board->grid[x][y] = EMPTY;

    // 弹出栈
    board->historyTop--;

    return 1;
}