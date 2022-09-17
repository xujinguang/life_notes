/*
 * 2022-05-09 19:45
 * 思路：对折交换数据
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xnx13t/
 * 考察溢出的判断，负数不用关心
 */

#include <stdio.h>
#include <limits.h>

int reverse(int x){
    if(x <= INT_MIN || x > INT_MAX) return 0;
    if(x < 10 && x > -10) return x;
    long long r = 0;
    int flag = x > 0;
    x = flag ? x : -x;
    while(x) {
        r = r * 10 + x % 10;
        x = x / 10;
        if(r > INT_MAX) return 0;
    }
    r = flag ? r : -r;
    if (r < INT_MIN) return 0;
    return r;
}

int reverse(int x){
    if(x <= INT_MIN || x > INT_MAX) return 0;
    if(x < 10 && x > -10) return x;
    long long r = 0;
    while(x) {
        r = r * 10 + x % 10;
        x = x / 10;
        if(r > INT_MAX || r < INT_MIN) return 0;
    }
    return r;
}

int reverse(int x) {
    int res = 0;
    while (x != 0) {
        int t = x % 10;
        int newRes = res * 10 + t;
        //如果数字溢出，直接返回0
        if ((newRes - t) / 10 != res)
            return 0;
        res = newRes;
        x = x / 10;
    }
    return res;
}

int main() {
    printf("%d\n", reverse(123));
    printf("%d\n", reverse(2147483647));
}