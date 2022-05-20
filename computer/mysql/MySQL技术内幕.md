## MySQL 技术内幕

### 1.体系结构

一张经典图

![](https://img2018.cnblogs.com/blog/403167/201901/403167-20190116145915277-683033214.jpg)

### 2. 存储引擎

1. MyISAM - 不支持事务，表锁，全文索引
2. InnoDB - 支持事务，行锁，外键，聚簇索引
3. NDB
4. Archive - 只支持insert、select，使用压缩
5. Federated
6. Memory - 内存表，一般用于临时表
7. Merge
8. MariaDB 
9. CSV
10. blockhole

问题：为何会有如此之多的存储引擎？
原因有2个，一个重要因素是数据库server和数据操作的分层设计。数据库服务只负责处理和数据无关的内容，真实数据操作交给引擎处理。换言之，定一个了一套通用的接口层。数据库server不关心底层的存储引擎。只调用底层数据的操作接口。这个思想应该出自在Unix/linux的文件系统设计思想。另外一个次要因素是不同的应用场景。
### 3.日志

#### 3.1  慢日志

记录所有超过设定时间的操作。

开启慢查询日志，设定时间long_query_time阈值,单位微秒，落地慢查询日志。

日志变大之后使用mysqldumpslow命令可以帮助查询。

另外，也可以通过log_output变量，将慢日志记录到table中。

#### 3.2 二进制日志

记录所有对DB的写操作。因此select，show等读操作不记录。

作用有两个：复制和恢复。

相关参数：

1. sync_binlog - Control how often the MySQL server synchronizes the binary log to disk. 次数，1标示同步写
2. binlog-do-db
3. binlog-ingore-db
4. log-slave-update
5. binlog_format - What form of binary logging the master will use. [STATEMENT | ROW | MIXED]
6. binlog_cache_size 事务没有提交前写入缓存，提交后写入binlog，这是设定缓冲池大小
7. binlog_cache_use/disk_use

STATEMENT  - sql语句；ROW  - 二进制；MIXED - 混合有限sql，某些情况用ROW

#### 3.3 重做日志

1. 实例的pid文件
2. 表结构frm文件
3. ibdata1文件 innodb_data_file_path设置

默认有两个log_file01

相关参数：

1. innodb_log_file_size - The size of each redo log file.
2. innodb_log_files_in_group
3. innodb_mirrored_log_groups