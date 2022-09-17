/*
 * 2022-05-08 19:55
 * 问题： 给定一个非空整数数组，除了某个元素只出现一次以外，其余每个元素均出现两次。找出那个只出现了一次的元素。
 * 思路1：排序后检测
 * 思路2：map统计每个词的个数
 * 思路3：执行异或运算， 因为两个相同的数异或后等于0
 */
int singleNumber(int* nums, int numsSize){
    int only = nums[0];
    int i = 1;
    while(i < numsSize) {
        only ^= nums[i];
        i++;
    }
    return only;
}

//这个并不比上面快，原因是内存访问的顺序化
int singleNumber(int* nums, int numsSize){
    for(; numsSize > 1; numsSize--) {
        nums[0] ^= nums[numsSize-1];
    }
    return nums[0];
}