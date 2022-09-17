/*
 * 2022-05-09 17:02
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/x2f9gg/
 */

bool isValidSudoku(char** board, int boardSize, int* boardColSize){
    int flag = 0;
    int i, j, m, n;
    for(i = 0; i < boardSize; i++) {
        flag = 0;
        for(j = 0; j < *boardColSize; j++) {
            if(board[i][j] == '.') continue;
            if (flag & (1 << (board[i][j] - '0'))) return false;
            else flag |= (1 << (board[i][j] - '0'));
        }
    }
    for(j = 0; j < *boardColSize; j++) {
        flag = 0;
        for(i = 0; i < boardSize; i++) {
            if(board[i][j] == '.') continue;
            if (flag & (1 << (board[i][j] - '0'))) return false;
            else flag |= (1 << (board[i][j] - '0'));
        }
    }
    // 这个很关键，虽然四层循环，其实依然是n方，并没有增加
    for(i = 0; i < boardSize; i += 3) {
        for(j = 0; j < *boardColSize; j += 3) {
            flag = 0;
            for(m = 0; m < 3; m++) {
                for(n = 0; n < 3; n++) {
                    if(board[i + m][j + n] == '.') continue;
                    if (flag & (1 << (board[i + m][j + n] - '0'))) return false;
                    else flag |= (1 << (board[i + m][j + n] - '0'));
                }
            }
        }
    }
    return true;
}