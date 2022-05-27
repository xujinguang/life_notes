## MySQL 札记

### 配置

#### 和性能有关的

1. max_connecttions：最大连接数
2. 缓存大小，包括table_cache，key_buffer_size，query_cache_size，sort_buffer_size，read_buffer_size
3. 慢查询日志：log_slow_queries，long_query_time，slow_query_log_file

## 优化
1. 专业的工具做专业的事情，不要让数据库做计算，应该交给内存和CPU。
2. 控制单表数据量，需要提前评估业务数据量。但是拆表也不要过多。
3. 单表字段个数，通过数据量的读写评估。比如1G数据，存储1000w记录，平均算一下一条记录就算出来了。
4. 拒绝3B:big sql；big trans; big batch
5. 选择合适的字段类型；常量优先使用enum和set；字符转换成整形
6. 避免使用NULL：1.null无法查询优化；null增加索引需要额外的存储，含null符合索引失效。
7. 尽量少用TEXT/BLLO以及存储二进制图片
8. 谨慎添加索引，不是越多越好。索引维护需要额外的空间以及增删成本。
9. 不在索引列执行数值计算，无法使用索引且全局扫描。
10. sql尽量小而短，避免大sql
11. 事务采用即开即用，用完关闭。与事务无关的放到事务外，减少锁表时间。
12. 明确指定列名select，避免select *
13. or改为in操作，但是in也不要过大。
14. 避免负向查询：not != <> not like, not in not exists
15. 避免模糊查询：B+树用不了索引，导致全表扫描
16. 避免count(*),非实时可以用cache算，实时指定主键计数
17. limit偏移量越大越慢，指定where条件，减少结果集后减少偏移量
18. 如果无去重要求，指定union all
19. 不推荐高并发场景使用join
20. 明确不排序：order by null 会节省时间。
21. 隔离线上线下环境
22. 永远不在客户端给数据库加锁
23. 统一字符集utf8
24. 统一命名规范

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