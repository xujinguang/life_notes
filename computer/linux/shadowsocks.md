@[TOC](搭建流程)
# shadowsocks
```shell
#1. 安装shadowsocks，未安python, pip自行安装
pip install shadowsocks
安装最新版本
pip install https://github.com/shadowsocks/shadowsocks/archive/master.zip
为了支持gcm加密，安装3.0版本`pip3 install https://github.com/shadowsocks/shadowsocks/archive/master.zip -U`
#2.编辑配置文件
touch ssconf.json
#3.填写ss服务配置如下
vim ssconf.json
```
```json
{
  "server": "your_ss_server", 
  "server_port": 2021,
  "password": "your_ss_server_key",
  "timeout": 300,
  "method": "aes-256-gcm",
  "local_address": "127.0.0.1",
  "local_port": 1080
}
```
说明：
1. `timeout`不设置，默认300
2. 加密方式`method`不设置，默认`aes-256-cfb`
3. `local_address`本地代理服务ip，默认使用`127.0.0.1`,`0.0.0.0`
4. `local_port`本地代理服务监听的端口，sock协议默认1080

```shell
#4.启动本地代理服务
sslocal -c ssconf.json
#或之使用命令参数制定远程ss服务的地址
sslocal -s server_ip -p server_port -k passwd
#如果以常住进程启动，则使用-d参数
sslocal -c ssconf.json -d start

#5. 如果机器关闭，那么代理服务进程会被关闭，下次开机需要手动启动。如果随机器启动，则配置
echo " nohup sslocal -c your_conf_path /dev/null 2>&1 &" /etc/rc.local

#6.本地代理服务测试
#6.1 查看服务启动情况
netstat -nlp | grep 1080
tcp        0      0 127.0.0.1:1080          0.0.0.0:*               LISTEN      19192/python3       
udp        0      0 127.0.0.1:1080          0.0.0.0:*                           19192/python3   
#6.2 如果返回配置中远程ss服务的ip，则成功
curl --socks5 127.0.0.1:1080 http://httpbin.org/ip
{
  "origin": "x.x.x.x"
}
```
### 问题
AttributeError: module 'collections' has no attribute 'MutableMapping'

```shell
>>> from collections import MutableSequence
Traceback (most recent call last):
  File "C:\Program Files\Python310\lib\code.py", line 90, in runcode
    exec(code, self.locals)
  File "<input>", line 1, in <module>
ImportError: cannot import name 'MutableSequence' from 'collections' (C:\Program Files\Python310\lib\collections\__init__.py)

>>> from collections.abc import MutableSequence
```

找到报错问题，将collections换成collections.abc即可

# genpac
```shell
#7. 使用pac模式，需要生成pac文件,安装genpac
pip install genpac

#8. 生成pac文件,生成时需要sslocal启动，否则访问不了gfwlist.txt文件
genpac --proxy="SOCKS5 127.0.0.1:1080" --gfwlist-proxy="SOCKS5 127.0.0.1:1080" -o autoproxy.pac --gfwlist-url="https://raw.githubusercontent.com/gfwlist/gfwlist/master/gfwlist.txt"
```
生成命令来源[^1]
将文件设置到网络代理上
`设置`-`网络`-`网络代理`-`自动`-配置url
```shell
file:///your_file_path/autoproxy.pac
```
# privoxy
Shadowsocks 是一个 socket5 服务，http服务请求需要切换到sock5服务上来，可以使用privoxy代理

```shell
#9.安装privoxy，此时是无法访问官为下载安装的，可以使用系统镜像
yum install privoxy -y

# 10. 配置privoxy
vim /etc/privoxy/config
#文件最后添加,注意最后的点
forward-socks5t / 127.0.0.1:1080 .

# 11.启动privoxy，不推荐使用root帐号启动privoxy，创建新用户
useradd privoxy
# 启动
sudo privoxy --user privoxy /etc/privoxy/config
```
#配置代理	
```shell
#12.设置http代理环境变量
export https_proxy=http://127.0.0.1:8118
export http_proxy=http://127.0.0.1:8118

#13.测试,返回内容，则大功告成
curl www.google.com
```
1. 8118是`privoxy`的默认端口
2. 此时设置只对终端有效，对于浏览器需要手动设置这环境变量

打开浏览器的设置，找到网络设置，代理设置成手动，设http_proxy, https_proxy为上述字段，到此完成。
# 写在最后
网络流量图
```mermaid
flowchart LR
	web-->privoxy-->sslocal-->ssserver-->google

```
1. 如果不想使用代理了，需要手动将浏览器的代理关闭
2. 如果想在开机时自动启动，参考上面设定

参考网文[^2][^3]

[^1]: [socks5通讯工具及genpac](https://www.jianshu.com/p/41c5c3d56f89)
[^2]: https://blog.csdn.net/lpwmm/article/details/104044963
[^3]: https://cjh.zone/2018/11/25/%E7%A7%91%E5%AD%A6%E4%B8%8A%E7%BD%91-Linux-Ubuntu-16-04-%E9%85%8D%E7%BD%AEshadowsocks%E5%AE%A2%E6%88%B7%E7%AB%AF/
