/*
 * 2022-05-08 22:09
 * 问题： 
 * 给定一个由 整数 组成的 非空 数组所表示的非负整数，在该数的基础上加一。
 * 最高位数字存放在数组的首位， 数组中每个元素只存储单个数字。
 * 你可以假设除了整数 0 之外，这个整数不会以零开头。
 * 
 * 思路：这就是一个大整数计算，主要解决进位问题，尤其是当第一位进位时，需要重新分配内存
 */
int* plusOne(int* digits, int digitsSize, int* returnSize){
    int idx, flag;
    idx = digitsSize - 1;
    flag = 1;
    do {
        digits[idx] += flag;
        if(digits[idx] == 10) {
            digits[idx] = 0;
            flag = 1;
        } else {
            flag = 0;
        }
        idx--;
    } while(idx >= 0 && flag);

    *returnSize = digitsSize;
    if(flag) {
        *returnSize += 1;
        int *ret = malloc(sizeof(int) * (*returnSize));
        ret[0] = 1;
        for(idx = 0; idx < digitsSize; idx++) {
            ret[idx + 1] = digits[idx];
        }
        return ret;
    } else {
        return digits;
    }
}

int* plusOne(int* digits, int digitsSize, int* returnSize){
    int idx, flag;
    idx = digitsSize - 1;
    flag = 1;
    while(idx >= 0 && flag) {
        if(digits[idx] == 9) { // 相比上面少了一次多次+计算
            digits[idx] = 0;
            flag = 1;
        } else {
            digits[idx] += 1;
            flag = 0;
            break;
        }
        idx--;
    }

    *returnSize = digitsSize;
    if(flag) {
        *returnSize += 1;
        int *ret = malloc(sizeof(int) * (*returnSize));
        ret[0] = 1;
        for(idx = 0; idx < digitsSize; idx++) {
            ret[idx + 1] = digits[idx];
        }
        return ret;
    } else {
        return digits;
    }
}
/*
执行结果： 通过 显示详情 执行用时： 0 ms , 在所有 C 提交中击败了 100.00% 的用户
内存消耗： 6 MB , 在所有 C 提交中击败了 5.06% 的用户
通过测试用例： 111 / 111
*/

int* plusOne(int* digits, int digitsSize, int* returnSize){
    int idx;
    idx = digitsSize - 1;
    while(idx >= 0) {
        if(digits[idx] == 9) { // 相比上面少了一次多次+计算
            digits[idx] = 0;
        } else {
            digits[idx] += 1;
            break;
        }
        idx--;
    }

    *returnSize = digitsSize;
    if(idx == -1) { // 使用索引值, 减少一个变量
        *returnSize += 1;
        int *ret = malloc(sizeof(int) * (*returnSize));
        ret[0] = 1;
        for(idx = 0; idx < digitsSize; idx++) {
            ret[idx + 1] = digits[idx];
        }
        return ret;
    } else {
        return digits;
    }
}
