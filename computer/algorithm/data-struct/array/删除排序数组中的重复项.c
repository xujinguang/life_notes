/*
 * 2022-05-08 14:57
 * 给你一个 升序排列 的数组 nums ，请你 原地 删除重复出现的元素，使每个元素 只出现一次，
 * 返回删除后数组的新长度。元素的 相对顺序 应该保持 一致 。
 */

#include <stdio.h>

int removeDuplicates(int* nums, int numsSize){
    int i = 0, j = 0;
    if(nums == NULL || numsSize <= 1) {
        return numsSize;
    }

    while(i < numsSize) {
        if (j == i || nums[i] == nums[j]) {
            i++;
        }  else { // j!=i && nums[i] != nums[j]
            j++;
            nums[j] = nums[i];
        }
    }
    return j + 1;
}

#define SIZE(array) sizeof(array)/sizeof(int)

int main() {
    int a[] = {1,2}, b[] = {1,1}, c[] = {1,1,1,2,2,3};
    int ret;
    ret = removeDuplicates(a, SIZE(a));
    printf("%d\n", ret);
    ret = removeDuplicates(b, SIZE(b));
    printf("%d\n", ret);
    ret = removeDuplicates(c, SIZE(c));
    printf("%d\n", ret);
}
