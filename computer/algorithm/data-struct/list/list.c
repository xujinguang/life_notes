/**
 * Definition for singly-linked list.
 */
 /**
 思路1: 转换成两个字符串，寻找最大后缀长度，空间复杂度On，时间复杂度O1
 思路2: 逆向一个单链表，然后计算长度，以A为例子
 lA = la + lc
 lB = lb + lc
 如果没有交汇点：
 LN = lb
 如果有交汇点：
 lN = lb + la + 1
 lc = ((lA+lB-lN) + 1)/2
 lb = lB - lc
 lb+1位置就是交叉点
 时间，空间复杂度O1
 思路3: |lA-lB|是相差的个数，然后让指针先走这几个，最后再一起往后走，当next相同时就是答案
*/
#include <stdio.h>

struct ListNode {
     int val;
     struct ListNode *next;
};
struct ListNode *getIntersectionNode(struct ListNode *headA, struct ListNode *headB) {
    int la = 0;
    int lb = 0;
    struct ListNode *tmp;
    //统计A的长度
    tmp = headA;
    while(tmp) {
        ++la;
        tmp = tmp->next;
    }
    if(la == 0) return NULL;
    //统计B的长度
    tmp = headB;
    while(tmp) {
        ++lb;
        tmp = tmp->next;
    }
    if(lb == 0) return NULL;

    //A长度，跳过A多余的节点;B长，跳过B多余的节点
    while(la > lb) {
        la--;
        headA = headA->next;
    }

    while(lb > la && headB) {
        lb--;
        headB = headB->next;
    }
    //同时往后走
    while(headA && headB && headA != headB) {
        headA = headA->next;
        headB = headB->next;
    }
    if(headA == headB) return headA;
    return NULL;
}