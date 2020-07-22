## Linux命令集

### iptables

iptables服务把用于处理或过滤流量的策略条目称之为规则，多条规则可以组成一个规则链，而规则链则依据数据包处理位置的不同进行分类：PREROUTING，INPUT，FORWORD，OUTPUT，POSTROUTING

匹配规则执行动作包括：ACCEPT（允许流量通过）、REJECT（拒绝流量通过）、LOG（记录日志信息）、DROP（拒绝流量通过）QUEUE(流入用户控件)，RETURN 停止遍历该链，并在前一个(调用)链中的下一个规则处继续,如果不是调用链，则执行默认规则。REJECT则会在拒绝流量后再回复一条信息.默认策略拒绝动作只能是DROP，而不能是REJECT

```shell
-t #目前有三个表 filter,nat,mangle;默认filter：INPUT，FORWORD，OUTPUT
-N #创建一个新链
-L #列出所有规则
-I/D/A/R #插入、删除、追加、替换
-p 协议
-s	#匹配来源地址IP/MASK，加叹号“!”表示除这个IP外
-d	#匹配目标地址
-i #网卡名称	匹配从这块网卡流入的数据
-o #网卡名称	匹配从这块网卡流出的数据
-p	#匹配协议，如TCP、UDP、ICMP
--dport num	#匹配目标端口号
--sport num	#匹配来源端口号
```

```shell
$ sudo iptables -L -nv
#从一个IP中删除流量
$ sudo iptables -I INPUT -s 198.51.100.0 -j DROP
#ipv6 - ip6tables
```



保存的路径：/etc/sysconfig/iptables

CentOS设置

```shell
1. 创建文件/etc/iptables/iptables.rules
2. sudo iptables-restore < /etc/iptables/iptables.rules
3. sudo service iptables save
```



### ip

用来显示或操纵Linux主机的路由、网络设备、策略路由和隧道，是Linux下较新的功能强大的网络配置工具。

```shell
Usage: ip [ OPTIONS ] OBJECT { COMMAND | help }
       ip [ -force ] -batch filename
where  OBJECT := { link | addr | addrlabel | route | rule | neigh | ntable |
                   tunnel | tuntap | maddr | mroute | mrule | monitor | xfrm |
                   netns | l2tp | tcp_metrics | token }
       OPTIONS := { -V[ersion] | -s[tatistics] | -d[etails] | -r[esolve] |
                    -h[uman-readable] | -iec |
                    -f[amily] { inet | inet6 | ipx | dnet | bridge | link } |
                    -4 | -6 | -I | -D | -B | -0 |
                    -l[oops] { maximum-addr-flush-attempts } |
                    -o[neline] | -t[imestamp] | -b[atch] [filename] |
                    -rc[vbuf] [size] | -n[etns] name | -a[ll] }
```

示例

```shell
$ ip "-6", "addr", "add", "::6/128", "dev", "lo" #设置网卡的地址
-6 ipv6
addr 地址
add 添加
::6/128 ip
dev 
lo 网卡
```

