/*
 * 2022-05-08 21:07
 * 问题：个整数数组 nums1 和 nums2 ，请你以数组形式返回两数组的交集。
 * 进阶：
 * 如果给定的数组已经排好序呢？你将如何优化你的算法？
 * 如果 nums1 的大小比 nums2 小，哪种方法更优？
 * 如果 nums2 的元素存储在磁盘上，内存是有限的，并且你不能一次加载所有的元素到内存中，你该怎么办？
 * 
 * 思路1： 排序然后遍历两个数组，等价于两个有序链表合并
 * 思路2： 使用map，将短的数组元素放入map并计数，然后遍历第2个数组检测元素key是否存在
 * 优化：优先使用短的，如果所有元素都存在就可以结束了，不必要遍历长数组的全部。
 * 进阶： 这个依据上面思路分批次加载也是可以的。
 */

#include <stdio.h>
#include <stdlib.h>

#define OUTPUT(array, size) for(int i = 0;i < size; i++) printf("%d,", array[i]); printf("\n")

void sort(int array[], int size) {
    int i, j, key;
    for(i = 1; i < size; i++) {
        key = array[i];
        j = i - 1;
        while(j >= 0 && array[j] > key) {
            array[j+1] = array[j];
            j--;
        }
        array[j + 1] = key; //+1的原因while循环j=-1, array[j] <= key
    }
}

int* intersect(int* nums1, int nums1Size, int* nums2, int nums2Size, int* returnSize){
    if(nums1 == NULL || nums2 == NULL || nums1Size == 0 || nums2Size == 0) return NULL;

    int size = nums1Size > nums2Size ? nums2Size : nums1Size;
    int *ret = malloc(sizeof(int) * size);
    if (ret == NULL) return ret;
    *returnSize = 0;
    sort(nums1, nums1Size);
    sort(nums2, nums2Size);

    int i, j;
    i = j = 0;

    while(i < nums1Size && j < nums2Size) {
        if(nums1[i] == nums2[j]) {
            ret[*returnSize] = nums1[i];
            *returnSize = *returnSize + 1;
            i++;
            j++;
        } else if(nums1[i] > nums2[j]) {
            j++;
        } else {
            i++;
        }
    }
    return ret;
}

int main() {
    int a[] = {1,2,2,1};
    int b[] = {2,2};
    int size;
    int *ret = intersect(a, 4, b, 2, &size);
    OUTPUT(ret, size);
    if(ret) free(ret); 
}