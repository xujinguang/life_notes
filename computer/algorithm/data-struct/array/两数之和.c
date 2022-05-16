/**
给定一个整数数组 nums 和一个整数目标值 target，请你在该数组中找出 和为目标值 target  的那 两个 整数，并返回它们的数组下标。

你可以假设每种输入只会对应一个答案。但是，数组中同一个元素在答案里不能重复出现。

你可以按任意顺序返回答案。

作者：力扣 (LeetCode)
链接：https://leetcode.cn/leetbook/read/top-interview-questions-easy/x2jrse/
来源：力扣（LeetCode）
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

 * Note: The returned array must be malloced, assume caller calls free().
 * 思路1: 暴力循环 O(n)
 * 思路2: 排序后遍历，加折半查找
 * 思路3: 遍历构建map，然后遍历差值是否在map中，map记录的是位置索引
 */
int* twoSum(int* nums, int numsSize, int target, int* returnSize){
    int i, j;
    int *ret = malloc(sizeof(int) * 2);
    *returnSize = 2;
    for(i = 0; i < numsSize; i++) {
        for(j = i + 1; j < numsSize; j++) {
            if(nums[i] + nums[j] == target) {
                ret[0] = i;
                ret[1] = j;
                return ret;
            }
        }
    }
    return NULL;
}