##Q: kubectl: Unable to connect to the server: Gateway Timeout
A: 
1.如果网络有墙，设置代理后启动
```sh
export HTTP_PROXY=http://<proxy hostname:port>
export HTTPS_PROXY=https://<proxy hostname:port>
export NO_PROXY=localhost,127.0.0.1,10.96.0.0/12,192.168.99.0/24,192.168.39.0/24,192.168.64.0/24
```
也可以在start时通过--docker-env指定，默认从环境变量获取

2.屏蔽自己ip
```
您似乎正在使用代理，但您的 NO_PROXY 环境不包含 minikube IP (192.168.64.4)。如需了解详情，请参阅 https://minikube.sigs.k8s.io/docs/reference/networking/proxy/
```
将提示ip加入NO_PROXY

##配置CPU、memory
```sh
minikube config set memory 4096
minikube config set cpus 2
```


