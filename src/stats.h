#ifndef STATS_H
#define STATS_H

typedef struct {
    // PVP 统计
    int pvpBlackWins;
    int pvpWhiteWins;
    int pvpTotalGames;   // 总对局数

    // PVC 统计
    int pvcPlayerWins;   // 玩家胜
    int pvcComputerWins; // 电脑胜
    int pvcTotalGames;   // 总对局数
} Stats;

void LoadStats(Stats* stats); //读取历史数据
void SaveStats(Stats* stats); //保存当前数据
void UpdateStatsPVP(Stats* stats, int winner);  //更新PVP数据
void UpdateStatsPVC(Stats* stats, int winner);  //更新PVC数据
void ResetStats(Stats* stats); //清空历史

#endif

//长期记忆数据