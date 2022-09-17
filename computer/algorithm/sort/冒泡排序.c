#include <stdio.h>

int* bubble_sort(int *array, int size) {
    int i, j, tmp;
    for(i = 1; i < size; i++) {
        for(j = i - 1; j >= 0; j--) {
            if(array[j] > array[j + 1]) {
                tmp = array[j + 1];
                array[j + 1] = array[j];
                array[j] = tmp; 
            }
        }
    }
    return  array;
}

#define SIZE(array) sizeof(array)/sizeof(int)
#define OUTPUT(array,size) for(int i = 0; i < size; i++) printf("%d,", *(array +i)); printf("\n")

int main() {
    int a1[] = {1}, a2[]={1,2}, a3[]={3,2,1}, ax[]={3,45,1,2,43,34,12};
    int *ret;
    ret = bubble_sort(a1, SIZE(a1));
    OUTPUT(ret, SIZE(a1));

    ret = bubble_sort(a2, SIZE(a2));
    OUTPUT(ret, SIZE(a2));

    ret = bubble_sort(a3, SIZE(a3));
    OUTPUT(ret, SIZE(a3));

    ret = bubble_sort(ax, SIZE(ax));
    OUTPUT(ret, SIZE(ax));
    return  0;
}