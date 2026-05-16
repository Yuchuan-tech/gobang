#include "raylib.h"
#include "board.h"
#include "logic.h"
#include "stats.h"
#include <stdlib.h>
#include <time.h>

#include "ai.h"

typedef enum {
    MENU,
    PLAYING,
    GAME_OVER
} GameState;//管理游戏状态

typedef enum {
    PVP,
    PVC
} GameMode;//管理游戏模式

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 700;
    const int offsetX = 50;
    const int offsetY = 50;
    const int cellSize = 40;//左上角是原点,向右x正向,向下y正向

    InitWindow(screenWidth, screenHeight, "Gobang");
    SetTargetFPS(60);
    InitAudioDevice();//启动音效,暂时没用,等你扩展

    srand((unsigned int)time(NULL));

    Board board;//落子只是修改棋盘矩阵状态,每帧都重新按矩阵画
    Stats stats;
    LoadStats(&stats);

    GameState state = MENU;
    GameMode mode = PVP;
    int gameOver = 0;//严格意义上是if_gameover
    int currentPiece = PIECE_BLACK;//永远黑先手
    int winner = EMPTY;
    int selectedOption = 0;//辅助选模式
    int statsUpdated = 0;//辅助更新(if_updated)

    while (!WindowShouldClose())
    {
        //逻辑区,为后续绘画提供实参,隐性数据
        // ========== 菜单逻辑 ==========
        if (state == MENU) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
                selectedOption = (selectedOption + 1) % 2;
            }//上下键选模式

            if (IsKeyPressed(KEY_ENTER)) {
                mode = (selectedOption == 0) ? PVP : PVC;
                InitBoard(&board);
                gameOver = 0;
                currentPiece = PIECE_BLACK;
                winner = EMPTY;
                statsUpdated = 0;
                state = PLAYING;
            }//enter确定模式

            if (IsKeyPressed(KEY_R)) {
                ResetStats(&stats);
            }//r清零历史数据(warning,不建议手欠)
        }

        // ========== 游戏逻辑 ==========
        if (state == PLAYING && !gameOver) {
            // 人人对战
            if (mode == PVP) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Vector2 mouse = GetMousePosition();
                    int x = (int)((mouse.x - offsetX + cellSize / 2) / cellSize);
                    int y = (int)((mouse.y - offsetY + cellSize / 2) / cellSize);//利用整数除法,每个落子位周围小范围都算落在其上

                    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                        if (PlacePiece(&board, x, y, currentPiece)) {
                            if (CheckWin(&board, x, y, currentPiece)) {
                                gameOver = 1;
                                winner = currentPiece;
                                state = GAME_OVER;
                            }
                            else {
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
                }//悔棋,两边都悔,利用栈后进先出,回溯弹出每一步
            }

            // 人机对战
            if (mode == PVC) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Vector2 mouse = GetMousePosition();
                    int x = (int)((mouse.x - offsetX + cellSize / 2) / cellSize);
                    int y = (int)((mouse.y - offsetY + cellSize / 2) / cellSize);

                    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                        if (PlacePiece(&board, x, y, currentPiece)) {
                            if (CheckWin(&board, x, y, currentPiece)) {
                                gameOver = 1;
                                winner = currentPiece;
                                state = GAME_OVER;
                            }
                            else {
                                currentPiece = PIECE_WHITE;
                            }
                        }
                    }
                }

                // 电脑落子（智能）
                if (!gameOver && currentPiece == PIECE_WHITE) {
                    static int delayCounter = 0;
                    delayCounter++;
                    if (delayCounter > 20) {
                        delayCounter = 0;

                        int x, y;
                        AISmartMove(&board, &x, &y, PIECE_WHITE);

                        if (PlacePiece(&board, x, y, PIECE_WHITE)) {
                            if (CheckWin(&board, x, y, PIECE_WHITE)) {
                                gameOver = 1;
                                winner = PIECE_WHITE;
                                state = GAME_OVER;
                            }
                            else {
                                currentPiece = PIECE_BLACK;
                            }
                        }
                    }
                }

                if (IsKeyPressed(KEY_U)) {
                    if (UndoMove(&board)) {
                        currentPiece = PIECE_WHITE;
                    }
                    if (UndoMove(&board)) {
                        currentPiece = PIECE_BLACK;
                    }
                }
            }
        }

        // ========== 游戏结束更新统计 ==========
        if (state == GAME_OVER && !statsUpdated) {
            if (mode == PVP) {
                UpdateStatsPVP(&stats, winner);
            }
            else {
                UpdateStatsPVC(&stats, winner);
            }
            statsUpdated = 1;
        }

        // ========== 重启 ==========
        if (state == GAME_OVER && IsKeyPressed(KEY_R)) {
            InitBoard(&board);
            gameOver = 0;
            currentPiece = PIECE_BLACK;
            winner = EMPTY;
            statsUpdated = 0;
            state = PLAYING;
        }

        // 返回菜单
        if (state != MENU && IsKeyPressed(KEY_BACKSPACE)) {
            state = MENU;
        }//退格回到菜单

        // ========== 绘制 ==========
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (state == MENU) {
            DrawText("GOBANG", 340, 50, 40, DARKGRAY);
            DrawText("Select Mode:", 330, 120, 25, DARKGRAY);

            if (selectedOption == 0) {
                DrawText("> Player vs Player <", 290, 170, 25, RED);
                DrawText("  Player vs Computer  ", 290, 210, 25, DARKGRAY);
            }
            else {
                DrawText("  Player vs Player  ", 290, 170, 25, DARKGRAY);
                DrawText("> Player vs Computer <", 290, 210, 25, RED);
            }//高亮显示当前模式选择

            // PVP 统计
            DrawText("--- PVP Statistics ---", 310, 270, 18, DARKGRAY);
            if (stats.pvpTotalGames > 0) {
                DrawText(TextFormat("Black: %d wins (%.1f%%)",
                    stats.pvpBlackWins,
                    stats.pvpTotalGames > 0 ? (stats.pvpBlackWins * 100.0 / stats.pvpTotalGames) : 0),
                    290, 300, 16, DARKGRAY);
                DrawText(TextFormat("White: %d wins (%.1f%%)",
                    stats.pvpWhiteWins,
                    stats.pvpTotalGames > 0 ? (stats.pvpWhiteWins * 100.0 / stats.pvpTotalGames) : 0),
                    290, 320, 16, DARKGRAY);
                DrawText(TextFormat("Total: %d games", stats.pvpTotalGames), 290, 340, 16, DARKGRAY);
            }
            else {
                DrawText("No PVP games played yet", 290, 300, 16, LIGHTGRAY);
            }//胜率和胜数,总场数

            // PVC 统计
            DrawText("--- PVC Statistics ---", 310, 390, 18, DARKGRAY);
            if (stats.pvcTotalGames > 0) {
                DrawText(TextFormat("You: %d wins (%.1f%%)",
                    stats.pvcPlayerWins,
                    stats.pvcTotalGames > 0 ? (stats.pvcPlayerWins * 100.0 / stats.pvcTotalGames) : 0),
                    290, 420, 16, DARKGRAY);
                DrawText(TextFormat("Computer: %d wins (%.1f%%)",
                    stats.pvcComputerWins,
                    stats.pvcTotalGames > 0 ? (stats.pvcComputerWins * 100.0 / stats.pvcTotalGames) : 0),
                    290, 440, 16, DARKGRAY);
                DrawText(TextFormat("Total: %d games", stats.pvcTotalGames), 290, 460, 16, DARKGRAY);
            }
            else {
                DrawText("No PVC games played yet", 290, 420, 16, LIGHTGRAY);
            }

            DrawText("ENTER:Start  UP/DOWN:Select  R:Reset Stats", 230, 530, 16, LIGHTGRAY);
            DrawText("(Stats saved to gobang_stats.dat)", 280, 560, 14, LIGHTGRAY);
        }//人类胜率,胜数,总场数
        else {
            // 绘制棋盘
            for (int i = 0; i < BOARD_SIZE; i++) {
                DrawLine(offsetX, offsetY + i * cellSize,
                    offsetX + (BOARD_SIZE - 1) * cellSize, offsetY + i * cellSize, BLACK);//(x1,y1,x2,y2)
                DrawLine(offsetX + i * cellSize, offsetY,
                    offsetX + i * cellSize, offsetY + (BOARD_SIZE - 1) * cellSize, BLACK);
            }

            DrawCircle(offsetX + 7 * cellSize, offsetY + 7 * cellSize, 5, DARKGRAY);//中央更大一点
            DrawCircle(offsetX + 3 * cellSize, offsetY + 3 * cellSize, 4, DARKGRAY);
            DrawCircle(offsetX + 11 * cellSize, offsetY + 3 * cellSize, 4, DARKGRAY);
            DrawCircle(offsetX + 3 * cellSize, offsetY + 11 * cellSize, 4, DARKGRAY);
            DrawCircle(offsetX + 11 * cellSize, offsetY + 11 * cellSize, 4, DARKGRAY);//五个星点

            for (int i = 0; i < BOARD_SIZE; i++) {
                for (int j = 0; j < BOARD_SIZE; j++) {
                    if (board.grid[i][j] == PIECE_BLACK) {
                        DrawCircle(offsetX + i * cellSize, offsetY + j * cellSize, cellSize / 2 - 2, BLACK);
                    }
                    else if (board.grid[i][j] == PIECE_WHITE) {
                        DrawCircle(offsetX + i * cellSize, offsetY + j * cellSize, cellSize / 2 - 2, WHITE);
                        DrawCircleLines(offsetX + i * cellSize, offsetY + j * cellSize, cellSize / 2 - 2, GRAY);
                    }
                }
            }

            // 显示统计
            if (mode == PVP) {
                DrawText(TextFormat("PVP - Black: %d  White: %d  (Total: %d)",
                    stats.pvpBlackWins, stats.pvpWhiteWins, stats.pvpTotalGames),
                    20, 20, 16, DARKGRAY);
            }
            else {
                float playerRate = stats.pvcTotalGames > 0 ?
                    (stats.pvcPlayerWins * 100.0f / stats.pvcTotalGames) : 0;
                DrawText(TextFormat("PVC - You: %d (%.1f%%)  Computer: %d  (Total: %d)",
                    stats.pvcPlayerWins, playerRate, stats.pvcComputerWins, stats.pvcTotalGames),
                    20, 20, 16, DARKGRAY);
            }

            if (state == PLAYING) {
                if (mode == PVP) {
                    DrawText(currentPiece == PIECE_BLACK ? "Black's turn" : "White's turn",
                        20, screenHeight - 30, 20, DARKGRAY);
                    DrawText("U:Undo  Bksp:Menu", 20, screenHeight - 60, 16, DARKGRAY);
                }
                else {
                    DrawText(currentPiece == PIECE_BLACK ? "Your turn" : "Computer thinking...",
                        20, screenHeight - 30, 20, DARKGRAY);
                    DrawText("U:Undo  Bksp:Menu", 20, screenHeight - 60, 16, DARKGRAY);
                }
            }
            else if (state == GAME_OVER) {
                if (mode == PVP) {
                    DrawText(winner == PIECE_BLACK ? "Black wins!" : "White wins!",
                        20, screenHeight - 30, 20, RED);
                }
                else {
                    DrawText(winner == PIECE_BLACK ? "You win!" : "Computer wins!",
                        20, screenHeight - 30, 20, RED);
                }
                DrawText("Press R to restart  Bksp:Menu", 20, screenHeight - 60, 16, DARKGRAY);
            }
        }

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}