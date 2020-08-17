## Golang陷阱与缺陷

CreateTime: 2020-08-14 19:23

## 一、语法糖

 go不允许存在无用的临时变量，不然会出现编译错误，解决方案是使用空标识符，它的名字是_(即下划线)。空标识符可以用在任何语法需要变量名但是程序逻辑不需要的地方。
Go里面不存在未初始化变量。否则编译不通过。 

### 可变参数接口类型

当可变参数是`interface{}`类型时，注意传入切片和传入数组效果不同的，比如`Println`,函数原型

```go
//package fmt
func Println(a ...interface{}) (n int, err error)
```

下面这两个输出结果完全不同

```go
a := []int{1,2,3}
fmt.Println(a) //输出[1,2,3]
fmt.Println(a...) //输出 1 2 3
```

### 数组是值传递

在函数调用参数中，数组是值传递，无法通过修改数组类型的参数返回结果

### map遍历时顺序不固定

### 返回值被屏蔽

在局部作用域中，命名的返回值被同名的局部变量屏蔽

### recover()必须在defer函数中运行

### main()函数提前退出

### 独占CPU导致其他Goroutine饿死

### 不同Goroutine之间不满足顺序一致性内存模型

### 闭包错误引用同一个变量

### 在循环内部执行defer语句

### 切片会导致整个底层数组被锁定

### 空指针和空接口不等价

### 内存地址会变化

### Goroutine泄漏

### 多goroutin操作map需要加锁问题

map

sync.map 

http://km.oa.com/group/29048/articles/show/394944?kmref=search&from_page=23&no=2

### Go调度机制

http://km.oa.com/group/39344/articles/show/413042?kmref=search&from_page=1&no=4

### Go内存泄露

### Go切片和数组

http://km.oa.com/group/11800/articles/show/406670?kmref=search&from_page=20&no=1

### Go时区

### Go50个坑

https://segmentfault.com/a/1190000013739000

### GO性能

http://km.oa.com/group/19253/articles/show/261601?kmref=search&from_page=27&no=4

http://km.oa.com/articles/show/445305?kmref=search&from_page=10&no=2

https://learnku.com/go/t/39490