## HTTP

HTTP有几个版本，1.0和1.1之间差异不大，但是到3之后，“HTTP建立在TCP上”这样的术语就不准确了。HTTP3实现的基于UDP的新传输协议QUIC

### 1.REST ful

1. **Resources** **就是网络上的一个实体，或者说是网络上的一个具体信息**
2. **表现层（Representation）** **"资源"具体呈现出来的形式**
3. **状态转化（State Transfer）** 将表现层从一种状态转换到另外一种状态

综合上面的解释，我们总结一下什么是RESTful架构：

　　（1）每一个URI代表一种资源；

　　（2）客户端和服务器之间，传递这种资源的某种表现层；

　　（3）客户端通过四个HTTP动词，对服务器端资源进行操作，实现"表现层状态转化"。

### 2. gRPC

优点：

* 服务网格生态标准，比如k8s，Istio都使用的grpc
* 易用，资料全
* pb协议，CS同一套
* grpc采用了http2, 性能比http1.1好
* 多路复用
* gRPC-go更优秀

劣势：

- 仅提供了RPC，即远程调用，缺少了像Dubbo那样的服务注册、服务治理功能
- 不具有像http一样的可读性，可能会对调试造成一定不便
- 没有连接池
- 日志，trace,context，监控都需要自己来搞。

### 参考文献

[1]. [理解RESTful架构](http://www.ruanyifeng.com/blog/2011/09/restful.html)

