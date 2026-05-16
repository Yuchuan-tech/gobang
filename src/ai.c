#include "ai.h"
#include <stdlib.h>

// 棋型权值表(越大越下)
#define WIN_SCORE       1000000  // 连五
#define LIVE_FOUR       100000   // 活四
#define SLEEP_FOUR       10000   // 冲四
#define LIVE_THREE        5000   // 活三
#define SLEEP_THREE        500   // 眠三
#define LIVE_TWO            50   // 活二
#define SLEEP_TWO             5   // 眠二

// 方向数组(米字)
static const int dx[4] = { 1, 0, 1, 1 };
static const int dy[4] = { 0, 1, 1, -1 };

static int CountDirection(Board* board, int x, int y, int dir, int piece) {
    int count = 1;

    for (int step = 1; step <= 5; step++) {
        int nx = x + dx[dir] * step;
        int ny = y + dy[dir] * step;
        if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) break;
        if (board->grid[nx][ny] == piece) count++;
        else break;
    }
    
    for (int step = 1; step <= 5; step++) {
        int nx = x - dx[dir] * step;
        int ny = y - dy[dir] * step;
        if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) break;
        if (board->grid[nx][ny] == piece) count++;
        else break;
    }
    return count;
}//不包括自己(肯定空着才下)

// 评估分数
static int EvaluatePosition(Board* board, int x, int y, int piece) {
    if (board->grid[x][y] != EMPTY) return 0;

    int totalScore = 0;

    for (int dir = 0; dir < 4; dir++) {
        int count = CountDirection(board, x, y, dir, piece);

        //根据连续棋子数给分
        if (count >= 5) totalScore += WIN_SCORE;
        else if (count == 4) totalScore += LIVE_FOUR;
        else if (count == 3) totalScore += LIVE_THREE;
        else if (count == 2) totalScore += LIVE_TWO;
        else if (count == 1) totalScore += 1;
    }

    return totalScore;
}

void AISmartMove(Board* board, int* x, int* y, int aiPiece) {
    int bestScore = -1;
    int bestX = -1, bestY = -1;
    int opponentPiece = (aiPiece == PIECE_BLACK) ? PIECE_WHITE : PIECE_BLACK;

    // 每个空位评估一次
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board->grid[i][j] != EMPTY) continue;

            // 进攻分
            int attackScore = EvaluatePosition(board, i, j, aiPiece);
            // 防守分
            int defendScore = EvaluatePosition(board, i, j, opponentPiece) * 8 / 10;  // 防守权重0.8,大小改变策略,可以试试0,只攻不防

            int totalScore = attackScore + defendScore;

            if (totalScore > bestScore) {
                bestScore = totalScore;
                bestX = i;
                bestY = j;
            }//记录要下的空位
        }
    }

    // 如果没找到合适位置随机找一个空位(不存在,除非先手)
    if (bestX == -1) {
        do {
            bestX = rand() % BOARD_SIZE;
            bestY = rand() % BOARD_SIZE;
        } while (board->grid[bestX][bestY] != EMPTY);
    }

    *x = bestX;
    *y = bestY;
}