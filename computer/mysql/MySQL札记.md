## MySQL 札记

### 配置

#### 和性能有关的

1. max_connecttions：最大连接数
2. 缓存大小，包括table_cache，key_buffer_size，query_cache_size，sort_buffer_size，read_buffer_size
3. 慢查询日志：log_slow_queries，long_query_time，slow_query_log_file

## 优化

### SQL

1. 通过慢查询得知相应SQL
2. explian 分析执行计划
3.  set profiling=1;  分析执行时间和资源消耗

### 架构

1. 分库分表
2. 集群读写分离
3. 增加业务cache
4. 连接池



## 容灾

1. 主从同步
2. mysql备份和恢复



## 引擎

1. innodb
2. MyISAM



## 测试

mysqlslap