## MySQL Proxy

### mysql协议

1. 握手链接
2. 权限验证
3. 客户端发送命令数据，由一个包组成
4. 服务端相应数据，可能包括多个包
5. 客户端结束则发送特殊的命令告诉服务端

 包有两种类型：压缩的和非压缩的。数据包包括：客户端的命令和服务端的相应。 服务器响应数据包分为四类：数据包，数据流结束包，成功报告（OK）数据包和错误消息数据包。

它是半双工通信，所谓半双工，就是双方都可以收发数据，但是是一方发完，才能发，类似于对讲机。

####  1.请求数据包

https://dev.mysql.com/doc/internals/en/mysql-packet.html  

 *`payload_length`*[3字节]  +  *`sequence_id`*  [1字节]+ *`payload`* 

采用小端存储。单个包的payload最大 size ($$2^{24}−1$$) 字节，如果超过了，则进行拆包发送，递增`sequence_id`

#### 2. 服务端数据包

通用数据包

https://dev.mysql.com/doc/internals/en/generic-response-packets.html

1. OK_Packet
2. ERR_Packet
3. EOF_Packet
4. Status Flags



### 常见代理

1. MySQL Proxy

官方proxy

2. Atlas

https://github.com/Qihoo360/Atlas

3. kingshard 

4. canal

阿里

5. Cobar 

阿里

### 

