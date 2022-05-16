/**
 * 2022-05-11 10:40
 */

/**
 * 二叉树的最大深度
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xnd69e/
 * 思路1:递归
 * 思路2:广度优先搜索，记录层数
 */

#include <stdio.h>

 struct TreeNode {
     int val;
     struct TreeNode *left;
     struct TreeNode *right;
 };


int maxDepth(struct TreeNode* root){
    if(root == NULL) return 0;
    int left = maxDepth(root->left);
    int right = maxDepth(root->right);
    return (left > right ? left : right) + 1;
}

/**
 * 验证二叉搜索树
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xn08xg/
 * 思路1:中序遍历存入栈中，遍历栈是否有序
 * 思路2:用递归来解决上面栈问题, 需要把前驱传入进去
 */

// 思路1
void outputBST(struct TreeNode* root, int *array, int *idx) {
    if(root->left)
        outputBST(root->left, array, idx);
    array[++(*idx)] = root->val;
    if(root->right)
        outputBST(root->right, array, idx);
}

bool isValidBST(struct TreeNode* root){
    if(root == NULL) return true;
    int array[10000];
    int idx = -1;
    outputBST(root, array, &idx);
    for(int i = 1; i <= idx; i++) {
        if(array[i] <= array[i - 1]) return false;
    }
    return true;
}

// 思路2 - 注意使用大于int的区间，否则对于边界值无法处理
bool checkBST(struct TreeNode* root, long long *prev) {
    if(root->left)
        if(!checkBST(root->left, prev)) return false;

    if(root->val <= *prev) return false;
    *prev = root->val;

    if(root->right)
        if(!checkBST(root->right, prev)) return false;
    return true;
}

bool isValidBST(struct TreeNode* root){
    if(root == NULL) return true;
    struct TreeNode* tmp = root;
    long long prev;
    while(tmp->left) tmp = tmp->left;
    prev = tmp->val - 1;
    
    return checkBST(root, &prev);
}

// 避免上述问题，可以使用全局Node指针

/**
 * 对称二叉树
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xn7ihv/
 * 思路1：层次遍历，使用队列记录当前层所有节点，包括空指针，然后对称检测
 * 思路2: 翻转右子树，然后遍历比对
 * 思路3: 先序遍历，记录所有所有节点，然后第一个节点后对称比对
 * 思路4: 在2的基础上，可以直接递归
 */

bool isSymmetric(struct TreeNode* root){
    // 数据大小依据数据量1000个节点
    #define MAX_SIZE 1000
    struct TreeNode *level[MAX_SIZE] = {0};
    int head = 0;
    int tail = 0;
    int curr = 0;
    int len = 0;
    level[tail++] = root;
    len++;
    while(head != tail) {
        //除了第一层，其他层都是偶数个节点
        if(len > 1 && len % 2 != 0) return false;
        //检测当前队列是否对称
        int i = 0;
        while(i < len / 2) {
            // 同时空指针, 非空且相等，则继续
            if(level[(head + i) % MAX_SIZE] == level[(tail - i + MAX_SIZE) % MAX_SIZE]) {
                i++;
                continue;
            }  
            if (level[i] && level[len - i - 1] && level[i]->val == level[len - i - 1]->val) {
                i++;
                continue;
            } else {
                return false;
            }
        }
        //TODO 遍历当前层，入队
    }
}

//思路2:
void swapTree(struct TreeNode* root) {
    if(root->left) 
        swapTree(root->left);
    if(root->right)
        swapTree(root->right);

    struct TreeNode* tmp;
    tmp = root->left;
    root->left = root->right;
    root->right = tmp;
}

bool checkSymmetric(struct TreeNode* left, struct TreeNode* right) {
    if(left == right) return true;
    if ((left == NULL && right != NULL) || (left != NULL && right == NULL)) return false;
    if(left->val != right->val) return false;
    return checkSymmetric(left->left, right->left) && checkSymmetric(left->right, right->right); 
}

bool isSymmetric(struct TreeNode* root) {
    //交换右子树
    if (root->right) {
        swapTree(root->right);
    }
    //遍历子树
    return checkSymmetric(root->left, root->right);
}

// 思路4：直接递归
bool checkSymmetric(struct TreeNode* left, struct TreeNode* right) {
    if(left == right) return true;
    if ((left == NULL && right != NULL) || (left != NULL && right == NULL)) return false;
    if(left->val != right->val) return false;
    return checkSymmetric(left->left, right->right) && checkSymmetric(left->right, right->left); 
}

bool isSymmetric(struct TreeNode* root) {
    //遍历子树
    return checkSymmetric(root->left, root->right);
}

/**
 * 二叉树的层序遍历
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xnldjj/
 * 思路：为了避免动态分配内存，这里使用C++的stl。如果用C实现需要自己做队列和链表，工作量较大
 *      如果遇到要和面试官沟通是否要写，还是只做本题的核心部分。
 */
vector<vector<int>> levelOrder(TreeNode* root) {
    queue<TreeNode*> level;
    vector<vector<int>> result;
    TreeNode *currNode;
    if(root == NULL) return result;
    level.push(root);
    while(level.size()) {
        vector<int> curr; // 当前层的遍历值
        int i = 0; //遍历当前层索引
        int len = level.size(); // 当前层的长度
        while(i < len) { //遍历当前层
            currNode = level.front(); //取出节点
            curr.push_back(currNode->val); //取出节点值
            if(currNode->left) //入队左孩子
                level.push(currNode->left);
            if(currNode->right) //入队右孩子
                level.push(currNode->right);
            level.pop(); //删除已访问节点
            i++; //本层下一个节点
        }
        // 一层遍历结束加入结果
        result.push_back(curr);
        //遍历下一层
    }
    return result;
}


/**
 * 将有序数组转换为二叉搜索树-AVL树
 * https://leetcode.cn/leetbook/read/top-interview-questions-easy/xninbt/
 * 思路1：遍历数组，然后插入一个数据旋转树
 * 思路2: 递归每次取数组的中间值
 */

struct TreeNode* createTree(int* nums, int start, int end) {
    if(start > end) return NULL;
    struct TreeNode* node = malloc(sizeof(struct TreeNode));
    int mid = (start + end) / 2;
    node->val = nums[mid];
    node->left = createTree(nums, start, mid - 1);
    node->right = createTree(nums,mid + 1, end);
    return node;
}

struct TreeNode* sortedArrayToBST(int* nums, int numsSize){
    return createTree(nums, 0, numsSize - 1);
}