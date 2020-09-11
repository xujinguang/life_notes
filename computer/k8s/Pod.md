## K8S Pod

### Pod

*Pod* （就像在鲸鱼荚或者豌豆荚中）是一组（一个或多个）[容器](https://kubernetes.io/docs/concepts/overview/what-is-kubernetes/#why-containers)（例如 Docker 容器），这些容器共享存储、网络、以及怎样运行这些容器的声明。

Kubernetes 对象模型中创建或部署的最小和最简单的单元。pod表示在集群中运行的进程。

Pod 封装了应用程序容器、存储资源、唯一网络 IP 以及控制容器应该如何运行的选项。 Pod表示部署单元。

如果希望横向扩展应用程序（例如，运行多个实例），则应该使用多个 Pod，每个应用实例使用一个 Pod 。在 Kubernetes 中，这通常被称为 *副本*。通常使用一个称为控制器的抽象来创建和管理一组副本 Pod。

### 网络

每个 Pod 分配一个唯一的 IP 地址。 Pod 中的每个容器共享网络命名空间，包括 IP 地址和网络端口。 *Pod 内的容器* 可以使用 `localhost` 互相通信。 当 Pod 中的容器与 *Pod 之外* 的实体通信时，它们必须协调如何使用共享的网络资源（例如端口）

### 存储

一个 Pod 可以指定一组共享存储[卷](https://kubernetes.io/docs/concepts/storage/volumes/)。 Pod 中的所有容器都可以访问共享卷，允许这些容器共享数据。 卷还允许 Pod 中的持久数据保留下来，以防其中的容器需要重新启动。

### 控制器

Kubernetes 使用了一个更高级的称为 *控制器* 的抽象，由它处理相对可丢弃的 Pod 实例的管理工作。 因此，虽然可以直接使用 Pod，但在 Kubernetes 中，更为常见的是使用控制器管理 Pod。

控制器包括：Deployment，StatefulSet, DaemonSet



