#include <stdio.h>

/*逆向插入，这个关键点在于第一个元素放置*/
int * insert_sort(int * array, int size) {
    if (array == NULL || size <= 1) {
        return array;
    }
    int i, j, tmp;
    for (i = 1; i < size; i++) {
        tmp = array[i];
        for(j = i - 1; j >= 0; j--) {
            if(array[j] > tmp) {
                array[j+1] = array[j];
                if (j == 0) {
                    array[0] = tmp;
                }
            } else {
                array[j+1] = tmp;
                break;
            }
        }
    }
    return array;
}

/*正向插入*/
int * insert_sort2(int * array, int size) {
    if (array == NULL || size <= 1) {
        return array;
    }
    int i, j, tmp;
    for (i = 1; i < size; i++) {
        // 寻找位置
        for (j = 0; j < i; j++) {
            if (array[j] < array[i]) continue; 
            else break;
        }
        //腾出位置
        tmp = array[i];
        for(int t = i-1; t >= j; t--) {
            array[t+1] = array[t];
        }
        // 归位
        array[j] = tmp;
    }
    return array;
}

#define SIZE(array) sizeof(array)/sizeof(int)
#define OUTPUT(array,size) for(int i = 0; i < size; i++) printf("%d,", *(array +i)); printf("\n")

int main() {
    int a1[] = {1}, a2[]={1,2}, a3[]={3,2,1}, ax[]={3,45,1,2,43,34,12};
    int *ret;
    ret = insert_sort2(a1, SIZE(a1));
    OUTPUT(ret, SIZE(a1));

    ret = insert_sort2(a2, SIZE(a2));
    OUTPUT(ret, SIZE(a2));

    ret = insert_sort2(a3, SIZE(a3));
    OUTPUT(ret, SIZE(a3));

    ret = insert_sort2(ax, SIZE(ax));
    OUTPUT(ret, SIZE(ax));
    return  0;
}
