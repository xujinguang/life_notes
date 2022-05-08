/*
 * 2022-05-08 15：37
 * 将数组中的元素向右轮转 k 个位置，其中 k 是非负数。
 * 本质是数据的偏移
 * 思路1：看成环
 * 思路2：扩容一倍，然后拷贝数据
 * 思路3：回环检测机制
 * 思路4：全部翻转，前半部翻转，后半部分翻转
 */

/*
 * 1. k = 0
 * 2. k = 1
 * 3. k > 1 && k < size
 * 4. k == size
 * 5. k > size
 * 2～5归为一类，求mod，本质把它看成环，然后重新计算位置
 */
#include <stdio.h>
void rotate0(int* nums, int numsSize, int k){
    k = k % numsSize;
    if (nums == NULL || numsSize <= 1 || k == 0) return;
    // 分配标志位
    int *flag = malloc(sizeof(int) * numsSize);

    int prev, curr;
    int i;
    // 遍历标志位
    for (int c = 0; c < numsSize; c++) {
        // 已处理
        if(flag[c] == 1)
            continue;
        // 下一个要处理的位置
        i = (c + k) % numsSize;
        // 要移动的第一个元素
        prev = nums[c];
        // 回环判断
        while(i != c) {
            // 标识已处理
            flag[i] = 1;
            // 被覆盖的位置数据
            curr = nums[i];
            // 旋转过来的数据
            nums[i] = prev;
            // 更新迭代
            prev = curr;
            i = (i + k) % numsSize;
        }
        // 最后归位
        nums[c] = prev;
        flag[c] = 1;
    }
    if (flag != NULL)
        free(flag);
}

// 空间复杂度降为O(1)需要判断回环检测， 从数学推导是k，n的gcd，可以用总个数简化
void rotate(int* nums, int numsSize, int k){
    k = k % numsSize;
    if (nums == NULL || numsSize <= 1 || k == 0) return;
    // 分配标志位
    int count;

    int prev, curr;
    int i;
    // 遍历标志位
    for (int c = 0; c < numsSize; c++) {
        // 下一个要处理的位置
        i = (c + k) % numsSize;
        // 要移动的第一个元素
        prev = nums[c];
        // 回环判断
        while(i != c) {
            count++;
            // 被覆盖的位置数据
            curr = nums[i];
            // 旋转过来的数据
            nums[i] = prev;
            // 更新迭代
            prev = curr;
            i = (i + k) % numsSize;
        }
        // 最后归位
        nums[c] = prev;
        count++;
        if(count == numsSize) break;
    }
}


// 思路2：使用扩展空间暂存数据
void rotate2(int* nums, int numsSize, int k){
    k = k % numsSize;
    if (nums == NULL || numsSize <= 1 || k == 0) return;
    int *ext = malloc(sizeof(int) * k);
    int i, j, n;
    i = numsSize - 1;
    j = numsSize - k - 1;
    n = k - 1;
    while(n >= 0) {
        ext[n] = nums[i];
        n--;
        i--;
    } 
    i = numsSize - 1;
    while(j >= 0) {
        nums[i] = nums[j];
        i--;
        j--;
    }

    n = 0;
    while(n < k) {
        nums[n] = ext[n];
        n++;
    }
    if (ext != NULL) free(ext);
}

void reverse(int* nums, int start, int end) {
    int tmp;
    while(start != end) {
        tmp = nums[end];
        nums[end] = nums[start];
        nums[start] = tmp;
        start ++;
        end --;
    }
}

void rotate3(int* nums, int numsSize, int k){
    // 0,1,2,3
    reverse(nums, 0, numsSize - 1); //0-3
    reverse(nums, 0, numsSize - 1 - k); //0-1
    reverse(nums, numsSize - k, numsSize - 1);//2-3
}