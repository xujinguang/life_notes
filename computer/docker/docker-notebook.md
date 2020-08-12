## Docker

##术语

1. CE和EE- 社区版本和企业版本
2. CLI- Command Line Interface

##第一部分 简介

### 1. 简介

- 开源
- Apache 2.0 协议
- Docker公司开发

### 2. 目标

- 提供简单，轻量的建模方式
- 职责的逻辑分离——开发关心容器中程序的运行；运维关心如何管理容器
- 快速高效的开发生命周期
- 鼓励面向服务架构——推荐一个容器运行一个进程

### 3. 组成

1. Docker引擎，包括服务端和客户端

   - C/S架构——通过命令行或者REST API交互。
   - <img src="https://docs.docker.com/engine/images/engine-components-flow.png">

2. Docker镜像

   * 容器的源代码，基于联合的文件系统的一种层次结构，即分层，迭代构建
   * 镜像的构建和打包阶段
   * 镜像是静态的，需要通过容器装载

3. Docker容器

   * 借用集装箱运输万物的设计哲学
   * 容器是镜像的启动和执行阶段
   * 容器具有生命周期

   > A container is a runnable instance of an image. You can create, start, stop, move, or delete a container using the Docker API or CLI. You can connect a container to one or more networks, attach storage to it, or even create a new image based on its current state.

4. Registry

   * 保存构建好的镜像；
   * 分为公有和私有。官方的称为Docker hub；也可以自己本地搭建私有Registry
   * 用户可以在Docker hub创建自己的镜像；

### 4. 技术依赖

1. 容器只能运行与底层宿主机相同或相似的操作系统——Linux只能运行Linux的相关发行版，不能运行Windows
2. 依赖内核特性技术：linux的control group——隔离CPU/内存， namespace；写时复制；Union file systems； container format 
   - **The pid namespace:** Process isolation (PID: Process ID).
   - **The net namespace:** Managing network interfaces (NET: Networking).
   - **The ipc namespace:** Managing access to IPC resources (IPC: InterProcess Communication).
   - **The mnt namespace:** Managing filesystem mount points (MNT: Mount).
   - **The uts namespace:** Isolating kernel and version identifiers. (UTS: Unix Timesharing System).
   - Docker Engine combines the namespaces, control groups, and UnionFS into a wrapper called a container format. The default container format is `libcontainer`. In the future, Docker may support other container formats by integrating with technologies such as BSD Jails or Solaris Zones.
3. 不依赖于模拟层和管理层，使用内核系统调用接口；
4. 日志，交互shell

### 5. 技术栈

* Docker命令
* Docker compose，consul，swarm 编配
* Docker API
* Docker原理



## 第二部分 Docker命令

```shell
$ docker info #检查docker是否正确安装,显示docker的配置信息
$ docker run -ti ubuntu /bin/bash #运行一个容器；
```

1. `-i STDIN -t` 伪tty终端

2. 启动过程原理参考[这里](https://docs.docker.com/engine/docker-overview/#CONTAINERS) 

   > When you use the `docker pull` or `docker run` commands, the required images are pulled from your configured registry. When you use the `docker push` command, your image is pushed to your configured registry.

3. `-- name `指定容器的名称，命名必须唯一

4. `-d` 创建守护容器

5. `-p` 指定映射地址和端口

6.  --link name:alias 连接容器

    

```shell
$ docker ps -a #查看所有容器
```

`-n` 显示最后`n`个容器

```shell
$ docker start/restart/stop/kill/attach/rm xxx #启动/重启/停止/杀死/附着/删除
```



```shell
$ docker logs/stats/top/inspec xxx #日志/状态/top进程/
```



### 2.镜像的管理

```shell
$ docker pull/push name:version #指定版本获取或推送镜像name
$ docker images [name] #显示所有镜像或者指定镜像
$ docker search name #搜索镜像
```

#### 2.1构建

```shell
$ docker commit ID name #将容器ID提交为镜像name
$ docker build #Dockerfile
$ docker images --tree #查看image之间的树形家族结构
```

​	第一个命令是对已有的容器执行某些操作之后，执行commit从而形成一个新的镜像。commit的只是创建的容器和当前状态之间差异的部分。commit可以使用`-m`提交镜像信息；`-a`作者信息。

​	Dockerfile文件所在目录称为构建上下文。Docker会将上下文中的文件和目录上传到Docker的守护进程。因此无关的文件不要放置于此。

​	Dockerfile的构建过程如下：

* docker 从基础镜像运行一个程序
* 执行一条指令，对容器作出修改
* 执行类似 docker commit的操作，提交一个新的镜像层
* 再基于刚提交的镜像运行一个新容器
* 执行Dockerfile的下一条指令，重复上述操作。直到所有的命令都执行完毕。

可以从构建终端信息感知这一过程。

如果中间某个指令失败了，再次执行构建会跳过已经构建成功的分层，从失败的分层开始。如果想全部重新构建，则需要指定参数`--no-cache`

由上述过程可知，Dockerfile中的每一条指令都会构建一个镜像分层，因为为了加快构建效率，Dockerfile不要写无关的指令，以及可以合并指令，压缩成单条指令，比如一条RUN

镜像的选择原则：如果只是运行服务，使用最小的基础镜像就好。构建环境的镜像一般比较大。也没有必要使用完整的OS基础镜像。

ADD，COPY，RUN每一条命令都会创建一个镜像层。

#### 2.2Dockerfile语法[DOC](https://docs.docker.com/engine/reference/builder/)

* 大小写无敏感，一般采用大小以区分参数；#为注释信息
* 顺序执行
* 必须以FROM开始；前导可以有ARG指令；以用于基础镜像

包括四个部分：基础镜像，维护人员，操作命令，启动指令

|    名称    | 作用                         | 说明                                                         |
| :--------: | ---------------------------- | ------------------------------------------------------------ |
|    FROM    | 指定基础镜像                 |                                                              |
| MAINTAINER | 维护人信息                   |                                                              |
|    CMD     | 容器启动时要运行的命令       | 类似于docker run会被run命令覆盖                              |
| ENTRYPOINT | 容器启动要运行的命令         | 无法被run 执行命令覆盖,只能指定一个，多个会使用最后一个      |
|  WORKDIR   | 工作目录                     | RUN CMD ENTRYPOINT的工作目录；可以当CD用；<br/>run -w参数可覆盖此命令 |
|    ENV     | 设置环境变量                 | 等价于 run -e 参数                                           |
|    USER    | 镜像以什么用户运行           | 不指定默认root                                               |
|   VOLUME   | 添加卷                       | 数据的存储和共享的方式                                       |
|    ADD     | 将文件或目录复制到镜像       | 源可以是URL，压缩包，并自动解压                              |
|    COPY    | 同ADD                        | 不具有解压功能                                               |
|   LABEL    | 添加元数据                   | 键值对方式提供                                               |
| STOPSIGNAL | 停止容器时发送什么信号给容器 |                                                              |
|    ARG     | 传递构建时的变量             | 通过build —build-arg参数传递；HTTP_PROXY等已预定义           |
|  ONBUILD   | 镜像触发器                   |                                                              |
|    RUN     | 执行命令                     | 每运营一条run命令，则镜像添加一层                            |
|   EXPOSE   | 容器内监听的端口             | 只声明，不会自动映射                                         |
|   SHELL    | 指定shell类型                |                                                              |
| HEATHCHECK | 检测容器状态                 | 新功能                                                       |

用法

| FROM       | FROM image\|:<tag>\|@<digit> <as name>                  |
| ---------- | ------------------------------------------------------- |
| LABEL      | LABEL <key>=<value> <key>=<value>                       |
| EXPOSE     | EXPOSE <port> <port>/<proto>                            |
| ENV        | ENV <key>=<value>                                       |
| ENTRYPOINT | ENTRYPOINT ["arg1" "arg2"] 或者 ENTRYPOINT arg1 arg2    |
| VOLUME     | VOLUME ["/data"]                                        |
| USER       | USER uid                                                |
| WORKDIR    | WORKDIR /ddata 可以多个使用。每次相对上一个为起始路径。 |
| ADD        | ADD <src> <dest> 相对路径是相对Dockerfile和WORKDIR目录  |
| COPY       | COPY <src> <dest> 与ADD区别是ADD支持压缩包              |

#### 2.3 Dockerfile的注意事项

1. 精简镜像用途，保证镜像的单一性。
2. 选择合适的基础镜像。
3. 完善的注释和版本信息。
4. 正确使用版本号
5. 减少镜像层数。
6. 使用.dockerignore 减少发送给docker服务的内容。
7. 删除临时文件和缓存
8. 合理使用cache
9. 调整命令顺序。

## 第三部分 Docker编配和服务发现

### Docker Compose

```shell
$ docker-compose up/down/start/stop/ps/logs
```



## 第四部分 DockerAPI



## 第五部分 Docker 原理

### 1. Docker镜像

文件系统叠加而成，最底端是引导文件系统bootfs。当一个容器启动后，它会被卸载，以节省内存空间。第二层是rootfs，它可以是一种或多种操作系统。rootfs会以只读的方式加载，利用`union mount`技术在rootfs上加载更多的只读文件系统。联合加载的各层文件系统叠加在一起，如此形成包含所有底层文件目录的文件系统。最上层是一个读写文件系统，程序的执行就是在此层中执行。

当容器启动时，初始化的读写层时空的，当文件系统变化时，这些变化会应用到这一层。换言之，修改文件，会从下面的只读层将数据复制到该层进行修改，然后隐藏掉只读层的数据。即用户只看到读写层的数据副本。这就是写时复制。

由于是只读的，所以不用担心被修改。由于镜像分层的，所以可以分层共享，提高存储以及拉取镜像的速度。修改后的镜像是一个新的镜像，只改变了一部分层而已。

### 2. Docker服务端

1. dockerd - 提供RestAPI，默认监听在unix://var/run/docker.sock
2. docker-proxy - 映射端口时候运行
3. containerd - grpc响应dockerd的请求，管理runC的镜像和容器
4. Containerd-shim - 为runC提供支持，同时作为容器内的根进程。

## 附录

1. [docker offical image]([https://docs.docker.com/samples/#Library%20references](https://docs.docker.com/samples/#Library references))
2. dockerfile[官方文档](https://docs.docker.com/engine/reference/builder/)

## 《Docker 进阶与实战》

### 与虚拟机的区别

没有Hypervisor层，因此开销更少，因为不需要资源的完全隔离

Docker不会直接和内核交互，而是通过更底层的Libcontainer与内核交户，它是真正的容器引擎。

### 第二章 容器的技术历史

1. 硬件的虚拟化，半虚拟化，操作系统虚拟化
2. Linux的原生特性
3. 主要包括namespace和cgroup内核特性
4. 前者是访问隔离，后者是资源控制。
5. 这个特性在2.6的版本就已经完善了。主要贡献公司是IBM，Parallels

#### 2.2 一分钟理解容器

容器 = cgroup + namespace + rootfs + 容器引擎

1. 通过 clone 系统调用传入namespace对应的flag，创建一个新子进程，该进程拥有自己的namespace。flag=CLONE_NEWPID|CLONE_NEWNS|CLONE_NEWUSER|CLONE_NEWNET|CLONE_NEWIPC|CLONE_NEWUTC
2. 将进程pid卸乳cgroup子系统种，如此该进程受到cgroup的资源限制；目录/sys/fs/cgroup
3. 通过privot_root系统调用进入一个新的rootfs。之后通过exec系统调用执行命令

#### 2.3 Cgroup子系统

1. control group的简写，隔离一组进程对系统资源的使用Qos
2. 资源包括：CPU，内存，block IO 网络带宽
3. cpuset/cpu/cpuacct/memory/blkio/devices子系统
4. docker的exec通过setns实现

####2.4 Namespace

namespace包括6种：IPC，Newwork，Mount，PID，UTS，User

#####UTS

对主机名和域名进行隔离

#####IPC

inter process communication - 进程间通信

#####PID

隔离ns下的进程ID

#####mount

隔离文件挂载点的

#####network

每个netword都有自己的网络设备，IP地址，路由表，net目录，端口号。

#####user

隔离用户和组ID

###第三章 理解Docker镜像

把运行环境和应用一起打包

#### 3.1 image的表示

imagehub/namespace/repo:tag

Layer:镜像有一系列的分层组成，每层用64位16进制表示

ImageID镜像最上层的layer id即为镜像的ID

#### 3.2 image 命令

* 显示悬挂镜像 —filer "dangling=true" 显示无名无 tag 的镜像，此镜像不会被任何镜像依赖，一般属于垃圾，最好删除
* 分析分层依赖使用工具dockviz

#####Build

1. 下载镜像
2. 导入save的镜像 `docker save -o busybox.tar busybox; docker load -i busybox.tar`
3. `docker export/import`
4. docker commit
5. 使用Dockerfile `docker build`

##### Ship

* 镜像仓库中转
* 传输 `docker export/save` 生成的tag包
* 分享Dockerfile

##### Run

`docker run/start/stop/kill/`等容器的生命周期管理命令

#### 3.3 image 数据

* 缓存在目录` /var/lib/docker` 下
* `graph`和`overlay`目录包含本地镜像库中所有的元数据和数据信息
* `docker inspec` 其实就是将上面目录下的数据可读话输出展示
* 包括：Id，parent，comment，container，config，author，arch，os等
* 其中Env，Cmd会作为容器启动的环境变量和启动命令

#### 3.4 扩展技术

##### 联合挂载

把多个目录挂载到同一个目录下，对外呈现这些目录的合集。多个目录可以是不同的文件系统。

* 1993 Inheriting FS开始
* 2003 unionfs
* 2004 Union mounts
* 2006 aufs
* 2014 OverlayFS（3.18 release） - 用于liveCD，只读（只能只读）挂载，然后增加读写层，依赖写时复制

overly 底层还可以嵌套挂载，上层文件会覆盖同名的下层文件。

不同的社区和厂家维护着不同的版本，都可以用来支持docker，因为对外的特性是一致的，但是存储结构和性能存在差异。

#####写时复制

最常见的就是fork进程；联合挂载也使用到了。即底层的文件的变更不会发生改变，而是在上面添加一层读写层覆盖掉底层（隐藏）的数据。底层的数据并没有变化，包括删除操作也不会影响底层数据，因此删除是不能减少存储空间的。节约空间的方式就是减少不必要的只读层，换言之，无关的无用的分层从一开始就不要让它出现。







