#include <stdio.h>


int hammingWeight(unsigned int n) {
    unsigned int res = n;
    res = (res & 0x55555555) + ((res >> 1) & 0x55555555);
    res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
    res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);
    res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);
    return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);
}

int hammingDistance(int x, int y){
    return hammingWeight(x^y);
}

unsigned int reverseBits(unsigned int n) {
    if(n == 0) return 0;
    unsigned int ret = 0;
    int i = 32;
    while(i) {
        ret <<= 1;
        ret |= n & 0x1;
        n >>= 1;
        i--;
    }
    return ret;
}

int main() {
    printf("%d", hammingWeight(15));
}
