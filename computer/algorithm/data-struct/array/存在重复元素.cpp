/*
 * 2022-05-08 19:16
 * 问题：给你一个整数数组 nums 。如果任一值在数组中出现 至少两次 ，返回 true ；如果数组中每个元素互不相同，返回 false 
 * 思路1：排序，然后遍历；优化：排序过程就直接返回了
 * 思路2：直接遍历
 * 思路3：借助map，判断key存在
 * 思路4：bitmap check标志位
 */
#include <vector>
#include <map>

bool containsDuplicate1(int* nums, int numsSize){
    if(numsSize == 1) return false;
    int i, j;
    for(i = 0; i < numsSize; i++) {
        for(j = 0; j < i; j++) {
            if(nums[i] == nums[j])
                return true;
        }
    }
    return false;
}

bool containsDuplicate(vector<int>& nums) {
    if(nums.size() == 1) return false;
    map<int, bool> exist;
    for(int i = 0; i < nums.size(); i++) {
        if(exist[nums[i]]) return true;
        exist[nums[i]] = true;
    }
    return false;
}