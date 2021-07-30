#[《Quic标准-RFC9000》](https://www.rfc-editor.org/rfc/rfc9000.html)

## 术语定义

文档中核心的几个概念

1. QUIC——全称，不是缩写。
2. Endpoint —— 通过生成、接收和处理 QUIC 数据包参与 QUIC 连接的实体。QUIC 中只有两种类型的端点: client和server
3. QUIC packet —— QUIC数据包，处理的完整单元，封装在UDP包中。一个UDP放不下怎么办？遵循MTU，因此不会放不下。
4. Ack-eliciting packet —— 除 ACK、 PADDING 和 connection_close 以外的帧的 QUIC 数据包
5. Frame —— 结构化协议信息的一个单位。有多种帧类型，每种类型携带不同的信息。帧包含在 QUIC 数据包中
6. Address ——  IP 版本、 IP 地址和 UDP 端口号的元组
7. Connection ID —— 标识端点上的 QUIC 连接，数据包中携带，区分包的所属连接。
8. Stream —— QUIC 连接中有序字节的单向或双向通道。为app提供轻量级的、有序的字节流抽象。 QUIC 连接可以同时传输多个流。
9. Application —— 使用 QUIC 发送和接收数据的实体， 比如http app

## 架构

<img src="F:\myself\life_notes\computer\net\quic\quic架构图.png" alt="image-20210730173911043" style="zoom:50%;" />

## 2.streams

### 2.0 特点

1. 流自身有序字节流
2. 流之间可以并发同时运行
3. 任何一方都可以创建，取消
4. 不同流之间字节序无法保证

### 2.1 流类型和标识

单向流和双向流

流 ID 是一个62位的整数，最低位用来标识client=0和server=1, 第2个低位标识流向：双向=0，单向=1；QUIC 端点不能在连接中重用流 ID。

```c
// 获取变长整型表达的整型值
int ReadVarint(int *data) {
    if (data == NULL) return -1;
    int v = *data; // 第一字节
    int prefix = v >> 6; // 取2bit
    int length = 1 << prefix; // 字节个数：00=1|01=2|10=4|11=8
    v = v & 0x3F; // 清空头2bit
    // 将后续字节加起来
    for(int i = 1; i < length; i++) {
        v = (v << 8) + *(data + i);
    }
    return v; // 长度值
}
```

