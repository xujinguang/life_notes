## etcd

 一个高可用强一致性的键值仓库 。 最经典的使用场景就是服务发现。它的角色不是其他NoSQL产品的替代品， 默认处理的数据都是系统中的控制数据，比如配置。

### 特点

- 简单：易于部署，易使用。基于 HTTP+JSON 的 API 让你用 curl 就可以轻松使用。
- 安全：可选 SSL 客户认证机制。
- 快速：每个实例每秒支持一千次写操作。
- 高可用：使用一致性 Raft 算法充分实现了分布式。

### 组件

<img src="https://user-gold-cdn.xitu.io/2019/10/20/16de6f9b9563c22d?imageView2/0/w/1280/h/960/format/webp/ignore-error/1" style="zoom:50%;" /> 

### 服务发现

 服务发现就是想要了解集群中是否有进程在监听 udp 或 tcp 端口，并且通过名字就可以查找和连接。需要以下能力支持，缺一不可。

1.  **一个强一致性、高可用的服务存储** 
2.  **一种注册服务和监控服务健康状态的机制**
3.  **一种查找和连接服务的机制** 

etcd就是它的可选实现之一。除此之外，可以使用zookeeper，redis。

### 要点



### 参考资料

【1】[etcd](https://juejin.im/post/6844903970461351944)