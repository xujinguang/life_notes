---
marp: true
theme: gaia
footer: 'FiT'
paginate: true
style: |
  section a {
      font-size: 30px;
  }
---
# Go设计与实现

### Go的创造者

* Ken Thompson
* Rob Pike
* Robert Griesemer

https://zhuanlan.zhihu.com/p/75373075

---
## 并发
Go语言正是在多核和网络化的时代背景下诞生的原生支持并发的编程语言
1. 多线程模型（内核多线程） - 大多数语言都支持
2. 消息传递模型（CSP） - Erlang、Go
>Do not communicate by sharing memory; instead, share memory by communicating.
不要通过共享内存来通信，而应通过通信来共享内存。

---
### 协程
go func()
1. 栈的空间大小问题
2. 调度器
---
### 调度器
* 半抢占式，只有在当前Goroutine发生阻塞时才会导致调度。同时发生在用户 态，调度器会根据具体函数只保存必要的寄存器，切换的代价要比系统线程低得 多。运行时有一个 runtime.GOMAXPROCS 变量，用于控制当前运行正常非阻塞 Goroutine的系统线程数目
---
### 优点
1. 启动简洁方便性
2. 调度成本低
3. 栈空间的动态扩展
4. 

---
### 顺序一致性内存模型
1. 多线程之间数据同步 —— 原子操作，同步锁
2. 

---
### 包的初始化
![width:1200](./image/包的初始化流程图.png)

---
### 基于Channel的通信
1. Channel通信是在Goroutine之间进行同步的主要方法
2. 同一个Goroutine之间进行发送接收可能会导致死锁，通常是不同协程之间

--- 
### 站在C/C++的肩上
CGO