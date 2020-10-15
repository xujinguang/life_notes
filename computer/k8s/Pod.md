## API资源配置

### 1. 资源配置格式

资源配置文件使用 `kubectl get type name -o json/yaml`获得，它包括apiVersion、kind、metadata、spec和status五个核心部分。

### 2. 资源配置文档

```shell
kubectl explain SOURCE
```

### 3. 资源管理方式

- 声明式配置 - apply 
- 陈述式配置 - create、delete、get和replace等
- 陈述式命令 - run、expose、delete和get等

工具kube-applier等一类的项目实现自动化声明式配置

## Pod

### Pod

*Pod* （就像在鲸鱼荚或者豌豆荚中）是一组（一个或多个）[容器](https://kubernetes.io/docs/concepts/overview/what-is-kubernetes/#why-containers)（例如 Docker 容器），这些容器共享存储、网络、以及怎样运行这些容器的声明。

Kubernetes 对象模型中创建或部署的最小和最简单的单元。pod表示在集群中运行的进程。

Pod 封装了应用程序容器、存储资源、唯一网络 IP 以及控制容器应该如何运行的选项。 Pod表示部署单元。

如果希望横向扩展应用程序（例如，运行多个实例），则应该使用多个 Pod，每个应用实例使用一个 Pod 。在 Kubernetes 中，这通常被称为 *副本*。通常使用一个称为控制器的抽象来创建和管理一组副本 Pod。

### 网络

每个 Pod 分配一个唯一的 IP 地址。 Pod 中的每个容器共享网络命名空间，包括 IP 地址和网络端口。 *Pod 内的容器* 可以使用 `localhost` 互相通信。 当 Pod 中的容器与 *Pod 之外* 的实体通信时，它们必须协调如何使用共享的网络资源（例如端口）

### 存储

一个 Pod 可以指定一组共享存储[卷](https://kubernetes.io/docs/concepts/storage/volumes/)。 Pod 中的所有容器都可以访问共享卷，允许这些容器共享数据。 卷还允许 Pod 中的持久数据保留下来，以防其中的容器需要重新启动。





## 架构

<img src="https://d33wubrfki0l68.cloudfront.net/7016517375d10c702489167e704dcb99e570df85/7bb53/images/docs/components-of-kubernetes.png" style="zoom:50%;" />

### 1. 节点

Kubernetes 通过将容器放入在节点（Node）上运行的 Pod 中来执行你的工作负载。 节点可以是一个虚拟机或者物理机器，取决于所在的集群配置。每个节点都包含用于运行 [Pod](https://kubernetes.io/docs/concepts/workloads/pods/pod-overview/) 所需要的服务，这些服务由 [控制面](https://kubernetes.io/zh/docs/reference/glossary/?all=true#term-control-plane)负责管理。

通常集群中会有若干个节点。节点上的[组件](https://kubernetes.io/zh/docs/concepts/overview/components/#node-components)包括 [kubelet](https://kubernetes.io/docs/reference/generated/kubelet)、 [容器运行时](https://kubernetes.io/docs/reference/generated/container-runtime)以及 [kube-proxy](https://kubernetes.io/zh/docs/reference/command-line-tools-reference/kube-proxy/)。

节点的注册包括两种：1.kubelet注册。2.手动注册。

节点状态：地址，状况，容量和信息。

某些情况下，当节点不可达时，API 服务器不能和其上的 kubelet 通信。删除 Pod 的决定不能传达给 kubelet，直到它重新建立和 API 服务器的连接为止。 与此同时，被计划删除的 Pod 可能会继续在游离的节点上运行。

#### 1.1 kubelet

kubelet 是在每个 Node 节点上运行的主要 “节点代理”。它可以通过以下方式向 apiserver 进行注册：主机名（hostname）；覆盖主机名的参数；某云服务商的特定逻辑。

kubelet 是基于 PodSpec 来工作的。每个 PodSpec 是一个描述 Pod 的 YAML 或 JSON 对象。kubelet 接受通过各种机制（主要是通过 apiserver）提供的一组 PodSpec，并确保这些 PodSpec 中描述的容器处于运行状态且运行状况良好。kubelet 不管理不是由 Kubernetes 创建的容器。

除了来自 apiserver 的 PodSpec 之外，还可以通过以下三种方式将容器清单（manifest）提供给 kubelet。

File（文件）：利用命令行参数给定路径。kubelet 周期性地监视此路径下的文件是否有更新。监视周期默认为 20s，且可通过参数进行配置。

HTTP endpoint（HTTP 端点）：利用命令行参数指定 HTTP 端点。此端点每 20 秒被检查一次（也可以使用参数进行配置）。

HTTP server（HTTP 服务器）：kubelet 还可以侦听 HTTP 并响应简单的 API（当前未经过规范）来提交新的清单。

`kubelet` 负责创建和更新 `NodeStatus` 和 `Lease` 对象。

#### 1.2 kube-proxy

#### 1.3 容器运行时

##### 容器的配置

1. 通过命令参数：Dockerfile中的ENTRYPOINT和CMD

2. 静态配置文件：Docker镜像时COPY配置文件的传入方式

3. 环境变量传入： b.yaml文件的envFrom挂载，ConfigMap就是其中一种。

4. 挂载宿主节点的文件，需要每个节点都需要同步一份

5. ConfigMap挂载卷

   在容器创建完成后，修改command和args并不会直接生效

### 2. 控制器

Kubernetes 使用了一个更高级的称为 *控制器* 的抽象，由它处理相对可丢弃的 Pod 实例的管理工作。 因此，虽然可以直接使用 Pod，但在 Kubernetes 中，更为常见的是使用控制器管理 Pod。

控制器包括：Deployment，StatefulSet, DaemonSet等等

控制器分为两类：原生控制器管理器和云控制器管理器

### 3. 调度器

### 4. API-Server

### 5. etcd



