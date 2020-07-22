## Linux命令集

### iptables

iptables服务把用于处理或过滤流量的策略条目称之为规则，多条规则可以组成一个规则链，而规则链则依据数据包处理位置的不同进行分类：PREROUTING，INPUT，FORWORD，OUTPUT，POSTROUTING

匹配规则执行动作包括：ACCEPT（允许流量通过）、REJECT（拒绝流量通过）、LOG（记录日志信息）、DROP（拒绝流量通过）QUEUE(流入用户控件)，RETURN 停止遍历该链，并在前一个(调用)链中的下一个规则处继续,如果不是调用链，则执行默认规则。REJECT则会在拒绝流量后再回复一条信息.默认策略拒绝动作只能是DROP，而不能是REJECT

DNAT

 它所做的是在作出路由决策之前将目的地址(和目的端口)更改为给定的值，并使路由决策基于新的参数。这里很重要的一点是，它实际上修改了 IP (和 TCP)报头，并且需要连接跟踪才能工作，因为应答包应该匹配并返回来。  这个解决方案需要在实际路由完成时在内核中启用 ip 转发。 

REDIRECT

 与规则匹配的传入数据包将其目的地址更改为接收接口的地址，并可选地将其目的端口更改为特定或随机端口 。与DNAT不同是，它只需要端口，不需要指定转发地址。因为有些时候不知道转发地址是哪里。

[TPROXY](http://gsoc-blog.ecklm.com/iptables-redirect-vs.-dnat-vs.-tproxy/) 透明代理

比上面两者更高级，只转发数据包，不修改包头，不需要连接跟踪。 

[透明代理](https://www.kernel.org/doc/Documentation/networking/tproxy.txt)通常包括路由器上的“拦截”流量。这通常是通过 iptables REDIRECT 目标完成的; 但是，该方法有严重的限制。其中一个主要问题是它实际上修改了数据包以更改目标地址——这在某些情况下可能是不可接受的。(例如，考虑代理 UDP: 您将无法找到原始的目标地址。即使在 TCP 获取原始目的地址的情况下也是可活动的。 



```shell
$ iptables --table nat --append PREROUTING --protocol tcp --dport 80 --jump DNAT --to-destination 10.0.4.2
$ iptables --table nat --append PREROUTING --protocol tcp --dport 80 --jump REDIRECT --to-ports 8080
$ iptables -t mangle -A PREROUTING -p tcp --dport 80 -j TPROXY \
  --tproxy-mark 0x1/0x1 --on-port 50080
```



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

### ss

ss is used to dump socket statistics. It allows showing information similar to netstat.  It can display more TCP and state informations than other tools.