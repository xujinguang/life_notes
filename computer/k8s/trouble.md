## 常见问题

### Pod长期处于Pending

```sh
kubectrl describe pod/node xxx
```
```sh
Events:
  Type     Reason            Age               From               Message
  ----     ------            ----              ----               -------
  Warning  FailedScheduling  3s (x7 over 34s)  default-scheduler  0/3 nodes are available: 1 Insufficient cpu, 2 node(s) didn't match node selector.

```

查看pod信息，查看选择器是否正确
nodeSelector/nodeName是否正确

### Readiness probe failed: HTTP probe failed with statuscode: 503

```sh
  Normal   Scheduled              10m                  default-scheduler      Successfully assigned istio-ingressgateway-645bfd8ddb-jzkm5 to 172.16.16.14
  Normal   SuccessfulMountVolume  10m                  kubelet, 172.16.16.14  MountVolume.SetUp succeeded for volume "ingressgatewaysdsudspath"
  Normal   SuccessfulMountVolume  10m                  kubelet, 172.16.16.14  MountVolume.SetUp succeeded for volume "ingressgateway-ca-certs"
  Normal   SuccessfulMountVolume  10m                  kubelet, 172.16.16.14  MountVolume.SetUp succeeded for volume "ingressgateway-certs"
  Normal   SuccessfulMountVolume  10m                  kubelet, 172.16.16.14  MountVolume.SetUp succeeded for volume "istio-ingressgateway-service-account-token-z4n6q"
  Normal   SuccessfulMountVolume  10m                  kubelet, 172.16.16.14  MountVolume.SetUp succeeded for volume "istio-certs"
  Normal   Pulled                 10m                  kubelet, 172.16.16.14  Container image "docker.io/istio/node-agent-k8s:1.1.3" already present on machine
  Normal   Created                10m                  kubelet, 172.16.16.14  Created container
  Normal   Started                10m                  kubelet, 172.16.16.14  Started container
  Normal   Pulled                 10m                  kubelet, 172.16.16.14  Container image "docker.io/istio/proxyv2:1.1.3" already present on machine
  Normal   Created                10m                  kubelet, 172.16.16.14  Created container
  Normal   Started                10m                  kubelet, 172.16.16.14  Started container
  Warning  Unhealthy              42s (x299 over 10m)  kubelet, 172.16.16.14  Readiness probe failed: HTTP probe failed with statuscode: 503
```



1. describe pod 报这个错误
2. 尝试进入istio-system 下 polite 容器
3. 重启polite

原因是proxy访问polite不通。