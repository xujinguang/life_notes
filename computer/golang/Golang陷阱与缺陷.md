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