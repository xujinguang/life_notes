## Go 信号量

### 1. 基础概述

信号量，英文**semaphore**

适用于控制一个仅支持有限个用户的共享资源，不需要忙等待。如果资源存在，则分配一个，否则进入等待队列，等待资源的到来。当资源的个数退化为1个时，就是常见的"互斥"问题。

*基本操作*：PV

1. 初始化非负数的整数值，代表共享资源的个数；
2. P-wait()，将信号量值减1，如果信号量变为负值，则进入等待区。否则，进入临界区
3. V-signal()，退出临界区，将信号量值加1。如果信号量值不为正，则唤醒一个等待的进程。

### 2. Sync.Cond

#### 2.1 数据结构

```go
//A Locker represents an object that can be locked and unlocked.
type Locker interface {
    Lock()
    Unlock()
}

type Cond struct {
	noCopy noCopy //go的禁止拷贝，因为go不具有原生的禁止拷贝，内嵌此结构为了使用go vet工具检测

	// L is held while observing or changing the condition
	L Locker

	notify  notifyList
	checker copyChecker
}
```

#### 2.2 方法

```go
    func NewCond(l Locker) *Cond //创建一个信号量
    func (c *Cond) Broadcast() //唤醒所有等待c上协程
    func (c *Cond) Signal() //退出临界区，唤醒等待c上一个协程
    func (c *Cond) Wait() //和OS不同，阻塞返回	
```



#### 2.3 示例

