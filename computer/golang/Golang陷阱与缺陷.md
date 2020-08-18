## Golang陷阱与缺陷

CreateTime: 2020-08-14 19:23

## 一、语法糖

### 1.1 左大括号不能换行

有个段子，这个世界上有两类程序员，一类是大括号不换行的；一类是大括号必换行的。还有一个段子，这个世界只有两类人， 吃鸡蛋敲小头还是敲大头的两类人。 然后有人

然而再go语言里，它将只有一类程序员。因为GO强制要求左大括号不能换行。

如何你是从C/C++语言过来的，并且喜欢windows的编码风格，那么这个没有讨论余地的语法特性，一开始会觉得很丑陋，让人十分不爽。也是新手十分纳闷的一个特性。

```c
//Windows风格
void foo()
{
    if(true) 
    {
        //搞事情
    } 
    else 
    {
        //不要搞事情
    }
    while(true)
    {
       //搞事情 
    }    
}

//传统的Unix风格
int foo() {
    if(true) {
        //搞事情
    } else {
        //不要搞事情
    }
    while(true){
       //搞事情 
    }
}

//混合版
int foo() 
{ //只是函数定义这里换行，其他语句块就不再换行了
    if(true) {
        //搞事情
    } else {
        //不要搞事情
    }
    while(true){
       //搞事情 
    }
}
```

左括号是否换行对于C/C++不是问题。空白符，换行符都只是作为分隔符而存在，并没有成为语法的一部分。因此可以根据自己的喜好随意调整。加多个空格或者换行符往往都是为了增加代码的美观度和可读性。

windows每个{}都独占一行，和不换行对比起来就像word的单倍行距和两倍行距的区别。前者看起来更加松散，更多的代码空间，后者更加紧凑。

而Go让两倍行距不再可能。{ 不能换行已经成为语法的一部分，换行会导致编译错误。这有点像强格式语言Python，但是又没有Python那么强格式要求，这就有点怪异了。

对于C++这种重语言，换行保持松散代码空间更合理，然而对于C这种简洁的不能再简洁的高级编程语言，换行就显得浪费代码空间了。而Go目前的语法特性处于C和C++的中间状态，结果来这一招也挺有意思。

如此规定让很多具有编码洁癖的人无法接受，其实这和缩进是2个空格还是4个空格一样，本身就没有对错之分，习惯就好。不要拿谁是世界上最好的编程语言情怀来辩论这个问题。当你习惯这个编码风格的时候，这个问题将不是问题，而且代码比较紧凑。



### 1.2 不允许存在无用的包和临时变量

 go不允许代码中存在无用的临时变量，以及无用的package。如果存在，会有如下编译错误。

```sh
# command-line-arguments
./example.go:7:2: imported and not used: "time"
./example.go:21:2: test declared but not used
```

提示存在导入未使用的包和声明未使用的变量。注意是临时变量，全局变量呢？可以。

有时候导入的包可能会使用它的初始化操作，并不直接引入其他内容，这是可是用使用空标识符_(即下划线)来限定。注意一旦用空标识符限定包，那么此包在当前文件里将不可用。对于函数调用返回值，不想关注部分返回值的时候，也可以使用空标识符来忽略它。

```go
import(
    "fmt"
	_ "time" //忽略time包
)
func main(){
    ret, _ := CallFunc() //忽略第二个返回值
    fmt.Println(ret) //ret使用了，不会报未使用错误
    fmt.Println(time.Now().Unix()) //error: time未定义
}
```



### 1.3 switch-case 叠落

C/C++当中switch的case语句后面需要break或者return语句来保证只执行当前匹配的case语句块，否则会从匹配的case开始一直执行直到swtich的语句块末尾。像这样

```c
switch(state) {
    case 1:
        printf("打开\n");
    case 2:
        printf("关闭\n");
    default:
        printf("unkown\n");
}
```

如果state=1，输出

```sh
打开
关闭
unknown
```

state=2，输出

```sh
关闭
unkown
```

state不匹配任何case时执行default语句输出unknown。

在这个示例中各种state状态之间是互斥的，因此，同时输出多个状态字符串是错误的。在每个case语句加上break或者return语句即可解决。

但是对于某些应用场景比如多个case具有相同操作，或者前后case存在相互依赖关系，可以从某个case开始执行到switch结束，那么这种自动“叠落”的操作是十分方便的。比如执行某个操作具有5个阶段，分别是阶段1,2,3,4,5.其中3,4阶段操作相同，那么就可以根据实际stage所处阶段来开始执行后续阶段的操作。

```c
switch(stage) { //分阶段执行
    case 1: 
        printf("执行步骤1\n");
    case 2:
        printf("执行步骤2\n");
    case 3:
    case 4:
        printf("执行剩余步骤\n");
    default:
        printf("操作完成\n");
}
```

如此对于操作在某个阶段失败后，发起重试然后继续执行，使得最终操作完成这类操作十分有用。因此，在C/C++中，case没有break/return语句，这不一定是错误，这要看具体的应用场景而定。使用方式上用户具有很强的灵活性。但是这种灵活性，也存在不直观和潜在的隐患，比如上面互斥的state状态，因为忘记break而出错。

从直观的使用方式上来讲，某个case匹配，就应该只执行当前case的语句块，不应该叠落执行。而如果需要叠落执行，应该有用户来显示告知。这样歧义性更小。Go就是这么做的。

Go默认是case语句块之间是不会叠落执行的，如果要想共用语句块，使用逗号表达式或者fallthrough语句,当然break特性还是保留的。

比如Go大小端判断接口

```go
func hostByteOrder() binary.ByteOrder {
        switch runtime.GOARCH {
        case "386", "amd64", "amd64p32",
                "arm", "arm64",
                "mipsle", "mips64le", "mips64p32le",
                "ppc64le",
                "riscv", "riscv64":
                return binary.LittleEndian
        case "armbe", "arm64be",
                "mips", "mips64", "mips64p32",
                "ppc", "ppc64",
                "s390", "s390x",
                "sparc", "sparc64":
                return binary.BigEndian
        }
        panic("unknown architecture")
}
```

比如编译器解析字符串操作接口

```go
func lexQuote(l *lexer) stateFn {
Loop:
        for {
                switch l.next() {
                case '\\':
                        if r := l.next(); r != eof && r != '\n' {
                                break
                        }
                        fallthrough
                case eof, '\n':
                        return l.errorf("unterminated quoted string")
                case '"':
                        break Loop
                }
        }
        l.emit(itemString)
        return lexInsideAction
}
```





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