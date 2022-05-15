#include<stdio.h>

// 注意这里当从0开始的时候，孩子的取值
#define LEFT(idx) (idx << 1) + 1
#define RIGHT(idx) (idx << 1) + 2
// 
void max_heapify(int* a, int size, int i) {
    // 计算左右孩子的位置
    int l = LEFT(i);
    int r = RIGHT(i);
    // 三个节点中选出最大的
    int largest = i;
    if(l < size && a[l] > a[i]) {
        largest = l;
    }
    if(r < size && a[r] > a[largest]) {
        largest = r;
    }
    // 如果不是当前节点，交换最大值到父节点，并递归调整
    if(largest != i) {
        int tmp = a[i];
        a[i] = a[largest];
        a[largest] = tmp;
        max_heapify(a, size, largest);
    }
}

void build_max_heap(int* a, int size) {
    for(int i = size/2 - 1; i >= 0; i--) {
        max_heapify(a, size, i);
    }
}

// 实际工程实现 start, end 都是有效索引 - 时间复杂度O(logN)
void max_heap(int* a, int start, int end) {
    int dad = start;
    int son = (dad << 1) + 1; // 注意这里左孩子的索引，实际当中如果想不到，可以写个数组算一下就知道了
    while(son <= end) {
        if(son + 1 <= end && a[son + 1] > a[son]) //右孩子大
            son++;
        if(a[dad] > a[son]) // 父亲比孩子都大
            return;
        else {
            //交换节点，最大放到父亲位置
            int tmp = a[dad];
            a[dad] = a[son];
            a[son] = tmp;
            // 继续往下调整
            dad = son;
            son = (dad << 1) + 1;
        }
    }
}

//时间复杂度 O(N * logN)
void init_max_heap(int* a, int size) {
    for(int i = size/2 - 1; i >= 0; i--) {
        max_heap(a, i, size - 1);
    }
}

//时间复杂度 O(N * logN)
void sort_heap(int*a, int size) {
    // 建堆 [0...n/2-1] 逐个调整
    for(int i = size/2 - 1; i >= 0; i--) {
        max_heap(a, i, size - 1);
    }
    int tmp;
    // 将0和i互换，然后调整[0,i-1]
    for(int i = size - 1; i > 0; i--) {
        tmp = a[i];
        a[i] = a[0];
        a[0] = tmp;
        max_heap(a, 0, i-1);
    }
}

#define SIZE(array) sizeof(array) / sizeof(int)
#define OUTPUT(array, size) for(int i = 0;i < size; i++) printf("%d,", array[i]); printf("\n")
int main() {
    int a[] = {27, 17,3, 16, 13, 10, 1, 5, 7, 12, 4, 8, 9, 0};
    max_heapify(a, sizeof(a)/sizeof(int), 2);
    OUTPUT(a, SIZE(a));

    int b[] = {1,2,3,4,5,6,7,8,9,10};
    //build_max_heap(b, SIZE(b));
    OUTPUT(b, SIZE(b));

    init_max_heap(b, SIZE(b));
    OUTPUT(b, SIZE(b));
    sort_heap(b, SIZE(b));
    OUTPUT(b, SIZE(b));
    
    return 0;
}