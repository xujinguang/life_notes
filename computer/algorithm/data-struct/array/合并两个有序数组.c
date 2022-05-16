/**
 * 2022-05-11 15:12
 * 给你两个按 非递减顺序 排列的整数数组 nums1 和 nums2，另有两个整数 m 和 n ，分别表示 nums1 和 nums2 中的元素数目。
 * 请你 合并 nums2 到 nums1 中，使合并后的数组同样按 非递减顺序 排列。
 * 链接：https://leetcode.cn/leetbook/read/top-interview-questions-easy/xnumcr/
 * 思路1：将数组1拷贝出来，然后两个指针遍历
 * 思路2: 三个指针，分别指向数组1，数组2当数组2插入数组1时，数组1的值拷贝末尾指针的位置上
 * 思路3: 这个是归并排序的最后一步。或者先合并后再排序一次。
 */

// 思路1
void merge(int* nums1, int nums1Size, int m, int* nums2, int nums2Size, int n){
    int p1 = 0;
    int p2 = 0;
    int *val = malloc(sizeof(int) * (nums1Size));
    int p = 0;

    while(p1 < m && p2 < n) {
        if(nums1[p1] < nums2[p2]) {
            val[p++] = nums1[p1++];
        } else if (nums1[p1] > nums2[p2]){
            val[p++] = nums2[p2++];
        } else {
            val[p++] = nums1[p1++];
            val[p++] = nums2[p2++];
        }
    }

    while(p1 < m) {
        val[p++] = nums1[p1++];
    }

    while(p2 < n) {
        val[p++] = nums2[p2++];
    }

    // copy
    memcpy(nums1, val, sizeof(int) * nums1Size);
    free(val);
}