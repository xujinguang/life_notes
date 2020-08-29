## Redis 实战

由于redis支持热备和冷备数据高可用，集群高负载以及单机QPS的高性能，成为缓存届一霸。此文简单介绍一下在实际使用中的问题。

Redis 最常见的两种使用方式：

1. 内存数据库；
2. Redis（缓存） + MySQL（持久化）；

追求的目标：数据一致性、缓存击穿、缓存雪崩等问题，降低了业务的复杂性，提升开发效率，降低运维成本

### 1. 内存数据库

redis提供丰富的数据类型，满足各种业务的需求。redis 相比mangodb，mysql更高的读写性能，而且提供持久化方案，因此在允许数据一定程度丢失的场景下，可以作为内存数据库来使用。

### 2. Redis + MySQL

假设在微服务背景下，逻辑层，Redis和MySQL都是独立的微服务，由逻辑层负责对数据的 CRUD ，展开讨论。大进程的操作模式都是"一荣俱荣，一损俱损"比较好分析不做展开。正常逻辑这没啥可说的。但是重点是非正常逻辑，这才是关键。

#### 1.Create

Sever 有两个操作：1.写Redis；2.写DB。

**方案1** ：Server 先写Redis，无论成功与否；继续写DB；

有如下几经情景：

1. 写Redis之前Server崩溃了，那么数据丢失；
2. 写DB之前Server崩溃了，那么数据丢失了，如果Redis写成功，则Redis存在脏数据。
3. 写Redis失败（崩溃），写DB成功；Redis恢复，Redis数据落后于DB，可以在读DB时候更新Redis；
4. 写DB失败，返回用户失败。如果Redis写入成功，需要回删Redis，删除失败，需要引入其他机制保证数据一致性。

这个方案对于重复请求有个弊端，写入之前如何检测Redis中是否存在，对于Redis写入成功，DB写入失败这种情况，就会导致误认为已经写入成功，而无法重写。此时只能通过已检测DB是否存在为主或者引入审计清理Redis中的脏数据。

**方案2**：Server先写DB，成功则写Redis，不管Redis是否成功。

1. Server写DB之前崩溃了，数据丢失；
2. Server写DB之后崩溃了，数据写入成功，只是Redis未更新
3. Redis写入失败，只上报监控，可以不做业务处理。

这个方案对于高并发有个弊端就是有那么一瞬间所有重复写请求都压在了DB上面。Redis起不到一定程度的抗压作用（因为多个请求都在写DB还没有到写Redis）。

**方案3**：Server并行写

这个其实和方案有可能会出现上面所有情况。由于Redis的高效性，Redis的写操作的延时作用影响不大，所以不推荐为了节省Redis的写时间而采用并发写。

#### 2.Read

Redis get如果不存在，则查询DB，查询到，则将数据更新到Redis。

1. 如果大数据量请求同一个数据，会导致多个请求重复写Redis，解决办法就是使用setnx，如果存在则什么都不做。
2. 如果Redis缓存为空，系统初次提供服务，导致DB请求负载过高。可以预设布隆过滤器。
3. 如果更新Redis失败，把重试交给下一个请求，无需处理。
4. 如果Redis缓存数据存在，直接返回。这是使用缓存的最终

#### 3.Update

创建一样。有两步操作：1.更新Redis的kv，2.更新DB。先更新谁呢？同理，也有三种方式，效果和写操作类似。比较常用的方案是先写DB，然后更新Redis。原因是如果写DB失败，则DB和Redis数据一致。如果写DB成功，写Redis失败，经过一定时间Redis对应key过期，最终更新到一致。对于实时要求一致性的场景，不建议使用缓存。

#### 4.Delete

两步操作：1.删除Redis的key；2.删除DB记录。无论删除Redis的key是否成功，可以忽略。保证删除DB成功即可。如果删除Redis的key失败，那么等到key过期，最终数据一致。反之，如果先删除Redis的key，那么在未删除DB

### 3. 数据备份

#### 是否备份？

如果Redis的数据不备份会怎么样？比如memorycache就是如此，一旦服务宕机，所有缓存数据被清空，曾经的热数据需要随业务请求重新来一遍。由于只是缓存数据，属于锦上添花，好像丢了也没有太大的影响。因此，备份是不是必须呢？

1. 如果是当做内存数据库的话，那么Redis的数据备份是必须的。
2. 如果和memorycache一样的缓存应用场景，数据丢失问题不大。
3. 如果在Redis+MySQL的组合情况下，先写的Redis，再写的DB，那么这时候持久化是必须的。

出于服务宕机，重启后减少对服务负载，数据备份是必须的。至于数据备份周期可以根据业务场景而定。

#### AOF 和 RDB 的选择

1. AOF

   ```yaml
   appendonly yes #默认是 no
   appendfilename "appendonly.aof" #aof文件名
   appendfsync everysec 
   # no/everysec/always - 后面两个都是独立线程来处理，某些Linux的配置下会使Redis因为 fsync()系统调用而阻塞
   # 注意，目前对这个情况还没有完美修正，甚至不同线程的 fsync() 会阻塞我们同步的write(2)调用。
   # 为了缓解这个问题，可以用下面这个选项。它可以在 BGSAVE 或 BGREWRITEAOF 处理时阻止fsync()。
   # 这就意味着如果有子进程在进行保存操作，那么Redis就处于"不可同步"的状态。
   # 这实际上是说，在最差的情况下可能会丢掉30秒钟的日志数据。（默认Linux设定）
   # 如果把这个设置成"yes"带来了延迟问题，就保持"no"，这是保存持久数据的最安全的方式。
   no-appendfsync-on-rewrite no
   # 自动重写AOF文件。如果AOF日志文件增大到指定百分比，Redis能够通过 BGREWRITEAOF 自动重写AOF日志文件。# 工作原理：Redis记住上次重写时AOF文件的大小（如果重启后还没有写操作，就直接用启动时的AOF大小）
   # 这个基准大小和当前大小做比较。如果当前大小超过指定比例，就会触发重写操作。你还需要指定被重写日志的最小尺寸，这样避免了达到指定百分比但尺寸仍然很小的情况还要重写。
   # 指定百分比为0会禁用AOF自动重写特性。
   auto-aof-rewrite-percentage 100
   auto-aof-rewrite-min-size 64mb
   ```

   

2. RDB

   ```yaml
   #  900秒（15分钟）之后，且至少1次变更
   #  300秒（5分钟）之后，且至少10次变更
   #  60秒之后，且至少10000次变更
   # 不写磁盘的话就把所有 "save" 设置注释掉就行了。
   # 通过添加一条带空字符串参数的save指令也能移除之前所有配置的save指令，如: save ""
   save 900 1
   save 300 10
   save 60 10000
   
   # 默认情况下如果上面配置的RDB模式开启且最后一次的保存失败，redis 将停止接受写操作，让用户知道问题的发生。
   # 如果后台保存进程重新启动工作了，redis 也将自动的允许写操作。如果有其它监控方式也可关闭。
   stop-writes-on-bgsave-error yes
   
   # 是否在备份.rdb文件时是否用LZF压缩字符串，默认设置为yes。如果想节约cpu资源可以把它设置为no。
   rdbcompression yes
   
   # 因为版本5的RDB有一个CRC64算法的校验和放在了文件的末尾。这将使文件格式更加可靠,
   # 但在生产和加载RDB文件时，这有一个性能消耗(大约10%)，可以关掉它来获取最好的性能。
   # 生成的关闭校验的RDB文件有一个0的校验和，它将告诉加载代码跳过检查
   rdbchecksum yes
   # rdb文件名称
   dbfilename dump.rdb
   
   # 备份文件目录，文件名就是上面的 "dbfilename" 的值。累加文件也放这里。
   # 注意你这里指定的必须是目录，不是文件名。
   dir /var/log/db
   
   # Remove RDB files used by replication in instances without persistence
   # enabled. By default this option is disabled, however there are environments
   # where for regulations or other security concerns, RDB files persisted on
   # disk by masters in order to feed replicas, or stored on disk by replicas
   # in order to load them for the initial synchronization, should be deleted
   # ASAP. Note that this option ONLY WORKS in instances that have both AOF
   # and RDB persistence disabled, otherwise is completely ignored.
   #
   # An alternative (and sometimes better) way to obtain the same effect is
   # to use diskless replication on both master and replicas instances. However
   # in the case of replicas, diskless is not always an option.
   rdb-del-sync-files no
   ```

   

3. AOF + RDB
   上面两个配置同时开启。  

参考[redis config](https://redis.io/topics/config)

### 4. 数据恢复

Redis 恢复加载实现逻辑：先检查AOF，存在则使用AOF；如果不存在，则检查RDB，如果都不存在，就是空DB。

### 5. 数据运维

#### 5.1 设置过期时间

假设存在恶意请求，读取的都是DB的冷数据。如果数据量巨大，缓存key不设置过期时间的话，那么内存很快就会被吃满告警。就像现在的冷不丁的暴雨让河水迅速爆满湖泊河道一样，如果不及时泄洪就必然导致洪灾。所以在创建缓存的时候就要一定为key设置好淘汰时间，即使短时间让数据留存，一段时间内就过期淘汰——定时泄洪。

缓存的目的是因为数据局部性原理——一段时间内，用户总是访问最近访问过的数据。过一段时间之后，数据可能访问到的概率大大降低，此时一段时间后数据没有留下的必要性。（本事只在低概率事件中体现，架构在大数据量下才可讨论，正所谓智慧存在于细微之中。小服务，大资源都没有讨论的必要性，随便造好了！）

此外，如果业务不对键设置过期时间。可以通过Redis的键设置淘汰策略。Redis提供6种淘汰策略。在Redis配置中指定，由Redis帮你淘汰数据。

```yaml
# 最大内存策略：如果达到内存限制了，Redis如何选择删除key。
# volatile-lru -> 根据LRU算法删除设置过期时间的key
# allkeys-lru -> 根据LRU算法删除任何key
# volatile-random -> 随机移除设置过过期时间的key
# allkeys-random -> 随机移除任何key
# volatile-ttl -> 移除即将过期的key(minor TTL)
# noeviction -> 不移除任何key，只返回一个写错误
# 注意：对所有策略来说，如果Redis找不到合适的可以删除的key都会在写操作时返回一个错误。
# 目前为止涉及的命令：set setnx setex append incr decr rpush lpush rpushx lpushx linsert lset rpoplpush sadd sinter sinterstore sunion sunionstore sdiff sdiffstore zadd zincrby zunionstore zinterstore hset hsetnx hmset hincrby incrby decrby getset mset msetnx exec sort
# 默认策略:
# maxmemory-policy noeviction

# LRU和最小TTL算法的实现都不是很精确，但是很接近（为了省内存），所以你可以用样本量做检测。 例如：默认Redis会检查3个key然后取最旧的那个，你可以通过下面的配置指令来设置样本的个数。
# 默认值为5，数字越大结果越精确但是会消耗更多CPU。
# maxmemory-samples 5
```



#### 5.2 缓存雪崩

设置过期时间有个特别注意事项。尽量设置key过期时间随机均匀分布。

对于低负载请求，如果所有key设置过期一样，那么问题不大。但是对于短时间内，比如1s内有几十万个新请求的情况下，他们的过期时间相同，那么会导致在同一时间失效，引发缓存雪崩。所有负载瞬间压向DB。简单的解决办法是设置key过期时间随机化。

#### 5.3 缓存穿透

这个就是5.1提到的大量的恶意冷数据请求，导致所有的缓存无法命中，请求压到DB。如果DB不存在这样的数据，其实就是无用的查询操作。如果DB存在这样的数据，读取后更新到缓存，又导致大量的冷数据占据了内存，而可能引发热数据的淘汰。

对于无用的查询操作解决办法就是使用布隆过滤器，将db数据哈希到一个bitmap中。查询前先经过布隆过滤器，可以将一定不存在的数据查询过滤掉。对于一定存在的数据查询，内存不足的情况可以设置随机淘汰策略。

#### 5.4 缓存击穿

对某些key可能会在某些时间点被超高并发地访问，是一种非常“热点”的数据，如果某个key突然失效，会到导致大量数据负载到DB。

针对这种情况，不能使用每个请求都去更新Redis操作。即使DB没垮掉，如此也会导致大量的重写Redis的操作。可靠的解决办法是先执行Redis的get请求，如果返回空，则使用setnx设置一个互斥锁。有拿到锁的请求去将过期的key更新到Redis，然后释放锁。没有拿到锁的请求sleep一段时间重新执行上述操作（先get...）。保证了只有一个请求去更新Redis。

