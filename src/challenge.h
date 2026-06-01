#ifndef CHALLENGE_H
#define CHALLENGE_H

#include "board.h"
#include "ai.h"

typedef enum {
    CHALLENGE_WAITING,
    CHALLENGE_PLAYING,
    CHALLENGE_WIN,
    CHALLENGE_LOSE,
    CHALLENGE_COMPLETE
} ChallengeState;

typedef struct {
    int currentLevel;
    int totalLevels;
    ChallengeState state;
    AIDifficulty aiDifficulty;
    int aiFirstMove;
    int playerPiece;
    int aiPiece;
} Challenge;

void InitChallenge(Challenge* challenge);
AIDifficulty GetChallengeAIDifficulty(int level);
void NextChallengeLevel(Challenge* challenge);
void ResetChallenge(Challenge* challenge);
const char* GetChallengeLevelDesc(int level);

#endif