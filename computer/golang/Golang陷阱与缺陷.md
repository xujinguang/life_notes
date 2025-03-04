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

### 1.4 中文编码

对于说中文的你，在写程序的时候有没有在未造变量标识符而苦苦思索？甚至有时候干脆直接用中文拼音？想必每个中国程序员都有遇到。

Golang相比某些语言的编码符号集合更大，因为它采用了Unicode字符集，它远远大于ASCII字符集。由于Unicode包含汉字，因此，汉字在Golang中可以作为合法的标识符。比如下面这个示例。

```go
package main

import(
	"fmt"
)

const(
	真 = true
	假 = false
	嗯 = true
	是的 = true
)

func 打印(输出的信息 ...interface{}) {
	fmt.Println(输出的信息)
}

type 整数 int
func 乘积(a, b 整数) 整数{
	return a * b
}

func (a 整数) 增加(s 整数) 整数{
	return a+s 
}

func main() {
	你好 := "Golang"
	打印(你好);
	吃饱了吗:= 真
	if(吃饱了吗 == 嗯) {
		打印("写点代码吧！")
	} else {
		打印("吃饱了再来。")
	}

	死循环 := 真
	高 := 整数(12)
	宽 := 整数(6)
	for(死循环 == 真){
		四边形面积 := 乘积(宽, 高)
		高.增加(3)
		宽.增加(2)
		打印(高,宽,四边形面积)
		死循环 = 假
	}
	return
}
```

注意，这个程序是完全正确的，不是什么特技，是完全合法的Golang程序。程序内容不用解释，一看就懂，代码犹如自带中文注释一样。显然这有点国产易语言的风格。由于Golang不支持C的宏定义，因此无法修改语言关键字的名字，还完全做不到易语言的全中文化。不过已经很大程度本地化了。

用我们中文母语编程，先避开中文是否方便编写的话题不论，单单用自己母语写程序这一点就要比非母语写程序对国人更加直观和容易理解。这也从侧面印证了西方人用英文程序写的为何那么溜，毕竟是他们自己的语言。不过由于Unicode包括各国的语言字符。如果每个国家都使用自己的语言开发程序，那么程序的传播范围必将受限。从目前来看还是英语为主导，地方语言编程只是小众。鉴于程序开源理念以及信息自由传播的思想，还是以国际语言英文为主。等到有朝一日中文成为国际语言的时候，我们再用中文来一统01江山。



### 1.5  返回局部变量的地址

C/C++中是不能返回局部变量的地址的，因为C/C++的局部变量都是在栈上分配的。函数调用结束，此函数的栈帧就完全弹出了。此时栈会分配给后续其他函数调用中使用，因此，局部变量的地址也就无效了。如果函数外引用函数内的局部变量地址，一般会引发段错误（栈空间已释放）或者异常（栈未释放，已无效。虽然可读写但是属于脏读，脏写）。

Go可以直接返回局部变量的地址，而且属于正常行为。为什么Go可以呢？

原因并不复杂。我们知道分配内存有两种方式，一种是堆，一种是栈。栈上的生命周期要短于堆。其次，编译器有一个能力叫“逃逸分析”，它的作用在编译阶段分析函数内的变量是否超过函数本身的作用域。如果超过了，那么这个变量就不会分配的栈上，而是在堆上为其分配空间。如此实现了可以返回局部变量的地址。

如果这个局部变量使用new创建的呢？和上面原则一样。通过逃逸分析此变量有没有逃逸。如果作用域逃出函数自身，则在堆上分配空间，否则在栈上分配。因此，函数内部的变量分配在栈上还是堆上和使用哪种方式创建无关， 只与它是否逃逸有关。在官方解释中，给出了一下原则：

1. Go编译器在给函数中的局部变量分配空间时会尽可能地分配在栈空间中，**但是如果编译器无法证明函数返回后是否还有该变量的引用存在，则编译器为避免悬挂空指针的错误，就会将该局部变量分配在堆空间中；**
2. **如果局部变量占用内存很大，Go编译器会认为将其存储在堆空间中更有意义；**
3. Go编译器如果看到了程序中有使用某个变量的地址，则该变量会变成在堆空间上分配内存的候选对象，**此时Go编译器会通过分析，判断出该指针的使用会不会超过函数的范围，如果没超过，该变量就可以驻留在栈空间上；如果超过了，就必须将其分配在堆空间中。**

注意这里讨论的是局部变量的地址，不是局部变量的值。C/C++/Go都是允许返回局部变量值的，如果是返回局部变量，要发生数据拷贝。在某些高效计算的场景下，数据拷贝影响效率，不推荐使用。为了避免数据拷贝，C/C++通过参数指针从函数调用方传入被调用者，在被调用者函数内部修改外部变量达到传递的效果，而Go直接通过返回局部变量的地址。

初看起来是十分有用的特性，让开发者而不用过多的关心空间的申请与释放，更加聚焦于程序逻辑。但是也存在负面影响。

首先会导致连续的变量，因为某个中间元素被返回，而导致内存不连续，如果想使用内存地址做某些特殊逻辑就不再可能了。比如一个数组arrray，包含10个元素，第5个元素的地址被return了。在C/C++中，第5个元素可以通过指针偏移方式来访问的，而Go不行，因为由于它地址被return，它分配在堆上，而数组其他元素没有，那么剩余9个元素在栈上。显然它们的地址是不连续的。Go解决这个问题也比较简单，禁止指针操作。

其次变量保存的位置会影响程序的运行效率。

```go
var global *int

func f() {
    var x int
    x = 1
    global = &x
}

func g() {
    y := new(int)
    *y = 1
}
```

 Go语言的自动垃圾收集器对编写正确的代码是一个巨大的帮助，但也并不是说你完全不用考虑内存了。你虽然不需要显式地分配和释放内存，但是要编写高效的程序你依然需要了解变量的生命周期。 

### 1.6 new 和 make

在Go语言里，new和make都不是关键字，它们都是内建函数，既然是内建函数，因此是开发者可以他用的。

```go
func foo(){
    var a int = 100
    b := 1
    c := make(chan int)
    f := new(float64)
    make := make(map[int]string) //合法
    new := new(struct{Val int}) //合法
}
```

为什么要引入new和make呢？好像没有明确的原因，有个背锅侠就是历史原因。像似一种同理心：“别的语言都有，那我也应该有”。“当你想起来用它的时候，它就在哪里，而不是不存在”。

使用new创建变量和普通局部变量取地址没有什么不同。唯一的优点就是new(T)可以直接在表达式中使用，而不用引入一个变量名。

有一个例外，两个变量的类型不携带任何信息且是零值，比如struct{}，[0]int，目前它们有相同的地址。

### 1.7 自增自减是表达式

Go语言中的自增（++）自减（--）是表达式，而不是像C/C++中的运算符。因此，这样的写是错误的。

```go
i := x++ //error
++x //error
--x //error
```

避免了先后增减的问题。只有加1和减1的一种操作。

### 1.8 分片半开区间

虽然语法上使用一对[]，但是它不具有数学的全闭区间含义，而是左闭，右开。这一特性是从其他编程语言继承过来的。

```go
s[m:n] //s[m,n) 左闭，右开。
s[0:len(s)] //s本身
s[0:i],s[i:j],s[j:len(s)] //分割点边界值重叠
```

这样有一个好处，就是可以使用数组的长度作为右边界。不过，多个连续分片实现对整体的划分时，边界值会重叠，注意这一点和数学含义不同。

`n-m`是分片元素的个数，但是`m=n`时，包括1个元素。`m,n`可以省略

```go
s[:n] //0,1,...,n-1
s[m:] //m,m+1,...,len(s) - 1
s[:] //s自身
```

### 1.9 包级别实体

包级别的实体，包括变量，常量，struct，func无论首字符是否大写。对包里面的所有源文件都可见。但是对包之外的源文件，只有实体名是大写字母开始的实体才对外可见。

### 1.10 多重赋值

函数的多个返回值可以直接用于多值初始化。函数的多返回值相比C/C++而言要十分方便。对于需要变量名，但是逻辑中又不使用的返回值的情况，可以使用空标识符 _ 来忽略掉返回值其中部分返回值。

```go
//方式1
var ret, err = os.Open(name)
//方式2
ret, err := os.Open(name)
//方式3
var ret int
var err error
ret, err = os.Open(name)
//方式4
ret, _ := os.Open(name)
```

这四种方式都是正确的。相比方式2，方式4较常用，也更简洁。方式3主要适用于多个局部作用域公用变量或者先声明，后面才对变量赋值的情况。方式1很少用，原因是方式2比它更简洁。

多重赋值另外一个特别的用途就是交换两数值

```go
x, y = y, x
```

赋值要遵循“可复制性”，规则很简单：类型必须精准匹配。即使是隐式赋值（比如return语句）依然需要类型匹配。nil可以赋值给任何接口变量或者引用类型。

### 1.11 短变量声明

短变量声明":="中，至少有一个新变量声明。否则编译无法通过。

```go
ret, err := fun()
//...
ret, err := fun2() //error - 因为上面同一个作用域内ret，err已经声明
```

如果同一语法块中已经存在变量，则短声明和赋值一样。否则，外层的声明将被忽略等于声明一个新变量

```go
ret, err := fun()
//...
rsp, err := call() //err是旧变量，等于赋值操作，rsp是新变量
{
    ret, err := fun() //ret, err都是新变量，外层的ret，err被忽略掉
}
```

因此在作用域上这里有潜在的风险

```go
func foo()  error{
    var err error
    if cond_true {
        _, err := somecall() //这里会屏蔽外面的err
    }
    return err
}
```

当进入if语句块时候，会引入一个新的作用域。由于使用短声明，因此会创建一个新的局部变量err，屏蔽了外面的二人，导致外部的err并没有得到赋值，返回nil。编译器可以告知用户内部的err没有使用的错误，但是加一个日志输出，就很容易让检查失效。这样让bug变得十分的隐晦。所以尽量避免使用短声明。



### 1.12  类型声明

使用type定义一个新的命名类型，即使它使用了底层类型，它们也不是不兼容了。不能够直接比较和计算。仍然需要显示的类型转换。比如

```go
type CM float64 //cm - 里面单位
type MM float64 //mm - 毫米单位
var l CM
var w MM
```

`float64, CM, MM`是三种数据类型。`l==w`这样的比较以及`l+w，l+2.0`等数值计算都是不合法的。只能够

```go
float64(l) + 2.0
CM(w) + l
w + MM(l)
float64(l) + float64(w)
```

对于每个类型T都有一个对应的类型转换操作T(x)将值x转换为类型T。如果两个类型具有相同的底层类型或者二者都是指向相同底层类型变量的未命名指针类型，则二者可以相互转换。这类类型转换不改变值的表达方式，仅改变类型。这句话意思就是不改变值的存储，只改变辅助类型属性。但是有些类型转换是改变表达方式的，比如浮点转换成整形。字符串转换成slice。

命名类型的底层类型决定了它的结构和表达方式，以及它支持的内部操作集合。比如`CM,MM`具有`float64`相同的算术，逻辑，关系操作。

### 1.13 包初始化

包的初始化从初始化包级别的变量开始。变量按照声明顺序初始化，依据依赖的顺序进行。如果包有多个文件，初始化顺序按照go编译器对go文件的排序顺序进行。

一般使用`init`初始化。它不能被调用和引用。由编译器用来执行用户自己设定的初始化操作。可以有任意多个init，执行顺序按照它们定义的先后顺序。main包是最后初始化的。

```go
func init(){}
```

### 1.14 作用域

作用域和生命周期要区别开来。作用域是编译属性，生命周期是运行时属性。

for, if, switch，case 都是一个独立的词法块。也就是一个新的作用域。在其中的声明，对外不可见。

```go
if ok := fun(); ok {
    fmt.Println("yes")
} else {
    fmt.Println(ok) //ok
}
fmt.Println(ok) //error
```

```go
//下面这三个x都是独立的
func foo() {
    x := "hello"
    for _, x := range x {
        x := x + 'A' - 'a';
        fmt.Println(x)
    }
}
```

虽然这是正确的代码，但是可读性太差了。要坚决避免。

## 二、数据类型

### 2.1 uint不能直接相减

两个uint类型数相减，如果是正数，则一切正常，如果是负数，那过结果将是一个非常大的正数。

```go
var a, b uint
a, b = 1, 2
fmt.Println( a-b) //输出 18446744073709551615
```

### 2.2 slice和append

```go
package main

import "fmt"

func main() {
        s1 := make([]int, 5)
        s2 := make([]int, 0, 5)
        fmt.Printf("s1=%d, s2=%d\n", len(s1), len(s2))
        s1 = append(s1, 1)
        s2 = append(s2, 1)
        fmt.Printf("s1=%d, s2=%d\n", len(s1), len(s2))
}
```

```sh
s1=5, s2=0
s1=6, s2=1
```

指定长度，append后长度加1。

### 2.3 slice和map的区别

map拷贝，则底层数据一致。slice拷贝，如果长度不变则底层数据一致，否则，是两个完全不同的slice

```go
func testMapSlice() {
        s := make([]int, 0, 3)
        s = append(s, 1)
        s1 := s
        fmt.Printf("s=%v, s1=%v\n", s, s1)
        s = append(s, 2)
        fmt.Printf("s=%v, s1=%v\n", s, s1)

        m := make(map[string]int)
        m["e1"] = 10
        m1 := m
        fmt.Printf("m=%v, m1=%v\n", m, m1)
        m["e2"] = 20
        fmt.Printf("m=%v, m1=%v\n", m, m1)
}
```

结果

```sh
s=[1], s1=[1]
s=[1 2], s1=[1] #s和s1是两个对象
m=map[e1:10], m1=map[e1:10]
m=map[e1:10 e2:20], m1=map[e1:10 e2:20]
```

### 2.4 []byte和string仅转换类型

 string 可看做 [2]uintptr，而 []byte可看作 [3]uintptr，便于编写代码，无需额外定义结构类型。如此，str2bytes 只需构建 [3]uintptr{ptr, len, len}，而 bytes2str 更简单，直接转换指针类型，忽略掉 cap 即可 

```go
func str2bytes(s string) []byte {
    x := (*[2]uintptr)(unsafe.Pointer(&s))
    h := [3]uintptr{x[0], x[1], x[1]}
    return *(*[]byte)(unsafe.Pointer(&h))
}
func bytes2str(b []byte) string {
    return *(*string)(unsafe.Pointer(&b))
}
```

### 2.5 指针接收者

 若使用值接收者，那么 `Scale` 方法会对原始 `Vertex` 值的副本进行操作。（对于函数的其它参数也是如此。）`Scale` 方法必须用指针接受者来更改 `main` 函数中声明的 `Vertex` 的值。 

### 2.6 类型断言

**类型断言** 提供了访问接口值底层具体值的方式。

```
t := i.(T)
```

该语句断言接口值 `i` 保存了具体类型 `T`，并将其底层类型为 `T` 的值赋予变量 `t`。

若 `i` 并未保存 `T` 类型的值，该语句就会触发一个恐慌。

为了 **判断** 一个接口值是否保存了一个特定的类型，类型断言可返回两个值：其底层值以及一个报告断言是否成功的布尔值。

```
t, ok := i.(T)
```

若 `i` 保存了一个 `T`，那么 `t` 将会是其底层值，而 `ok` 为 `true`。

否则，`ok` 将为 `false` 而 `t` 将为 `T` 类型的零值，程序并不会产生



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

### map未初始化

map定义之后直接使用会报nil 而 panic

```golang
var kv map[string]string // 未初始化
val, ok := kv["key"] // panic
```

map 作为结构体成员的时候，很容易忘记对它的初始化。

### map遍历时顺序不固定

每次遍历的结果可能不同，不能依靠map的遍历顺序

### 并发读写map不安全

普通的map是并发读写不安全。所谓不安全就是多个线程同时进行读写时，可能导致互相覆盖。

比如线程A读到变量a的值是1，然后进行后续逻辑执行之后再设定2，再写入之前，由于CPU的调度切换，此时另外一个线程B读到1并将它设置为3，然后切换回A继续执行，然后线程A将变量写入2，因此，B的更新被A给覆盖掉了。

有两种普遍的解决办法：互斥锁和乐观锁。前者保证了每一刻只有一个线程读写。也就是A将1变更2完成之后，再运行线程B读写。

比如对map绑定一个sync.RWMutex 

```golang
type RWMap struct { // 一个读写锁保护的线程安全的map
    sync.RWMutex // 读写锁保护下面的map字段
    m map[int]int
}
// 新建一个RWMap
func NewRWMap(n int) *RWMap {
    return &RWMap{
        m: make(map[int]int, n),
    }
}
func (m *RWMap) Get(k int) (int, bool) { //从map中读取一个值
    m.RLock()
    defer m.RUnlock()
    v, existed := m.m[k] // 在锁的保护下从map中读取
    return v, existed
}

func (m *RWMap) Set(k int, v int) { // 设置一个键值对
    m.Lock()              // 锁保护
    defer m.Unlock()
    m.m[k] = v
}
```

当map变得比较大的时候，并发访问量比较大的时候，整个map加锁就就十分低效了。在有些场景里尽量减少临界区里的操作，从而降低加锁的时间。但是对于上面操作逻辑，只有一条语句，显然不能再压缩了。但是把map展开，本质是每个key同时多个线程访问，因此可以从map上解决，比如把加锁粒度缩小到每个key。

```golang
type Value struct {
   sync.RWMutex
   val int
}

type RWMap struct {
    m map[int]Value
}
```

这个粒度就达到最小了。

但是这个粒度锁个数随kv增加而增大。一个变体就是分片加锁，每个key做一次hash分到对应的桶。每个桶分配一个锁。

```golang
type SliceMapItem struct {
  KV map[string]interface{}
  sync.RWMutex
}

type SliceMap []*SliceMapItem

func New() SliceMap {
  sm := make(SliceMap, SLICE_SIZE)
  for i := 0; i < SLICE_SIZE; i++ {
    sm[i] = &SliceMapItem{KV: make(map[string]interface{})}
  }
  return sm
}

func (sm SliceMap) GetItem(key string) *SliceMapItem {
  return sm[atoi(hash(key))%SLICE_SIZE]
}
```



### map扩容按倍增方式缩容空间不变



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

[《Understanding Real-World Concurrency Bugs in Go》](https://songlh.github.io/paper/go-study.pdf)

[Go2.0草案](https://go.googlesource.com/proposal/+/master/design/go2draft.md)