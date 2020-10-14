## LB

### 加权轮询

#### 1.朴素实现

```shell
Supposing that there is a server set S = {S0, S1, …, Sn-1};
W(Si) indicates the weight of Si;
i indicates the server selected last time, and i is initialized with -1;
cw is the current weight in scheduling, and cw is initialized with zero; 
max(S) is the maximum weight of all the servers in S;
gcd(S) is the greatest common divisor of all server weights in S;
while (true) {
    i = (i + 1) mod n;
    if (i == 0) {
        cw = cw - gcd(S); 
        if (cw <= 0) {
            cw = max(S);
            if (cw == 0)
            return NULL;
        }
    } 
    if (W(Si) >= cw) 
        return Si;
}
```

这个算法实现的思想很朴实。基于其自身特点：权重越大的分配的次数越多，权重越小的节点分配的次数越少。如何做到呢，那就是设定一个阈值，先从最大的权重分配，然后逐渐降低权重阈值，大于阈值的权重所属节点都分配一次。这样保证了大权重的每次都会被分配到，而最小权重的分配次数最少。

1. 每重复一轮，则减少权重阈值。
2. 每轮把大于当前阈值的节点分配一遍。
3. 每次求gcd，因为有可能会有节点动态变化。

#### 2. kingshared的实现

在kingshared中提供了另外一种实现【[源码](https://github.com/flike/kingshard/blob/master/backend/balancer.go)】。它的实现比上面要更容易懂。它将每个权重除最大公约数，然后将商求和，以此和分配一个索引数组。每个商值作为一个区段，索引值设置为相同的节点ID。为了增加随机性，将此数组打散。然后使用轮询方式挨个获取即可。

__初始化分配数组__

```go
func (n *Node) InitBalancer() {
	var sum int
	n.LastSlaveIndex = 0
	gcd := Gcd(n.SlaveWeights) //获取最大公约数

    //计算商和
	for _, weight := range n.SlaveWeights {
		sum += weight / gcd
	}
	//分配一个数组用于轮询分配
	n.RoundRobinQ = make([]int, 0, sum)
    //初始化索引
	for index, weight := range n.SlaveWeights {
		for j := 0; j < weight/gcd; j++ {
			n.RoundRobinQ = append(n.RoundRobinQ, index)
		}
	}
	//索引结果：[0,0,1,1,1,1,2,2,2...] 每个索引值连续的个数是其权重除公约数的商
	//使用洗牌算法，将分配数组随机化
	if 1 < len(n.SlaveWeights) {
		r := rand.New(rand.NewSource(time.Now().UnixNano()))
		for i := 0; i < sum; i++ {
			x := r.Intn(sum)
			temp := n.RoundRobinQ[x]
			other := sum % (x + 1)
			n.RoundRobinQ[x] = n.RoundRobinQ[other]
			n.RoundRobinQ[other] = temp
		}
	}
}
```

__轮询分配__

```go
func (n *Node) GetNextSlave() (*DB, error) {
	var index int
	queueLen := len(n.RoundRobinQ)
    //异常
	if queueLen == 0 {
		return nil, errors.ErrNoDatabase
	}
    //朴素
	if queueLen == 1 {
		index = n.RoundRobinQ[0]
		return n.Slave[index], nil
	}
	//一般化，循环分配
	n.LastSlaveIndex = n.LastSlaveIndex % queueLen
	index = n.RoundRobinQ[n.LastSlaveIndex]
	if len(n.Slave) <= index {
		return nil, errors.ErrNoDatabase
	}
	db := n.Slave[index]
	n.LastSlaveIndex++
	n.LastSlaveIndex = n.LastSlaveIndex % queueLen
	return db, nil
}
```

GCD的实现比较简单了，先遍历求得最小值，然后遍历数组和最小值相除，除不尽则递减最小值。直到最小值为1或者被所有成员除尽。

这个实现有个问题，如果实现动态扩缩容以及调整权重呢？答案是每次有节点变化时重新初始化。