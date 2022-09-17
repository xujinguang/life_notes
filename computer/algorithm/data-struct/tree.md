<h5>作者：徐金光		日期：2020年9月12日</h5>

树由于其结构特性，非常适合使用递归来解决。但是要明白递归的限制是栈的深度。如果树的高度超过64，那么会引发栈溢出。一般不会超过这个深度。所以绝大多数树的操作使用递归都是没有问题的。

## 1.检测二叉树

### 1.1 j检测平衡二叉树 

需要一个获取左右子树深度的辅助函数。如果某一层不是平衡二叉树，则直接层层返回。

```c++
/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
 * };
 */
class Solution {
public:
    bool isBalanced(TreeNode* root) {
        return deepTree(root) != -1;
    }

    int deepTree(TreeNode *root) {
        if(root== NULL) return 0;
        int left = deepTree(root->left);
        if(left == -1) return -1;
        int right = deepTree(root->right);
        if(right == -1) return -1;
        if(abs(left - right) <= 1) //高度三种情况
            return max(left, right) + 1;
        else 
            return -1;
    }
};
```

### 1.2 检测搜索二叉树

有两个思路，第一个就是中序遍历，将树序列化。如果中序遍历有序，则二叉树是搜索二叉树，否则不是。所以时间复杂度是`O(n)`.

```c++
class Solution {
public:
    bool isValidBST(TreeNode* root) {
        if(!root) return true;
        vector<int> seq;
        seqTree(root, seq);
        for(int i = 0; i < seq.size() - 1; i++) {
            if(seq[i] >= seq[i + 1]) return false;
        }
        return true;   
    }

    void seqTree(TreeNode *root, vector<int> &seq){
       if(root == NULL) return;
       seqTree(root->left, seq);
       seq.push_back(root->val);
       seqTree(root->right, seq);
    }
};
```

很显然，入栈这个操作可以优化掉，只需要在入栈的时候和它的前驱直接做个比较。把空间复杂度降低为`O(1)`

```c++
class Solution {
public:
    bool isValidBST(TreeNode* root) {
        if(!root) return true;
        TreeNode *tmp = root;
        while(tmp->left) tmp = tmp->left;
        prev = long(tmp->val) - 1;
        //cout << prev;
        return seqTree(root);
    }

    bool seqTree(TreeNode *root){
       if(root == NULL) return true;
       bool l = seqTree(root->left);
       if(l == false) return false;
       if(root->val <= prev) return false;
       prev = root->val;
       bool  r = seqTree(root->right);
       if(r == false) return false;
       return true;
    }
private:
    long prev;
};
```



## 2. 子树操作

### 2.1 删除匹配子树

删除操作重点是从父节点角度来操作，否则无法修改父节点的指针，好比链表要首先找到前驱来删除指定节点。

```c++
class Solution {
public:
    TreeNode* pruneTree(TreeNode* root) {
        if(deleteSubTree(root) && root) {
            //delete(root);
            return NULL;
        }
        return root;
    }

    bool deleteSubTree(TreeNode *root) {
        if(root == NULL) return true;
        bool left = deleteSubTree(root->left);
       
        if(left && root->left) {
            delete(root->left);
            root->left = NULL;
        }
        bool right = deleteSubTree(root->right);
        if(right && root->right) {
            delete(root->right);
            root->right = NULL;
        }
        if(left == false || right == false || root->val == 1) return false;
        return true;
    }
};
```

### 2.2 检测B树是不是A的子树

```c++
/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
 * };
 */
class Solution {
public:
    bool isSubStructure(TreeNode* A, TreeNode* B) {
        if(A == B) return true;
        if(A == NULL || B == NULL) return false;
        if(checkNode(A, B)) return true;
        if(isSubStructure(A->left, B)) return true;
        if(isSubStructure(A->right, B)) return true;
        return false;
    }

    bool checkNode(TreeNode *A, TreeNode *B) {
        if(A == B) return true;
        if(A == NULL) return false;
        if(B == NULL) return true;
        if(A->val == B->val) {
            return checkNode(A->left, B->left) && checkNode(A->right, B->right);
        }
        return false;
    }
};
```



## 3. 二叉树遍历

### 3.1 前序遍历

### 3.2 中序遍历

#### 3.2.1 中序遍历并调树为右分支单链表

```c++
//思路1：先找到头，设定当前节点指针，然后中序递归调整每个节点
class Solution0 {
public:
    TreeNode* increasingBST(TreeNode* root) {
        if(root == NULL) return root;
        TreeNode *head = root;
        while(head->left) {
            head = head->left;
        }
        
        curr = &tmp;
        helper(root);
        return head;
    }
    void helper(TreeNode* root) {
        if(root == NULL) return;
        helper(root->left);
        //挂载当前节点
        curr->right = root;
        curr->left = NULL;
        curr = root;
        helper(root->right);
        return;
    }
private:
    TreeNode tmp;
    TreeNode *curr;
};
//中序遍历存储到数组里，然后遍历这个调整即可。
class Solution {
public:
    TreeNode* increasingBST(TreeNode* root) {
        vector<TreeNode *> seq;
        midWalk(root, seq);
        int len = seq.size();
        int i;
        for(i = 0; i < len  - 1; i++) {
            //cout << seq.size() - 1;
            seq[i]->left = NULL;
            seq[i]->right = seq[i + 1];
        }
        if(seq.size())
            seq[i]->left = seq[i]->right = NULL;
        return seq.size() ? seq[0] : NULL;
    }

    void midWalk(TreeNode* root,  vector<TreeNode *> & seq) {
        if(root == NULL) {
            return;
        }
        midWalk(root->left, seq);
        seq.push_back(root);
        midWalk(root->right, seq);
        return;
    }
};
```

#### 3.2.2 二叉查找树第K个最大数

```c++
class Solution {
public:
    int kthLargest(TreeNode* root, int k) {
        getkthLargest(root, k);
        return mval;
    }

    void getkthLargest(TreeNode* root, int & k) {
        if(root == NULL) return;
        getkthLargest(root->right, k);
        --k;
        if(k == 0) {
            mval = root->val;
            return;
        }
        getkthLargest(root->left, k);
    }
private:
    int mval;
};
```

查找二叉树特点是左边小，右边大。中序遍历先访问右边，再访问左边，就是逆序排列。递归过程中减少计数器。当计数器归0了节点就是第K个值。

## 3.3 后序遍历

## 3.4 层级遍历

### 3.4.1 每层节点值以链表形式返回

```c++
/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
 * };
 */
/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode(int x) : val(x), next(NULL) {}
 * };
 */
class Solution {
public:
    vector<ListNode*> listOfDepth(TreeNode* tree) {
        vector<ListNode *> list;
        if(!tree) return list;
        vector <TreeNode *> level;
        level.push_back(tree);
        int i = 0, j = 1, curr;
        ListNode head, *p;
        while(i < j) {
            curr = j;
            p = &head;
            while(i < curr) {
                p->next = new ListNode(level[i]->val);
                p = p->next;
                if(level[i]->left) {
                    level.push_back(level[i]->left);
                    j++;
                }
                if(level[i]->right) {
                    level.push_back(level[i]->right);
                    j++;
                }
                i++;
            }
            list.push_back(head.next);
        }
        return list;
    }
};
```

### 3.4.2 每层作为二维数组行输出二叉树

这个和上面的一题基本一致，只是一个是建立单链表，这个是使用数组。上面一题使用一个数组记录每层的树节点值，并使用一个分界指针指定当前层和下一层在数组中的分割点方便遍历。遍历结束记录数组中包含所有树的节点。这个空间复杂度是`O(n)`，因此还有另外一种降低空间复杂度的方式就是使用两个队列。

```c++
class Solution {
public:
    vector<vector<int>> levelOrder(TreeNode* root) {
        vector<vector<int>> a;
        if(!root) return a;
        queue <TreeNode *> lq[2];
        lq[0].push(root);
        int i = 0, j = 1;
        while(lq[i].size()) {
            vector<int> lval;
            while(lq[i].size()) {
                root = lq[i].front();
                lval.push_back(root->val);
                if(i) j = 0;
                else j = 1;
                if(root->left) {
                    lq[j].push(root->left);
                }
                if(root->right) {
                    lq[j].push(root->right);
                }
                lq[i].pop();
            }
            i = j;
            a.push_back(lval);
        }
        return a;
    }
};
```

时间复杂度没有变化，都是`O(n)`.	

## 3.5 先序中序遍历数组构建二叉树
```c
struct TreeNode* createTree(int* preorder, int preorderSize, int* curr,
 int* inorder, int start, int end) {
     if(*curr >= preorderSize)
        return NULL;
     struct TreeNode* node = malloc(sizeof(struct TreeNode));
     node->val = preorder[*curr];
     int i;
     for(i = start; i <= end; i++) {
         if(inorder[i] == preorder[*curr]) break;
     }
     if(i > start) {//创建左子树
        (*curr)++;
        node->left = createTree(preorder, preorderSize, curr, inorder, start, i - 1);
     } else {
         node->left = NULL;
     }
     if(i < end) { //创建右子树
         (*curr)++;
         node->right = createTree(preorder, preorderSize, curr, inorder, i + 1, end);
     } else {
         node->right = NULL;
     }
    return node;
}

struct TreeNode* buildTree(int* preorder, int preorderSize, int* inorder, int inorderSize){
    int curr = 0;
    return createTree(preorder, preorderSize, &curr, inorder, 0, inorderSize - 1);
}
```

## 3.6 二叉树转成单链表
```c
// prev记录先序遍历的前驱节点
void createFlatten(struct TreeNode* root, struct TreeNode **prev) {
    // 暂时存储左右节点
    struct TreeNode* left = root->left;
    struct TreeNode* right = root->right;
    //如果不是树根，则修订前驱的左右指针
    if(*prev != NULL) {
        (*prev)->left = NULL;
        (*prev)->right = root;
    }
    //更新前驱节点
    *prev = root;
    // 遍历左子树
    if(left)
        createFlatten(left, prev);
    // 遍历右子树
    if(right)
        createFlatten(right, prev);
}

void flatten(struct TreeNode* root){
    if(root == NULL) return;
    struct TreeNode* prev = NULL;
    createFlatten(root, &prev);
}
//方法2:使用一个队列记录先序遍历的节点指针，然后遍历队列逐个修订指针
/*这两个方法的时间和空间复杂度是一样的，当时递归更好一些，因为好理解，同时解决了动态分配队列的问题*/
```