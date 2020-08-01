## Kafka

### 生产者

```sh
$ bin/kafka-console-producer.sh --bootstrap-server 9.139.3.226:9092 --topic iot-channel-request
```



###消费者

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