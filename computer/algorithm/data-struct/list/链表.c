/*
 * 2022-05-10 14:26
 * 删除链表中的节点, 不能访问头节点，题目保证需要删除的节点不是末尾节点 。
 * 思路：将后继数据复制过来，然后删除后继
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xnarn7/
 */

#include <stdio.h>

//Definition for singly-linked list.
struct ListNode {
    int val;
    struct ListNode *next;
};

 
void deleteNode(struct ListNode* node) {
    if(node == NULL) return;
    struct ListNode *next = node->next;
    node->val = next->val;
    node->next = next->next;
    next->next = NULL;
    free(next);
}

/**
 * 删除链表的倒数第N个节点：给你一个链表，删除链表的倒数第 n 个结点，并且返回链表的头结点。
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xn2925/
 * 你能尝试使用一趟扫描实现吗？
 * 思路1: 一遍计数；二遍删除
 * 思路2: 尾指针先遍历n个节点，然后首尾两个指针同步继续遍历后续节点。尾指针结束，首指针节点就是要删除的节点。
 * 
 * 情况
 * 1. 删除首节点 1,1;12,2
 * 2. 删除尾节点 12,1
 * 3. 删除中间节点123,2
 */
struct ListNode* removeNthFromEnd(struct ListNode* head, int n){
    struct ListNode tmp;
    tmp.next = head;
    struct ListNode *start = &tmp;
    struct ListNode *end = &tmp;
    while(n && end->next) {
        end = end->next;
        n--;
    }
    if(n) return head; // 长度不够，原样返回
    while(end->next) {
        start = start->next;
        end = end->next;
    }
    end = start->next;
    start->next = start->next->next;
    free(end);
    return tmp.next;
}

/**
 * 给你单链表的头节点 head ，请你反转链表，并返回反转后的链表。
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xnnhm6/
 * 思路：遍历头插入法
 */
struct ListNode* reverseList(struct ListNode* head){
    struct ListNode tmp;
    tmp.next = NULL;
    struct ListNode *next;
    while(head) {
        next = head->next;
        head->next = tmp.next;
        tmp.next = head;
        head = next;
    }
    return tmp.next;
}

/**
 * 合并两个有序链表
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xnnbp2/
 * 思路：创建一个临时节点，两个指针指向两个链表；每次选择小的插入链表，并偏移下一个
 */
struct ListNode* mergeTwoLists(struct ListNode* list1, struct ListNode* list2){
    struct ListNode head;
    head.next = NULL;
    struct ListNode *curr = &head;
    struct ListNode *p1 = list1;
    struct ListNode *p2 = list2;
    while(p1 && p2) {
        if(p2->val > p1->val) {
            curr->next = p1;
            p1 = p1->next;
        } else {
            curr->next = p2;
            p2 = p2->next;
        }
        curr = curr->next;
    }
    if(p1) {
        curr->next = p1;
    }
    if(p2) {
        curr->next = p2;
    }
    return head.next;
}

/**
 * 回文链表
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xnv1oc/
 * 思路1: 创建一个栈，遍历前一半节点入栈，遍历后一半节点时匹配时出栈，否则返回false。如果栈空，则true
 * 思路2: 如果允许修改原链表。遍历一遍长度；遍历后面一半节点头插入到链表头；然后对半比对
 * 思路3: 由思路1延伸出来，可以用递归来完成
 */
bool isPalindrome(struct ListNode* head){
    struct ListNode tmp;
    tmp.next = head;
    struct ListNode *part = head;
    int len = 0;
    int i;
    while(head) {
        len++;
        head = head->next;
    }
    head = part;
    if(len % 2) {
        i = len / 2;
    } else {
        i = len / 2 - 1;
    }
    while(i) {
        i--;
        head = head->next;
    }
    struct ListNode *next;
    next = head->next;
    head->next = NULL;
    head = next;
    while(head) {
        next = head->next;
        head->next = tmp.next;
        tmp.next = head;
        head = next;
    }
    head = tmp.next;
    i = len / 2;
    while(i) {
        if(head->val != part->val) return false;
        head = head->next;
        part = part->next;
        i--;
    }
    return true;
}

/**
 * 给你一个链表的头节点 head ，判断链表中是否有环。
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xnwzei/
 * 思路1: 快慢指针
 * 思路2: 集合map
 * 思路3: 翻转链表
 **/

bool hasCycle(struct ListNode *head) {
    if (head == NULL)
        return false;
    //快慢两个指针
    struct ListNode *slow = head;
    struct ListNode *fast = head;
    while (fast != NULL && fast->next != NULL) {
        //慢指针每次走一步
        slow = slow->next;
        //快指针每次走两步
        fast = fast->next->next;
        //如果相遇，说明有环，直接返回true
        if (slow == fast)
            return true;
    }
    //否则就是没环
    return false;
}

    