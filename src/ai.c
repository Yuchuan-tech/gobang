#include "ai.h"
#include <stdlib.h>

// 估值权重（越大越好）
#define WIN_SCORE       1000000  // 五连
#define LIVE_FOUR        100000  // 活四
#define SLEEP_FOUR        10000  // 眠四
#define LIVE_THREE         5000  // 活三
#define SLEEP_THREE         500  // 眠三
#define LIVE_TWO             50  // 活二
#define SLEEP_TWO             5  // 眠二

// 四个方向
static const int dx[4] = { 1, 0, 1, 1 };
static const int dy[4] = { 0, 1, 1, -1 };

// ========== 工具函数 ==========

// 从某点沿方向数连续同色子（含自身）
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
}

// 评估某个空位对某方的价值
static int EvaluatePosition(Board* board, int x, int y, int piece) {
    if (board->grid[x][y] != EMPTY) return 0;
    int totalScore = 0;
    for (int dir = 0; dir < 4; dir++) {
        int count = CountDirection(board, x, y, dir, piece);
        if (count >= 5) totalScore += WIN_SCORE;
        else if (count == 4) totalScore += LIVE_FOUR;
        else if (count == 3) totalScore += LIVE_THREE;
        else if (count == 2) totalScore += LIVE_TWO;
        else if (count == 1) totalScore += 1;
    }
    return totalScore;
}

// 检查某空位落子后能否立即获胜
static int CanWinImmediately(Board* board, int x, int y, int piece) {
    board->grid[x][y] = piece;
    int win = 0;
    for (int dir = 0; dir < 4; dir++) {
        if (CountDirection(board, x, y, dir, piece) >= 5) {
            win = 1;
            break;
        }
    }
    board->grid[x][y] = EMPTY;
    return win;
}

// ========== 简单难度：纯随机 ==========
static void AISimpleMove(Board* board, int* x, int* y, int aiPiece) {
    (void)aiPiece;
    int emptyCount = 0;
    int emptyX[225], emptyY[225];

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board->grid[i][j] == EMPTY) {
                emptyX[emptyCount] = i;
                emptyY[emptyCount] = j;
                emptyCount++;
            }
        }
    }

    int idx = rand() % emptyCount;
    *x = emptyX[idx];
    *y = emptyY[idx];
}

// ========== 简单-中等难度：有估值意识但防守弱 ==========
static void AIEasyMediumMove(Board* board, int* x, int* y, int aiPiece) {
    int bestScore = -1;
    int bestX = -1, bestY = -1;
    int opponentPiece = (aiPiece == PIECE_BLACK) ? PIECE_WHITE : PIECE_BLACK;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board->grid[i][j] != EMPTY) continue;
            int attackScore = EvaluatePosition(board, i, j, aiPiece);
            int defendScore = EvaluatePosition(board, i, j, opponentPiece) * 3 / 10;
            int totalScore = attackScore + defendScore;
            if (totalScore > bestScore) {
                bestScore = totalScore;
                bestX = i;
                bestY = j;
            }
        }
    }

    if (bestX == -1 || bestScore < 50) {
        int emptyCount = 0;
        int emptyX[225], emptyY[225];
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board->grid[i][j] == EMPTY) {
                    emptyX[emptyCount] = i;
                    emptyY[emptyCount] = j;
                    emptyCount++;
                }
            }
        }
        int idx = rand() % emptyCount;
        bestX = emptyX[idx];
        bestY = emptyY[idx];
    }

    *x = bestX;
    *y = bestY;
}

// ========== 中等难度：攻守平衡估值 ==========
static void AIMediumMove(Board* board, int* x, int* y, int aiPiece) {
    int bestScore = -1;
    int bestX = -1, bestY = -1;
    int opponentPiece = (aiPiece == PIECE_BLACK) ? PIECE_WHITE : PIECE_BLACK;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board->grid[i][j] != EMPTY) continue;
            int attackScore = EvaluatePosition(board, i, j, aiPiece);
            int defendScore = EvaluatePosition(board, i, j, opponentPiece) * 8 / 10;
            int totalScore = attackScore + defendScore;
            if (totalScore > bestScore) {
                bestScore = totalScore;
                bestX = i;
                bestY = j;
            }
        }
    }

    if (bestX == -1) {
        do {
            bestX = rand() % BOARD_SIZE;
            bestY = rand() % BOARD_SIZE;
        } while (board->grid[bestX][bestY] != EMPTY);
    }

    *x = bestX;
    *y = bestY;
}

// ========== 中等-困难难度：估值 + 1步浅层搜索 ==========
static void AIMediumHardMove(Board* board, int* x, int* y, int aiPiece) {
    int bestScore = -99999999;
    int bestX = -1, bestY = -1;
    int opponentPiece = (aiPiece == PIECE_BLACK) ? PIECE_WHITE : PIECE_BLACK;

    // 先检查能否立即获胜
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board->grid[i][j] != EMPTY) continue;
            if (CanWinImmediately(board, i, j, aiPiece)) {
                *x = i;
                *y = j;
                return;
            }
        }
    }

    // 标记有棋子周围2格
    int searchMap[BOARD_SIZE][BOARD_SIZE] = {0};
    int hasPiece = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board->grid[i][j] != EMPTY) {
                hasPiece = 1;
                for (int di = -2; di <= 2; di++) {
                    for (int dj = -2; dj <= 2; dj++) {
                        int ni = i + di, nj = j + dj;
                        if (ni >= 0 && ni < BOARD_SIZE && nj >= 0 && nj < BOARD_SIZE) {
                            searchMap[ni][nj] = 1;
                        }
                    }
                }
            }
        }
    }

    if (!hasPiece) { *x = 7; *y = 7; return; }

    // 对候选位置用快速估值筛选前20个
    #define CANDIDATE_COUNT 225
    typedef struct { int x, y, score; } Candidate;
    Candidate candidates[CANDIDATE_COUNT];
    int candidateCount = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (!searchMap[i][j]) continue;
            if (board->grid[i][j] != EMPTY) continue;
            candidates[candidateCount].x = i;
            candidates[candidateCount].y = j;
            candidates[candidateCount].score = EvaluatePosition(board, i, j, aiPiece)
                                             + EvaluatePosition(board, i, j, opponentPiece);
            candidateCount++;
        }
    }

    // 部分排序取前20
    #define TOP_N_MH 20
    for (int i = 0; i < TOP_N_MH && i < candidateCount; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < candidateCount; j++) {
            if (candidates[j].score > candidates[maxIdx].score) maxIdx = j;
        }
        Candidate temp = candidates[i];
        candidates[i] = candidates[maxIdx];
        candidates[maxIdx] = temp;
    }

    int searchCount = (candidateCount < TOP_N_MH) ? candidateCount : TOP_N_MH;

    // 对前20个做深度1的Minimax搜索
    for (int k = 0; k < searchCount; k++) {
        int i = candidates[k].x;
        int j = candidates[k].y;

        board->grid[i][j] = aiPiece;
        int score = -99999999;
        
        // 模拟对手最优回应
        for (int oi = 0; oi < BOARD_SIZE; oi++) {
            for (int oj = 0; oj < BOARD_SIZE; oj++) {
                if (!searchMap[oi][oj]) continue;
                if (board->grid[oi][oj] != EMPTY) continue;
                
                board->grid[oi][oj] = opponentPiece;
                int oppScore = EvaluatePosition(board, oi, oj, opponentPiece);
                board->grid[oi][oj] = EMPTY;
                
                if (-oppScore > score) score = -oppScore;
            }
        }
        
        board->grid[i][j] = EMPTY;

        if (score > bestScore) {
            bestScore = score;
            bestX = i;
            bestY = j;
        }
    }

    if (bestX == -1) { *x = 7; *y = 7; }
    else { *x = bestX; *y = bestY; }
}

// ========== 困难难度：Minimax + Alpha-Beta 剪枝 ==========
static int Minimax(Board* board, int depth, int isMaximizing, int aiPiece, int alpha, int beta) {
    int opponentPiece = (aiPiece == PIECE_BLACK) ? PIECE_WHITE : PIECE_BLACK;
    int piece = isMaximizing ? aiPiece : opponentPiece;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board->grid[i][j] != EMPTY) continue;
            if (CanWinImmediately(board, i, j, piece)) {
                return isMaximizing ? WIN_SCORE + depth : -WIN_SCORE - depth;
            }
        }
    }

    if (depth == 0) {
        int score = 0;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board->grid[i][j] != EMPTY) continue;
                score += EvaluatePosition(board, i, j, aiPiece);
                score -= EvaluatePosition(board, i, j, opponentPiece);
            }
        }
        return score;
    }

    int bestScore = isMaximizing ? -99999999 : 99999999;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board->grid[i][j] != EMPTY) continue;

            board->grid[i][j] = piece;
            int score = Minimax(board, depth - 1, !isMaximizing, aiPiece, alpha, beta);
            board->grid[i][j] = EMPTY;

            if (isMaximizing) {
                if (score > bestScore) bestScore = score;
                if (bestScore > alpha) alpha = bestScore;
            } else {
                if (score < bestScore) bestScore = score;
                if (bestScore < beta) beta = bestScore;
            }
            if (alpha >= beta) break;
        }
        if (alpha >= beta) break;
    }

    return bestScore;
}

static void AIHardMove(Board* board, int* x, int* y, int aiPiece) {
    int bestScore = -99999999;
    int bestX = -1, bestY = -1;
    int opponentPiece = (aiPiece == PIECE_BLACK) ? PIECE_WHITE : PIECE_BLACK;

    int searchMap[BOARD_SIZE][BOARD_SIZE] = {0};
    int hasPiece = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board->grid[i][j] != EMPTY) {
                hasPiece = 1;
                for (int di = -2; di <= 2; di++) {
                    for (int dj = -2; dj <= 2; dj++) {
                        int ni = i + di, nj = j + dj;
                        if (ni >= 0 && ni < BOARD_SIZE && nj >= 0 && nj < BOARD_SIZE) {
                            searchMap[ni][nj] = 1;
                        }
                    }
                }
            }
        }
    }

    if (!hasPiece) { *x = 7; *y = 7; return; }

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (!searchMap[i][j]) continue;
            if (board->grid[i][j] != EMPTY) continue;
            if (CanWinImmediately(board, i, j, aiPiece)) {
                *x = i; *y = j; return;
            }
        }
    }

    #define MAX_CANDIDATES 225
    typedef struct { int x, y, score; } Candidate;
    Candidate candidates[MAX_CANDIDATES];
    int candidateCount = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (!searchMap[i][j]) continue;
            if (board->grid[i][j] != EMPTY) continue;
            int attackScore = EvaluatePosition(board, i, j, aiPiece);
            int defendScore = EvaluatePosition(board, i, j, opponentPiece);
            candidates[candidateCount].x = i;
            candidates[candidateCount].y = j;
            candidates[candidateCount].score = attackScore + defendScore;
            candidateCount++;
        }
    }

    #define TOP_N 15
    for (int i = 0; i < TOP_N && i < candidateCount; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < candidateCount; j++) {
            if (candidates[j].score > candidates[maxIdx].score) maxIdx = j;
        }
        Candidate temp = candidates[i];
        candidates[i] = candidates[maxIdx];
        candidates[maxIdx] = temp;
    }

    int searchCount = (candidateCount < TOP_N) ? candidateCount : TOP_N;

    for (int k = 0; k < searchCount; k++) {
        int i = candidates[k].x;
        int j = candidates[k].y;

        board->grid[i][j] = aiPiece;
        int score = Minimax(board, 2, 0, aiPiece, -99999999, 99999999);
        board->grid[i][j] = EMPTY;

        if (score > bestScore) {
            bestScore = score;
            bestX = i;
            bestY = j;
        }
    }

    if (bestX == -1) { *x = 7; *y = 7; }
    else { *x = bestX; *y = bestY; }
}

// ========== 统一入口 ==========
void AISmartMove(Board* board, int* x, int* y, int aiPiece, AIDifficulty difficulty) {
    switch (difficulty) {
        case AI_EASY:
            AISimpleMove(board, x, y, aiPiece);
            break;
        case AI_EASY_MEDIUM:
            AIEasyMediumMove(board, x, y, aiPiece);
            break;
        case AI_MEDIUM:
            AIMediumMove(board, x, y, aiPiece);
            break;
        case AI_MEDIUM_HARD:
            AIMediumHardMove(board, x, y, aiPiece);
            break;
        case AI_HARD:
            AIHardMove(board, x, y, aiPiece);
            break;
        default:
            AIMediumMove(board, x, y, aiPiece);
            break;
    }
}