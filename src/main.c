#include "raylib.h"
#include "board.h"
#include "logic.h"
#include "stats.h"
#include <stdlib.h>
#include <time.h>
#include "ai.h"
#include "musicplayer.h"
#include "challenge.h"

typedef enum {
    MENU,
    DIFFICULTY_SELECT,
    SIDE_SELECT,
    CHALLENGE_INTRO,
    CHALLENGE_LEVEL,
    PLAYING,
    GAME_OVER
} GameState;

typedef enum {
    PVP,
    PVC,
    CHALLENGE
} GameMode;

typedef enum {
    PLAYER_FIRST,
    COMPUTER_FIRST
} FirstMove;

AIDifficulty aiDifficulty = AI_MEDIUM;
FirstMove firstMove = PLAYER_FIRST;

static bool IsMouseInRect(int x, int y, int width, int height) {
    Vector2 mouse = GetMousePosition();
    return (mouse.x >= x && mouse.x <= x + width && mouse.y >= y && mouse.y <= y + height);
}

static void StartPVCGame(GameMode* mode, Board* board, int* gameOver, int* currentPiece,
                          int* winner, int* statsUpdated, int* aiFirstMoveDone,
                          int* clickCooldown, FirstMove firstMoveSetting) {
    *mode = PVC;
    InitBoard(board);
    *gameOver = 0;
    *winner = EMPTY;
    *statsUpdated = 0;
    *aiFirstMoveDone = 0;
    *clickCooldown = 15;
    
    if (firstMoveSetting == PLAYER_FIRST) {
        *currentPiece = PIECE_BLACK;
    } else {
        *currentPiece = PIECE_WHITE;
    }
}

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 960;
    const int offsetX = 160;
    const int offsetY = 100;
    const int cellSize = 60;

    InitWindow(screenWidth, screenHeight, "Gobang - Five in a Row");
    SetTargetFPS(60);
    InitAudioDevice();

    InitBackgroundMusic("music/background.wav");

    Sound tapSound    = LoadSound("music/tap.wav");
    Sound selectSound = LoadSound("music/select.wav");
    Sound winSound    = LoadSound("music/win.wav");
    Sound loseSound   = LoadSound("music/lose.wav");
    Sound menuSound   = LoadSound("music/menu.wav");
    
    bool soundLoaded = (tapSound.frameCount > 0);

    srand((unsigned int)time(NULL));

    Board board;
    Stats stats;
    LoadStats(&stats);

    Challenge challenge;
    InitChallenge(&challenge);

    GameState state = MENU;
    GameMode mode = PVP;
    int gameOver = 0;
    int currentPiece = PIECE_BLACK;
    int winner = EMPTY;
    int selectedOption = 0;
    int selectedDifficulty = 1;
    int selectedSide = 0;
    int statsUpdated = 0;
    int waitForKeyRelease = 0;
    int aiFirstMoveDone = 0;
    int clickCooldown = 0;
    int challengeLevelShown = 0;
    int gameOverCooldown = 0;

    const int menuBtnX = 400, menuBtnW = 480, menuBtnH = 60;
    const int menuBtnGap = 80;
    int menuBtnY = 220;
    const int diffBtnX = 400, diffBtnY = 280, diffBtnW = 480, diffBtnH = 75;
    const int diffBtnGap = 95;
    const int sideBtnX = 380, sideBtnY = 320, sideBtnW = 520, sideBtnH = 80;
    const int sideBtnGap = 120;

    while (!WindowShouldClose())
    {
        UpdateBackgroundMusic();

        if (clickCooldown > 0) clickCooldown--;
        if (gameOverCooldown > 0) gameOverCooldown--;

        // ========== 菜单逻辑 ==========
        if (state == MENU) {
            if (IsKeyPressed(KEY_UP)) {
                selectedOption = (selectedOption + 2) % 3;
                if (soundLoaded) PlaySound(selectSound);
            }
            if (IsKeyPressed(KEY_DOWN)) {
                selectedOption = (selectedOption + 1) % 3;
                if (soundLoaded) PlaySound(selectSound);
            }

            if (IsMouseInRect(menuBtnX, menuBtnY, menuBtnW, menuBtnH)) {
                if (selectedOption != 0) { if (soundLoaded) PlaySound(selectSound); }
                selectedOption = 0;
            }
            if (IsMouseInRect(menuBtnX, menuBtnY + menuBtnGap, menuBtnW, menuBtnH)) {
                if (selectedOption != 1) { if (soundLoaded) PlaySound(selectSound); }
                selectedOption = 1;
            }
            if (IsMouseInRect(menuBtnX, menuBtnY + menuBtnGap * 2, menuBtnW, menuBtnH)) {
                if (selectedOption != 2) { if (soundLoaded) PlaySound(selectSound); }
                selectedOption = 2;
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (IsMouseInRect(menuBtnX, menuBtnY, menuBtnW, menuBtnH) ||
                    IsMouseInRect(menuBtnX, menuBtnY + menuBtnGap, menuBtnW, menuBtnH) ||
                    IsMouseInRect(menuBtnX, menuBtnY + menuBtnGap * 2, menuBtnW, menuBtnH)) {
                    if (soundLoaded) PlaySound(menuSound);
                    if (selectedOption == 0) {
                        mode = PVP;
                        InitBoard(&board);
                        gameOver = 0;
                        currentPiece = PIECE_BLACK;
                        winner = EMPTY;
                        statsUpdated = 0;
                        aiFirstMoveDone = 0;
                        clickCooldown = 15;
                        state = PLAYING;
                    } else if (selectedOption == 1) {
                        selectedDifficulty = 1;
                        state = DIFFICULTY_SELECT;
                        waitForKeyRelease = 1;
                    } else {
                        InitChallenge(&challenge);
                        state = CHALLENGE_INTRO;
                        waitForKeyRelease = 1;
                    }
                }
            }

            if (IsKeyPressed(KEY_ENTER)) {
                if (soundLoaded) PlaySound(menuSound);
                if (selectedOption == 0) {
                    mode = PVP;
                    InitBoard(&board);
                    gameOver = 0;
                    currentPiece = PIECE_BLACK;
                    winner = EMPTY;
                    statsUpdated = 0;
                    aiFirstMoveDone = 0;
                    clickCooldown = 15;
                    state = PLAYING;
                } else if (selectedOption == 1) {
                    selectedDifficulty = 1;
                    state = DIFFICULTY_SELECT;
                    waitForKeyRelease = 1;
                } else {
                    InitChallenge(&challenge);
                    state = CHALLENGE_INTRO;
                    waitForKeyRelease = 1;
                }
            }

            if (IsKeyPressed(KEY_R)) {
                ResetStats(&stats);
                if (soundLoaded) PlaySound(menuSound);
            }
        }

        // ========== 难度选择逻辑 ==========
        if (state == DIFFICULTY_SELECT) {
            if (waitForKeyRelease) {
                if (!IsKeyDown(KEY_ENTER) && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    waitForKeyRelease = 0;
                }
            }
            else {
                if (IsKeyPressed(KEY_UP)) {
                    selectedDifficulty = (selectedDifficulty + 2) % 3;
                    if (soundLoaded) PlaySound(selectSound);
                }
                if (IsKeyPressed(KEY_DOWN)) {
                    selectedDifficulty = (selectedDifficulty + 1) % 3;
                    if (soundLoaded) PlaySound(selectSound);
                }

                if (IsMouseInRect(diffBtnX, diffBtnY, diffBtnW, diffBtnH)) {
                    if (selectedDifficulty != 0) { if (soundLoaded) PlaySound(selectSound); }
                    selectedDifficulty = 0;
                }
                if (IsMouseInRect(diffBtnX, diffBtnY + diffBtnGap, diffBtnW, diffBtnH)) {
                    if (selectedDifficulty != 1) { if (soundLoaded) PlaySound(selectSound); }
                    selectedDifficulty = 1;
                }
                if (IsMouseInRect(diffBtnX, diffBtnY + diffBtnGap * 2, diffBtnW, diffBtnH)) {
                    if (selectedDifficulty != 2) { if (soundLoaded) PlaySound(selectSound); }
                    selectedDifficulty = 2;
                }

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    bool clicked = false;
                    if (IsMouseInRect(diffBtnX, diffBtnY, diffBtnW, diffBtnH)) {
                        selectedDifficulty = 0; clicked = true;
                    }
                    if (IsMouseInRect(diffBtnX, diffBtnY + diffBtnGap, diffBtnW, diffBtnH)) {
                        selectedDifficulty = 1; clicked = true;
                    }
                    if (IsMouseInRect(diffBtnX, diffBtnY + diffBtnGap * 2, diffBtnW, diffBtnH)) {
                        selectedDifficulty = 2; clicked = true;
                    }
                    if (clicked) {
                        if (soundLoaded) PlaySound(menuSound);
                        selectedSide = 0;
                        state = SIDE_SELECT;
                        waitForKeyRelease = 1;
                    }
                }

                if (IsKeyPressed(KEY_ENTER)) {
                    switch (selectedDifficulty) {
                        case 0: aiDifficulty = AI_EASY;    break;
                        case 1: aiDifficulty = AI_MEDIUM;  break;
                        case 2: aiDifficulty = AI_HARD;    break;
                    }
                    if (soundLoaded) PlaySound(menuSound);
                    selectedSide = 0;
                    state = SIDE_SELECT;
                    waitForKeyRelease = 1;
                }

                if (IsKeyPressed(KEY_BACKSPACE)) {
                    state = MENU;
                    if (soundLoaded) PlaySound(menuSound);
                }
            }
        }

        // ========== 先手选择逻辑 ==========
        if (state == SIDE_SELECT) {
            if (waitForKeyRelease) {
                if (!IsKeyDown(KEY_ENTER) && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    waitForKeyRelease = 0;
                }
            }
            else {
                if (IsKeyPressed(KEY_UP)) {
                    selectedSide = (selectedSide + 1) % 2;
                    if (soundLoaded) PlaySound(selectSound);
                }
                if (IsKeyPressed(KEY_DOWN)) {
                    selectedSide = (selectedSide + 1) % 2;
                    if (soundLoaded) PlaySound(selectSound);
                }

                if (IsMouseInRect(sideBtnX, sideBtnY, sideBtnW, sideBtnH)) {
                    if (selectedSide != 0) { if (soundLoaded) PlaySound(selectSound); }
                    selectedSide = 0;
                }
                if (IsMouseInRect(sideBtnX, sideBtnY + sideBtnGap, sideBtnW, sideBtnH)) {
                    if (selectedSide != 1) { if (soundLoaded) PlaySound(selectSound); }
                    selectedSide = 1;
                }

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (IsMouseInRect(sideBtnX, sideBtnY, sideBtnW, sideBtnH)) {
                        selectedSide = 0;
                        firstMove = PLAYER_FIRST;
                        if (soundLoaded) PlaySound(menuSound);
                        StartPVCGame(&mode, &board, &gameOver, &currentPiece, &winner, &statsUpdated, &aiFirstMoveDone, &clickCooldown, firstMove);
                        state = PLAYING;
                    }
                    if (IsMouseInRect(sideBtnX, sideBtnY + sideBtnGap, sideBtnW, sideBtnH)) {
                        selectedSide = 1;
                        firstMove = COMPUTER_FIRST;
                        if (soundLoaded) PlaySound(menuSound);
                        StartPVCGame(&mode, &board, &gameOver, &currentPiece, &winner, &statsUpdated, &aiFirstMoveDone, &clickCooldown, firstMove);
                        state = PLAYING;
                    }
                }

                if (IsKeyPressed(KEY_ENTER)) {
                    firstMove = (selectedSide == 0) ? PLAYER_FIRST : COMPUTER_FIRST;
                    if (soundLoaded) PlaySound(menuSound);
                    StartPVCGame(&mode, &board, &gameOver, &currentPiece, &winner, &statsUpdated, &aiFirstMoveDone, &clickCooldown, firstMove);
                    state = PLAYING;
                }

                if (IsKeyPressed(KEY_BACKSPACE)) {
                    state = DIFFICULTY_SELECT;
                    if (soundLoaded) PlaySound(menuSound);
                }
            }
        }

        // ========== 挑战模式介绍 ==========
        if (state == CHALLENGE_INTRO) {
            if (waitForKeyRelease) {
                if (!IsKeyDown(KEY_ENTER) && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    waitForKeyRelease = 0;
                }
            }
            else {
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (soundLoaded) PlaySound(menuSound);
                    mode = CHALLENGE;
                    InitChallenge(&challenge);
                    challenge.state = CHALLENGE_PLAYING;
                    challengeLevelShown = 0;
                    state = CHALLENGE_LEVEL;
                    waitForKeyRelease = 1;
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    state = MENU;
                    if (soundLoaded) PlaySound(menuSound);
                }
            }
        }

        // ========== 挑战关卡过渡 ==========
        if (state == CHALLENGE_LEVEL) {
            if (waitForKeyRelease) {
                if (!IsKeyDown(KEY_ENTER) && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    waitForKeyRelease = 0;
                }
            }
            else {
                if (challengeLevelShown == 0) {
                    challengeLevelShown = 90;
                }
                if (challengeLevelShown > 1) {
                    challengeLevelShown--;
                }
                else if (challengeLevelShown == 1) {
                    challengeLevelShown = 0;
                    InitBoard(&board);
                    gameOver = 0;
                    winner = EMPTY;
                    statsUpdated = 0;
                    aiFirstMoveDone = 0;
                    clickCooldown = 15;
                    currentPiece = challenge.playerPiece;
                    state = PLAYING;
                }
            }
        }

        // ========== 游戏逻辑 ==========
        if (state == PLAYING && !gameOver) {
            if (mode == PVP) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && clickCooldown == 0) {
                    Vector2 mouse = GetMousePosition();
                    int x = (int)((mouse.x - offsetX + cellSize / 2) / cellSize);
                    int y = (int)((mouse.y - offsetY + cellSize / 2) / cellSize);

                    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                        if (PlacePiece(&board, x, y, currentPiece)) {
                            if (soundLoaded) PlaySound(tapSound);
                            if (CheckWin(&board, x, y, currentPiece)) {
                                gameOver = 1;
                                winner = currentPiece;
                                state = GAME_OVER;
                                gameOverCooldown = 30;
                                if (soundLoaded) PlaySound(winSound);
                            } else {
                                currentPiece = (currentPiece == PIECE_BLACK) ? PIECE_WHITE : PIECE_BLACK;
                            }
                        }
                    }
                }

                if (IsKeyPressed(KEY_U)) {
                    if (UndoMove(&board)) {
                        currentPiece = (currentPiece == PIECE_BLACK) ? PIECE_WHITE : PIECE_BLACK;
                    }
                    if (UndoMove(&board)) {
                        currentPiece = (currentPiece == PIECE_BLACK) ? PIECE_WHITE : PIECE_BLACK;
                    }
                }
            }

            if (mode == PVC || mode == CHALLENGE) {
                AIDifficulty currentAIDifficulty;
                int playerPiece, aiPiece;
                
                if (mode == PVC) {
                    currentAIDifficulty = aiDifficulty;
                    playerPiece = (firstMove == PLAYER_FIRST) ? PIECE_BLACK : PIECE_WHITE;
                    aiPiece = (firstMove == PLAYER_FIRST) ? PIECE_WHITE : PIECE_BLACK;
                } else {
                    currentAIDifficulty = challenge.aiDifficulty;
                    playerPiece = challenge.playerPiece;
                    aiPiece = challenge.aiPiece;
                }

                // AI先手第一步
                if (!aiFirstMoveDone && currentPiece == playerPiece && 
                    ((mode == PVC && firstMove == COMPUTER_FIRST) ||
                     (mode == CHALLENGE && challenge.aiFirstMove))) {
                    aiFirstMoveDone = 1;
                    int x, y;
                    AISmartMove(&board, &x, &y, aiPiece, currentAIDifficulty);
                    PlacePiece(&board, x, y, aiPiece);
                    if (soundLoaded) PlaySound(tapSound);
                    currentPiece = playerPiece;
                }

                // 玩家落子
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && currentPiece == playerPiece && clickCooldown == 0) {
                    Vector2 mouse = GetMousePosition();
                    int x = (int)((mouse.x - offsetX + cellSize / 2) / cellSize);
                    int y = (int)((mouse.y - offsetY + cellSize / 2) / cellSize);

                    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                        if (PlacePiece(&board, x, y, currentPiece)) {
                            if (soundLoaded) PlaySound(tapSound);
                            if (CheckWin(&board, x, y, currentPiece)) {
                                gameOver = 1;
                                winner = currentPiece;
                                state = GAME_OVER;
                                gameOverCooldown = 30;
                                if (mode == CHALLENGE) {
                                    // 玩家赢了，进入下一关
                                    NextChallengeLevel(&challenge);
                                }
                                if (soundLoaded) PlaySound(winSound);
                            } else {
                                currentPiece = (currentPiece == PIECE_BLACK) ? PIECE_WHITE : PIECE_BLACK;
                            }
                        }
                    }
                }

                // AI落子
                if (!gameOver) {
                    if (currentPiece == aiPiece) {
                        static int delayCounter = 0;
                        delayCounter++;
                        if (delayCounter > 20) {
                            delayCounter = 0;
                            int x, y;
                            AISmartMove(&board, &x, &y, aiPiece, currentAIDifficulty);
                            if (PlacePiece(&board, x, y, aiPiece)) {
                                if (soundLoaded) PlaySound(tapSound);
                                if (CheckWin(&board, x, y, aiPiece)) {
                                    gameOver = 1;
                                    winner = aiPiece;
                                    state = GAME_OVER;
                                    gameOverCooldown = 30;
                                    if (mode == CHALLENGE) {
                                        // 玩家输了，重置挑战
                                        ResetChallenge(&challenge);
                                        challenge.state = CHALLENGE_WAITING;
                                    }
                                    if (soundLoaded) PlaySound(loseSound);
                                } else {
                                    currentPiece = (currentPiece == PIECE_BLACK) ? PIECE_WHITE : PIECE_BLACK;
                                }
                            }
                        }
                    }
                }
            }
        }

        // ========== 游戏结束统计 ==========
        if (state == GAME_OVER && !statsUpdated && gameOverCooldown == 0) {
            if (mode == PVP) {
                UpdateStatsPVP(&stats, winner);
            } else if (mode == PVC) {
                UpdateStatsPVC(&stats, winner);
            }
            statsUpdated = 1;
        }

        // ========== GAME_OVER 后的操作 ==========
        if (state == GAME_OVER && gameOverCooldown == 0) {
            if (mode == CHALLENGE) {
                if (challenge.state == CHALLENGE_COMPLETE) {
                    // 全部通关
                    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (soundLoaded) PlaySound(menuSound);
                        state = MENU;
                    }
                } else if (challenge.state == CHALLENGE_PLAYING) {
                    // 玩家赢了当前关，进入下一关
                    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (soundLoaded) PlaySound(menuSound);
                        challengeLevelShown = 0;
                        state = CHALLENGE_LEVEL;
                        waitForKeyRelease = 1;
                    }
                } else {
                    // 玩家输了
                    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (soundLoaded) PlaySound(menuSound);
                        InitChallenge(&challenge);
                        mode = CHALLENGE;
                        challenge.state = CHALLENGE_PLAYING;
                        challengeLevelShown = 0;
                        state = CHALLENGE_LEVEL;
                        waitForKeyRelease = 1;
                    }
                }
            } else {
                if (IsKeyPressed(KEY_R)) {
                    InitBoard(&board);
                    gameOver = 0;
                    winner = EMPTY;
                    statsUpdated = 0;
                    aiFirstMoveDone = 0;
                    clickCooldown = 0;
                    if (mode == PVC) {
                        currentPiece = (firstMove == PLAYER_FIRST) ? PIECE_BLACK : PIECE_WHITE;
                    } else {
                        currentPiece = PIECE_BLACK;
                    }
                    state = PLAYING;
                }
            }
        }

        if (state != MENU && state != DIFFICULTY_SELECT && state != SIDE_SELECT && 
            state != CHALLENGE_INTRO && state != CHALLENGE_LEVEL && state != GAME_OVER && 
            IsKeyPressed(KEY_BACKSPACE)) {
            state = MENU;
        }

        // ========== 渲染 ==========
        BeginDrawing();

        if (state == MENU) {
            ClearBackground((Color){ 245, 240, 230, 255 });
            DrawRectangle(0, 0, screenWidth, 130, (Color){ 40, 40, 50, 255 });
            DrawText("GOBANG", 460, 20, 65, (Color){ 220, 200, 120, 255 });
            DrawText("Five in a Row", 540, 85, 24, (Color){ 180, 170, 140, 255 });

            DrawRectangle(340, 200, 600, 360, (Color){ 255, 255, 255, 200 });
            DrawRectangleLines(340, 200, 600, 360, (Color){ 180, 160, 120, 255 });
            DrawText("Select Mode", 520, 225, 34, (Color){ 60, 50, 40, 255 });
            DrawLine(400, 270, 880, 270, (Color){ 180, 160, 120, 255 });

            Color pvpColor = (selectedOption == 0) ? (Color){ 200, 50, 50, 255 } : (Color){ 80, 80, 80, 255 };
            DrawRectangle(menuBtnX, menuBtnY, menuBtnW, menuBtnH, (Color){ 255, 255, 255, 150 });
            if (selectedOption == 0) DrawRectangleLines(menuBtnX, menuBtnY, menuBtnW, menuBtnH, (Color){ 200, 50, 50, 200 });
            DrawText("Player vs Player", menuBtnX + 50, menuBtnY + 12, 34, pvpColor);

            Color pvcColor = (selectedOption == 1) ? (Color){ 200, 50, 50, 255 } : (Color){ 80, 80, 80, 255 };
            DrawRectangle(menuBtnX, menuBtnY + menuBtnGap, menuBtnW, menuBtnH, (Color){ 255, 255, 255, 150 });
            if (selectedOption == 1) DrawRectangleLines(menuBtnX, menuBtnY + menuBtnGap, menuBtnW, menuBtnH, (Color){ 200, 50, 50, 200 });
            DrawText("Player vs Computer", menuBtnX + 50, menuBtnY + menuBtnGap + 12, 34, pvcColor);

            Color chalColor = (selectedOption == 2) ? (Color){ 200, 50, 50, 255 } : (Color){ 80, 80, 80, 255 };
            DrawRectangle(menuBtnX, menuBtnY + menuBtnGap * 2, menuBtnW, menuBtnH, (Color){ 255, 255, 255, 150 });
            if (selectedOption == 2) DrawRectangleLines(menuBtnX, menuBtnY + menuBtnGap * 2, menuBtnW, menuBtnH, (Color){ 200, 50, 50, 200 });
            DrawText("Challenge Mode", menuBtnX + 60, menuBtnY + menuBtnGap * 2 + 12, 34, chalColor);

            DrawText("UP/DOWN: Select    ENTER: Confirm    R: Reset Stats    Click: Select & Start",
                     240, 540, 18, (Color){ 140, 130, 120, 255 });

            DrawRectangle(340, 600, 600, 120, (Color){ 255, 255, 255, 200 });
            DrawRectangleLines(340, 600, 600, 120, (Color){ 180, 160, 120, 255 });
            DrawText("PVP Statistics", 530, 618, 28, (Color){ 60, 50, 40, 255 });
            if (stats.pvpTotalGames > 0) {
                DrawText(TextFormat("Black: %d wins (%.1f%%)    White: %d wins (%.1f%%)",
                    stats.pvpBlackWins, stats.pvpTotalGames > 0 ? (stats.pvpBlackWins * 100.0 / stats.pvpTotalGames) : 0,
                    stats.pvpWhiteWins, stats.pvpTotalGames > 0 ? (stats.pvpWhiteWins * 100.0 / stats.pvpTotalGames) : 0),
                    370, 660, 18, (Color){ 70, 70, 70, 255 });
                DrawText(TextFormat("Total: %d games", stats.pvpTotalGames), 540, 688, 18, (Color){ 100, 100, 100, 255 });
            } else {
                DrawText("No PVP games played yet", 490, 660, 18, (Color){ 160, 160, 160, 255 });
            }

            DrawRectangle(340, 745, 600, 120, (Color){ 255, 255, 255, 200 });
            DrawRectangleLines(340, 745, 600, 120, (Color){ 180, 160, 120, 255 });
            DrawText("PVC Statistics", 530, 763, 28, (Color){ 60, 50, 40, 255 });
            if (stats.pvcTotalGames > 0) {
                DrawText(TextFormat("You: %d wins (%.1f%%)    Computer: %d wins (%.1f%%)",
                    stats.pvcPlayerWins, stats.pvcTotalGames > 0 ? (stats.pvcPlayerWins * 100.0 / stats.pvcTotalGames) : 0,
                    stats.pvcComputerWins, stats.pvcTotalGames > 0 ? (stats.pvcComputerWins * 100.0 / stats.pvcTotalGames) : 0),
                    370, 805, 18, (Color){ 70, 70, 70, 255 });
                DrawText(TextFormat("Total: %d games", stats.pvcTotalGames), 540, 833, 18, (Color){ 100, 100, 100, 255 });
            } else {
                DrawText("No PVC games played yet", 490, 805, 18, (Color){ 160, 160, 160, 255 });
            }

            DrawRectangle(0, screenHeight - 70, screenWidth, 70, (Color){ 40, 40, 50, 255 });
            DrawText("ENTER: Start    UP/DOWN: Select    R: Reset Stats    ESC: Quit",
                     250, screenHeight - 50, 18, (Color){ 200, 190, 160, 255 });
            DrawText("Stats saved to gobang_stats.dat", 480, screenHeight - 25, 14, (Color){ 140, 130, 120, 255 });
        }
        else if (state == DIFFICULTY_SELECT) {
            ClearBackground((Color){ 40, 40, 50, 255 });
            DrawText("SELECT DIFFICULTY", 380, 80, 65, (Color){ 220, 200, 120, 255 });

            DrawRectangle(340, 200, 600, 440, (Color){ 255, 255, 255, 200 });
            DrawRectangleLines(340, 200, 600, 440, (Color){ 180, 160, 120, 255 });
            DrawText("Choose Difficulty Level", 450, 225, 32, (Color){ 60, 50, 40, 255 });
            DrawLine(400, 275, 880, 275, (Color){ 180, 160, 120, 255 });

            Color easyColor = (selectedDifficulty == 0) ? (Color){ 200, 50, 50, 255 } : (Color){ 80, 80, 80, 255 };
            DrawRectangle(diffBtnX, diffBtnY, diffBtnW, diffBtnH, (Color){ 255, 255, 255, 150 });
            if (selectedDifficulty == 0) DrawRectangleLines(diffBtnX, diffBtnY, diffBtnW, diffBtnH, (Color){ 200, 50, 50, 200 });
            DrawText("Easy", diffBtnX + 180, diffBtnY + 16, 36, easyColor);
            DrawText("Random moves, perfect for beginners", diffBtnX + 30, diffBtnY + 52, 20, (Color){ 120, 120, 120, 255 });

            Color mediumColor = (selectedDifficulty == 1) ? (Color){ 200, 50, 50, 255 } : (Color){ 80, 80, 80, 255 };
            DrawRectangle(diffBtnX, diffBtnY + diffBtnGap, diffBtnW, diffBtnH, (Color){ 255, 255, 255, 150 });
            if (selectedDifficulty == 1) DrawRectangleLines(diffBtnX, diffBtnY + diffBtnGap, diffBtnW, diffBtnH, (Color){ 200, 50, 50, 200 });
            DrawText("Medium", diffBtnX + 155, diffBtnY + diffBtnGap + 16, 36, mediumColor);
            DrawText("Balanced offense and defense", diffBtnX + 60, diffBtnY + diffBtnGap + 52, 20, (Color){ 120, 120, 120, 255 });

            Color hardColor = (selectedDifficulty == 2) ? (Color){ 200, 50, 50, 255 } : (Color){ 80, 80, 80, 255 };
            DrawRectangle(diffBtnX, diffBtnY + diffBtnGap * 2, diffBtnW, diffBtnH, (Color){ 255, 255, 255, 150 });
            if (selectedDifficulty == 2) DrawRectangleLines(diffBtnX, diffBtnY + diffBtnGap * 2, diffBtnW, diffBtnH, (Color){ 200, 50, 50, 200 });
            DrawText("Hard", diffBtnX + 180, diffBtnY + diffBtnGap * 2 + 16, 36, hardColor);
            DrawText("Look-ahead strategy, challenging", diffBtnX + 50, diffBtnY + diffBtnGap * 2 + 52, 20, (Color){ 120, 120, 120, 255 });

            DrawText("ENTER: Next Step    BACKSPACE: Back    Click: Select & Continue",
                     250, 670, 20, (Color){ 180, 170, 140, 255 });

            DrawRectangle(0, screenHeight - 70, screenWidth, 70, (Color){ 40, 40, 50, 255 });
            DrawText("ENTER: Choose Side    UP/DOWN: Select Difficulty    BACKSPACE: Return",
                     230, screenHeight - 50, 20, (Color){ 200, 190, 160, 255 });
        }
        else if (state == SIDE_SELECT) {
            ClearBackground((Color){ 40, 40, 50, 255 });
            DrawText("SELECT SIDE", 420, 80, 65, (Color){ 220, 200, 120, 255 });
            
            const char* diffName;
            switch (selectedDifficulty) {
                case 0: diffName = "Easy"; break;
                case 1: diffName = "Medium"; break;
                case 2: diffName = "Hard"; break;
                default: diffName = "Medium"; break;
            }
            DrawText(TextFormat("Difficulty: %s", diffName), 540, 150, 24, (Color){ 180, 170, 140, 255 });

            DrawRectangle(300, 200, 680, 350, (Color){ 255, 255, 255, 200 });
            DrawRectangleLines(300, 200, 680, 350, (Color){ 180, 160, 120, 255 });
            DrawText("Who goes first?", 450, 225, 36, (Color){ 60, 50, 40, 255 });
            DrawLine(360, 280, 920, 280, (Color){ 180, 160, 120, 255 });

            Color playerFirstColor = (selectedSide == 0) ? (Color){ 200, 50, 50, 255 } : (Color){ 80, 80, 80, 255 };
            DrawRectangle(sideBtnX, sideBtnY, sideBtnW, sideBtnH, (Color){ 255, 255, 255, 150 });
            if (selectedSide == 0) DrawRectangleLines(sideBtnX, sideBtnY, sideBtnW, sideBtnH, (Color){ 200, 50, 50, 200 });
            DrawText("I'll go first (Black)", sideBtnX + 60, sideBtnY + 20, 34, playerFirstColor);

            Color compFirstColor = (selectedSide == 1) ? (Color){ 200, 50, 50, 255 } : (Color){ 80, 80, 80, 255 };
            DrawRectangle(sideBtnX, sideBtnY + sideBtnGap, sideBtnW, sideBtnH, (Color){ 255, 255, 255, 150 });
            if (selectedSide == 1) DrawRectangleLines(sideBtnX, sideBtnY + sideBtnGap, sideBtnW, sideBtnH, (Color){ 200, 50, 50, 200 });
            DrawText("Computer goes first (Black)", sideBtnX + 40, sideBtnY + sideBtnGap + 20, 34, compFirstColor);

            DrawText("UP/DOWN: Select    ENTER: Start Game    BACKSPACE: Back    Click: Select & Start",
                     170, 580, 20, (Color){ 180, 170, 140, 255 });

            DrawRectangle(0, screenHeight - 70, screenWidth, 70, (Color){ 40, 40, 50, 255 });
            DrawText("ENTER: Start Game    UP/DOWN: Select Side    BACKSPACE: Return",
                     270, screenHeight - 50, 20, (Color){ 200, 190, 160, 255 });
        }
        else if (state == CHALLENGE_INTRO) {
            ClearBackground((Color){ 40, 40, 50, 255 });
            DrawText("CHALLENGE MODE", 370, 80, 60, (Color){ 220, 200, 120, 255 });

            DrawRectangle(240, 200, 800, 450, (Color){ 255, 255, 255, 200 });
            DrawRectangleLines(240, 200, 800, 450, (Color){ 180, 160, 120, 255 });
            DrawText("Challenge Rules", 480, 225, 36, (Color){ 60, 50, 40, 255 });
            DrawLine(300, 275, 980, 275, (Color){ 180, 160, 120, 255 });

            DrawText("Defeat 5 AI opponents in a row to win!", 310, 300, 26, DARKGRAY);
            DrawText("If you lose any match, you start over from Level 1.", 310, 340, 26, DARKGRAY);

            DrawLine(300, 385, 980, 385, (Color){ 180, 160, 120, 255 });

            DrawText("Level 1: Novice AI      (Easy)", 350, 410, 24, (Color){ 60, 50, 40, 255 });
            DrawText("Level 2: Apprentice AI  (Easy-Medium)", 350, 445, 24, (Color){ 60, 50, 40, 255 });
            DrawText("Level 3: Skilled AI     (Medium)", 350, 480, 24, (Color){ 60, 50, 40, 255 });
            DrawText("Level 4: Expert AI      (Medium-Hard)", 350, 515, 24, (Color){ 60, 50, 40, 255 });
            DrawText("Level 5: Master AI      (Hard)", 350, 550, 24, (Color){ 60, 50, 40, 255 });

            DrawText("ENTER: Start Challenge    BACKSPACE: Back to Menu",
                     260, 680, 22, (Color){ 180, 170, 140, 255 });

            DrawRectangle(0, screenHeight - 70, screenWidth, 70, (Color){ 40, 40, 50, 255 });
            DrawText("ENTER: Begin Challenge    BACKSPACE: Return to Menu",
                     290, screenHeight - 50, 20, (Color){ 200, 190, 160, 255 });
        }
        else if (state == CHALLENGE_LEVEL) {
            ClearBackground((Color){ 40, 40, 50, 255 });
            DrawText(GetChallengeLevelDesc(challenge.currentLevel), 300, 350, 45, (Color){ 220, 200, 120, 255 });
            if (challenge.aiFirstMove) {
                DrawText("Computer plays first (Black)", 380, 420, 30, (Color){ 180, 170, 140, 255 });
            } else {
                DrawText("You play first (Black)", 420, 420, 30, (Color){ 180, 170, 140, 255 });
            }
            DrawText("Get ready...", 520, 500, 28, (Color){ 200, 200, 200, 255 });
        }
        else {
            ClearBackground((Color){ 245, 240, 230, 255 });

            DrawRectangle(offsetX - 35, offsetY - 35,
                         (BOARD_SIZE - 1) * cellSize + 70,
                         (BOARD_SIZE - 1) * cellSize + 70,
                         (Color){ 220, 180, 140, 255 });

            for (int i = 0; i < BOARD_SIZE; i++) {
                DrawLine(offsetX, offsetY + i * cellSize,
                         offsetX + (BOARD_SIZE - 1) * cellSize, offsetY + i * cellSize,
                         (Color){ 60, 40, 20, 255 });
                DrawLine(offsetX + i * cellSize, offsetY,
                         offsetX + i * cellSize, offsetY + (BOARD_SIZE - 1) * cellSize,
                         (Color){ 60, 40, 20, 255 });
            }

            int starPoints[5][2] = {{7, 7}, {3, 3}, {11, 3}, {3, 11}, {11, 11}};
            for (int i = 0; i < 5; i++) {
                DrawCircle(offsetX + starPoints[i][0] * cellSize,
                          offsetY + starPoints[i][1] * cellSize, 8, (Color){ 60, 40, 20, 255 });
            }

            for (int i = 0; i < BOARD_SIZE; i++) {
                for (int j = 0; j < BOARD_SIZE; j++) {
                    int px = offsetX + i * cellSize;
                    int py = offsetY + j * cellSize;
                    int r = cellSize / 2 - 3;
                    if (board.grid[i][j] == PIECE_BLACK) {
                        DrawCircle(px, py, r, (Color){ 30, 30, 30, 255 });
                        DrawCircle(px - 6, py - 6, r - 7, (Color){ 80, 80, 80, 255 });
                    } else if (board.grid[i][j] == PIECE_WHITE) {
                        DrawCircle(px, py, r, (Color){ 240, 240, 240, 255 });
                        DrawCircleLines(px, py, r, (Color){ 160, 160, 160, 255 });
                        DrawCircle(px - 5, py - 5, r - 7, (Color){ 255, 255, 255, 255 });
                    }
                }
            }

            DrawRectangle(0, 0, screenWidth, 60, (Color){ 40, 40, 50, 255 });
            if (mode == PVP) {
                DrawText(TextFormat("PVP | Black: %d wins  White: %d wins  Total: %d games",
                    stats.pvpBlackWins, stats.pvpWhiteWins, stats.pvpTotalGames),
                    30, 15, 22, (Color){ 200, 190, 160, 255 });
            } else if (mode == PVC) {
                const char* diffText;
                switch (aiDifficulty) {
                    case AI_EASY: diffText = "Easy"; break;
                    case AI_MEDIUM: diffText = "Medium"; break;
                    case AI_HARD: diffText = "Hard"; break;
                    default: diffText = "Unknown"; break;
                }
                const char* sideText = (firstMove == PLAYER_FIRST) ? "You first" : "Computer first";
                DrawText(TextFormat("PVC (%s, %s) | You: %d wins  Computer: %d wins  Total: %d games",
                    diffText, sideText, stats.pvcPlayerWins, stats.pvcComputerWins, stats.pvcTotalGames),
                    30, 15, 22, (Color){ 200, 190, 160, 255 });
            } else {
                DrawText(TextFormat("CHALLENGE - Level %d/5", challenge.currentLevel),
                    30, 15, 22, (Color){ 220, 200, 120, 255 });
            }

            if (state == PLAYING) {
                if (mode == PVP) {
                    const char* turnText = (currentPiece == PIECE_BLACK) ? "BLACK'S TURN" : "WHITE'S TURN";
                    Color turnColor = (currentPiece == PIECE_BLACK) ? (Color){ 50, 50, 50, 255 } : (Color){ 200, 200, 200, 255 };
                    DrawRectangle(30, screenHeight - 110, 280, 45, (Color){ 255, 255, 255, 180 });
                    DrawRectangleLines(30, screenHeight - 110, 280, 45, (Color){ 180, 160, 120, 255 });
                    DrawText(turnText, 50, screenHeight - 100, 28, turnColor);
                } else {
                    int myPiece = (mode == PVC) ? 
                        ((firstMove == PLAYER_FIRST) ? PIECE_BLACK : PIECE_WHITE) : 
                        challenge.playerPiece;
                    const char* turnText = (currentPiece == myPiece) ? "YOUR TURN" : "COMPUTER THINKING...";
                    DrawRectangle(30, screenHeight - 110, 370, 45, (Color){ 255, 255, 255, 180 });
                    DrawRectangleLines(30, screenHeight - 110, 370, 45, (Color){ 180, 160, 120, 255 });
                    DrawText(turnText, 50, screenHeight - 100, 28, (Color){ 60, 50, 40, 255 });
                }
            } else if (state == GAME_OVER) {
                const char* winText;
                if (mode == CHALLENGE) {
                    if (challenge.state == CHALLENGE_COMPLETE) {
                        winText = "Congratulations! You win!";
                    } else if (challenge.state == CHALLENGE_PLAYING) {
                        winText = TextFormat("Level %d Clear!", challenge.currentLevel - 1);
                    } else {
                        winText = "You Lose! Try Again!";
                    }
                } else if (winner == PIECE_BLACK) {
                    winText = (mode == PVP) ? "BLACK WINS!" : "YOU WIN!";
                } else {
                    winText = (mode == PVP) ? "WHITE WINS!" : "COMPUTER WINS!";
                }
                int textWidth = MeasureText(winText, 50);
                DrawRectangle(screenWidth/2 - textWidth/2 - 40, screenHeight - 170, textWidth + 80, 80,
                             (Color){ 255, 255, 255, 220 });
                DrawRectangleLines(screenWidth/2 - textWidth/2 - 40, screenHeight - 170, textWidth + 80, 80,
                                  (Color){ 200, 50, 50, 255 });
                DrawText(winText, screenWidth/2 - textWidth/2, screenHeight - 155, 50, (Color){ 200, 50, 50, 255 });

                if (mode == CHALLENGE && gameOverCooldown == 0) {
                    DrawText("Press ENTER or Click to continue",
                             screenWidth/2 - 150, screenHeight - 80, 20, (Color){ 180, 170, 140, 255 });
                }
            }

            if (state != GAME_OVER) {
                DrawText("U: Undo    Backspace: Menu    R: Restart",
                         30, screenHeight - 145, 18, (Color){ 120, 120, 120, 255 });
            }
        }

        EndDrawing();
    }

    CloseBackgroundMusic();
    if (soundLoaded) {
        UnloadSound(tapSound);
        UnloadSound(selectSound);
        UnloadSound(winSound);
        UnloadSound(loseSound);
        UnloadSound(menuSound);
    }
    CloseAudioDevice();
    CloseWindow();
    return 0;
}