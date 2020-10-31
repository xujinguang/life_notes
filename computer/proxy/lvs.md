## LVS

 LVS，全称Linux Virtual Server 。基于四层、具有强大性能的反向代理服务器。 

LVS通过工作于内核的ipvs模块来实现功能，其主要工作于netfilter 的INPUT链上。
 而用户需要对ipvs进行操作配置则需要使用ipvsadm这个工具。
 ipvsadm主要用于设置lvs模型、调度方式以及指定后端主机。

### 三种模式

1. NAT模式
2. IP隧道模式
3. 直接路由模式

###  八种调度算法 

1.  轮叫调度(Round-RobinScheduling)
2.  加权轮叫调度(WeightedRound-RobinScheduling) 
3.  最小连接调度(Least-ConnectionScheduling) 
4.  加权最小连接调度(WeightedLeast-ConnectionScheduling) 
5.  基于局部性的最少链接(Locality-BasedLeastConnectionsScheduling) 
6.  带复制的基于局部性最少链接(Locality-BasedLeastConnectionswithReplicationScheduling) 
7.  目标地址散列调度(DestinationHashingScheduling)
8.  源地址散列调度(SourceHashingScheduling) 