##TCP

### 1. 三次握手

### 2. 四次挥手

### 3. TIME_WAIT状态

### 4. RST包

- 端口未打开，Linux服务器会回包RST
-  设置了recv的超时时间，服务端会回RST
- 服务端提前关闭连接
-  客户端在服务端已经关闭掉socket之后，仍然在发送数据。这时服务端会产生RST。 

不会发送RST包情况

*  使用**shutdown、close**关闭套接字，发送的是FIN 
*   套接字关闭前，对运行的程序**Ctrl+C**，发送的是FIN 
*  套接字关闭前，执行**return、exit(0)、exit(1)**，会发送FIN 

