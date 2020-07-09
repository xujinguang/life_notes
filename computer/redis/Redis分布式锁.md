

# Redis分布式锁

CreateTime: 2020-07-07 20:54

##分布式锁

###什么是分布式锁

在分布式系统中，多个服务或者请求同时操作互斥数据，保证它们有序的访问共享资源，避免竞争而发生数据不一致等问题，引入分布式锁，保证它们互斥的访问共享资源。

###分布式锁特点

1. 互斥性
2. 可重入性
3. 锁超时
4. 高可用
5. 阻塞和非阻塞特性

### 分布式锁实现方式

1. 基于数据库
2. 基于Redis
3. 基于zookeeper

##Redis分布式锁

```
SET key value [EX seconds|PX milliseconds] [NX|XX] [KEEPTTL]
EX seconds -- Set the specified expire time, in seconds.
PX milliseconds -- Set the specified expire time, in milliseconds.
NX -- Only set the key if it does not already exist.
XX -- Only set the key if it already exist.
KEEPTTL -- Retain the time to live associated with the key.
```

```
The command SET resource-name anystring NX EX max-lock-time is a simple way to implement a locking system with Redis.

A client can acquire the lock if the above command returns OK (or retry after some time if the command returns Nil), and remove the lock just using DEL.

The lock will be auto-released after the expire time is reached.

It is possible to make this system more robust modifying the unlock schema as follows:

Instead of setting a fixed string, set a non-guessable large random string, called token.
Instead of releasing the lock with DEL, send a script that only removes the key if the value matches.
This avoids that a client will try to release the lock after the expire time deleting the key created by another client that acquired the lock later.

An example of unlock script would be similar to the following:

if redis.call("get",KEYS[1]) == ARGV[1]
then
    return redis.call("del",KEYS[1])
else
    return 0
end
The script should be called with EVAL ...script... 1 resource-name token-value
```

官网已经教我们怎么做了。说的十分的详细，有几个要点强调一下：

#### 1.为什么setnx 不可以？

虽然`setnx`本身可以保证原子性，但是还要单独调用一次`expired`设置锁的超时时间。那么这两个命令不能保证原子执行，如果`setnx`成功后，获得锁的服务异常，超时时间又无法设置，导致死锁了，其他服务无法拿到锁。有补救办法，当然引入了复杂度，那不如让`set`命令一步到位。

#### 2.为何要设置超时时间？

避免某些服务超长持有锁，超长持有锁可以是：1.临界区运行过长；2.阻塞在临界区；3.服务crash。设置一个合理的超时时间，保证锁在合理时间内总被释放掉。

#### 2.为什么要设置随机唯一字符串？

为锁设置唯一的随机字符串，保证这个锁只能有持有锁的服务释放，好似一把钥匙。避免服务A释放掉服务B锁。什么时候会发生这种情况。当持有锁的服务A，由于某种原因阻塞在临界区或者在临界区执行时间过长，锁超时释放了，然后被新的服务B获得。如果不检测锁的钥匙值，阻塞的服务A醒来再去释放锁，会将服务B持有的锁释放掉，造成问题。通过锁的`value`验证，可以避免这种情况。

#### 3.为什么要用lua脚本释放锁？



