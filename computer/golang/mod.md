mod中旧的import路径可以使用replace替代
、go mod常用命令
go build, go test：下载并添加依赖到go.mod中。
go mod tidy：整理，增加缺失的包，移除没用的包。
go mod graph：显示模块间的依赖图。
go mod why：显示为什么需要包。
go list -m all：查看所有的依赖。
go get：下载依赖并更新到go.mod中。
4、go get命令
@v0.3.2：指定tag，Release时建议此方法。
@master：master分支最新commit。
@342b2e：指定commit。go.mod中的表现格式为：v0.0.0-20190529093157-265ded9b6495。
@latest、不指定：默认行为，最新版本；有tag则最新tag，无tag则master分支最新commit。
version前使用>，>=，<，<=：大于/小于指定版本。
v0.0.1-pre：开发时版本较多时使用，不会被 @latest、不指定 自动拉取到。



```go
package main

import (
    "fmt"
    _ "go.uber.org/automaxprocs"
    "runtime"
)

func main() {
    // Your application logic here.
    fmt.Println("real GOMAXPROCS", runtime.GOMAXPROCS(-1))
}
```

>在容器中，Golang程序获取的是宿主机的CPU核数导致GOMAXPROCS设置的过大。比如在笔者的服务中，宿主机是48cores，而实际container只有4cores。
>线程过多，会增加上线文切换的负担，白白浪费CPU。
>[uber-go/automaxprocs](https://github.com/uber-go/automaxprocs) 可以解决这个问题



## Go 协程

> *golang runtime是有个sysmon的协程，他会轮询的检测所有的P上下文队列，***只要 G-M 的线程长时间在阻塞状态，那么就重新创建一个线程去从runtime P队列里获取任务。先前的阻塞的线程会被游离出去了，当他完成阻塞操作后会触发相关的callback回调，并加入回线程组里。***简单说，如果你没有特意配置runtime.SetMaxThreads，那么在可没有复用的线程下会一直创建新线程。*

`G`代表`goroutine`, `P`代表`Logical Processor`, `M`是操作系统线程



## Channel

使用通信来共享数据，而不是通过共享数据来通信。