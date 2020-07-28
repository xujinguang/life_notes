## 什么是 Istio?

1. Istio 有助于减少这些部署的复杂性，并减轻开发团队的压力。部署的复杂性，是因为微服务的增加所带来的。开发团队的什么压力呢？主要就是业务开发有时候不得不关注基础的监控，日志，跟踪等遥测能力，而让业务开发变得复杂。服务网格可以把这些东西剥离出来并下沉成框架能力。
2. 它是一个平台，具有很强的扩展性，集成到不同的日志、遥测或策略系统中。具有多样特性，对外提供统一的方式来保护、连接和监视微服务。

## 什么是服务网格？

针对这个问题，我曾经写过一篇文章详细的介绍过。简单的讲就是在微服务的时代背景下，微服务的维护，管理，通信等需求变得异常复杂。这些需求至少包括：服务发现、负载平衡、故障恢复、度量和监视， A/B 测试、探测器展示、速率限制、访问控制和端到端身份验证。

## 为什么使用 Istio？

1. 一个完整的解决方案来满足微服务应用程序的各种需求。

2. 对业务服务采用非侵入式的集成，即不用修改代码或者很少的代码修改就可以支持。

3. 能够支持

   3.1 HTTP、 gRPC、 WebSocket 和 TCP 流量的自动负载平衡。

   3.2 使用丰富的路由规则、重试、故障转移和故障注入对流量行为进行细粒度控制。

   3.3 一个可插拔的策略层和配置 API，支持访问控制、速率限制和配额。

   3.4 集群内所有通信的自动度量、日志和跟踪，包括集群的进入和出口。

   3.5 集群中具有强基于身份的身份验证和授权的安全服务对服务通信。

4. Istio 具有很强的扩展性，可以满足不同的部署需求。

## 核心功能

这个也是istio的主要功能

### 流量管理

通过配置路由调整服务之间的流量，支持AB测试，金丝雀测试和流量百分比，支持断路器，超时和重试。

流量管理 的API 资源

- virtual service 虚拟服务
- Destination rules
- gateway
- service entries
- sidecars

#### virtual service

virtual service 和 Destination rules 是Istio 流量路由功能的关键构件。基于istio平台的连接和发现，通过虚拟服务配置如何将请求路由到 Istio 服务网格中的服务。

如果没有它，默认使用Envoy的轮循模型在每个服务的负载平衡池中分配流量，轮流向每个池成员发送请求。这种分发方式，缺少一定灵活性，比如无法实现AB测试的百分比流量分发。

VS，使得请求和工作负载强分离。并处于二者之间的中间地位，实现请求到后端的流量控制。

```yaml
apiVersion: networking.istio.io/v1alpha3
kind: VirtualService #虚拟服务的api的资源类型
metadata:
  name: reviews #虚拟服务的名称
spec:
  hosts: # 虚拟服务的主机
  - reviews
  http: # HTTP 流量路由规则的有序列表，支持http1.1 http2，grpc
  - match:  # 匹配条件
    - headers: #匹配指定header字段值的请求量，支持前缀匹配，精确匹配和正则表达式匹配
        end-user:
          exact: jason
    route: # 路由规则
    - destination: # 目的地的主机必须是 Istio 服务注册中心中的真实目的地
        host: reviews
        subset: v2
    - uri: # 基于uri的匹配请求
        prefix: /reviews #前缀匹配
    route: # 匹配的uri请求转发目的地
    - destination:
        host: reviews
  - route: # 默认转发
    - destination:
        host: reviews
        subset: v3
```

基于百分比的分发

```yaml
spec:
  hosts:
  - reviews
  http:
  - route:
    - destination:
        host: reviews
        subset: v1
      weight: 75 # reviews:v1承载75%的流量
    - destination:
        host: reviews
        subset: v2
      weight: 25 # reviews:v2承载25%的流量
```

路由规则按从上到下的顺序进行计算，逐个匹配，如果有一个匹配，则执行然后返回。如果没有匹配规则，则转发到默认规则上，否则丢弃。

VS支持http，tls，tcp的流量控制。同时通过exportto配置支持跨名称空间边界的虚拟服务的可见性。如果没有指定名称空间，则默认情况下将虚拟服务导出到所有名称空间。目前只支持当前和所有。可以配置指定应用这些路由的网关和边车的名称。

一个功能示例：

```yaml
---
apiVersion: networking.istio.io/v1beta1
kind: VirtualService
metadata:
  name: reviews-route
spec:
  hosts:
  - reviews.prod.svc.cluster.local
  http: #http/grpc流量
  - timeout: 5s # Envoy等待服务的答复的时间，默认15s
  - name: reviews-v2-routes
    match:
    - sourceLabels:
        env: prod
    - headers:
        end-user:
          exact: jason
    - uri:
        prefix: "/wpcatalog"
    - uri:
        exact: "/consumercatalog"
    - ignoreUriCase: false
    - gateways:
      - test.com
    rewrite:
      uri: "/newcatalog"
    redirect:
      uri: "/v1/bookRatings"
      authority: newratings.default.svc.cluster.local
    route:
    - destination:
        host: reviews
        subset: v2
    retries: # Envoy 连接服务的重试, 重试之间的间隔(25ms +),由istio自动调整，防止淹没服务
      attempts: 3 # 尝试最大次数
      perTryTimeout: 2s #每次重试的超时时间
      retryOn: gateway-error,connect-failure,refused-stream
    corsPolicy: #跨域策略
      allowOrigin:
      - example.com
      allowMethods:
      - POST
      - GET
      allowCredentials: false
      allowHeaders:
      - X-Foo-Bar
      exposeHeaders:
      - Token
      maxAge: 24h
    fault: # 故障注入
      abort: # 中止崩溃失败故障
        percentage:
          value: 0.1
        httpStatus: 400
      delay: #延时故障
        percentage: # 每1000个对评级服务的请求中的1个引入5秒延迟
          value: 0.1
        fixedDelay: 5s
  - name: reviews-v1-route
    route:
    - destination:
        host: reviews.prod.svc.cluster.local
        subset: v2
      weight: 25
    - destination:
        host: reviews.prod.svc.cluster.local
        subset: v1
      weight: 751
  tcp: # tcp流量
  - match:
    - port: 27017
    route:
    - destination:
        host: mongo.backup.svc.cluster.local
        port:
          number: 5555
  tls: #https流量
  - match:
    - port: 443
      sniHosts:
      - login.bookinfo.com
    route:
    - destination:
        host: login.prod.svc.cluster.local
  - match:
    - port: 443
      sniHosts:
      - reviews.bookinfo.com
    route:
    - destination:
        host: reviews.prod.svc.cluster.local
```

其中涉及超时，重试，故障注入

Crd 的资源定义文件：istio/manifests/charts/base/crds/crd-all.gen.yaml，官方有更详细的[http流量配置](https://istio.io/latest/docs/reference/config/networking/virtual-service/#HTTPRoute)说明，以及VSAPI资源。

####  Destination rules

虚拟服务看作是如何将流量路由到给定目的地，然后使用目的地规则来配置该目的地的流量发生的情况。真实目的的访问配置。

支持配置负载均衡，基于哈希的一致性负载平衡，连接池，断路器，连接设置，tls证书设置

负载均衡策略：随机，加权，最少请求。

#### Gateway

使用网关管理进出的流量，网关配置应用于在网格边缘运行的独立的Envoy代理，而不是随着服务部署sidecar Envoy代理。

指定路由和网关按预期的方式工作，还必须将网关绑定到虚拟服务。

#### Service entries

使用一个服务条目向 Istio 内部维护的服务注册中心添加一个条目。添加服务条目后，Envoy代理可以向服务发送流量，就好像它是您网格中的服务一样。通过配置服务条目，您可以管理运行在网格之外的服务的流量.

可以配置虚拟服务和目的地规则，以更细粒度的方式控制服务条目的流量，就像为网格中的任何其他服务配置流量一样

[网关文档](https://istio.io/latest/docs/reference/config/networking/gateway/)

#### Sidecars

用于 配置应用于特定命名空间中的所有工作负载，或者使用 workloadSelector 选择特定的工作负载

#### 如何实现的呢？

### 安全

Istio 提供底层的安全通信通道，使开发人员可以将重点放在应用程序级别的安全性上。

### 可观察性

Istio 的健壮跟踪、监视和日志记录特性使您能够深入了解服务网格部署。

Istio 生成以下类型的遥测数据，以提供全面的服务网状观测能力:

1. 延迟、流量、错误和饱和度
2. 为每个服务生成分布式的跟踪范围，为运营商提供一个详细的了解调用流和网格中的服务依赖关系
3. 为流入网格的流量，生成每个请求的完整记录，包括源和目标元数据

监控服务和istio自身。度量分类：

1. Proxy-level
2. Service-level
3. Control-level

另外就是服务负载维度的访问日志。

### 平台支持

istio 是独立于平台的，可以在各种环境中运行，包括云、内部设备、 Kubernetes、 Mesos 等等

### 集成和定制

这是可扩展性，指的可以集成现有的 acl、日志记录、监控、配额、审计等解决方案。

扩展性目前使用Proxy-Wasm sandbox API 取代了 Mixer 作为主要的扩展机制。

![istio扩展架构](./wasm-extending.svg)wasm的优点：

1. 效率——扩展增加了低延迟、 CPU 和内存开销
2. 功能——扩展可以执行策略，收集遥测数据，并执行有效负载突变
3. 隔离-一个插件的编程错误或崩溃不会影响其他插件
4. 配置-这些插件使用与其他 Istio API 一致的 API 进行配置。扩展可以动态配置
5. 操作员——扩展可以分为只对日志、故障打开或故障关闭三种方式部署
6. 扩展开发人员-该插件可以用几种编程语言编写

## 系统架构

 Istio 服务网格在逻辑上划分为一个数据平面和一个控制平面。

- 数据平面由一组作为边车部署的智能代理Envoy组成。这些代理协调和控制微服务之间的所有网络通信。并收集并报告所有网状通信的遥测数据。
- 控制平面管理和配置代理来路由流量

![](./arch1.6.svg)

### Envoy

1. 使用的是Envoy的扩展版本；Envoy 高性能的C++代理，支持L4/L7层代理转发
2. 以边车部署的方式，负责服务的所有入站和出站流量
3. Envoy的内置功能扩充服务的能力包括：
   - 动态服务发现
   - 负载平衡
   - TLS 终止
   - HTTP/2和 gRPC 代理
   - 断路器
   - 健康检查
   - 基于百分比的流量分发
   - 故障注入
   - 丰富的指标
4. Envoy给istio带来的特性
   - 流量控制特性: 针对 HTTP、 gRPC、 WebSocket 和 TCP 流量实施具有丰富路由规则的细粒度流量控制。
   - 网络弹性特性: 设置重试、故障恢复、断路器和故障注入。
   - 安全性和身份验证特性: 实施安全策略并实施通过配置 API 定义的访问控制和速率限制。
   - 基于 WebAssembly 的可插拔扩展模型，允许自定义策略实施和网格流量的遥测生成。

#### sidecar的优势

以边车部署，并获取所有流量就可以提取丰富的流量行为属性，并将其发送给监控系统，形成整个istio网格的网络信息。

这种方式可以避免侵入性，可以让服务不修改任何代码，或者做很小的变更。

### Istiod

提供服务发现，配置和管理。（pilot+citadel+galley）

1. 将控制流量行为的高级路由规则转换为Envoy的配置
2. 置身份和凭证管理支持强大的服务到服务和最终用户身份验证
3. 维护一个 CA 并生成证书，以允许数据平面中的安全 mTLS 通信

## 系统部署

部署模式包括：

1. 单集群

2. 多集群

   - 故障隔离
   - 服务高可用：故障转移，就近路由
   - 团队项目隔离，每个团队维护一个集群，只要统一API接口即可

网络模式：

1. 单一网络
2. 多网络

控制平面模型

身份和信任模型

网格模型

租聘模型

istio占用的[端口列表](https://istio.io/latest/docs/ops/deployment/requirements/)



## 参考资料

1. [官方文档What is Istio?](https://istio.io/latest/docs/concepts/what-is-istio/)
2. [流量管理配置](https://istio.io/latest/docs/reference/config/networking/)

## 附件：

```json
{
	"apiVersion": "networking.istio.io/v1beta1",
	"kind": "VirtualService",
	"metadata": {
		"name": "reviews-route"
	},
	"spec": {
		"hosts": [
			"reviews.prod.svc.cluster.local" 
		],
		"http": [
			{
				"timeout": "5s"
			},
			{
				"name": "reviews-v2-routes",
				"match": [
					{
						"sourceLabels": {
							"env": "prod"
						}
					},
					{
						"headers": {
							"end-user": {
								"exact": "jason"
							}
						}
					},
					{
						"uri": {
							"prefix": "/wpcatalog"
						}
					},
					{
						"uri": {
							"exact": "/consumercatalog"
						}
					},
					{
						"ignoreUriCase": false
					},
					{
						"gateways": [
							"test.com"
						]
					}
				],
				"rewrite": {
					"uri": "/newcatalog"
				},
				"redirect": {
					"uri": "/v1/bookRatings",
					"authority": "newratings.default.svc.cluster.local"
				},
				"route": [
					{
						"destination": {
							"host": "reviews",
							"subset": "v2"
						}
					}
				],
				"retries": {
					"attempts": 3,
					"perTryTimeout": "2s",
					"retryOn": "gateway-error,connect-failure,refused-stream"
				},
				"corsPolicy": {
					"allowOrigin": [
						"example.com"
					],
					"allowMethods": [
						"POST",
						"GET"
					],
					"allowCredentials": false,
					"allowHeaders": [
						"X-Foo-Bar"
					],
					"exposeHeaders": [
						"Token"
					],
					"maxAge": "24h"
				},
				"fault": {
					"abort": {
						"percentage": {
							"value": 0.1
						},
						"httpStatus": 400
					},
					"delay": {
						"percentage": {
							"value": 0.1
						},
						"fixedDelay": "5s"
					}
				}
			},
			{
				"name": "reviews-v1-route",
				"route": [
					{
						"destination": {
							"host": "reviews.prod.svc.cluster.local",
							"subset": "v2"
						},
						"weight": 25
					},
					{
						"destination": {
							"host": "reviews.prod.svc.cluster.local",
							"subset": "v1"
						},
						"weight": 751
					}
				]
			}
		],
		"tcp": [
			{
				"match": [
					{
						"port": 27017
					}
				],
				"route": [
					{
						"destination": {
							"host": "mongo.backup.svc.cluster.local",
							"port": {
								"number": 5555
							}
						}
					}
				]
			}
		],
		"tls": [
			{
				"match": [
					{
						"port": 443,
						"sniHosts": [
							"login.bookinfo.com"
						]
					}
				],
				"route": [
					{
						"destination": {
							"host": "login.prod.svc.cluster.local"
						}
					}
				]
			},
			{
				"match": [
					{
						"port": 443,
						"sniHosts": [
							"reviews.bookinfo.com"
						]
					}
				],
				"route": [
					{
						"destination": {
							"host": "reviews.prod.svc.cluster.local"
						}
					}
				]
			}
		]
	}
}
```

