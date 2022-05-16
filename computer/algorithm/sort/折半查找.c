// The API isBadVersion is defined for you.
// bool isBadVersion(int version);

int firstBadVersion(int n) {
    long long start = 1;
    long long end = n;
    long long mid;
    while(start < end) {
        mid = (start + end) / 2;
        if(isBadVersion(mid)) {
            end = mid - 1;
        } else {
            start = mid + 1;
        }
        //printf("%d-%d\n", start, end);
    }
    if(isBadVersion(start)) return start;
    else return start + 1;
}

/**
 * 写代码注意事项：
 * 1. 入参数数据合法性校验
 * 2. while循环条件的初始化
 * 3. while循环条件的变更
 * 4. while循环退出条件判断
 * 5. while循环结束的收尾操作
 */