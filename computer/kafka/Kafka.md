# Kafka

## 1. 基本概念

kafka里面有四个核心元素：生产者-Productor，消费者-Consumer，代理-broker和zookeeper集群。


## 实战
接近点对点的业务耦合和不可扩展性。基于订阅发布的方式，作为中间层存在于多个系统之间。
使用时思考的点：
1. 消息的重要程度如何？（I-重要)
2. 是否允许消息丢失？(M-丢失)
3. 是否接受重复消息？(R-重复)
4. 对延迟和吞吐量的要求？(P-性能)
   

## broker
一个独立的kafka服务器称为broker
broker接收来自生产者的消息，为消息设置偏移量，并提交消息到磁盘。为消费者读取分区的请求作出
响应，返回已经提交的消息给消费者。

broker是集群的组成部分。每个集群都有一个broker同时充当了集群控制器的角色（自动从集群的活跃成员中选举出来）。控制器负责管理工作，包括将分区分配给broker和监控broker。在集群中，一个分区从属于一个broker，该broker被称为分区的首领。一个分区可以分配给多个broker，这个时候会发生分区复制。这种复制机制为分区提供了消息冗余，如果有一个broker失效，其他broker可以接管领导权。
## 生产者

```sh
$ bin/kafka-console-producer.sh --bootstrap-server 9.139.3.226:9092 --topic iot-channel-request
```



## 消费者

```sh
#列出所有消費組
$ bin/kafka-consumer-groups.sh --bootstrap-server 9.139.3.226:9092 --list

#查看实例特定的消费组详情
$ bin/kafka-consumer-groups.sh --bootstrap-server 9.139.3.226:9092 --describe --group asyncdeviceprocessor
#消费组并没有详情。这表示消费者客户端没有使用 consumerGroup 机制去消费数据。
#即客户端没有往服务端提交消费详情，服务端没有存储消费数据，则不会正常显示。

$ bin/kafka-console-consumer.sh --bootstrap-server 9.139.3.226:9092 --from-beginning --topic iot-channel-request --group asyncdeviceprocessor

#ERROR Error processing message, terminating consumer process:  (kafka.tools.ConsoleConsumer$)
#org.apache.kafka.common.protocol.types.SchemaException:
#Error reading field 'owned_partitions': java.nio.BufferUnderflowException
#crash , 原因这个topic只有一个分区，这个消费群已经将分区占用了，再执行消费就没有分区了，所以crash了
```