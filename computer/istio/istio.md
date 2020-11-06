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

/Users/kowalskixu/Desktop/work/develop/service-mesh/src/istio.io/istio/pilot/cmd/pilot-discovery/main.go

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



/Users/kowalskixu/Desktop/work/develop/service-mesh/src/istio.io/istio/pilot/pkg/bootstrap/server.go

```go
func NewServer(args *PilotArgs) (*Server, error) {
}
```



