## HashMap

### 1.数据结构

```c
typedef struct dictEntry {
    void *key;
    union {
        void *val;
        uint64_t u64;
        int64_t s64;
        double d;
    } v;
    struct dictEntry *next;
} dictEntry;

typedef struct dictType {
    uint64_t (*hashFunction)(const void *key);
    void *(*keyDup)(void *privdata, const void *key);
    void *(*valDup)(void *privdata, const void *obj);
    int (*keyCompare)(void *privdata, const void *key1, const void *key2);
    void (*keyDestructor)(void *privdata, void *key);
    void (*valDestructor)(void *privdata, void *obj);
} dictType;

typedef struct dictht {
    dictEntry **table;
    unsigned long size;
    unsigned long sizemask;
    unsigned long used;
} dictht;

typedef struct dict {
    dictType *type;
    void *privdata;
    dictht ht[2];
    long rehashidx; /* rehashing not in progress if rehashidx == -1 */
    unsigned long iterators; /* number of iterators currently running */
} dict;
/* If safe is set to 1 this is a safe iterator, that means, you can call
 * dictAdd, dictFind, and other functions against the dictionary even while
 * iterating. Otherwise it is a non safe iterator, and only dictNext()
 * should be called while iterating. */
typedef struct dictIterator {
    dict *d;
    long index;
    int table, safe;
    dictEntry *entry, *nextEntry;
    /* unsafe iterator fingerprint for misuse detection. */
    long long fingerprint;
} dictIterator;
```



### 2. 重点

1. 初始化时 dictht.size = 4
2. dict.rehashidx != -1 时字典处于rehash中，rehashidx指向rehash的字典
3. rehash时字典大小按照旧的size*2扩容或者size/2缩容，一定是2的倍数。因为方便设置sizemask
4. rehash过程中，查找，删除，更新都需要涉及旧字典的查询，同时rehash到新字典中；这是一种渐进式方式进行，防止过大字典的一次性卡顿。
5. 每次对100个节点进行rehash操作，共执行1毫秒；
6. 指针切换h[0],h[1]完成rehash

### 3. 字典遍历

有两个目标：

1. 不重复
2. 全部遍历到

解决的问题在遍历时候如何处理rehash。Redis提供两种方式：间断和全遍历。

**全遍历**包括两种：

1）普通迭代器，只遍历数据；意思就是不能做任何其他操作：修改、添加、删除、查找，由于redis是单线程，这个可以做到。指纹也是用于此的。

2）安全迭代器，遍历的同时可以删除数据。实现策略是遍历时候不进行refresh。

**间断遍历**

全遍历对大数据量的耗时阻塞redis可用性。间断遍历，可以一次只返回一个槽位的key，不阻断Rehash。采用的技术叫高位加法。

基于一个数学原理。扩缩容是2的紧邻倍数。扩容，等于将原有链表节点分开两部分。比如101，扩容后rehash结果应该是0101和1101，为啥是这样呢？逆向思维一下，因为他们mod 111都是101，同时这也是缩小容量的操作。高位加法主要是保证rehash后的1101能够处于遍历的低位。换言之小于1101的保证都已经遍历。

## 整数集合

### 数据结构

```c
typedef struct intset {
    uint32_t encoding; //编码，支持8，16，32，64
    uint32_t length; //按照指定编码时元素个数
    int8_t contents[]; //字节数组，按照编码宽度解析
} intset;
```

### 要点

1. 有序的，存储整数集合，不是浮点值。
2. 存储范围64位有符号整数
3. 默认超过一定元素之后会转换成字典存储
4. 除了3，当对key添加非整数元素时，也会发生存储结构转换。
5. 当插入最大值或者最小值的时候，会发生扩容。注意不会缩容。
6. 有序数组使用的折半查找。
7. 插入移动元素使用的memmove不是memcpy，因为可能是同一段内存。memmove会逆向拷贝，避免覆盖。

