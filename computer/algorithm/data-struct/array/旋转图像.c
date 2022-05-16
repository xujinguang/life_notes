/*
 * 2022-05-09 17:31
 * 给定一个 n × n 的二维矩阵 matrix 表示一个图像。请你将图像顺时针旋转 90 度。
 * 思路1: 强撸，这个不好设置索引，容易把自己转晕。分层
 * 思路2: 先上下对换，然后再对角交换
 */

#include <stdio.h>

void rotate(int** matrix, int matrixSize, int* matrixColSize){
    int i, j, tmp;
    for(i = 0; i < matrixSize/2; i++) {
        /*
        for(j = 0; j < *matrixColSize; j++) {
            tmp = matrix[i][j];
            matrix[i][j] = matrix[matrixSize - i - 1][j];
            matrix[matrixSize - i - 1][j] = tmp;
        }
        */
       int *ptmp = matrix[i];
       matrix[i] = matrix[matrixSize - i - 1];
       matrix[matrixSize - i - 1] = ptmp;
    }

    for(i = 0; i < matrixSize; i++) {
        for(j = i + 1; j < *matrixColSize; j++) {
            tmp = matrix[i][j];
            matrix[i][j] = matrix[j][i];
            matrix[j][i] = tmp;
        }
    }
}

int main() {
    int a[3][3], b[4][4];
    int tmp;
    tmp=3;
    rotate(a, 3, &tmp);
    printf("\n");
    tmp = 4;
    rotate(b, 4, &tmp);
}