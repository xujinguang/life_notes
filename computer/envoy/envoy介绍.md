### 1. xDS 动态配置

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



###  2.gRPC

除了在数据层面上代理 gRPC 外，Envoy 在控制层面也使用了 gRPC，它从中[获取管理服务器的配置](https://www.servicemesher.com/envoy/configuration/overview/v2_overview.html#config-overview-v2)以及过滤器中的配置，例如用于[速率限制](https://www.servicemesher.com/envoy/configuration/http_filters/rate_limit_filter.html#config-http-filters-rate-limit))或授权检查。我们称之为 *gRPC 服务*。

当指定 gRPC 服务时，必须指定使用 [Envoy gRPC](https://www.envoyproxy.io/docs/envoy/latest/api-v2/api/v2/core/grpc_service.proto.html#envoy-api-field-core-grpcservice-envoy-grpc) 客户端或 [Google C ++ gRPC 客户端](https://www.envoyproxy.io/docs/envoy/latest/api-v2/api/v2/core/grpc_service.proto.html#envoy-api-field-core-grpcservice-google-grpc)。我们在下面的这个选择中讨论权衡。

Envoy gRPC 客户端是使用 Envoy 的 HTTP/2 上行连接管理的 gRPC 的最小自定义实现。服务被指定为常规 Envoy [集群](https://www.servicemesher.com/envoy/intro/arch_overview/cluster_manager.html#arch-overview-cluster-manager)，定期处理[超时、重试](https://www.servicemesher.com/envoy/intro/arch_overview/http_connection_management.html#arch-overview-http-conn-man)、[终端发现](https://www.servicemesher.com/envoy/intro/arch_overview/dynamic_configuration.html#arch-overview-dynamic-config-sds)、[负载平衡、故障转移](https://www.servicemesher.com/envoy/intro/arch_overview/load_balancing.html#arch-overview-load-balancing)、负载报告、[断路](https://www.servicemesher.com/envoy/intro/arch_overview/circuit_breaking.html#arch-overview-circuit-break)、[健康检查](https://www.servicemesher.com/envoy/intro/arch_overview/health_checking.html#arch-overview-health-checking)、[异常检测](https://www.servicemesher.com/envoy/intro/arch_overview/connection_pooling.html#arch-overview-conn-pool)。它们与 Envoy 的数据层面共享相同的[连接池](https://www.servicemesher.com/envoy/intro/arch_overview/connection_pooling.html#arch-overview-conn-pool)机制。同样，集群[统计信息](https://www.servicemesher.com/envoy/intro/arch_overview/statistics.html#arch-overview-statistics)可用于 gRPC 服务。由于客户端是简化版的 gRPC 实现，因此不包括诸如 [OAuth2](https://oauth.net/2/) 或 [gRPC-LB](https://grpc.io/blog/loadbalancing) 之类的高级 gRPC 功能后备。

Google C++ gRPC 客户端基于 Google 在 https://github.com/grpc/grpc 上提供的 gRPC 参考实现。它提供了 Envoy gRPC 客户端中缺少的高级 gRPC 功能。Google C++ gRPC 客户端独立于 Envoy 的集群管理，执行自己的负载平衡、重试、超时、端点管理等。Google C++ gRPC 客户端还支持[自定义身份验证插件](https://grpc.io/docs/guides/auth.md#extending-grpc-to-support-other-authentication-mechanisms)。

在大多数情况下，当你不需要 Google C++ gRPC 客户端的高级功能时，建议使用 Envoy gRPC 客户端。这使得配置和监控更加简单。如果 Envoy gRPC 客户端中缺少你所需要的功能，则应该使用 Google C++ gRPC 客户端。



## 参考资料

1. [《Envoy Proxy使用介绍教程（七）：envoy动态配置xDS的使用方法》](https://www.lijiaocn.com/%E9%A1%B9%E7%9B%AE/2018/12/29/envoy-07-features-2-dynamic-discovery.html#%E4%B8%80%E4%B8%AA%E7%AE%80%E5%8D%95%E7%9A%84management-server%E5%AE%9E%E7%8E%B0)