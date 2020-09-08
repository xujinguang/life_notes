### xDS 动态配置

 要实现动态配置，需要有一个实现了[data-plane-api](https://github.com/envoyproxy/data-plane-api)的 Server。 Data-plane-api是envoy项目设计的数据平面的接口标准。包括：CDS、EDS、HDS、LDS、MS、RLS、RDS、SDS、ADS

比如 LDS

```protobuf
service ListenerDiscoveryService {
  option (envoy.annotations.resource).type = "envoy.api.v2.Listener";

  rpc DeltaListeners(stream DeltaDiscoveryRequest) returns (stream DeltaDiscoveryResponse) {
  }

  rpc StreamListeners(stream DiscoveryRequest) returns (stream DiscoveryResponse) {
  }

  rpc FetchListeners(DiscoveryRequest) returns (DiscoveryResponse) {
    option (google.api.http).post = "/v2/discovery:listeners";
    option (google.api.http).body = "*";
  }
}
```

API标准统一使用gRPC协议,  长连接、流更新（stream）。  Envoy提供了一个[go-control-plane](https://github.com/envoyproxy/go-control-plane)，是data-plane-api的go语言实现的库，是开发SDK，已封装了Envoy的gRPC通信。 

数据接口定义 https://github.com/envoyproxy/data-plane-api

数据平面sdk  https://github.com/envoyproxy/go-control-plane



 HttpConnectionManager，全权负责HTTP连接和请求的处理 ；使用宏REGISTER_FACTORY，将HttpConnectionManagerConfig静态注册为NetworkFilter 



## 参考资料

1. [《Envoy Proxy使用介绍教程（七）：envoy动态配置xDS的使用方法》](https://www.lijiaocn.com/%E9%A1%B9%E7%9B%AE/2018/12/29/envoy-07-features-2-dynamic-discovery.html#%E4%B8%80%E4%B8%AA%E7%AE%80%E5%8D%95%E7%9A%84management-server%E5%AE%9E%E7%8E%B0)