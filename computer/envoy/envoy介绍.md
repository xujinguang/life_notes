## Envoy 介绍

### xds 配置

 Domain Specific Language 

1. RDS - 路由规则
2. CDS - 集群规则
3. EDS - 后端规则
4. LDS - 监听规则
5. SDS - 证书规则

数据接口定义 https://github.com/envoyproxy/data-plane-api

数据下发控制 https://github.com/envoyproxy/go-control-plane

 HttpConnectionManager，全权负责HTTP连接和请求的处理 ；使用宏REGISTER_FACTORY，将HttpConnectionManagerConfig静态注册为NetworkFilter 

