/*
2022-05-09 15:11

由范围 [0,n] 内所有整数组成的 n + 1 个整数的排列序列可以表示为长度为 n 的字符串 s ，其中:
如果 perm[i] < perm[i + 1] ，那么 s[i] == 'I' 
如果 perm[i] > perm[i + 1] ，那么 s[i] == 'D' 
链接：https://leetcode.cn/problems/di-string-match

输入：s = "IDID"
输出：[0,4,1,3,2]

思路1：
s[i] = 'I', a[i+1] = i + 1
s[i] = 'D', a[i+1] = a[i], 回溯s[0...i] 连续的'D'，a[i]++

测试：
"I"
"D"
"III"
"DDD"
"IDID"
"DIDI"
"IIDD"
"DDII"

总结：
1. for数组索引一定要判断上下界
2. malloc一定要memset初始化，因为不同环境下编译器不一定初始化内存
3. 事前：初始化；事中：数组边界；事后：收尾处理。
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int* diStringMatch(char * s, int* returnSize){
    int len = -1;
    while(s[++len] != '\0');
    if(len == 0) return NULL;
    int *perm = malloc(sizeof(int) * (len + 1));
    memset(perm, 0, sizeof(int) * (len + 1));
    int i = 0;
    int j = 0;
    for(i = 0; i < len; i++) {
        if (s[i] == 'I') {
            perm[i+1] = i + 1;
        } else {
            perm[i+1] = perm[i];
            for(j = i; j >= 0 && s[j] == 'D'; j--)
                perm[j]++;
        }
    }
    *returnSize = len + 1;
    return perm;
}

int main() {
    char *s[8] ={
        "I",
        "D",
        "III",
        "DDD",
        "IDID",
        "DIDI",
        "IIDD",
        "DDII"
    };
    for(int j = 0; j < 8; j++) {
        int size;
        int *ret = diStringMatch(s[j], &size);
        for(int i = 0; i < size; i++) {
            printf("%d,", ret[i]);
        }
        printf("\n");
        if(ret) free(ret);
    }
    return 0;
}