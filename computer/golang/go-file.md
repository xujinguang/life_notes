
---
Go语言正是在多核和网络化的时代背景下诞生的原生支持并发的编程语言
1. 多线程模型（内核多线程） - 大多数语言都支持
2. 消息传递模型（CSP） - Erlang、Go

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
vertical
---
### 顺序一致性内存模型
1. 多线程之间数据同步 —— 原子操作，同步锁
2. 

---
### 基于Channel的通信
1. Channel通信是在Goroutine之间进行同步的主要方法
2. 同一个Goroutine之间进行发送接收可能会导致死锁，通常是不同协程之间


---
参考
### 编程上的正交
从数学上引进正交这个词，用于表示指相互独立，相互间不可替代，并且可以组合起来实现其它功能。比如 if 和 for 语句是正交的，但 for 和 while 语句的功能是有重叠的。逻辑运算 not、and、or 也是正交的，其它复杂的逻辑运算都可以用这三种基本运算叠加起来。

编程语言经常定义一组正交语法特性，相互间不可替代，组合起来可以其它功能。而为了更方便使用，在基础特性之上，再添加一些额外特性。这些非基本的额外特性，称为语法糖（Syntactic sugar）。语法糖对语言的功能没有太大影响，有可以，没有也可以，但有了之后，代码写起来更方便些。

- small, concise, crisp —— 小，简洁，干脆
- procedural —— 程序性
- strongly typed —— 强类型
- few, orthogonal, and general concepts —— 很少，正交，一般的概念
- avoid repetition of declarations —— 避免重复声明

---
- multi-threading support in the language —— 语言层面支持多线程
- garbage collected —— GC
- containers w/o templates —— 没有模板的容器
- compiler can be written in Go and so can it's GC —— 自举
- very fast compilation possible (1MLOC/s stretch goal) —— 编译十分快
- reasonably efficient (C ballpark) —— 相当高效，相当于C
- compact, predictable code —— 紧凑、可预测的代码
  (local program changes generally have local effects)
- no macros —— 没有宏
 ---
 go的提交点
 https://github.com/golang/go/commits/weekly.2009-11-06?before=9ad14c94db182dd3326e4c80053e0311f47700ce+3850&branch=weekly.2009-11-06

 ---
 January: First month
2: Second day
15: 3PM
04: Fourth minute
05: Fifth second
2006: Sixth year of the new 21st century

---
面向对象 Object Oriented
面向过程 Procedural Programming

---

如何判断chan已经关闭？使用select并且接收返回值。
如果避免二次关闭，使用onlyonce或者recover
https://www.gushiciku.cn/pl/phYg/zh-hk

资料：
https://learnku.com/docs/effective-go/2020/format/6237
https://speakerdeck.com/kavya719/understanding-channels?slide=48
https://levelup.gitconnected.com/how-does-golang-channel-works-6d66acd54753
defer
https://developpaper.com/detailed-explanation-of-golang-defer/

GMP
https://learnku.com/articles/41728
https://medium.com/@ankur_anand/illustrated-tales-of-go-runtime-scheduler-74809ef6d19b