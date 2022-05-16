/*
 * 2022-05-09 19:45
 * 思路：对折交换数据
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xnhbqj/
 */
void reverseString(char* s, int sSize){
    int i = 0;
    int j = sSize - 1;
    char c;
    while(i < j) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
        i++;
        j--;
    }
}