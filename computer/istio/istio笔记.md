## istio笔记

1. 对于一个小团队来说，微服务架构可能很难证明是合理的，因为仅仅需要处理微服务本身的部署和管理——有人将其描述为“微服务税”——带来收获小于投入。只有使得很多人收益时，才具有合理性。
2. 微服务更加适用与SaaS软件，提供独立可发布性和可扩展性。
3. 微服务与技术无关特性可以充分利用云平台。



1. 南北流量是服务器到客户端的流量，而东西是服务器到服务器的流量——因为通常为服务器-客户端流量绘制垂直线，为服务器到服务器流量绘制水平线。
2. 服务网格是一个专用的基础设施层，用于处理服务到服务的通信，以使其可见、可管理和可控。这和端到端的网络模型有结构相似性。
3. 不可靠网络传输的远程过程调用转换成分布式系统编程，需要代码实现大量定制工具（比如RPC，弹性网络库），这会融入业务程序中而变得困难。服务网格减轻了开发人员的负担，将这些工具从应用程序剥离，并下推到基础设施层。
4. 分布式的谬误：网络可靠，延迟为0，带宽无限，网络是安全的，拓扑不变，有一位管理员，运输成本为0，网络是统治的
5. 考虑的出发点：是否存在大量的服务间通信？

[《你需要服务网格吗？》](https://www.oreilly.com/content/do-you-need-a-service-mesh/)



1. 服务网格提供了关键的可观察性、可靠性和安全特性,应用程序不需要实现这些特性，甚至不需要意识到服务网格就在那里！
2. Web 应用程序的典型“三层”架构: 应用程序，web服务，存储逻辑。层之间的通信最多只有两跳。谷歌、Netflix 和 Twitter 等公司面临着巨大的流量需求，他们实施了有效的云原生方法的前身：应用层被拆分为微服务，层成为*拓扑*。这些系统通过采用一般化的通信层来解决这种复杂性，通常采用库的形式[——Twitter](https://twitter.github.io/finagle/)的[Finagle](https://twitter.github.io/finagle/)、Netflix 的[Hystrix](https://github.com/Netflix/Hystrix)和谷歌的 Stubby 就是典型的例子。

[《什么是服务网格？为什么我需要一个？》](https://buoyant.io/what-is-a-service-mesh/)



1. 物理机-》虚拟机-》IaaS-》PaaS-》容器-》FaaS
2. FaaS的特点

##### 无服务器的优势

- 零管理
- 按执行付费
- 空闲时间零成本
- 自动缩放
- 更快的上市时间
- 微服务性质 —> 清晰的代码库分离
- 显着减轻管理和维护负担

##### 无服务器的缺点

- 没有标准化（尽管 CNCF 正在努力）
- “黑匣子”环境
- 供应商锁定
- 冷启动
- 复杂的应用程序可能很难构建

[《serverless-faas-vs-containers》](https://www.serverless.com/blog/serverless-faas-vs-containers)

服务网格的对比：https://servicemesh.es/



2021-07-07 21:52

1. Istio 的核心确实做了四件事：连接性、可观察性、安全性和流量控制。
2. 这里的建议是，对于大多数公司来说，Istio 可能过于复杂，无法一次性推出所有功能。因此，请先关注一两个方面，然后再添加其他方面。
3. Pilot 连接到每个 istio-proxy，每个 istio-proxy 都会检查并报告每个请求的 Mixer
4. 随着服务增加，日志，追踪，调用链提供更多容量

[我们在生产环境中运行 Istio 所学到的一切](https://engineering.hellofresh.com/everything-we-learned-running-istio-in-production-part-1-51efec69df65)

1. Kubernetes 以负载平衡、入口管理、自动缩放等形式提供编排和健壮性，仍然存在问题。您如何管理集群中微服务之间的网络连接和连接策略？这就是istio的工作
2. Istio 支持网络策略、可靠性、安全性、遥测和测试选项的声明式配置和管理。它可以被认为是一个抽象层，运行在编排基础架构 Kubernetes 之上并对其进行管理。
3. 我们集群中微服务的所有进出和之间的流量都被 sidecar 代理拦截。它们拥有有关我们网络的所有数据，并支持广泛的管理和监控功能。

[Machine Learning in Production: Using Istio to Mesh Microservices in Google Kubernetes Engine](https://medium.com/retina-ai-health-inc/machine-learning-in-production-using-istio-to-mesh-microservices-in-google-kubernetes-engine-9b15fb643bab)

美军在尝试云服务，并在F16上使用k8s和istio

[Google云攻城狮介绍Istio的ppt](https://docs.google.com/presentation/d/1T64Rj5A3E0QQI_Ei3k45V5zOS_Xz8nAOKkbH2JTv--k/edit#slide=id.g7d347b868d_0_106)

