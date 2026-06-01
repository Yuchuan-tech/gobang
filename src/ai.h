#ifndef AI_H
#define AI_H

#include "board.h"

typedef enum {
    AI_EASY,          // 简单：纯随机
    AI_EASY_MEDIUM,   // 简单-中等：轻度估值
    AI_MEDIUM,        // 中等：攻守平衡
    AI_MEDIUM_HARD,   // 中等-困难：估值 + 浅层搜索
    AI_HARD           // 困难：深度搜索
} AIDifficulty;

void AISmartMove(Board* board, int* x, int* y, int aiPiece, AIDifficulty difficulty);

#endif