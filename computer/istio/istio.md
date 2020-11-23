## Istio

### 1. Mixer

所有流量通过Mixer，通过Mixer 统一采集和上报所有的遥测数据和服务间访问鉴权，导致一旦规模上来，Mixer 非常容易成为性能瓶颈。

### 2. Mixerless

将Mixer 从主流程抽离。 将遥测和服务鉴权能力下沉到每个服务的代理Proxy Envoy中.Mixer 是一个热拔插组件，有极好的扩展性。 支持代码中注册编写Adapter并重新打包的方式更新遥测对接的配置，也支持配置 `Out Of Process Adapter` 这样的外部适配器，进行遥测的对接和动态配置。但提供灵活性同时，也成为了整个ISTIO 明显的性能瓶颈。

ISTIO 支持[EnvoyFilter](https://istio.io/docs/reference/config/networking/envoy-filter/) 这个CRD，它提供一种机制，可以帮助我们定制 Pilot 所生成并下发给Envoy 配置，在EnvoyFilter中我们能够定义生效的pod范围，注入的时机以及执行内容。 同样EnvoyFilter支持WebAssembly, 我们可以通过EnvoyFilter 配置WebAssembly的执行时机，实现Envoy 的流量管理策略的更新

<img src="https://ata2-img.cn-hangzhou.oss-pub.aliyun-inc.com/3ed7400fbaa4df7e897788283192aedb.png" alt="filter" style="zoom:50%;" />



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

### 3. inject

观测inject做了些什么，下面istio源码，然后执行命令

```shell
istioctl kube-inject -f samples/health-check/liveness-command.yaml
```

得到的文件和源文件的区别，它修改了deployment的配置，包括

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  creationTimestamp: null
  name: liveness
spec:
  selector:
    matchLabels:
      app: liveness
  strategy: {}
  template:
    metadata:
      annotations:
        prometheus.io/path: /stats/prometheus
        prometheus.io/port: "15020"
        prometheus.io/scrape: "true"
        sidecar.istio.io/interceptionMode: REDIRECT
        sidecar.istio.io/status: '{"version":"8e6e902b765af607513b28d284940ee1421e9a0d07698741693b2663c7161c11","initContainers":["istio-init"],"containers":["istio-proxy"],"volumes":["istio-envoy","istio-data","istio-podinfo","istiod-ca-cert"],"imagePullSecrets":null}'
        traffic.sidecar.istio.io/excludeInboundPorts: "15020"
        traffic.sidecar.istio.io/includeOutboundIPRanges: '*'
      creationTimestamp: null
      labels:
        app: liveness
        istio.io/rev: ""
        security.istio.io/tlsMode: istio
    spec:
      containers:
      #业务设定
      - args:
        - /bin/sh
        - -c
        - touch /tmp/healthy; sleep 3600
        image: k8s.gcr.io/busybox
        livenessProbe:
          exec:
            command:
            - cat
            - /tmp/healthy
          initialDelaySeconds: 5
          periodSeconds: 5
        name: liveness
        resources: {}
      #inject内容
      - args:
        - proxy
        - sidecar
        - --domain
        - $(POD_NAMESPACE).svc.cluster.local
        - --serviceCluster
        - liveness.$(POD_NAMESPACE)
        - --proxyLogLevel=warning
        - --proxyComponentLogLevel=misc:error
        - --trust-domain=cluster.local
        - --concurrency
        - "2"
        env:
        - name: JWT_POLICY
          value: first-party-jwt
        - name: PILOT_CERT_PROVIDER
          value: istiod
        - name: CA_ADDR
          value: istiod.istio-system.svc:15012
        - name: POD_NAME
          valueFrom:
            fieldRef:
              fieldPath: metadata.name
        - name: POD_NAMESPACE
          valueFrom:
            fieldRef:
              fieldPath: metadata.namespace
        - name: INSTANCE_IP
          valueFrom:
            fieldRef:
              fieldPath: status.podIP
        - name: SERVICE_ACCOUNT
          valueFrom:
            fieldRef:
              fieldPath: spec.serviceAccountName
        - name: HOST_IP
          valueFrom:
            fieldRef:
              fieldPath: status.hostIP
        - name: CANONICAL_SERVICE
          valueFrom:
            fieldRef:
              fieldPath: metadata.labels['service.istio.io/canonical-name']
        - name: CANONICAL_REVISION
          valueFrom:
            fieldRef:
              fieldPath: metadata.labels['service.istio.io/canonical-revision']
        - name: PROXY_CONFIG
          value: |
            {"proxyMetadata":{"DNS_AGENT":""}}
        - name: ISTIO_META_POD_PORTS
          value: |-
            [
            ]
        - name: ISTIO_META_APP_CONTAINERS
          value: liveness
        - name: ISTIO_META_CLUSTER_ID
          value: Kubernetes
        - name: ISTIO_META_INTERCEPTION_MODE
          value: REDIRECT
        - name: ISTIO_META_WORKLOAD_NAME
          value: liveness
        - name: ISTIO_META_OWNER
          value: kubernetes://apis/apps/v1/namespaces/default/deployments/liveness
        - name: ISTIO_META_MESH_ID
          value: cluster.local
        - name: DNS_AGENT
        - name: ISTIO_KUBE_APP_PROBERS
          value: '{}'
        image: docker.io/istio/proxyv2:1.7.0
        imagePullPolicy: Always
        name: istio-proxy
        ports:
        - containerPort: 15090
          name: http-envoy-prom
          protocol: TCP
        readinessProbe:
          failureThreshold: 30
          httpGet:
            path: /healthz/ready
            port: 15021
          initialDelaySeconds: 1
          periodSeconds: 2
        resources:
          limits:
            cpu: "2"
            memory: 1Gi
          requests:
            cpu: 10m
            memory: 40Mi
        securityContext:
          allowPrivilegeEscalation: false
          capabilities:
            drop:
            - ALL
          privileged: false
          readOnlyRootFilesystem: true
          runAsGroup: 1337
          runAsNonRoot: true
          runAsUser: 1337
        volumeMounts:
        - mountPath: /var/run/secrets/istio
          name: istiod-ca-cert
        - mountPath: /var/lib/istio/data
          name: istio-data
        - mountPath: /etc/istio/proxy
          name: istio-envoy
        - mountPath: /etc/istio/pod
          name: istio-podinfo
      initContainers:
      - args:
        - istio-iptables
        - -p
        - "15001"
        - -z
        - "15006"
        - -u
        - "1337"
        - -m
        - REDIRECT
        - -i
        - '*'
        - -x
        - ""
        - -b
        - '*'
        - -d
        - 15090,15021,15020
        env:
        - name: DNS_AGENT
        image: docker.io/istio/proxyv2:1.7.0
        imagePullPolicy: Always
        name: istio-init
        resources:
          limits:
            cpu: "2"
            memory: 1Gi
          requests:
            cpu: 10m
            memory: 10Mi
        securityContext:
          allowPrivilegeEscalation: false
          capabilities:
            add:
            - NET_ADMIN
            - NET_RAW
            drop:
            - ALL
          privileged: false
          readOnlyRootFilesystem: false
          runAsGroup: 0
          runAsNonRoot: false
          runAsUser: 0
      securityContext:
        fsGroup: 1337
      volumes:
      - emptyDir:
          medium: Memory
        name: istio-envoy
      - emptyDir: {}
        name: istio-data
      - downwardAPI:
          items:
          - fieldRef:
              fieldPath: metadata.labels
            path: labels
          - fieldRef:
              fieldPath: metadata.annotations
            path: annotations
        name: istio-podinfo
      - configMap:
          name: istio-ca-root-cert
        name: istiod-ca-cert
```

注入的方式就是修改deployment的配置，生成包含proxy和istio-iptables操作的新配置文件。从而实现pod中包含Envoy和业务容器。

另外一种方式就是自动注入， 将 sidecar 代理添加到用户创建的 pod 。它使用 `MutatingWebhook` 机制在 pod 创建的时候将 sidecar 的容器和卷添加到每个 pod 的模版里。

用户可以通过 webhooks `namespaceSelector` 机制来限定需要启动自动注入的范围，也可以通过注解的方式针对每个 pod 来单独启用和禁用自动注入功能。

Sidecar 是否会被自动注入取决于下面 3 条配置和 2 条安全规则：

配置：

- webhooks `namespaceSelector`
- 默认策略
- pod 级别的覆盖注解

安全规则:

- sidecar 默认不能被注入到 `kube-system` 和 `kube-public` 这两个 namespace
- sidecar 不能被注入到使用 `host network` 网络的 pod 里

关于webhook是k8s访问API服务时准入插件的组成部分，它包括两个控制器： MutatingAdmissionWebhook 和 ValidatingAdmissionWebhook 。前者 “变更” 操作 ，后者执行  “验证”  操作。 准入控制过程分为两个阶段。第一阶段，运行变更准入控制器。第二阶段，运行验证准入控制器。   如果任何一个阶段的任何控制器拒绝了该请求，则整个请求将立即被拒绝，并向终端用户返回一个错误。 【[准入控制器](https://kubernetes.io/zh/docs/reference/access-authn-authz/admission-controllers/)】

```yaml
# kubectl get mutatingwebhookconfigurations istio-sidecar-injector -o yaml
apiVersion: admissionregistration.k8s.io/v1
kind: MutatingWebhookConfiguration
metadata:
  labels:
    app: sidecar-injector
    install.operator.istio.io/owning-resource: installed-state
    install.operator.istio.io/owning-resource-namespace: istio-system
    istio.io/rev: default
    operator.istio.io/component: Pilot
    operator.istio.io/managed: Reconcile
    operator.istio.io/version: 1.7.0
    release: istio
  name: istio-sidecar-injector
webhooks: #配置一个或多个webhook
- admissionReviewVersions: #指定AdmissionReview 版本
  - v1beta1
  - v1
  clientConfig: #请求应发送到的 webhook
    caBundle: xxx #如果指定，则使用tls链接
    service:
      name: istiod
      namespace: istio-system
      path: /inject
      port: 443
  failurePolicy: Fail #如何处理 admission webhook 中无法识别的错误和超时错误
  matchPolicy: Exact #匹配策略-解决不同api组时Exact还是Equivalent匹配。后者按照功能匹配，不按api组
  name: sidecar-injector.istio.io
  namespaceSelector: #ns选择器
    matchLabels:
      istio-injection: enabled
  objectSelector: {}
  reinvocationPolicy: Never #再调用策略
  rules: #匹配请求规则,其中任何一个规则匹配，则该请求将发送到 Webhook。
  - apiGroups: #一个或多个要匹配的 API 组
    - "" #核心 API 组。"*" 匹配所有 API 组
    apiVersions: #一个或多个要匹配的 API 版本
    - v1
    operations: #一个或多个要匹配的操作
    - CREATE #UPDATE、DELETE、CONNECT 或 *
    resources: #一个或多个要匹配的资源
    - pods #子资源通过路径指定，比如pods/*
    scope: '*' #要匹配的范围, "Cluster"、"Namespaced" 和 "*"
  sideEffects: None #带外更改
  timeoutSeconds: 30 #API 服务器等待 Webhook 响应的时间长度：[1,30]s
```

更详细解析参考：【[动态准入控制](https://kubernetes.io/zh/docs/reference/access-authn-authz/extensible-admission-controllers/)】

### pilot

15014是debug端口

proxy默认是localhost:15000地址，需要	登录pod才能访问。

```shell
kubectl exec productpage-v1-797898bc54-hnd8v  -c istio-proxy curl http://localhost:15000/help
```

#### 抽象模型

对不同的服务注册中心（Kubernetes、consul） 的支持，需要对不同的输入来源的数据有一个统一的存储格式。 它包括信息： HostName（[service](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#service) 名称）、Ports（[service](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#service) 端口）、Address（[service](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#service)ClusterIP）、Resolution （负载均衡策略） 等

#### 平台适配器 

既然是抽象模型，那么它就是统一的。无法八门的注册中心需要适配它，显然不可能。所以中间需要转化操作，这就是平台适配器，实现服务注册中心数据到抽象模型之间的数据转换。比如k8s，consul的适配器将注册中心的服务信息获取到然后转换成抽象模型。

#### xDS API

目前Istio采用的数据平面是Envoy，抽象模型获取到的数据需要提供给数据平面。Pilot](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#pilot) 使用了一套起源于 [Envoy](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#envoy) 项目的标准数据面 API 来将服务信息和流量规则下发到数据面的 [sidecar](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#sidecar)中。这套标准数据面 API，也叫 xDS。

#### user API

[Pilot](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#pilot) 还定义了一套用户 API， 用户 API 提供了面向业务的高层抽象，可以被运维人员理解和使用。

运维人员使用该 API 定义流量规则并下发到 [Pilot](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#pilot) ，这些规则被 [Pilot](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#pilot) 翻译成数据面的配置，再通过标准数据面 API 分发到 [sidecar](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#sidecar) 实例，可以在运行期对微服务的流量进行控制和调整。

通过运用不同的流量规则，可以对网格中微服务进行精细化的流量控制，如按版本分流、断路器、故障注入、灰度发布等。	

![pilot架构](https://www.servicemesher.com/istio-handbook/images/pilot.png)

- Discovery [service](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#service)：即 [pilot](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#pilot)-discovery，主要功能是从 [Service](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#service) provider（如 kubernetes 或者 consul ）中获取服务信息，从 Kubernetes API Server 中获取流量规则（Kubernetes [CRD](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#crd) Resource），并将服务信息和流量规则转化为数据面可以理解的格式，通过标准的数据面 API 下发到网格中的各个 [sidecar](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#sidecar)中。
- agent：即 [pilot](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#pilot)-agent 组件，该进程根据 Kubernetes API Server 中的配置信息生成 [Envoy](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#envoy) 的配置文件，负责启动、监控 [sidecar](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#sidecar) 进程。
- proxy：既 [sidecar](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#sidecar) proxy，是所有服务的流量代理，直接连接 [pilot](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#pilot)-discovery ，间接地从 Kubernetes 等服务注册中心获取集群中微服务的注册情况。
- [service](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#service) A/B：使用了 [Istio](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#istio) 的应用，如 [Service](https://www.servicemesher.com/istio-handbook/GLOSSARY.html#service) A/B，的进出网络流量会被 proxy 接管。

https://www.servicemesher.com/istio-handbook/concepts/pilot.html?q=



### pilot-discovery代码剖析

https://github.com/istio/istio/tree/master/pilot/cmd/pilot-discovery/main.go

```go
// Create the stop channel for all of the servers.
			stop := make(chan struct{})

			// Create the server for the discovery service.
			discoveryServer, err := bootstrap.NewServer(serverArgs) //创建discoveryServer实例
			if err != nil {
				return fmt.Errorf("failed to create discovery service: %v", err)
			}

			// Start the server
			if err := discoveryServer.Start(stop); err != nil {
				return fmt.Errorf("failed to start discovery service: %v", err)
			}
		
			cmd.WaitSignal(stop)
			// Wait until we shut down. In theory this could block forever; in practice we will get
			// forcibly shut down after 30s in Kubernetes.
			discoveryServer.WaitUntilCompletion()
```



https://github.com/istio/istio/tree/master/pilot/pkg/bootstrap/server.go

```go
func NewServer(args *PilotArgs) (*Server, error) {
}
```



### proxyv2探究

对这个系统的探究，采用逆向分析。从表及里，从使用到原理的方式，而不是上来就搞源码解读，那样对于新手不友好，也不容易看懂。

#### 1. 用户层面

用户最直接感知的层面就是使用kubectl命令操作k8s。是入门的前提。

按照官网的指引安装istio，然后安装示例之后会有如下pod, 其name在其命名空间下唯一。

```sh
# kubectl get po -n default
NAME                              READY   STATUS    RESTARTS   AGE
details-v1-5974b67c8-lbdm9        2/2     Running   0          39d
productpage-v1-797898bc54-lp5lq   2/2     Running   0          39d
ratings-v1-c6cdf8d98-t7czd        2/2     Running   0          21d
reviews-v1-8bdc65f7b-bght7        2/2     Running   0          21d
reviews-v1-8bdc65f7b-dzbk9        2/2     Running   0          39d
reviews-v1-8bdc65f7b-vxbpj        2/2     Running   0          39d
reviews-v2-868d77d678-2bhg2       2/2     Running   0          38d
reviews-v3-6c9b646cb4-w754w       2/2     Running   0          38d
```

我们查看productpage pod中的镜像

```shell
# kubectl get po productpage-v1-797898bc54-lp5lq -o jsonpath="{..image}"
docker.io/istio/proxyv2:1.7.0 docker.io/istio/examples-bookinfo-productpage-v1:1.16.2 docker.io/istio/proxyv2:1.7.0 istio/proxyv2:1.7.0 istio/proxyv2:1.7.0 istio/examples-bookinfo-productpage-v1:1.16.2
```

由于json指定递归所有的image，所以返回的结果中出现重复内容，我们采用绝对路径

```shell
# kubectl get po productpage-v1-797898bc54-lp5lq -o jsonpath="{.spec.containers[*].image}"
docker.io/istio/examples-bookinfo-productpage-v1:1.16.2 docker.io/istio/proxyv2:1.7.0
```

可能会好奇如何得知jsonpath的具体路径呢，这个是K8S部署资源pod配置文件的固定格式。可以获取到两个容器镜像地址，其个数和READY描述计数一致。examples-bookinfo-productpage-v1是业务服务容器，并指定版本v1.16.2。proxyv2就是istio中的sidecar概念的真身，也即是服务网格中的数据面板。

如果使用logs命令时不指定`-c`参数，会报错提示用户需要指定要查看的容器名称

```shell
# kubectl logs -f  productpage-v1-797898bc54-lp5lq
Error from server (BadRequest): a container name must be specified for pod productpage-v1-797898bc54-lp5lq, choose one of: [productpage istio-proxy] or one of the init containers: [istio-init]
```

它包括三个容器名称：productpage， istio-proxy和istio-init。那么它和上面镜像关系可以通过yaml(json)文件获得,这个文件比较大，不完全展开，格式化后可以看到三个容器，两个镜像。

<img src="/Users/kowalskixu/Desktop/work/doc/life_notes/computer/istio/imag.png" alt="image-20201107133719920" style="zoom:50%;" />

productpage是业务自己的服务，通过监听指定的端口，对外提供http或者grpc服务。这个是开发人员关心的。这里要关心的重点是后面两个。

从Pod的配置文件中可以看到istio-proxy和istio-init使用的是同一个镜像 proxyv2。二者区别在于它们的启动参数args和env是不同的。我们看看proxyv2的镜像是如何生成的。

#### 2. proxyv2 镜像的创建

源码[istio/pilot/docker/Dockerfile.proxyv2](https://github.com/istio/istio/blob/master/pilot/docker/Dockerfile.proxyv2)是创建proxyv2镜像的dockerfile。官方使用此文件来构建的proxyv2镜像，并打上tag标签推送到docker.io/istio/proxyv2镜像仓库。这里使用的1.7官方镜像版本。

文件中关注的两个重要操作，一个是安装envoy和pilot-agent，第二个就是pilot-agent作为程序的启动入口。

```shell
...
ARG SIDECAR=envoy

# Install Envoy.
COPY $SIDECAR /usr/local/bin/$SIDECAR
...
# 安装 pilot-agent
COPY pilot-agent /usr/local/bin/pilot-agent

# The pilot-agent will bootstrap Envoy.
ENTRYPOINT ["/usr/local/bin/pilot-agent"]
```

众所周知，Docker镜像启动容器时运行的应用程序由Dockerfile中的ENTRYPOINT指令进行定义，传递给程序的参数则通过CMD指令指定，ENTRYPOINT指令不存在时，CMD可用于同时指定程序及其参数。而proxyv2的Dockerfile只有ENTRYPOINT，没有CMD。这种情况要么是应用程序不需要参数，要么通过pod的args参数动态传递给应用程序。显然，这里属于后者。(扩展资料[Docker 的 ENTRYPOINT 和 CMD 参数](https://aws.amazon.com/cn/blogs/china/demystifying-entrypoint-cmd-docker/))

从入口点可以看到容器启动之后，运行的是pilot-agent进程，并没有直接运行Envoy。我们知道作为sidecar的envoy必须要运行来拦截服务的出入流量的，因此可以猜想到pilot-agent进程要负责拉起envoy代理。后面我们会证实这一点。

其次，istio-proxy和istio-init使用的同一个镜像 proxyv2，而镜像运行入口只有一个，比如通过参数进行区分。这个参数在pod的配置中指定的。

#### 3. proxyv2 的运行

##### 3.1 注入

镜像创建完成后使用k8s部署到集群，常规方式是需要编写deployment，使用`kubectl apply`下发配置文件即可。作为服务网格的基础的组件，以及尽量对应用程序透明和无侵入的原则。proxy的配置不会让用户在创建服务时，手动配置。但是它和是必须和应用程序一块部署的。那怎么办呢？

这就是自动注入了。这里不做具体展开。因为它是另外要讨论的模块。这里简单说下过程。由pilot起一个webhookserver，提供注入API，k8s在执行下发时，以插件形式调用webhook，修改deployment，注入这些配置内容，然后回复给k8s执行部署。

注入的内容包括两个部分，一个初始化容器istio-init和代理istio-proxy。

istio-init的启动参数

```json
"args":[
  "istio-iptables",
  "-p",
  "15001",
  "-z",
  "15006",
  "-u",
  "1337",
  "-m",
  "REDIRECT",
  "-i",
  "*",
  "-x",
  "",
  "-b",
  "*",
  "-d",
  "15090,15021,15020"
],
"env":[
  {
    "name":"DNS_AGENT"
  }
],

```

Istio-proxy的启动参数

```json
{
  "apiVersion":"v1",
  "kind":"Pod",
  "metadata":{
    "namespace":"default",
    "spec":{
      "containers":[
        {
          "name":"istio-proxy",
          "args":[
            "proxy",
            "sidecar",
            "--domain",
            "$(POD_NAMESPACE).svc.cluster.local",
            "--serviceCluster",
            "productpage.$(POD_NAMESPACE)",
            "--proxyLogLevel=warning",
            "--proxyComponentLogLevel=misc:error",
            "--trust-domain=cluster.local",
            "--concurrency",
            "2"
          ],
          "env":[
            {
              "name":"POD_NAMESPACE",
              "valueFrom":{
                "fieldRef":{
                  "apiVersion":"v1",
                  "fieldPath":"metadata.namespace"
                }
              }
            } 
          ]
        }
      ]
    }
  }
```

args的使用规则：

> 如果在配置文件中设置了容器启动时要执行的命令及其参数，那么容器镜像中自带的命令与参数将会被覆盖而不再执行。如果配置文件中只是设置了参数，却没有设置其对应的命令，那么容器镜像中自带的命令会使用该新参数作为其执行时的参数。

docker和k8s参数的映射

| 功能           | Docker     | K8S    |
| -------------- | ---------- | ------ |
| 容器执行的命令 | Entrypoint | ommand |
| 传给命令的参数 | Cmd        | args   |

另外，args字段不是运行程序的全部参数。只是启动参数的一部分，剩余的参数会通过设置环境变量，在进程中获取的方式传入，即env配置说明。这个在源码分析有用到的时候再说。

(参考资料[为容器设置启动时要执行的命令和参数](https://kubernetes.io/zh/docs/tasks/inject-data-application/define-command-argument-container/))

综上，istio-proxy和istio-init的启动命令分别是

```shell
#istio-init
/usr/local/bin/pilot-agent istio-iptables -p 15001 -z 15006 -u 1337 -m REDIRECT -i * -x  -b * -d 15090,15021,15020

#istio-proxy
/usr/local/bin/pilot-agent proxy sidecar --domain $(POD_NAMESPACE).svc.cluster.local --serviceCluster productpage.$(POD_NAMESPACE) --proxyLogLevel=warning --proxyComponentLogLevel=misc:error --trust-domain=cluster.local --concurrency 2
```

POD_NAMESPACE从环境变量中获取,此环境变量从pod的配置文件的metadata.namespace取值.。

##### 3.2 运行istio-init

关于[init 容器概念参考官网文档](https://kubernetes.io/zh/docs/concepts/workloads/pods/init-containers/)，其核心思想就是Pod中会运行多个容器，某些场景中会需要一个或者多个容器早于业务服务容器执行，比如初始化环境等等，而且多个初始化容器存在先后运行关系，那么这些容器就可以使用init 容器方式依次配置执行即可。官网对init容器的行为说明更加具体

>具体行为
>
>在 Pod 启动过程中，每个 Init 容器在网络和数据卷初始化之后会按顺序启动。 每个 Init 容器成功退出后才会启动下一个 Init 容器。 如果它们因为容器运行时的原因无法启动，或以错误状态退出，它会根据 Pod 的 `restartPolicy` 策略进行重试。 然而，如果 Pod 的 `restartPolicy` 设置为 "Always"，Init 容器失败时会使用 `restartPolicy` 的 "OnFailure" 策略。
>
>在所有的 Init 容器没有成功之前，Pod 将不会变成 `Ready` 状态。 Init 容器的端口将不会在 Service 中进行聚集。正在初始化中的 Pod 处于 `Pending` 状态， 但会将状况 `Initializing` 设置为 true。
>
>如果 Pod [重启](https://kubernetes.io/zh/docs/concepts/workloads/pods/init-containers/#pod-restart-reasons)，所有 Init 容器必须重新执行。
>
>对 Init 容器规约的修改仅限于容器的 `image` 字段。 更改 Init 容器的 `image` 字段，等同于重启该 Pod。
>
>因为 Init 容器可能会被重启、重试或者重新执行，所以 Init 容器的代码应该是幂等的。 特别地，基于 `emptyDirs` 写文件的代码，应该对输出文件可能已经存在做好准备。
>
>Init 容器具有应用容器的所有字段。然而 Kubernetes 禁止使用 `readinessProbe`， 因为 Init 容器不能定义不同于完成态（Completion）的就绪态（Readiness）。 Kubernetes 会在校验时强制执行此检查。
>
>在 Pod 上使用 `activeDeadlineSeconds` 和在容器上使用 `livenessProbe` 可以避免 Init 容器一直重复失败。`activeDeadlineSeconds` 时间包含了 Init 容器启动的时间。
>
>在 Pod 中的每个应用容器和 Init 容器的名称必须唯一； 与任何其它容器共享同一个名称，会在校验时抛出错误。

由于它是pod创建时候执行的容器，已经运行完成，因此无法登录容器进一步探究，只能查看其运行结果日志

```shell
# kubectl logs -f  productpage-v1-797898bc54-lp5lq  -c istio-init
Environment:
------------
ENVOY_PORT=
INBOUND_CAPTURE_PORT=
ISTIO_INBOUND_INTERCEPTION_MODE=
ISTIO_INBOUND_TPROXY_MARK=
ISTIO_INBOUND_TPROXY_ROUTE_TABLE=
ISTIO_INBOUND_PORTS=
ISTIO_OUTBOUND_PORTS=
ISTIO_LOCAL_EXCLUDE_PORTS=
ISTIO_SERVICE_CIDR=
ISTIO_SERVICE_EXCLUDE_CIDR=

Variables:
----------
PROXY_PORT=15001
PROXY_INBOUND_CAPTURE_PORT=15006
PROXY_TUNNEL_PORT=15008
PROXY_UID=1337
PROXY_GID=1337
INBOUND_INTERCEPTION_MODE=REDIRECT
INBOUND_TPROXY_MARK=1337
INBOUND_TPROXY_ROUTE_TABLE=133
INBOUND_PORTS_INCLUDE=*
INBOUND_PORTS_EXCLUDE=15090,15021,15020
OUTBOUND_IP_RANGES_INCLUDE=*
OUTBOUND_IP_RANGES_EXCLUDE=
OUTBOUND_PORTS_INCLUDE=
OUTBOUND_PORTS_EXCLUDE=
KUBEVIRT_INTERFACES=
ENABLE_INBOUND_IPV6=false

Writing following contents to rules file:  /tmp/iptables-rules-1601296396725186907.txt554275073
* nat
-N ISTIO_INBOUND
-N ISTIO_REDIRECT
-N ISTIO_IN_REDIRECT
-N ISTIO_OUTPUT
-A ISTIO_INBOUND -p tcp --dport 15008 -j RETURN
-A ISTIO_REDIRECT -p tcp -j REDIRECT --to-ports 15001
-A ISTIO_IN_REDIRECT -p tcp -j REDIRECT --to-ports 15006
-A PREROUTING -p tcp -j ISTIO_INBOUND
-A ISTIO_INBOUND -p tcp --dport 22 -j RETURN
-A ISTIO_INBOUND -p tcp --dport 15090 -j RETURN
-A ISTIO_INBOUND -p tcp --dport 15021 -j RETURN
-A ISTIO_INBOUND -p tcp --dport 15020 -j RETURN
-A ISTIO_INBOUND -p tcp -j ISTIO_IN_REDIRECT
-A OUTPUT -p tcp -j ISTIO_OUTPUT
-A ISTIO_OUTPUT -o lo -s 127.0.0.6/32 -j RETURN
-A ISTIO_OUTPUT -o lo ! -d 127.0.0.1/32 -m owner --uid-owner 1337 -j ISTIO_IN_REDIRECT
-A ISTIO_OUTPUT -o lo -m owner ! --uid-owner 1337 -j RETURN
-A ISTIO_OUTPUT -m owner --uid-owner 1337 -j RETURN
-A ISTIO_OUTPUT -o lo ! -d 127.0.0.1/32 -m owner --gid-owner 1337 -j ISTIO_IN_REDIRECT
-A ISTIO_OUTPUT -o lo -m owner ! --gid-owner 1337 -j RETURN
-A ISTIO_OUTPUT -m owner --gid-owner 1337 -j RETURN
-A ISTIO_OUTPUT -d 127.0.0.1/32 -j RETURN
-A ISTIO_OUTPUT -j ISTIO_REDIRECT
COMMIT

iptables-restore --noflush /tmp/iptables-rules-1601296396725186907.txt554275073
Writing following contents to rules file:  /tmp/ip6tables-rules-1601296396994996978.txt804935788

ip6tables-restore --noflush /tmp/ip6tables-rules-1601296396994996978.txt804935788
iptables-save 
# Generated by iptables-save v1.6.1 on Mon Sep 28 12:33:16 2020
*nat
:PREROUTING ACCEPT [0:0]
:INPUT ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
:POSTROUTING ACCEPT [0:0]
:ISTIO_INBOUND - [0:0]
:ISTIO_IN_REDIRECT - [0:0]
:ISTIO_OUTPUT - [0:0]
:ISTIO_REDIRECT - [0:0]
-A PREROUTING -p tcp -j ISTIO_INBOUND
-A OUTPUT -p tcp -j ISTIO_OUTPUT
-A ISTIO_INBOUND -p tcp -m tcp --dport 15008 -j RETURN
-A ISTIO_INBOUND -p tcp -m tcp --dport 22 -j RETURN
-A ISTIO_INBOUND -p tcp -m tcp --dport 15090 -j RETURN
-A ISTIO_INBOUND -p tcp -m tcp --dport 15021 -j RETURN
-A ISTIO_INBOUND -p tcp -m tcp --dport 15020 -j RETURN
-A ISTIO_INBOUND -p tcp -j ISTIO_IN_REDIRECT
-A ISTIO_IN_REDIRECT -p tcp -j REDIRECT --to-ports 15006
-A ISTIO_OUTPUT -s 127.0.0.6/32 -o lo -j RETURN
-A ISTIO_OUTPUT ! -d 127.0.0.1/32 -o lo -m owner --uid-owner 1337 -j ISTIO_IN_REDIRECT
-A ISTIO_OUTPUT -o lo -m owner ! --uid-owner 1337 -j RETURN
-A ISTIO_OUTPUT -m owner --uid-owner 1337 -j RETURN
-A ISTIO_OUTPUT ! -d 127.0.0.1/32 -o lo -m owner --gid-owner 1337 -j ISTIO_IN_REDIRECT
-A ISTIO_OUTPUT -o lo -m owner ! --gid-owner 1337 -j RETURN
-A ISTIO_OUTPUT -m owner --gid-owner 1337 -j RETURN
-A ISTIO_OUTPUT -d 127.0.0.1/32 -j RETURN
-A ISTIO_OUTPUT -j ISTIO_REDIRECT
-A ISTIO_REDIRECT -p tcp -j REDIRECT --to-ports 15001
COMMIT
# Completed on Mon Sep 28 12:33:16 2020
```

主要工作是使用工具istio-iptables执行流量劫持配置。

容器的status状态

```json
"initContainerStatuses":[
      {
        "containerID":"docker://34ddf73c204ebe1ef3cb981465144cfb4150a020cf4de0f5efd18ab7b6496863",
        "image":"istio/proxyv2:1.7.0",
        "imageID":"docker-pullable://istio/proxyv2@sha256:c1f1b45a4162509f86aa82d0148aef55824454e7204f27f23dddc9d7f4ae7cd1",
        "lastState":{
          
        },
        "name":"istio-init",
        "ready":true,
        "restartCount":0,
        "state":{
          "terminated":{
            "containerID":"docker://34ddf73c204ebe1ef3cb981465144cfb4150a020cf4de0f5efd18ab7b6496863",
            "exitCode":0,
            "finishedAt":"2020-09-28T12:33:17Z",
            "reason":"Completed",
            "startedAt":"2020-09-28T12:33:16Z"
          }
        }
      }
    ],
```

`initContainerStatuses`记录了执行的结果状态，在`state.terminated`记录init容器结束的容器id，应用程序退出码，起止时间点，终止原因。这里`Completed`表示执行完成正常结束。

##### 3.3 运行istio-proxy

对istio-proxy容器发送命令，查看进程关系

```shell
# kubectl exec productpage-v1-797898bc54-lp5lq  -c istio-proxy -- ps -eHo pid,ppid,command
  PID  PPID COMMAND
19540     0 ps -eHo pid,ppid,command
    1     0 /usr/local/bin/pilot-agent 
   15     1   /usr/local/bin/envoy 
```

pid=1的进程是pilot-agent，它创建了子进程envoy，上文我们看Dockerfile时猜测的结论这里可以得到确认。这里有个扩展思考：pilot-agent进程为何是pid=1。有些应用程序作为pid=1进程会出现僵尸进程无法回收的坑，比如nodejs。参考资料：

[DOCKER DEMONS: PID-1, ORPHANS, ZOMBIES, AND SIGNALS](https://www.fpcomplete.com/blog/2016/10/docker-demons-pid1-orphans-zombies-signals/)

cmd的具体内容

```shell
# kubectl exec productpage-v1-797898bc54-lp5lq  -c istio-proxy -- ps -o pid,command
  PID COMMAND
    1 /usr/local/bin/pilot-agent proxy sidecar --domain default.svc.cluster.local --serviceCluster productpage.default --proxyLogLevel=warning --proxyComponentLogLevel=misc:error --trust-domain=cluster.local --concurrency 2
   15 /usr/local/bin/envoy -c etc/istio/proxy/envoy-rev0.json --restart-epoch 0 --drain-time-s 45 --parent-shutdown-time-s 60 --service-cluster productpage.default --service-node sidecar~172.16.0.58~productpage-v1-797898bc54-lp5lq.default~default.svc.cluster.local --local-address-ip-version v4 --log-format-prefix-with-location 0 --log-format %Y-%m-%dT%T.%fZ.%l.envoy %n.%v -l warning --component-log-level misc:error --concurrency 2
19546 ps -o pid,command
```

和Pod的配置文件一致。

格式化一下更清晰一些

```shell
/usr/local/bin/pilot-agent 
proxy 
sidecar #node type
--domain default.svc.cluster.local #DNS域名后缀，默认${POD_NAMESPACE}.svc.cluster.local
--serviceCluster productpage.default #Flags for proxy configuration
--proxyLogLevel=warning #启动proxy的日志级别："trace", "debug", "info", "warning", "error", "critical", "off"
--proxyComponentLogLevel=misc:error #
--trust-domain=cluster.local 
--concurrency 2 #number of worker threads to run

## envoy
/usr/local/bin/envoy 
-c etc/istio/proxy/envoy-rev0.json 
--restart-epoch 0 
--drain-time-s 45 
--parent-shutdown-time-s 60 
--service-cluster productpage.default 
--service-node sidecar~172.16.0.58~productpage-v1-797898bc54-lp5lq.default~default.svc.cluster.local 
--local-address-ip-version v4 
--log-format-prefix-with-location 0 
--log-format %Y-%m-%dT%T.%fZ.%l.envoy %n.%v 
-l warning 
--component-log-level misc:error 
--concurrency 2
```

查看容器的日志，很多envoy的日志，量比较大，而且是实时终端日志，自行查看。

#### 4. pilot-agent原理

这一部分，我们做源码剖析。

##### 4.1 pilot-agent cmd

源码的入口点：[main函数](https://github.com/istio/istio/tree/master/pilot/cmd/pilot-agent/main.go#L170)

```go
//全局变量存储proxy的所有参数，在启动的时候初始化
role  = &model.Proxy{}
func(c *cobra.Command, args []string) error {
   //1.默认使用sidecar，如果用户指定，则使用用户的第一个参数。目前支持两种proxy: sidecar和router
  	role.Type = model.SidecarProxy
    if len(args) > 0 {
       role.Type = model.NodeType(args[0])
    }
   //2.获取node的ip，如果没有则 ipv4- 127.0.0.1 和 ipv6- ::1 存储到role的ip列表
   //3.role ID设置为pod名字和命名空间的组合
   role.ID = podName + "." + podNamespace
   /*4.构建Proxy的配置实例：ProxyConfig
   		4.1 从/etc/istio/pod/annotations 读取内容
   		4.2 如果指定meshConfigFile了，则从指定地址读取mesh配置，否则使用PROXY_CONFIG环境变量获取
   		4.3 如果4.2 中存在默认的proxyconfig，则使用这个配置，否则创建一个新的默认proxyconfig
   			默认配置的初始化内容如下：
   			4.3.1 proxy的配置路径：/etc/istio/proxy 【必填】
   			4.3.2 istio-proxy: service cluster name used in xDS calls 【必填】
   			4.3.3 端口：15000 【必填】
   			4.3.4 工作线程个数2个
   			4.3.5 服务发现的服务地址：istiod.istio-system.svc:15012【必填】
   			4.3.6 proxy的源文件：/usr/local/bin/envoy 【必填】
   			4.3.7 状态端口：15020
   			4.3.8 和控制面板的鉴权策略：mTLS【必填】
   		4.4 使用用户指定的 --serviceCluster，--concurrency参数覆盖默认的ServiceCluster和Concurrency
   		4.5 检测4.3中配置的必填参数必须合法。
   		4.6 如果4.1的内容指定了服务发现的地址和status端口，则更新到proxy配置中。
   			这是因为用户通过API修改了配置并生成annotations内容
   */
   //这一步是十分关键的一步，它初始化了envoy启动所需的配置内容
   proxyConfig, err := constructProxyConfig()
   //5.设置DNS域名为用户指定--domain参数值，否则设置为${POD_NAMESPACE}.svc.cluster.local
   //6.初始化安全相关配置：jwt，CA, istio agent的配置参数
   secOpts := &security.Options{}
   //		6.1 是否启动SDS
   secOpts.EnableWorkloadSDS = true
	 secOpts.EnableGatewaySDS = enableGatewaySDSEnv
   //7.创建AgentConfig实例，初始化证书，从环境变化初始化参数
   //8.使用第4，6，7步参数，创建istio Agent实例
   sa := istio_agent.NewAgent(&proxyConfig, agentConfig, secOpts)
   //9.Start in process SDS.
	 _, err = sa.Start(role.Type == model.SidecarProxy, podNamespaceVar.Get())
   //10.第4步状态端口不为空，启动探测服务
  	  if proxyConfig.StatusPort > 0 {
				if err := initStatusServer(ctx, proxyIPv6, proxyConfig); err != nil {
					return err
				}
			}
    //11.创建envoy代理实例
    envoyProxy := envoy.NewProxy(envoy.ProxyConfig{})
  	//12.创建envoy agent
    agent := envoy.NewAgent(envoyProxy, drainDuration)
    /* 此实例作为envoy的管理代理，用于管理envoy的启动和清理工作
        12.1 将envoyProxy保存下来, 它是实际执行envoy的实例
        12.2 初始化exitStatus管道，用于proxy退出的通知
        12.3 activeEpochs 记录当前活动proxy，一个epoch代表一个proxy，并递增互斥访问
        12.4 currentEpoch 初始化为-1
        12.5 初始化终止所有proxy等待的时间
     */
        
    //13.Watcher is also kicking envoy start.
			watcher := envoy.NewWatcher(agent.Restart)
			go watcher.Run(ctx)
    /*监控配置的变化，当变化时重启envoy，保证重启完成，以及过程中只有一个envoy在运行
    		使用空字符串生成md5作为config传入更新配置，第一次必然执行拉起
  			13.1 执行加锁
  			13.2 比对currentConfig和传入配置的，如果相等，则解锁退出，否则继续
  			13.3 记录当前活动 epoch，epoch加1赋值给 currentEpoch，并以currentEpoch为key映射abortChan
  			13.4 解锁
  			13.5 如果有活动的epoch，则等待它启动成功，否则以currentEpoch异步方式启动一个新的proxy
  				13.5.1 等方式设置20s超时时间和500ms周期的tick轮询 epoch是否在activeEpochs map中
  				13.5.2 envoy的启动参见下文小节
  				13.5.3 启动完成，从activeEpochs字典中删除epoch，完成清理
  	 */
  
		//14. On SIGINT or SIGTERM, cancel the context, triggering a graceful shutdown
			go cmd.WaitSignalFunc(cancel)
		//15.启动agent，阻塞执行，直到第14步协程收到信号后执行ctx.Cancel()才会退出
			return agent.Run(ctx)
    /* 启动envoy的管家
    		15.1 如果从exitStatus收到消息，则表示有envoy结束了运行，则从中取出它的epoch从activeEpochs删除
    		15.2 如果activeEpochs为空，则退出整个程序的运行。
    		15.3 如何ctx收到取消通知，则收到退出信号，结束整个程序的运行。
    */
}
```

##### 4.2 启动 envoy

envoy实例的启动操作是[对象envoy](/Users/kowalskixu/Desktop/work/develop/service-mesh/src/istio.io/istio/pkg/envoy/proxy.go#41)的方法，它是上一步[envoy.NewProxy()](/Users/kowalskixu/Desktop/work/develop/service-mesh/src/istio.io/istio/pkg/envoy/proxy.go#63)实例化

```go
type envoy struct {
	ProxyConfig
	extraArgs []string
}
```

envoy的启动有上一节agent.Restart调启[(e *envoy) Run()](/Users/kowalskixu/Desktop/work/develop/service-mesh/src/istio.io/istio/pkg/envoy/proxy.go#145)

```go
func (e *envoy) Run(config interface{}, epoch int, abort <-chan error) error {
   //1.获取envoy的配置模板，默认是/var/lib/istio/envoy/envoy_bootstrap_tmpl.json
   //2.使用4.3.1中配置目录和epoch，和文件格式生成配置文件名，比如envoy-rev0.json
   //3.使用已有的配置信息初始化第1步的模板，将配置信息写入第2步的文件
   out, err := bootstrap.New(bootstrap.Config{...}).CreateFileForEpoch(epoch)
   //4.生成envoy的启动参数,这就是第3部分中看到的参数
   args := e.args(out, epoch, ISTIO_BOOTSTRAP_OVERRIDE) //第三个是环境变量
   /*
    	4.1 初始化基础参数
        -c etc/istio/proxy/envoy-rev0.json 
        --restart-epoch 0 
        --drain-time-s 45 
        --parent-shutdown-time-s 60 
        --service-cluster productpage.default 
        --local-address-ip-version v4 
        --log-format-prefix-with-location 0 
        --log-format %Y-%m-%dT%T.%fZ.%l.envoy %n.%v 
        -l warning 
        --component-log-level misc:error 
        --concurrency 2
        最新版本增加了--bootstrap-version=3
      4.2 如果设定了ISTIO_BOOTSTRAP_OVERRIDE，则读取文件内容，并设置给--config-yaml参数
   */
   //5.使用第4步的参数和源程序路径，启动envoy
    cmd := exec.Command(e.Config.BinaryPath, args...)
    cmd.Stdout = os.Stdout
    cmd.Stderr = os.Stderr
    if err := cmd.Start(); err != nil {
      return err
    }
    done := make(chan error, 1)
    go func() {
      done <- cmd.Wait()
    }()
  
  	//6.等待envoy启动完成或者中止启动
    select {
    case err := <-abort:
      log.Warnf("Aborting epoch %d", epoch)
      if errKill := cmd.Process.Kill(); errKill != nil {
        log.Warnf("killing epoch %d caused an error %v", epoch, errKill)
      }
      return err
    case err := <-done:
      return err
    }
}

```

##### 4.3 istio agent

在4.1小节的第8步，创建了istio agent，[NewAgent()](/Users/kowalskixu/Desktop/work/develop/service-mesh/src/istio.io/istio/pkg/istio-agent/agent.go#162),随后在第9步启动了它，我们看看这个[Start()](/Users/kowalskixu/Desktop/work/develop/service-mesh/src/istio.io/istio/pkg/istio-agent/agent.go#229)

```go
func (sa *Agent) Start(isSidecar bool, podNamespace string) (*sds.Server, error) {
   //1.生成WorkloadSecrets，gatewaySecretCache
   //2.创建一个sds server
  server, err := sds.NewServer(sa.secOpts, sa.WorkloadSecrets, gatewaySecretCache)
   /*creates and starts the Grpc server for SDS.
   		2.1 创建一个Server实
   		2.2 以 WorkloadSecrets 创建SDSService 初始化workloadSds成员变量
   		2.3 以 gatewaySecretCache 创建SDSService 初始化gatewaySds成员变量
   			2.3.1 起一个协程，创建一个ticker，定时清理过期的连接
   			2.3.2 清理方式就是从全局的sdsClients map中删除staledClientKeys中的连接
   		2.4 依据4.1小节第6步的设置，如果 EnableWorkloadSDS = true,则 initWorkloadSdsService，同理
   			依据EnableGatewaySDS 是否 initGatewaySdsService
   */
   //3. Start the local XDS generator.
  sa.startXDSGenerator(sa.proxyConfig, sa.WorkloadSecrets, podNamespace)
   //4.初始化本地DNS server
  sa.initLocalDNSServer(isSidecar)
   //5.如果设置了ProxyXDSViaAgent，从环境变量PROXY_XDS_VIA_AGENT获取，默认True，则初始化XdsProxy
  sa.xdsProxy, err = initXdsProxy(sa)
  /* 
  		5.1 创建一个XdsProxy实例
  		5.2 /etc/istio/proxy/XDS创建  unix socket 下游的grpc server，等待下游envoy的连接请求
  		5.3 listener, server 记录到 XdsProxy实例
  		5.4 创建上游（agent请求的服务）dial client的参数
  		5.5 协程启动XdsProxy实例的server
  		5.6 初始化证书的监控：rootCert, certFile, keyFile，当证书变更时要端口上游的连接重连
  		5.7 启动proxy.healthChecker
  		5.8 返回XdsProxy实例
   */
}
```





