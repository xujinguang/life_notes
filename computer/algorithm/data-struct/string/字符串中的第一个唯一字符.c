/*
 * 2022-05-10 11:11
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xn5z8r/
 * 思路1: 遍历搜索 O(n^2)
 * 思路2: 第一遍遍历，计数各个字母到nr[26]；第2遍遍历字符串，检查第一个为1的字母
 * 思路3: 上面可以用map代替, 当字母的个数不确定的时候，比如unicode
 * 思路4: 修改原字符串，相同字母，则设置为.,最后遍历第一个不是.的位置
 */

#include <stdio.h>

int firstUniqChar1(char * s){
    if(s == NULL) return -1;
    int i = 0;
    int j = 0;
    int exist = 0;
    while(s[i] != '\0') {
        j = i + 1;
        exist = 0;
        while(s[j] != '\0') {
            if(s[i] == s[j]) {
                s[j] = '.';
                exist++;
            }
            j++;
        }
        if(exist) {
            s[i] = '.';
        }
        i++;
    }
    i = 0;
    while(s[i] != '\0' && s[i] == '.') i++;
    if(s[i] == '\0') return -1;
    return i;
}

int firstUniqChar(char * s){
    if(s == NULL) return -1;
    int i = 0;
    int exist[26] = {0};
    while(s[i] != '\0') {
        exist[s[i] - 'a'] ++;
        i++;
    }
    i = 0;
    while(s[i] != '\0' && exist[s[i] - 'a'] != 1) i++;
    if(s[i] == '\0') return -1;
    return i;
}

int main() {
    char test[] = "test";
    int ret = firstUniqChar(test);
    printf("%d:%s", ret, test);
}