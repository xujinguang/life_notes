## Istio

### 1.Mixer

所有流量通过Mixer，通过Mixer 统一采集和上报所有的遥测数据和服务间访问鉴权，导致一旦规模上来，Mixer 非常容易成为性能瓶颈。

### 2. Mixerless

将Mixer 从主流程抽离。 将遥测和服务鉴权能力下沉到每个服务的代理Proxy Envoy中.Mixer 是一个热拔插组件，有极好的扩展性。 支持代码中注册编写Adapter并重新打包的方式更新遥测对接的配置，也支持配置 `Out Of Process Adapter` 这样的外部适配器，进行遥测的对接和动态配置。但提供灵活性同时，也成为了整个ISTIO 明显的性能瓶颈。

ISTIO 支持[EnvoyFilter](https://istio.io/docs/reference/config/networking/envoy-filter/) 这个CRD，它提供一种机制，可以帮助我们定制 Pilot 所生成并下发给Envoy 配置，在EnvoyFilter中我们能够定义生效的pod范围，注入的时机以及执行内容。 同样EnvoyFilter支持WebAssembly, 我们可以通过EnvoyFilter 配置WebAssembly的执行时机，实现Envoy 的流量管理策略的更新

![filter](https://ata2-img.cn-hangzhou.oss-pub.aliyun-inc.com/3ed7400fbaa4df7e897788283192aedb.png)



### 通过iptables规则来捕获流量

```go
"ip6tables -t nat -N ISTIO_INBOUND",
"ip6tables -t nat -N ISTIO_REDIRECT",
"ip6tables -t nat -N ISTIO_IN_REDIRECT",
"ip6tables -t nat -N ISTIO_OUTPUT",
"ip6tables -t nat -A ISTIO_INBOUND -p tcp --dport 15008 -j RETURN",
"ip6tables -t nat -A ISTIO_REDIRECT -p tcp -j REDIRECT --to-ports 15001",
"ip6tables -t nat -A ISTIO_IN_REDIRECT -p tcp -j REDIRECT --to-ports 15006",
"ip6tables -t nat -A OUTPUT -p tcp -j ISTIO_OUTPUT",
"ip6tables -t nat -A ISTIO_OUTPUT -o lo -s ::6/128 -j RETURN",
"ip6tables -t nat -A ISTIO_OUTPUT -o lo ! -d ::1/128 -m owner --uid-owner 1337 -j ISTIO_IN_REDIRECT",
"ip6tables -t nat -A ISTIO_OUTPUT -o lo -m owner ! --uid-owner 1337 -j RETURN",
"ip6tables -t nat -A ISTIO_OUTPUT -m owner --uid-owner 1337 -j RETURN",
"ip6tables -t nat -A ISTIO_OUTPUT -o lo ! -d ::1/128 -m owner --gid-owner 1337 -j ISTIO_IN_REDIRECT",
"ip6tables -t nat -A ISTIO_OUTPUT -o lo -m owner ! --gid-owner 1337 -j RETURN",
"ip6tables -t nat -A ISTIO_OUTPUT -m owner --gid-owner 1337 -j RETURN",
"ip6tables -t nat -A ISTIO_OUTPUT -d ::1/128 -j RETURN",
```





