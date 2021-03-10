## 高性能 MySQL

### 10. 复制

#### 10.1 概述

##### 1.用途

1. 数据分布
2. 负载均衡
3. 备份
4. 高可用
5. 升级测试

##### 2. 复制过程

1. 主库上把数据记录到二进制日志中，bin-log
2. 备库将主库上的日志复制到自己的中继日志中，独立的IO线程完成
3. 备库读取中继日志的事件，将其重放到备库数据上。SQL线程重放

#### 10.2 配置复制

##### 1. 创建复制账号

在主库创建一个用户，并赋予replication slave权限

```sql
grant replicatioin slave, replication client on *.* to copy@'x.x.x.0' identified by 'password';
```

##### 2.配置主库和备库

1. master配置开启binlog
2. slave配置relay-log
3. 配置主备的服务器ID

```sql
show master status
show slave status
```

重要配置

```sh
sync_binlog=1 #每次提交事务之前将二进制日志同步到磁盘上，只适用于binlog
log-bin=/log/mysql-bin #不是必须，但是有更有优势
server_id=123 #
#备库
relay_log=xxx #
skip_slave_start #阻止备库在崩溃后自动启动复制
read_only #只读
```



##### 3.备库从主库复制数据

```sql
> change master to master_host='server'，
master_user='copy',
master_password='password',
master_log_file='mysql-bin.00001',
master_log_pos=0;
```





