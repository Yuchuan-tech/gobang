#ifndef BOARD_H
#define BOARD_H

#define BOARD_SIZE 15
#define EMPTY 0
#define PIECE_BLACK 1
#define PIECE_WHITE 2

typedef struct {
    int grid[BOARD_SIZE][BOARD_SIZE];
    int historyX[225];      // 悔棋历史：x坐标
    int historyY[225];      // 悔棋历史：y坐标
    int historyPiece[225];  // 悔棋历史：棋子类型
    int historyTop;         // 栈顶指针
} Board;

void InitBoard(Board* board); //初始化15*15棋盘
int PlacePiece(Board* board, int x, int y, int piece); //放置棋子
int IsValidMove(Board* board, int x, int y); //检测是否可以落子
int UndoMove(Board* board);  // 悔棋：返回1表示成功

#endif


//定义基本数据类型