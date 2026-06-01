#include "challenge.h"
#include <stdlib.h>

void InitChallenge(Challenge* challenge) {
    challenge->currentLevel = 1;
    challenge->totalLevels = 5;
    challenge->state = CHALLENGE_WAITING;
    challenge->aiDifficulty = GetChallengeAIDifficulty(1);
    challenge->aiFirstMove = 0;
    challenge->playerPiece = PIECE_BLACK;
    challenge->aiPiece = PIECE_WHITE;
}

AIDifficulty GetChallengeAIDifficulty(int level) {
    switch (level) {
        case 1: return AI_EASY;
        case 2: return AI_EASY_MEDIUM;
        case 3: return AI_MEDIUM;
        case 4: return AI_MEDIUM_HARD;
        case 5: return AI_HARD;
        default: return AI_MEDIUM;
    }
}

void NextChallengeLevel(Challenge* challenge) {
    challenge->currentLevel++;
    if (challenge->currentLevel > challenge->totalLevels) {
        challenge->state = CHALLENGE_COMPLETE;
    } else {
        challenge->aiDifficulty = GetChallengeAIDifficulty(challenge->currentLevel);
        challenge->state = CHALLENGE_PLAYING;
        challenge->aiFirstMove = (challenge->currentLevel % 2 == 0) ? 1 : 0;
        if (challenge->aiFirstMove) {
            challenge->playerPiece = PIECE_WHITE;
            challenge->aiPiece = PIECE_BLACK;
        } else {
            challenge->playerPiece = PIECE_BLACK;
            challenge->aiPiece = PIECE_WHITE;
        }
    }
}

void ResetChallenge(Challenge* challenge) {
    challenge->currentLevel = 1;
    challenge->aiDifficulty = GetChallengeAIDifficulty(1);
    challenge->state = CHALLENGE_WAITING;
    challenge->aiFirstMove = 0;
    challenge->playerPiece = PIECE_BLACK;
    challenge->aiPiece = PIECE_WHITE;
}

const char* GetChallengeLevelDesc(int level) {
    switch (level) {
        case 1: return "Level 1: Novice AI (Easy)";
        case 2: return "Level 2: Apprentice AI (Easy-Medium)";
        case 3: return "Level 3: Skilled AI (Medium)";
        case 4: return "Level 4: Expert AI (Medium-Hard)";
        case 5: return "Level 5: Master AI (Hard)";
        default: return "Unknown";
    }
}