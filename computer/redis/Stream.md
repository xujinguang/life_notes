## Stream

消息队列的作用：**异步处理，应用解耦，限流削峰**

产品：RabbitMQ，RocketMQ，Kafka，Redis

### 构成

消息，生产者，消费者，消费组

**消息**

1. 唯一ID，ID严格递增
2. 消息内容多个kv组成。

**生产者**

生成消息到消息队列

**消费者**

1. 消费消息
2. 可以属于或不属于消费组

**消费组**

1. 消费组有自己的唯一名称
2. 每个消费组隔离，意思消费消息进度互不影响
3. 消费组里的成员是竞争关系，一个消息只能被消费组内一个消费者消费。
4. 组内成员消费消息后需要确认，每个消息组都有一个待确认消息队列
5. 消费组中的每个成员也有一个待确认消息队列，维护着该消费者已经消费尚未确认的消息

### 实现

listpack以及Rax树

#### listpack

看源码发现这竟然没有struct定义，一切都是字节操作。心中有形，万物无形的最高境界。

结构：Totalbyte + ElementNum + [ encode + content + backlen ] + ... + End

1. Total 4B,最大字节数4GB
2. EN 2B
3. End 1B，固定0xFF

所以分配空listpack就是7字节。LP_HDR_SIZE = 6 ;EN=65535时，元素需要遍历才能得知。

1. encode  1B，这个用的真的是一个bit都不浪费。
2. content NB，字节内容，不关心具体存啥。
3. backlen <5B, 上面两者的长度，不包括自身，为了从后往前遍历。每个字节只使用7个bit，字节高位bit作为标识，1标识结束。

整形存储不存储负数，负数映射到正数。在13位整型存储中，存储范围为[0,8191]，其中[0, 4095]对应非负的[0, 4095]。

#### 前缀树Rax

1. 基树的一种实现
2. 每个节点存储一个字符，Rax采用压缩方式，存储了字符串，减少了节点
3. 由于2，导致插入一个新的key的时候，会导致Rax树的分裂。

源码的注释写的很好

```c
/* Representation of a radix tree as implemented in this file, that contains
 * the strings "foo", "foobar" and "footer" after the insertion of each
 * word. When the node represents a key inside the radix tree, we write it
 * between [], otherwise it is written between ().
 *
 * This is the vanilla representation:
 *
 *              (f) ""
 *                \
 *                (o) "f"
 *                  \
 *                  (o) "fo"
 *                    \
 *                  [t   b] "foo"
 *                  /     \
 *         "foot" (e)     (a) "foob"
 *                /         \
 *      "foote" (r)         (r) "fooba"
 *              /             \
 *    "footer" []             [] "foobar"
 *
 * However, this implementation implements a very common optimization where
 * successive nodes having a single child are "compressed" into the node
 * itself as a string of characters, each representing a next-level child,
 * and only the link to the node representing the last character node is
 * provided inside the representation. So the above representation is turend
 * into:
 *
 *                  ["foo"] ""
 *                     |
 *                  [t   b] "foo"
 *                  /     \
 *        "foot" ("er")    ("ar") "foob"
 *                 /          \
 *       "footer" []          [] "foobar"
 *
 * However this optimization makes the implementation a bit more complex.
 * For instance if a key "first" is added in the above radix tree, a
 * "node splitting" operation is needed, since the "foo" prefix is no longer
 * composed of nodes having a single child one after the other. This is the
 * above tree and the resulting node splitting after this event happens:
 *
 *
 *                    (f) ""
 *                    /
 *                 (i o) "f"
 *                 /   \
 *    "firs"  ("rst")  (o) "fo"
 *              /        \
 *    "first" []       [t   b] "foo"
 *                     /     \
 *           "foot" ("er")    ("ar") "foob"
 *                    /          \
 *          "footer" []          [] "foobar"
 *
 * Similarly after deletion, if a new chain of nodes having a single child
 * is created (the chain must also not include nodes that represent keys),
 * it must be compressed back into a single node.
 *
 */
```

##### 结构体

```c
typedef struct rax {
    raxNode *head;
    uint64_t numele;
    uint64_t numnodes;
} rax;

typedef struct raxNode {
    uint32_t iskey:1;     /* Does this node contain a key? */
    uint32_t isnull:1;    /* Associated value is NULL (don't store it). */
    uint32_t iscompr:1;   /* Node is compressed. */
    uint32_t size:29;     /* Number of children, or compressed string len. */
    unsigned char data[];
} raxNode;
```

