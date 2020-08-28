## Kafkacat手册

### 0.介绍

kafka官方的一堆脚本用来验证操作kafka消息很好用，但是无法查看kafka消息的header。查了一下使用kafkacat可以，而且操作也十分方便。

kafkacat - 生产和消费分别通过stdin，stdout输入输出，主要用来调试查阅消息。十分快速小巧的工具。

### 1.下载

【[Kafkacat GitHub](https://github.com/edenhill/kafkacat)】的[release](https://github.com/edenhill/kafkacat/releases)下载最新源码到机器上

```shell
wget https://github.com/edenhill/kafkacat/archive/1.6.0.tar.gz -O kafkacat_1.6.0.tar.gz
```

### 2.编译

```shell
tar -xvf kafkacat_1.6.0.tar.gz
#进入源码
./configure
make
```

依赖项

```shell
yum install gcc -y
yum install librdkafka-dev -y
```

### 3.使用简介

#### 3.0 kafkacat的基础参数

* -P  **P**roduce data 
* -C = **C**onsume data
* -L = **L**ist metadata
* -Q = **Q**uery
* -b = **b**roker
* -t = **t**opic

#### 3.1 生产者

```shell
kafkacat -b 172.18.0.16:9092 -t status-sync -P -H cookie=status -H type=request -K:
message:{"hello":"kafka"}
```

* -H = **H**eader，多个header需要重复使用-H参数
* -K = **K**eyValue，添加消息kv对
* -l 从文件获取消息内容
* -z =**z**ip，指定消息压缩格式： snappy, gzip and lz4. 
* -G = **G**roup 指定消费组或者加入已存在的消费组

header不是必须的。

```shell
# 从文件获得消息内容,并采用gzip压缩消息
kafkacat -b 172.18.0.16:9092 -t status-sync -P  -K: -l data.txt -z gzip
```

#### 3.2 消费者

```shell
kafkacat -b 172.18.0.16:9092 -t status-sync  -C -c1 -o 7425
```

* -c1 消费的消息个数，c后面指定消息的个数，如果topic里的消息不足指定的个数，则会阻塞等待
* -o = **o**ffset 指定消费的偏移量
  * 正值 - 从起始偏移
  * 负值 - 从末尾逆向偏移
  * 关键字 - beginning / end
  * 起始时间戳 - s@start_timestamp
  * 结束时间戳 -  e@end_timestamp
* -f = **f**ormat 格式化输出
  * Topic (%t),
  * partition (%p)
  * offset (%o)
  * timestamp (%T)
  * message key (%k)
  * message value (%s)
  * message headers (%h)
  * key length (%K)
  * value length (%S)

```shell
# 从末尾阻塞等待新的消息到来，并格式化输出
kafkacat -b 172.18.0.16:9092 -t status-sync  -C -o end -f '\nKey (%K bytes): %k
  Value (%S bytes): %s
  Timestamp: %T
  Partition: %p
  Offset: %o
  Headers: %h\n'
```

#### 3.3 其他

```shell
#query
kafkacat -b localhost:9092 -Q -t topic1:1:1588534509794
#list metadata
#display the current state of the Kafka cluster and its topics and partitions
kafkacat -L -b localhost:9092 -t topic1
```

更多使用方式参考【[官方示例](https://github.com/edenhill/kafkacat#examples)】

