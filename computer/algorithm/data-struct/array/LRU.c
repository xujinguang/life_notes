#include <stdio.h>

struct LNode {
    struct LNode *next; //指向后驱
    struct LNode *prev; //指向前驱
    int key; //指向hash的索引
    int val; //哈希值
};

typedef struct {
    struct LNode **hash; //快速查找key，指向链表节点
    struct LNode *head; //双向链表的头节点
    int capacity; //LRU的容量
    int size; //当前使用大小
} LRUCache;


LRUCache* lRUCacheCreate(int capacity) {
    if(capacity <= 0 || capacity > 3000)
        return NULL;
    LRUCache *cache = malloc(sizeof(LRUCache));
    if(cache == NULL)
        return NULL;
    cache->hash = malloc(sizeof(struct LNode*) * 10001); //按照key最大分配
    for(int i = 0; i < 10001; i++) {
        cache->hash[i] = NULL;
    }
    cache->capacity = capacity;
    cache->head = NULL;
    cache->size = 0;
    return cache;
}

void adjustReadNode(LRUCache* obj, struct LNode *p) {
    if(p == obj->head)
        return;
          
    //删除节点
    p->prev->next = p->next;
    p->next->prev = p->prev;
    
    //头插入到链表头,更新LRU
    p->prev = obj->head->prev;
    obj->head->prev->next = p;
    p->next = obj->head;
    obj->head->prev = p;
    obj->head = p;
}

int lRUCacheGet(LRUCache* obj, int key) {
    if(obj == NULL || key < 0 || key > 10000)
        return -1;
    struct LNode *p = obj->hash[key];
    if(p != NULL) { //存在
        adjustReadNode(obj, p);
        return obj->hash[key]->val;
    } else {
        return -1;
    }
}

void output(LRUCache* obj) {
    struct LNode *tmp = obj->head;
    if(tmp == NULL)
        return;
    while(tmp->next != obj->head) {
        printf("%d,", tmp->val);
        tmp = tmp->next;
    }
    printf("%d\n", tmp->val);
}

void lRUCachePut(LRUCache* obj, int key, int value) {
    if(obj == NULL || key < 0 || key > 10000) return;
    struct LNode *p = obj->hash[key];
    //output(obj);
    if(p != NULL) { //存在
        //1.修改旧值
        p->val = value;
        adjustReadNode(obj, p);
        return;
    }

     //删除链表结尾
    if (obj->size && obj->size == obj->capacity) {
        //printf("%d\n", obj->head->prev->val);
        struct LNode *tail = obj->head->prev;
        if(tail != obj->head) { //多个节点
            tail->prev->next = tail->next;
            tail->next->prev = tail->prev;
        } else { //只有一个节点
            obj->head = NULL;
        }
        obj->hash[tail->key] = NULL; //清空哈希
        free(tail); //释放直接内存
        obj->size--;
    }

    //创建新节点插入
    struct LNode *node = malloc(sizeof(struct LNode));
    node->val = value;
    node->key = key;

    if(obj->head) {
        //头插入法
        node->prev = obj->head->prev;
        node->prev->next = node;
        node->next = obj->head;
        node->next->prev = node;
    } else { //只有一个节点
        node->prev = node;
        node->next = node;
    }
    obj->head = node;
    //建立hash
    obj->hash[key] = node;
    obj->size++;
    //output(obj);
    return;
}

void lRUCacheFree(LRUCache* obj) {
    if(obj == NULL) return;
    //释放链表
    if(obj->head) {
        struct LNode *node = obj->head->next;
        while(node && node != obj->head) {
            struct LNode *tmp;
            tmp = node;
            node = node->next;
            free(tmp);
        }
    }
    if(obj->head)
        free(obj->head);
    if(obj->hash)
        free(obj->hash);
    free(obj);
}

/**
 * Your LRUCache struct will be instantiated and called as such:
 * LRUCache* obj = lRUCacheCreate(capacity);
 * int param_1 = lRUCacheGet(obj, key);
 
 * lRUCachePut(obj, key, value);
 
 * lRUCacheFree(obj);

 ["LRUCache","put","put","get","put","get","put","get","get","get"]
[[2],[1,1],[2,2],[1],[3,3],[2],[4,4],[1],[3],[4]]
["LRUCache","put","get","put","get"]
[[1],[1,1],[1],[2,2],[2]]
*/