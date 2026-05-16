#include "stats.h"
#include "board.h"
#include <stdio.h>

#define STATS_FILE "gobang_stats.dat"

void LoadStats(Stats* stats) {
    FILE* fp = fopen(STATS_FILE, "rb");
    if (fp != NULL) {
        fread(stats, sizeof(Stats), 1, fp);
        fclose(fp);
    }
    else {
        stats->pvpBlackWins = 0;
        stats->pvpWhiteWins = 0;
        stats->pvpTotalGames = 0;
        stats->pvcPlayerWins = 0;
        stats->pvcComputerWins = 0;
        stats->pvcTotalGames = 0;//未找到历史文件,初始化
    }
}

void SaveStats(Stats* stats) {
    FILE* fp = fopen(STATS_FILE, "wb");
    if (fp != NULL) {
        fwrite(stats, sizeof(Stats), 1, fp);
        fclose(fp);
    }
}

void UpdateStatsPVP(Stats* stats, int winner) {
    if (winner == PIECE_BLACK) {
        stats->pvpBlackWins++;
    }
    else if (winner == PIECE_WHITE) {
        stats->pvpWhiteWins++;
    }
    stats->pvpTotalGames++;
    SaveStats(stats);
}

void UpdateStatsPVC(Stats* stats, int winner) {
    if (winner == PIECE_BLACK) {
        stats->pvcPlayerWins++;
    }
    else if (winner == PIECE_WHITE) {
        stats->pvcComputerWins++;
    }
    stats->pvcTotalGames++;
    SaveStats(stats);
}

void ResetStats(Stats* stats) {
    stats->pvpBlackWins = 0;
    stats->pvpWhiteWins = 0;
    stats->pvpTotalGames = 0;
    stats->pvcPlayerWins = 0;
    stats->pvcComputerWins = 0;
    stats->pvcTotalGames = 0;
    SaveStats(stats);
}