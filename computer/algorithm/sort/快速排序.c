/*
 * 2022-05-09 11:28
 * 快速排序的最坏运行情况是 O(n²)，比如说顺序数列的快排。但它的平摊期望时间是 O(nlogn)，且 O(nlogn) 记号中隐含的常数因子很小，
 * 比复杂度稳定等于 O(nlogn) 的归并排序要小很多。所以，对绝大多数顺序性较弱的随机数列而言，快速排序总是优于归并排序。
 * 算法：
 *  1. 从数列中挑选一个基准（pivot）;
 *  2. 重新排列数列，小于基准值的放在基准之前，大于基准值的放在基准后面，基准就处于最终位置上，这个称为分区（partition）操作；
 *  3. 递归地（recursive）把小于基准值元素的子数列和大于基准值元素的子数列排序；
 * 与归并算法的区别：
 * 1.都使用了分而治之的算法思想：大问题划分为子问题
 * 2.归并排序是先解决子问题，然后归并起来解决大问题，从下而上
 * 3.快速排序是从上而下
 */
#include <stdio.h>

// 辅助操作：数据交换 swap
void swap(int array[], int left, int right) {
    if(left == right) return;
    int tmp;
    tmp = array[left];
    array[left] = array[right];
    array[right] = tmp;
}

void quick_sort(int array[], int left, int right) {
    if (right <= left) return;
    int idx = left; // 以第一个元素为基准

    for(int i = left + 1; i <= right; i++) { // 从left+1到right重排
        if (array[i] < array[left]) {
            swap(array, ++idx, i); // 分区操作：讲小于基准的值交换到前面
        }
    }
    // 分区遍历完成，idx是分割点
    swap(array, left, idx); // 基准归位
    quick_sort(array, left, idx - 1);
    quick_sort(array, idx + 1, right);
}

void quick_sort2(int array[], int left, int right) {
    if (right <= left) return;
    int key = array[left];
    int start = left;
    int end = right;
    while(start < end) {
        while(start < end && array[end] > key) {
            end--;
        }
        array[start] = array[end];
        while(start < end && array[start] < key) {
            start++;
        }
        array[end] = array[start];
    }
    array[start] = key;
    quick_sort2(array, left, start - 1);
    quick_sort2(array, start + 1, right);
}
/*
 * 两种方式的差别是前者代码简约。相比而言swap要多一次数据拷贝。
 */
#define SIZE(array) sizeof(array)/sizeof(int)
#define OUTPUT(array,size) for(int i = 0; i < size; i++) printf("%d,", *(array +i)); printf("\n")

int main() {
    int a1[] = {1}, a2[]={1,2}, a3[]={3,2,1}, ax[]={3,45,1,2,43,34,12};

    quick_sort2(ax, 0, 6);
    OUTPUT(ax, 7);
    return  0;
}