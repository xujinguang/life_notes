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



```sh
2020-09-28T12:24:06.288358Z     info    FLAG: --concurrency="2"
2020-09-28T12:24:06.288419Z     info    FLAG: --disableInternalTelemetry="false"
2020-09-28T12:24:06.288426Z     info    FLAG: --domain="default.svc.cluster.local"
2020-09-28T12:24:06.288430Z     info    FLAG: --help="false"
2020-09-28T12:24:06.288433Z     info    FLAG: --id=""
2020-09-28T12:24:06.288436Z     info    FLAG: --ip=""
2020-09-28T12:24:06.288438Z     info    FLAG: --log_as_json="false"
2020-09-28T12:24:06.288446Z     info    FLAG: --log_caller=""
2020-09-28T12:24:06.288449Z     info    FLAG: --log_output_level="default:info"
2020-09-28T12:24:06.288451Z     info    FLAG: --log_rotate=""
2020-09-28T12:24:06.288454Z     info    FLAG: --log_rotate_max_age="30"
2020-09-28T12:24:06.288457Z     info    FLAG: --log_rotate_max_backups="1000"
2020-09-28T12:24:06.288460Z     info    FLAG: --log_rotate_max_size="104857600"
2020-09-28T12:24:06.288462Z     info    FLAG: --log_stacktrace_level="default:none"
2020-09-28T12:24:06.288467Z     info    FLAG: --log_target="[stdout]"
2020-09-28T12:24:06.288471Z     info    FLAG: --meshConfig="./etc/istio/config/mesh"
2020-09-28T12:24:06.288473Z     info    FLAG: --mixerIdentity=""
2020-09-28T12:24:06.288476Z     info    FLAG: --outlierLogPath=""
2020-09-28T12:24:06.288478Z     info    FLAG: --proxyComponentLogLevel="misc:error"
2020-09-28T12:24:06.288481Z     info    FLAG: --proxyLogLevel="warning"
2020-09-28T12:24:06.288484Z     info    FLAG: --serviceCluster="details.default"
2020-09-28T12:24:06.288486Z     info    FLAG: --serviceregistry="Kubernetes"
2020-09-28T12:24:06.288490Z     info    FLAG: --stsPort="0"
2020-09-28T12:24:06.288494Z     info    FLAG: --templateFile=""
2020-09-28T12:24:06.288499Z     info    FLAG: --tokenManagerPlugin="GoogleTokenExchange"
2020-09-28T12:24:06.288504Z     info    FLAG: --trust-domain="cluster.local"
2020-09-28T12:24:06.288549Z     info    Version 1.7.0-2022348138e47498c4b54995b4cb5a1656817c4e-Clean
2020-09-28T12:24:06.288699Z     info    Obtained private IP [172.16.0.26]
2020-09-28T12:24:06.288815Z     info    Apply proxy config from env {"proxyMetadata":{"DNS_AGENT":""}}

2020-09-28T12:24:06.289961Z     info    Effective config: binaryPath: /usr/local/bin/envoy
concurrency: 2
configPath: ./etc/istio/proxy
controlPlaneAuthPolicy: MUTUAL_TLS
discoveryAddress: istiod.istio-system.svc:15012
drainDuration: 45s
envoyAccessLogService: {}
envoyMetricsService: {}
parentShutdownDuration: 60s
proxyAdminPort: 15000
proxyMetadata:
  DNS_AGENT: ""
serviceCluster: details.default
statNameLength: 189
statusPort: 15020
terminationDrainDuration: 5s
tracing:
  zipkin:
    address: zipkin.istio-system:9411

2020-09-28T12:24:06.290035Z     info    Proxy role: &model.Proxy{Type:"sidecar", IPAddresses:[]string{"172.16.0.26"}, ID:"details-v1-5974b67c8-xr7pf.default", Locality:(*envoy_config_core_v3.Locality)(nil), DNSDomain:"default.svc.cluster.local", ConfigNamespace:"", Metadata:(*model.NodeMetadata)(nil), SidecarScope:(*model.SidecarScope)(nil), PrevSidecarScope:(*model.SidecarScope)(nil), MergedGateway:(*model.MergedGateway)(nil), ServiceInstances:[]*model.ServiceInstance(nil), IstioVersion:(*model.IstioVersion)(nil), ipv6Support:false, ipv4Support:false, GlobalUnicastIP:"", XdsResourceGenerator:model.XdsResourceGenerator(nil), Active:map[string]*model.WatchedResource(nil), ActiveExperimental:map[string]*model.WatchedResource(nil), RequestedTypes:struct { CDS string; EDS string; RDS string; LDS string }{CDS:"", EDS:"", RDS:"", LDS:""}}
2020-09-28T12:24:06.290047Z     info    JWT policy is first-party-jwt
2020-09-28T12:24:06.290144Z     info    PilotSAN []string{"istiod.istio-system.svc"}
2020-09-28T12:24:06.290155Z     info    MixerSAN []string{"spiffe://cluster.local/ns/istio-system/sa/istio-mixer-service-account"}
2020-09-28T12:24:06.290202Z     info    sa.serverOptions.CAEndpoint == istiod.istio-system.svc:15012
2020-09-28T12:24:06.290215Z     info    Using user-configured CA istiod.istio-system.svc:15012
2020-09-28T12:24:06.290219Z     info    istiod uses self-issued certificate
2020-09-28T12:24:06.290285Z     info    the CA cert of istiod is: -----BEGIN CERTIFICATE-----
MIIC3jCCAcagAwIBAgIRAMT3Ue5pThhDnLEItKfN92kwDQYJKoZIhvcNAQELBQAw
GDEWMBQGA1UEChMNY2x1c3Rlci5sb2NhbDAeFw0yMDA4MjYwNzM5MzRaFw0zMDA4
MjQwNzM5MzRaMBgxFjAUBgNVBAoTDWNsdXN0ZXIubG9jYWwwggEiMA0GCSqGSIb3
DQEBAQUAA4IBDwAwggEKAoIBAQDEJKneOp9lr1SL51b66TlxQnnYtOm7eRzRd4zk
u6xs82B8/R4e0r/QiObPSwV5WB9Mt+WUa/3L85B18MxKmwyslPSXamfrDGZWAzdQ
0KqbakCEQt/lUhbD8PXy4RIVmu6yf0qCKo7xvKqbif1fh+lsmEa+Unw7Noxj6Gms
FplOj88kwxCdaWE1NUFNgSc3xMrrUxKYT0bDA2XktIPLQdczjYCWOg00BzPOdrNZ
w6okiiU6pZrPmO09P5GlzB8AOYOZBa18beGWRknRl6zue9c6f0cA7Ou66qLGD8qV
xFnz4LGIqlv2NW05LLuCC2xFeHUBDQxf0eSV/diolwckp/6LAgMBAAGjIzAhMA4G
A1UdDwEB/wQEAwICBDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IB
AQA3U/6VfDksku0tCE87QMKz/7QVlZK52+8T2aQKsbNNLtjssEle4Uta48f+cDsn
pF87d01kvFH71O7SKy5D0I/7BUEDUGyvZxXUdNPGYZBhqMcKYoX7lkkkKlkclmdw
l3I9T3kw8T+QV4a7iRKAm89ODy6FY772uM6L6BK8vk1AmAc5C5FpJJ07Vm+QVNki
R9YWapLcAV+PBGFNoSXShpBfiyx+rZlL3m53RIAFRAv6xxZzJ2vbu91Vo9DrhNAF
IQ1B5CZUgdxZszTIvDBykB+0TMhzWKFnvzbLJQ5yOevZlrzC1FPieanoET3QsKx6
0rZeI+yNoJIJ3keqRjSfcwPa
-----END CERTIFICATE-----

2020-09-28T12:24:06.341301Z     info    sds     SDS gRPC server for workload UDS starts, listening on "./etc/istio/proxy/SDS" 

2020-09-28T12:24:06.341373Z     info    Starting proxy agent
2020-09-28T12:24:06.341433Z     info    sds     Start SDS grpc server
2020-09-28T12:24:06.341531Z     info    Opening status port 15020

2020-09-28T12:24:06.341756Z     info    Received new config, creating new Envoy epoch 0
2020-09-28T12:24:06.341824Z     info    Epoch 0 starting
2020-09-28T12:24:06.349146Z     info    Envoy command: [-c etc/istio/proxy/envoy-rev0.json --restart-epoch 0 --drain-time-s 45 --parent-shutdown-time-s 60 --service-cluster details.default --service-node sidecar~172.16.0.26~details-v1-5974b67c8-xr7pf.default~default.svc.cluster.local --local-address-ip-version v4 --log-format-prefix-with-location 0 --log-format %Y-%m-%dT%T.%fZ    %l      envoy %n        %v -l warning --component-log-level misc:error --concurrency 2]
2020-09-28T12:24:06.387262Z     warning envoy runtime   Unable to use runtime singleton for feature envoy.reloadable_features.activate_fds_next_event_loop
2020-09-28T12:24:06.430387Z     warning envoy config    StreamAggregatedResources gRPC config stream closed: 14, no healthy upstream
2020-09-28T12:24:06.430433Z     warning envoy config    Unable to establish new stream
2020-09-28T12:24:06.441219Z     info    sds     resource:default new connection
2020-09-28T12:24:06.441438Z     info    sds     Skipping waiting for gateway secret
2020-09-28T12:24:06.680159Z     info    cache   Root cert has changed, start rotating root cert for SDS clients
2020-09-28T12:24:06.680189Z     info    cache   GenerateSecret default
2020-09-28T12:24:06.680674Z     info    sds     resource:default pushed key/cert pair to proxy
2020-09-28T12:24:06.683165Z     warning envoy main      there is no configured limit to the number of allowed active connections. Set a limit via the runtime key overload.global_downstream_max_connections
2020-09-28T12:24:06.863212Z     info    sds     resource:ROOTCA new connection
2020-09-28T12:24:06.863307Z     info    sds     Skipping waiting for gateway secret
2020-09-28T12:24:06.863336Z     info    cache   Loaded root cert from certificate ROOTCA
2020-09-28T12:24:06.863565Z     info    sds     resource:ROOTCA pushed root cert to proxy
2020-09-28T12:24:06.929540Z     warning envoy config    gRPC config for type.googleapis.com/envoy.config.listener.v3.Listener rejected: Error adding/updating listener(s) 172.16.254.175_15012: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.254.231_15443: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.255.10_31400: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.254.175_443: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.252.1_443: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.255.10_15443: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.255.10_443: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.253.206_53: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.254.231_443: Unknown gRPC client cluster 'envoy_accesslog_service'
0.0.0.0_31387: Unknown gRPC client cluster 'envoy_accesslog_service'
0.0.0.0_80: Unknown gRPC client cluster 'envoy_accesslog_service'
0.0.0.0_11800: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.254.254_443: Unknown gRPC client cluster 'envoy_accesslog_service'
0.0.0.0_9100: Unknown gRPC client cluster 'envoy_accesslog_service'
0.0.0.0_15014: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.254.175_853: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.252.223_12800: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.253.151_9200: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.254.89_5601: Unknown gRPC client cluster 'envoy_accesslog_service'
0.0.0.0_9300: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.255.34_9300: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.255.10_15021: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.253.254_80: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.255.148_24224: Unknown gRPC client cluster 'envoy_accesslog_service'
0.0.0.0_9080: Unknown gRPC client cluster 'envoy_accesslog_service'
0.0.0.0_15010: Unknown gRPC client cluster 'envoy_accesslog_service'
0.0.0.0_9200: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.252.242_80: Unknown gRPC client cluster 'envoy_accesslog_service'
0.0.0.0_8090: Unknown gRPC client cluster 'envoy_accesslog_service'
172.16.252.130_8080: Unknown gRPC client cluster 'envoy_accesslog_service'
virtualOutbound: Unknown gRPC client cluster 'envoy_accesslog_service'
virtualInbound: Unknown gRPC client cluster 'envoy_accesslog_service'

2020-09-28T12:24:06.931158Z     critical        envoy backtrace Caught Segmentation fault, suspect faulting address 0x0
2020-09-28T12:24:06.931180Z     critical        envoy backtrace Backtrace (use tools/stack_decode.py to get line numbers):
2020-09-28T12:24:06.931183Z     critical        envoy backtrace Envoy version: f642a7fd07d0a99944a6e3529566e7985829839c/1.15.0/Clean/RELEASE/BoringSSL
2020-09-28T12:24:06.931429Z     critical        envoy backtrace #0: __restore_rt [0x7f319d8558a0]
2020-09-28T12:24:06.936583Z     info    sds     resource:ROOTCA connection is terminated: rpc error: code = Canceled desc = context canceled
2020-09-28T12:24:06.936623Z     error   sds     Remote side closed connection
2020-09-28T12:24:06.936725Z     info    sds     resource:default connection is terminated: rpc error: code = Canceled desc = context canceled
2020-09-28T12:24:06.936798Z     error   sds     Remote side closed connection
2020-09-28T12:24:06.936840Z     error   Epoch 0 exited with error: signal: segmentation fault
2020-09-28T12:24:06.936858Z     info    No more active epochs, terminating
[root@VM_77_80_centos ~/kowalskixu/proxy-test/gw-proxy]# istioctl pc  
A group of commands used to retrieve information about proxy configuration from the Envoy config dump
```



## 参考资料

1. [《Envoy Proxy使用介绍教程（七）：envoy动态配置xDS的使用方法》](https://www.lijiaocn.com/%E9%A1%B9%E7%9B%AE/2018/12/29/envoy-07-features-2-dynamic-discovery.html#%E4%B8%80%E4%B8%AA%E7%AE%80%E5%8D%95%E7%9A%84management-server%E5%AE%9E%E7%8E%B0)