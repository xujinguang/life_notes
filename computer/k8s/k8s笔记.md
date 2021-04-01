# k8s笔记

## 云发展

资源上云和应用上云，从Iaas,PaaS到SaaS

## 应用

1. 依赖的系统资源
   1. 特权指令，操作系统延伸出来的内核态和用户态。
   2. 大内核-单进程，所有的功能都是函数调用
   3. 微内核-只保留核心功能，驱动相关的热插拔方式
   4. extos，一个失败的项目，但是思想是正确的，很巧妙的在虚拟化里实现了。思想就是每个应该有自己的OS，真实的OS越小越好。每个应用占用资源不同，因此根据需要提供私有的OS中间层，OS中间层和真实的OS交互。
2. 应用的管理

## 第一部分 基础内容

### 1.1 容器与镜像

#### 1.1 什么是容器

提供独立的运行环境，具有独立的文件系统，资源控制，视图隔离的进程集合

依赖于Cgroup，NS，rootfs技术



#### 1.2 什么是镜像

运行容器所需的文件集合——镜像。

镜像是分层的，带来的优势是复用，叠加，快速分发和减少磁盘占用。本质上带来的就是资源的节约。

1. 多个镜像相同分层的部分则复用，比如基础镜像。避免数据副本，减少磁盘存储
2. 拉取或者推送镜像到镜像仓库只需要操作变更的分层，减少IO流量
3. 已构建的分层不用重复构建，加快构建速度



#### 1.3 容器和镜像之间的关系

容器是镜像的运行时。



#### 1.4 Docker镜像的构建

1. 使用Dockerfile ` docker build`
2. 对容器修改后，执行提交`docker commit ID name`

第一种方式，实现了一个标准化的构建方式，提供了很多灵活的构建命令，主佳。

* docker 从基础镜像运行一个程序

* 执行一条指令，对容器作出修改

* 执行类似 docker commit的操作，提交一个新的镜像层

* 再基于刚提交的镜像运行一个新容器

* 执行Dockerfile的下一条指令，重复上述操作。直到所有的命令都执行完毕。

  

#### 1.5 Docker容器的生命周期

`docker run/start/stop/kill/`等容器的生命周期管理命令



#### 1.6 Docker镜像管理

1. pull / push - 获取和推送镜像
2. images - 查看镜像
3. rmi - 删除镜像



### 1.2 容器管理

#### 1.2.1 常用命令

```shell
$ docker login #登录镜像仓库
$ docker build  #构建镜像
$ docker pull #拉取镜像
$ docker push #推送镜像
$ docker search #搜索镜像
$ docker rmi #删除镜像
$ docker ps #查看容器
$ docker rm #删除容器
$ docker run #执行容器
$ docker stop #停止容器
$ docker start #启动容器
$ docker pause #暂停容器
$ docker exec #执行命令
```

全部命令参考[官网](https://docs.docker.com/engine/reference/commandline/docker/)



容器和服务器之间互相拷贝文件，查看容器日志信息

学会分析容器为何报错及解决问题的能力

数据卷的使用

容器中变量的使用

容器的端口映射

link方式实现容器互联

### **docker网络管理**

容器互联

练习：用wordpress+MySQL搭建个人博客

### **自定义镜像**

使用Dockerfile自定义镜像的流程

如何在安装软件包

如何把文件拷贝到新镜像里

如何指定数据卷

如何在新镜像里指定变量

练习1：创建一个NGINX镜像

练习2：创建个具有ssh功能的镜像

### **本地仓库管理**

使用register 搭建本地仓库

往本地docker仓库推送镜像

在客户端上设置默认仓库地址

删除本地仓库里的镜像

### **限制容器资源**

了解为什么要限制资源及cgroup介绍

限制内存资源

限制容器CPU资源

### 用cAdvisor监控容器

cadvisor介绍

部署cadvisor监控容器负载

### compose的使用

了解docker-compose

编写docker-compose.yml文件

### 使用harbor搭建私有仓库

安装harbor

配置harbor

往harbor里上传镜像

从harbor下载镜像

## 第二部分 K8S部分

### 2.1 什么是kubernetes

kubernetes-希腊单词，舵手。由于单词10个字符太长，不好书写，将中间8个字符省略，缩写k8s。

容器是集装箱的概念，那么k8s呢就想成为一个轮船的舵手，将容器编排到各种环境中。因此，它是**自动化的容器编排平台**。

核心功能：

1. 服务发现和LB
2. 容器的自动装箱
3. 自动发布和回滚【1】
4. 存储编排
5. 配置秘钥管理
6. 批量执行
7. 自动容器恢复【2】
8. 水平伸缩【3】



### 2.2 kubernetes框架

<img src="./image/k8s.png" style="zoom:50%;" />

#### 2.2.1 控制平面组件

一般运行在同一个Node上

1. kube-apiserver:资源以RESTful API的方式提供管理，也是唯一和etcd交互的组件，可以多实例；
2. etcd:分布式一致性和高可用性的键值存储数据库
3. kube-scheduler： 它负责在Kubernetes集群中为一个Pod资源对象找到合适的节点并在该节点上运行
4. kube-controller-manager：节点控制器，副本控制器，端点控制器，服务帐户和令牌控制器等等。

#### 2.2.2 Node组件

1. kubelet：用来接收、处理、上报kube-apiserver组件下发的任务。kubelet进程启动时会向kube-apiserver注册节点自身信息。它主要负责所在节点（Node）上的Pod资源对象的管理
2. kube-proxy：集群中每个节点上运行的网络代理
3. continer runtime: 负责运行容器的软件

#### 2.2.3 CLI

##### 2.2.3.1 kubectl

官方CLI，用于和API-server交互，功能详情参考官方命令[一览表](https://kubernetes.io/docs/reference/generated/kubectl/kubectl-commands)

##### 2.2.3.2 client-go

CLI库，提供开发者二次开发，kubectl就是基于此实现

##### 2.2.3.3 web dashboard

### 2.3 kubernetes核心概念

k8s的核心抽象概念包括以下几个部分：

1. 容器
2. 工作负载：pod，控制器
3. 服务，负载均衡
4. 策略
5. 存储和配置
6. 安全
7. 网络
8. 调度和驱逐
9. 扩展k8s

### 2.4 kubernets资源

#### 2.4.1 对象

k8s的控制面的核心是API-server，它提供HTTP API，以供用户、集群中的不同部分和集群外部组件相互通信。注意这句话中的三个客体。对于用户来说，就是通过API来查询和操作k8s对象。使用kubectl官方客户端，然后k8s的资源以yaml或者json声明形式描述，一般包括以下5个部分

```yaml
apiVersion: v1 #资源版本
kind: Pod #资源类型
metadata: #资源元数据，名称，命令空间，标签关于容器
spec: #实际的pod说明
status: #pod的当前信息
```

apiVersion 以URL的方式指定，格式是：API资源分组/资源版本，资源分组不一定有，但是资源版本必须指定。资源版本分为三类：alpha，beta和不包括前面两个的稳定版本。	这样做的有个优势，版本共存互不干扰，而且使得API分组。

kind指定资源类型。

metadata资源相关的重要的三个字段：

```yaml
metadata:
	label: #用于帅选资源
	annotations: #注解，比label大，可以包括特殊字符或结构化内容
	owerreference: #资源所有者，归属类，用于反查对象
```

```shell
$ kubectl get pods --show-labels # -l key=value 标签过滤，多个是与的关系，或的关系使用in
$ kubectl label pods ${POD} key=value #--overwrite
$ kubectl label pods ${POD} key- #取消标签
```

Kubernetes 对象是 “目标性记录” —— 一旦创建对象，Kubernetes 系统将持续工作以确保对象存在。 通过创建对象，本质上是在告知 Kubernetes 系统，所需要的集群工作负载看起来是什么样子的， 这就是 Kubernetes 集群的 **期望状态（Desired State）**，由Spec字段描述。

更多api规范参考[这里](https://github.com/kubernetes/community/blob/master/contributors/devel/sig-architecture/api-conventions.md)

#### 2.4.2 命令和声明的区别

资源的控制有两种方式，命令方式和声明的方式

| 分类   | 命令                                    | 声明                                                 |
| ------ | --------------------------------------- | ---------------------------------------------------- |
| 示例   | 滚/过来/放下/站住                       | 距离我3km/需要近距离依靠/今年KPI达到100亿            |
| 特点   | 精确的，必须的，一蹴而就的              | 模糊的，没有硬性要求的，逐渐收敛的                   |
| 失败   | 1. 可重入性 - 重试；2.不可重入-数据修正 | 自身可重入，可反复重入：说一百遍100亿目标还是100亿！ |
| 成功   | 无状态，需要额外的操作日志              | 自身具有状态信息                                     |
| 多操作 | 不可重入的需要加锁                      | 多个操作可以合并                                     |



### 2.4 Pod

#### 2.4.1 为什么需要pod

容器是被隔离的，资源受限的进程，容器里PID=1的进程就是应用本身。这也是容器设计的初衷，PID1可以直接接受信号，进程终止意味着容器的退出。

Linux操作系统里有进程组的概念，它包含一组进程，共享一些资源。比如对一个进程组发出的的信号，会被递送到这个组群下的每个进程成员中，目的就是让逻辑上协同的进程统一管理。

如果将进程组的多个进程放入同一个容器，那么PID=1必须是具有

1. 类似Linux的init进程来管理其他进程生命周期的能力（核心是僵尸进程资源的回收），否则，当PID=1进程出现异常挂掉，其他多个进程就成了孤儿进程。比如Nodejs就明确强调不要作为PID=1的进程，容器中僵尸进程的问题一直都存在，目前没有很好的解决办法，只能从使用上尽量避免。（之前有看到过相关文章）
2. 管理日志。多个进程的标准输出日志的拆分就需要用户自己去挑了。而单进程就可以直接输出到终端上，不会有什么混淆。

某些场景的需要多个容器组成类似进程组的需要共享资源，因此，Pod就充当了“进程组”的角色。它表示一组容器的逻辑单元，标识容器集合，作为k8s的最小执行和调度单元，并为包含的容器提供共享的资源，比如Network，UTS和IPC命名空间（linux相同的命名空间下是资源共享的）。Pod和进程组一样只是一个上层逻辑实体，而真实运行的实体是容器。

容器和POD的关系，我认为是进程和进程组在云的延伸，或者映射，并不难理解。在裸机上往往存在多个进程之间的协作关系，Google在Borg项目中同样发现很多应用需要协同工作，并需要部署在一台机器上来共享一些资源。

此外，更重要的一点。分别运行于各自容器的进程之间无法实现基于IPC的通信机制，此时，容器间的隔离机制对于依赖于此类通信方式的进程来说却又成了阻碍。Pod资源抽象正是用来解决此类问题。

说pod是进程组只是理解的角度而已，其实它不是进程组，它标识的容器集合。一般也会把pod理解成逻辑主机或者VM。

#### 2.4.2 为什么pod是调度单位

正如上面小节提到，多个容器需要共享资源，有些资源是无法跨Node共享的，需要部署在同一个Node上。

逻辑上协同的容器，需要同时启动或者退出，扩缩容等统一的调度。

如果多个容器分开调度，某些资源条件也不一定能同时满足。

最重要还是命名空间的资源共享。

#### 2.4.3 pod和容器

1. 同一个pod下的容器共享Network命名空间，所以具有相同的ip地址和端口空间，因此容器ip和端口不能冲突
2. 不同pod下的容器ip和端口互不干扰，因此ip端口可以相同，要不然也不能水平扩缩容了
3. 同一个pod下的容器共享IPC命名空间，因此具有相同的主机名，域名，以及localhost可以互相访问。
4. 常见的用法是一个pod中只包含一个容器，原因有两个
   1. 扩缩容的解耦，k8s面向pod扩缩容的，不是容器。业务包括AB服务，服务A扩容2个，服务B并不一定需要。
   2. 充分利用计算资源，基于第一点，自然可以理解这一点。比如两个Node，可以让AB分别跑在不同Node上。
5. 一个pod运行多个容器情况，需要做业务之外的辅助工作，sidecar proxy容器，日志，数据，监控收集器，适配器(sidecar模型，大师模型，适配器模型)。

#### 2.4.4 pod管理

使用yaml或json创建,包括三部分

```yaml
apiVersion: v1
kind: Pod
metadata: #资源元数据，名称，命令空间，标签关于容器
spec: #实际的pod说明
	containers: #容器列表
	- name: #容器的名称
		image: #容器的镜像
			imagePullPolicy: #镜像的拉取策略，Always,IfNotPresent,Never
		command: #容器的命令，如果不指定，则使用容器自己的
		args: #容器的参数
		env: #环境变量
		securityContext: #安全上下文
status: #pod的当前信息
```



```shell
$ kubectl apply -f pod.yaml #创建
$ kubectl get po {POD} #查询 -w -o yaml
$ kubectl replace -f pod.yaml #替换 --force 强制
$ kubectl logs {POD} #pod日志
$ kubectl delete {POD} #删除pod
```

### 2.5 控制器

因为pod有生命周期，为了降低用户管理pod的复杂度（CRUD），k8s提供一些负责资源来帮忙管理pod，这些负载资源称为控制器。

#### 2.5.1 控制器分类

k8s已有的控制器包括如下：

1. ReplicationController - 管理副本 【不推荐使用，由2，3替代】
2. ReplicaSet - 控制pod副本
3. Deployment - 部署无状态应用
4. StatefulSet - 部署有状态应用
5. DaemonSet - 部署宿主应用
6. Job - 部署任务
7. CronJob - 部署定时任务
8. 垃圾收集 - *属主资源* 被删除时在集群中清理这些对象
9. 结束后存在时间 - Job 结束之后的指定时间间隔之后删除它们

##### 2.5.1.1 ReplicationController

这个已经不再建议使用，有兴趣可以了解。为了方便后面的对比，提到一点：它本身既不执行就绪态探测，也不执行活跃性探测。 它不负责执行自动缩放，而是由外部自动缩放器控制。

##### 2.5.1.2 ReplicaSet

它通常用来保证给定数量的、完全相同的 Pod 的可用性。pod的ReplicaSet 的信息被设置在 metadata 的`ownerReferences` 字段中。[yaml示例](https://kubernetes.io/examples/controllers/frontend.yaml)

1. 创建 - 编写描述问文件：<br/>一个用来识别可获得的 Pod 的集合的选择算符`.spec.selector`<br/>一个用来标明应该维护的副本个数的数值`.spec.replicas`<br>一个用来指定应该创建新 Pod 以满足副本个数条件时要使用的 Pod 模板`.spec.template`
2. 描述 - kubectl get rs RS -o yaml|kubectl  get pod 的`ownerReferences`字段
3. 删除 - 只删除rs指定参数 `--cascade=false`，注意此时pod都正常存在（可以创建新的rs指定相同的selector接管，但并不会更新老pod的镜像），否则pod会被垃圾控制器同时删除。
4. 缩放 - 更新`.spec.replicas`数值。

建议：一般不用直接管理rs，推荐使用Deployment

##### 2.5.1.3 Deployment

它为pod和rs提供声明方式的更新能力。用户描述 Deployment 中的 *目标状态*，而 Deployment 控制器以受控速率更改实际状态， 使其变为期望状态。

更新 Deployment 的 PodTemplateSpec，新的 ReplicaSet 会被创建，Deployment 以受控速率将 Pod 从旧 ReplicaSet 迁移到新 ReplicaSet。 每个新的 ReplicaSet 都会更新 Deployment 的修订版本。

1. 创建- 使用yaml声明描述，然后apply文件
2. 更新
   1. `kubectl set image deployment/$dep $name=$image --record`
   2. `kubectl edit deployment/$dep`
   3. 默认情况下，它确保至少所需 Pods 75% 处于运行状态（最大不可用比例 25%MaxUnavaliable） 
   4. 默认情况下，它可确保启动的 Pod 个数比期望个数最多多出 25%  MaxSurge
3. 查看
   1. `kubectl rollout status deployment/$dep`
   2. `kubectl get deployment`
   3. `kubectl get rs`
   4. `kubectl describe deployments`
   5. `kubectl rollout history deployment`
4. 回滚 
   1. 使用set image
   2. `kubectl rollout undo deployment` 指定版本--to-revision=x
5. 缩放 - scale
6. 暂停/恢复 - `kubectl rollout pause/resume  deployment`

**更新策略：重建和滚动**

- `.spec.strategy.type==Recreate`，在创建新 Pods 之前，所有现有的 Pods 会被杀死。
- `.spec.strategy.type==RollingUpdate`时，采取 滚动更新的方式更新 Pods。你可以指定 `maxUnavailable` 和 `maxSurge` 来控制滚动更新 过程。

`maxUnavailable`保证了服务的质量。`maxSurge` 保证在资源有限情况的控制。比如资源比较紧张而限制超出的数量。这两个参数需要取得一个平衡。这两个值不能同时为0.

**版本管理**

`.spec.revisionHistoryLimit` 字段以指定保留此 Deployment 的多少个旧有 ReplicaSet。其余的 ReplicaSet 将在后台被垃圾回收。 默认情况下，此值为 10。如果设置0， 意味着将清理所有具有 0 个副本的旧 ReplicaSet。 在这种情况下，无法撤消新的 Deployment 上线

**部署失败常见问题**

- 配额（Quota）不足
- 就绪探测（Readiness Probe）失败
- 镜像拉取错误
- 权限不足
- 限制范围（Limit Ranges）问题
- 应用程序运行时的配置错误

##### 2.5.1.4 StatefulSets

StatefulSet 用来管理 Deployment 和扩展一组 Pod，并且能为这些 Pod 提供*序号和唯一性保证*,实现有状态应有的管理。和 Deployment 不同的是，StatefulSet 为它们的每个 Pod 维护了一个固定的 ID。这些 Pod 是基于相同的声明来创建的，但是不能相互替换：无论怎么调度，每个 Pod 都有一个永久不变的 ID。

1. 部署，按照编号依次创建，前驱running或者ready时，才会创建下一个，除非设定并行启动
2. 缩容- 按照编号逆序删除
3. 更新 - 同样有两种更新策略：重建和滚动，遵循1,2点

##### 2.5.1.5 DaemonSet

*DaemonSet* 确保全部（或者某些）节点上运行一个 Pod 的副本。 当有节点加入集群时， 也会为他们新增一个 Pod 。 当有节点从集群移除时，这些 Pod 也会被回收。删除 DaemonSet 将会删除它创建的所有 Pod。

使用场景：

- 在每个节点上运行集群守护进程
- 在每个节点上运行日志收集守护进程
- 在每个节点上运行监控守护进程

1. 创建部署 - `.spec.selector` 必须填写 Pod 选择算符，而且一旦创建不可修改，否则出现悬挂pod

##### 2.5.1.6 Jobs

​	Job 会创建一个或者多个 Pods，并确保指定数量的 Pods 成功终止。 随着 Pods 成功结束，Job 跟踪记录成功完成的 Pods 个数。 当数量达到指定的成功个数阈值时，任务（即 Job）结束。 删除 Job 的操作会清除所创建的全部 Pods。也可以使用 Job 以并行的方式运行多个 Pod。

- `.spec.selector` 是可选
- `.spec.completions` 字段设置为非 0 的正数值
- 不设置，complettion，默认值为 `.spec.parallelism=1`，可以指定并发数

##### 2.5.1.7 CronJob

CronJob 仅负责创建与其调度时间相匹配的 Job，并周期性地在给定的调度时间执行 Job，而 Job 又负责管理其代表的 Pod	

##### 2.5.1.8 垃圾收集

Kubernetes 垃圾收集器的作用是删除某些曾经拥有属主（Owner）但现在不再拥有属主的对象。属主通过资源的`metadata.ownerReferences` 字段指定。比如由 ReplicationController、ReplicaSet、StatefulSet、DaemonSet、Deployment、 Job 和 CronJob 所创建或管理的对象。如果删除对象时，不自动删除它的附属，这些附属被称作 *孤立对象*

在上面rs控制器中提到可以指删除rs而不删除关联的pod，方法就是指定级联参数为false。默认是级联删除。包括2种模式：

1. 前台级联删除
2. 后台级联删除

两种删除的区别在于删除属主的时候是否立即删除附属资源。前者是先删除附属资源然后在删除属主。后者则只删除属主，让垃圾收集后台删除。可以通过`deleteOptions.propagationPolicy` 字段配置：`Orphan`、`Foreground` 或者 `Background`

##### 2.5.1.9 TTL控制器

TTL 控制器现在只支持 Job。集群操作员可以通过指定 Job 的 `.spec.ttlSecondsAfterFinished`字段来自动清理已结束的作业（`Complete` 或 `Failed`）

##### 2.5.1.10 总结

通过前9个小节每个控制器的介绍，这里做一个总结和对比。

1. 它们都统一的声明式描述资源对象。比如apiVersion,kind,metadata,spec以及运行状态status。
2. 全部都约束使用selector选择器，有些控制器是必须的，比如，有些是可选的
3. 每个控制器for不同的场景。当然用途最广的也是最必须的就是ReplicaSet和Deployment控制器。
4. 垃圾收集是隐藏的，只是在其他控制器中一个配置项而存在。
5. 

#### 2.5.2 控制器原理

![](./image/controller-loop.png)

给定一个预期状态输入Spec到Controller, 它和系统的反馈的当前Status执行diff操作，然后根据具体状态差异转换成相应的Op操作下发给System，系统的再将结果状态通过输出到传感器。如此迭代循环，直到Status收敛到Spec为止。

这是一个思想流程图，映射成k8s，system就是apiserver, sensor和controller更具体一点就是这个图了

![controller](https://raw.githubusercontent.com/kubernetes/sample-controller/master/docs/images/client-go-controller-interaction.jpeg)

它分为两个部分：client-go和custom controller，图中的每个组件的作用详细说明看[这里](https://github.com/kubernetes/sample-controller/blob/master/docs/controller-client-go.md)

#### 2.5.2 控制器工作示例

阿里云公开课里有个示例，修改rs=2为3，变化过程如图：

![](.\image\sensor-example1.png)

![](image\sensor-example2.png)

#### 2.5.3 控制器源码分析

参考[这里](https://www.huweihuang.com/kubernetes-notes/code-analysis/kube-controller-manager/sharedIndexInformer.html)

#### 2.5. 自定义控制器

这个图片和上面的图本质是一样的，但是更容易理解一点是，蓝色部分是client-go负责，红色部分是业务自己实现的部分。

![](https://res.cloudinary.com/dqxtn0ick/image/upload/v1555472372/article/code-analysis/informer/client-go.png)

[教学视频](https://v.qq.com/x/page/c03641vzw2m.html?start=43)

对于CRD的话，client-go是没有相应的informer和clientset等等的，那么怎么办呢？k8s提供了自动生成工具。

[示例教学](https://tangxusc.github.io/2019/05/code-generator%E4%BD%BF%E7%94%A8/)

### 2.6  应用配置

#### 2.6.1 ConfigMap

1. 创建 kubectl create configmap NAME DATA， DATA-文件，目录，键值对
2. 使用 挂载配置文件，环境变量，命令行参数

#### 2.6.2 Secret 

存储密码等敏感信息，使用base64存储 ， 1MB，

### 2.7 持久化

有两个问题，pod的数据如何持久化以及pod如果通过磁盘实现数据通信？k8s通过Volumes 来解决。

#### 2.7.1 持久化的问题

volumne的类型：

1. 本地存储 emptydir 、hostpath
2. 网络存储 out of tree
3. secret/CM
4. PV（Persistent Volumes）和PVC

前3点面临问题包括：

1. pod销毁重建后，如何数据复用？
2. Node节点宕机后，上面的pod需要重建，核心在如何将数据迁移
3. 同一个pod内容器可以共享，但是多个pod之间如何共享数据？
4. 数据卷如何做扩展功能？

这些问题导致4的诞生。也是本小节的重点

#### 2.7.2 PVC和PV

有了pv为什么还要有个PVC呢？

1. 职责分离，PVC只负责声明，PV真是的存储管理
2. PVC简化用户的使用
3. 二者关系是面向对象的接口和实现

如果实现的呢？分为静态和动态方式创建PV。静态是管理员预先创建PV，然后PVC绑定关系。这样增加了管理员和资源的浪费。最好的方式动态创建PV。方式通过中间模板StorageClass，PVC提交自己的需求，指定SC，k8s结合二者动态生成PV，然后将PVC和PV绑定。后者是用户需要多少资源，则创建多大的PV。PVC是用户感知的需求。管理员只负责StorageClass即可，这个包括具体的存储参数。

```yaml
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: slow-pv
provisioner: kubernetes.io/aws-ebs #重点：使用哪个卷插件制备 PV，这个插件对于云厂商使用标准库自行开发
parameters: #制备器的参数，最多支持512，总长度不超过256kB
  type: gp2
reclaimPolicy: Retain #PV在PVC删除时的回收策略， Delete 或者 Retain，默认删除
allowVolumeExpansion: true
mountOptions:
  - debug
volumeBindingMode: Immediate #绑定和制造PV的时刻
	#Immediate-PVC创建了就立即创建PV并绑定；WaitForFirstConsumer 延迟绑定
allowedTopologies: #WaitForFirstConsumer时有效，限定允许的拓扑
- matchLabelExpressions:
  - key: failure-domain.beta.kubernetes.io/zone
    values:
    - us-central1-a
    - us-central1-b
```

pvc使用

```yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: disc-pvc
spec:
  accessModes:
    - ReadWriteOnce
  storageClassName: slow-pv #指定StorageClass Name
  resources:
    requests:
      storage: 30Gi #声明自己的需求
```

pod挂载

```yaml
  volumes:
    - name: data
      persistentVolumeClaim:
        claimName: disc-pvc #指定pvc
```

官方也支持很多标准的存储卷。

PV的状态：pending->avaliable->bound->released->deleted/failed

处于released状态的PV无法回到起点的。因此次PV是不可用的，想复用手动新建PV或者不删除PVC。复用PVC，这也是StatefulSet的策略。

### 3. 服务发现

#### 3.1 为什么需要服务发现

1. pod动态变化，ip不固定
2. 统一的访问入口和负载均衡
3. 不同环境下同样的部署拓扑

#### 3.2 service的工作原理

创建一个虚拟IP:port，按照指定的负载均衡将请求路由到后端pod的

服务的类型:clusterIP, NodePort,LoadBalancer，ingress

服务的发现

环境变量

DNS的发现方式

### **健康性检查**

pod的默认检查策略

通过liveness对pod健康性检查

使用readiness对pod健康性检查

健康性检查在各种环境中的应用

### **其他控制器**

了解什么是daemonset及为什么要用到daemonset

通过标签设置pod运行在特定的机器上

了解并配置ReplicationController

了解并配置ReplicaSet

了解并配置StatefulSet

对比所有这些控制器的区别及应用范围





## 第三部分 K8S实战

### **kubernetes安装**

通过kubeadmin方式安装kubernetes

安装后需要做的完善工作

添加和删除节点

安装metric server 实现监控节点及pod负载

了解什么是命名空间，并对命名空间进行管理

多集群之间切换

### **存储管理**

#### 本地卷

emptyDir

hostPath

#### 网络数据卷

NFS

iscsi

#### 持久性存储

persistent volume

persistent volume claim

persistent volume回收策略

### **job**

创建job

了解job中pod的重启策略

计划任务job

### **网络模型**

kubernetes跨机器通信原理

配置calico网络实现跨主机通信

网络策略

### **Kubernetes包管理Helm3**

Helm工具的架构和安装使用

Charts的概念和使用

搭建私有源

### **helm3实战-kubernetes日志管理**EFK

Kubernetes日志收集方法介绍

ELK概述及部署

ELK日志管理

### **helm3实战-kubernetes监控**

监控方案

使用prometheus监控

### **安全及配额管理**

#### 了解kubernetes的验证方式

用户名和密码的登录方式 创建用户

kubeconfig的登录方式 创建kubeconfig

#### 了解基于角色的访问控制RBAC

了解什么是角色，给角色设置权限

如何把角色绑定给用户

什么是集群角色，如何把集群角色绑定用户

了解什么Service Account（SA）

如何给SA设置权限

#### 资源限制

了解pod里resource字段的意思并验证

LimitRange

ResourceQuota

### **DevOps**

了解devops

gitlab配置

安装及配置Jenkins

使用gitlab+Jenkins+kubernetes建立CI/CD解决方