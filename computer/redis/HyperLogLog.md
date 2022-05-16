## HyperLogLog

### 需求

1. UV - 网站的访问用户量、网站的请求IP量
2. 关键词搜索
3. 数据分析

### 直觉方法

记录集合中所有不重复的元素集合；当新来一个元素，若中不包含元素，则将加入；否则不加入，计数值就是的元素数量。

可用数据结构：HaspMap，Set，ZSet（底层也是哈希表），B树，bitmap

### 存储元素算法

#### HashMap

1. 计算`value`的`hash`值，然后分配节点并存入`value`.
2. 判断哈希表对应槽位上是否存在元素；
3. 如果有，则遍历比对冲突链表；
4. 如果查找存在，则结束；否则，插入链表结尾；
5. 如果哈希表装填因子变大，则进行`rehash`操作

#### B树

1. 查找树节点，比对`value`值
2. 找到，则放弃插入；否则，分配节点并存入`value`.
3. 执行B树插入操作。
4. 树的节点个数即为元素个数。

#### BitMap

1. 计算value的整形哈希值，以此哈希值为索引设置位图对应`bit`
2. 如果对应`bit`位已经设置为1，则放弃；否则，置位1；
3. 统计位图1的个数，即为元素个数。
4. 为了防止冲突，需要比较均匀的哈希函数，并且根据访问量增加而放大位图。

| 对比项 | HashMap           | B树      | BitMap                                                 |
| ------ | ----------------- | -------- | ------------------------------------------------------ |
| 查找   | 最快O(1),最坏O(n) | O(log^n) | O(1)                                                   |
| 插入   | O(1)              | O(log^n) | O(1)                                                   |
| 存储   | O(n)              | O(n)     | O(n)                                                   |
| 准确度 | 精确              | 精确     | 如果请求id是整形，则比较精确<br>如果id是哈希值，则粗略 |
| 值     | 元素本身          | 元素本身 | 元素本身/哈希值                                        |

很显然，三种方法随着数据量增大而增大是主要问题。查找和插入不是性能的瓶颈。如果只是单纯的统计UV，对于亿万数据量级的存储显得没有必要性。

### 问题

1. 存储空间随元素线性增长，比如亿万数量级的元素；
2. 元素越多，判断是否加入成本越大；
3. 数据合并成本大。

### 非存储元素算法

不追求绝对准确，保证误差控制在一定范围。不存储元素，节省内存。基于概率。Probabilistic cardinality estimators - 概率基数估计器。

比如：Linear Counting/LogLog/HyperLogLog

HyperLogLog 是早期 LogLog 算法的扩展，后者本身源自1984年 Flajolet-Martin 算法。在 Flajolet 等人的原始论文[1]和关于计数不同问题的相关文献中，术语“基数”用于指具有重复元素的数据流中不同元素的数量。然而，在多集合论中，这个术语指的是多集合中每个成员的多重数之和。

>  the term "cardinality" is used to mean the number of distinct elements in a data stream with repeated elements. However in the theory of [multisets](https://en.wikipedia.org/wiki/Multiset) the term refers to the sum of multiplicities of each member of a multiset. 

### HyperLogLog[wiki]

HyperLogLog 算法的基础是观察到一个**均匀分布**的随机数的**多重集的基数**可以通过计算该集中每个数的**二进制表示**中的**前导零的最大个数**来估计。如果观察到的前导零的最大数目是 $$n$$，则该集合中不同元素的数目估计为$$2^n$$.

在 HyperLogLog 算法中，对原多集中的每个元素应用一个哈希函数，得到与原多集具有相同基数的均匀分布随机数的多集。然后可以使用上述算法估计这个随机分布集的基数。如此算法得到的基数的简单估计具有方差大的缺点。在 HyperLogLog 算法中，方差最小化的方法是将多重集分割成多个子集，计算每个子集中前导零的最大数目，并使用调和平均值将每个子集的估计值合并为整个集的基数估计值。

HyperLogLog数组大小$$m$$, 数组记为M$$

#### Add

已知输入值$$v$$,哈希函数$$h$$:

<img src="https://render.githubusercontent.com/render/math?math=\large \begin{equation} \begin{aligned} b%26=\log_2^m\\x%26=h(v)\\j %26= 1 + \langle x_1,x_2,\cdots,x_b\rangle \\ w%26=x_{b+1}x_{b+2}\cdots\\M[j]%26=\max(M[j],\theta(w)) \end{aligned} \end{equation}">

注：github展示的方法，将&替换%26,以图片展示

数学中的计算从`1`开始，不是计算机的`0`下标开始. 计算` b`为了取得哈希值的高位作为数组下标索引`j`。取低`bit`位中前导0最大的数目$$\theta(w)$$,取最大值设置$$M[j]$$.

#### Count

<img src="https://render.githubusercontent.com/render/math?math=\large\begin{equation} \begin{aligned} Z%26=\left(\sum_{j=1}^m{\frac{1}{2^{M[j]}}}\right)^{-1} \\ \alpha_m%26=\left(m \int_0^\infty\left(\log_2^{\left(\frac{2 + u}{1+u}\right)}\right)^mdu\right)^{-1}\\E%26=\alpha_m m^2 Z \end{aligned} \end{equation} ">

#### Merge

<img src="https://render.githubusercontent.com/render/math?math=\large hll_union[j] = \max(hll_1[j],hll_2[j]),j = 1,2,\cdots" alt="hll_union[j] = \max(hll_1[j],hll_2[j]),j = 1,2,\cdots">

对应元素取最大值

### redis中hyperloglog

| cmd     | 时间复杂度 | 功能                 |
| ------- | ---------- | -------------------- |
| pfadd   | O(1)       | 将元素放入hll        |
| pfcount | O(1)~O(N)  | 统计hll中的元素个数  |
| pfmerge | O(N)~const | 合并多个hll到指定hll |

pf是算法发明人的`Philippe Flajolet`的首字母。

#### 12KB

redis中实现用的
$$2^{14}=16384$$

个桶，每个桶占用 `6`个`bit`,为什么是6个。因为redis将value哈希成`64bit`值，分桶占去14个，剩余50个bit，最坏情况下，低位第一个1出现在第50个bit，那么需要最少6bit ：
$$2^{6}=64 > 50$$
来表示这个位置。所以每个桶用6bit。最后的大小
<img src="https://render.githubusercontent.com/render/math?math=\large \frac{2^{14} \times 6  (bits)}{2^{13}} = 2 \times 6(KB) = 12KB">

一个对象的UV统计占用`12kb`,如果有个`n`个对象的uv需要统计，那么需要
$$n*12$$

### 参考资料

[HyperLogLog Wiki](https://en.wikipedia.org/wiki/HyperLogLog)