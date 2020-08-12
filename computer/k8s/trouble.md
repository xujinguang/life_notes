## 常见问题

### 1. Pod长期处于Pending

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



### 2.Pod无法创建

```sh
# kubectl get deployment
NAME             READY   UP-TO-DATE   AVAILABLE   AGE
details-v1       0/1     0            0           4h6m
productpage-v1   0/1     0            0           4h6m
ratings-v1       0/1     0            0           4h6m
reviews-v1       0/1     0            0           4h6m
reviews-v2       0/1     0            0           4h6m
reviews-v3       0/1     0            0           4h6m
```



```shell
# kubectl get deployment details-v1 -o yaml
status:
  conditions:
  - lastTransitionTime: "2020-07-20T03:52:59Z"
    lastUpdateTime: "2020-07-20T03:52:59Z"
    message: Deployment does not have minimum availability.
    reason: MinimumReplicasUnavailable
    status: "False"
    type: Available
  - lastTransitionTime: "2020-07-20T03:52:59Z"
    lastUpdateTime: "2020-07-20T03:52:59Z"
    message: 'admission webhook "sidecar-injector.istio.io" denied the request: template:
      inject:140:7: executing "inject" at <gt .ProxyConfig.Concurrency 0>: error calling
      gt: invalid type for comparison'
    reason: FailedCreate
    status: "True"
    type: ReplicaFailure
  - lastTransitionTime: "2020-07-20T04:03:00Z"
    lastUpdateTime: "2020-07-20T04:03:00Z"
    message: ReplicaSet "details-v1-78db589446" has timed out progressing.
    reason: ProgressDeadlineExceeded
    status: "False"
    type: Progressing
  observedGeneration: 1
  unavailableReplicas: 1
```

查看是否开始

```shell
# kubectl get namespace -L istio-injection
bookinfo          Active   124d   enabled
default           Active   125d   enabled
istio-system      Active   12d    disabled
```

获取

```shell
#kubectl get mutatingwebhookconfiguration istio-sidecar-injector -o yaml
apiVersion: admissionregistration.k8s.io/v1
kind: MutatingWebhookConfiguration
metadata:
  annotations:
    kubectl.kubernetes.io/last-applied-configuration: |
      {"apiVersion":"admissionregistration.k8s.io/v1beta1","kind":"MutatingWebhookConfiguration","metadata":{"annotations":{},"labels":{"app":"sidecar-injector","install.operator.istio.io/owning-resource":"installed-state","install.operator.istio.io/owning-resource-namespace":"istio-system","istio.io/rev":"default","operator.istio.io/component":"Pilot","operator.istio.io/managed":"Reconcile","operator.istio.io/version":"1.6.3","release":"istio"},"name":"istio-sidecar-injector"},"webhooks":[{"clientConfig":{"caBundle":"","service":{"name":"istiod","namespace":"istio-system","path":"/inject"}},"failurePolicy":"Fail","name":"sidecar-injector.istio.io","namespaceSelector":{"matchLabels":{"istio-injection":"enabled"}},"rules":[{"apiGroups":[""],"apiVersions":["v1"],"operations":["CREATE"],"resources":["pods"]}],"sideEffects":"None"}]}
  creationTimestamp: "2020-04-29T01:30:22Z"
  generation: 11
  labels:
    app: sidecar-injector
    install.operator.istio.io/owning-resource: installed-state
    install.operator.istio.io/owning-resource-namespace: istio-system
    istio.io/rev: default
    operator.istio.io/component: Pilot
    operator.istio.io/managed: Reconcile
    operator.istio.io/version: 1.6.3
    release: istio
  name: istio-sidecar-injector
  resourceVersion: "7942709016"
  selfLink: /apis/admissionregistration.k8s.io/v1/mutatingwebhookconfigurations/istio-sidecar-injector
  uid: 676f2599-f9fc-4fbf-8729-d368d3dd735b
webhooks:
- admissionReviewVersions:
  - v1beta1
  clientConfig:
    caBundle: LS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSUMzVENDQWNXZ0F3SUJBZ0lRUzBZWUdZVGVBcW9ydEVsenFZNHA5ekFOQmdrcWhraUc5dzBCQVFzRkFEQVkKTVJZd0ZBWURWUVFLRXcxamJIVnpkR1Z5TG14dlkyRnNNQjRYRFRJd01EY3dPREEzTkRJek5sb1hEVE13TURjdwpOakEzTkRJek5sb3dHREVXTUJRR0ExVUVDaE1OWTJ4MWMzUmxjaTVzYjJOaGJEQ0NBU0l3RFFZSktvWklodmNOCkFRRUJCUUFEZ2dFUEFEQ0NBUW9DZ2dFQkFNZDF1Q3l0YzNJM01iMmxBWTloVUw2b2UwNTZPMkZ4c1FwSVV5YU8KaCs4dkFhSnZZT3dxUkNKbjVDZHBLQ0Faem9LeVhKbkxvdVVlaWV3VUV4N2czVVN3djlZblFkT2xWMWUvR1VhMQpLaHcwRHhCRWIxdlExQ3BUTGdkRHdsMTBmcVh0cE1aTnZtY09hdmJwamxodlAxSUNHMktjQVlJRXkybEJtMUVSCjdjdjFHbGgraE1jdHFzeDV4c1lvcTNXaWUxYzQ3TWxCVkVlaDM5VUcwdEVtQXpMSEVuTDF5bzFhRHVIVHRuN2QKendHN21Na1hBN2c1anRVWmFCeHlxSWpORUV2dFRHb3UxbEYvZ0E2eHhjVkFyRlA0VWpKSWZpTmFlVnE4di9nMgpEZytSMG5wSjlwek9DV1dZQ3lXaU50SVZNSmNvbEs4N0VsWHVIYWFlQzc2VWJNa0NBd0VBQWFNak1DRXdEZ1lEClZSMFBBUUgvQkFRREFnSUVNQThHQTFVZEV3RUIvd1FGTUFNQkFmOHdEUVlKS29aSWh2Y05BUUVMQlFBRGdnRUIKQUdLbG5QTUtZbWUwV0lDM0pGeUFBY0k4b0FtRlFMNzVaQWsrbDk2WWw4Vk1ZTUI3VDlteFFSaTZ2czVZbElZeQp1ZDFqNTNEYzB5NVBjTU9UdzZXYzF0QkdqL1U0alArSm5DT1U3SXRwVTRPWmlZd3o2OEJvZGFJalVYWVJWVGpaClBVWk5ZZDA0aW5IMDJCQ1k5em9TaHdLaW1jSnE0bUlGWW9NRTN6WXRYdEY5Ry8zNTRiNG9tU1A1bzEzRXRUVk8KQUZTc0FxRWdiNGhoQURwUWt0TU01RE1iVHp6OWhQakNlREswTXRWQUt0dVNiTE5kVG1uazd1eHB5N3ZuQThLMgpTcEVNZlA4YmtDdGp6Q3NhRzFKOE80ZEJ5czByeXk3RWhrWHk2L1FVenVUVXlTYlp2WkhFZWlKelhmZVhlbFlXClNrRStjYnZ5WHdWQmVqdm1ObndXNERBPQotLS0tLUVORCBDRVJUSUZJQ0FURS0tLS0tCg==
    service:
      name: istiod
      namespace: istio-system
      path: /inject
      port: 443
  failurePolicy: Fail
  matchPolicy: Exact
  name: sidecar-injector.istio.io
  namespaceSelector:
    matchLabels:
      istio-injection: enabled
  objectSelector: {}
  reinvocationPolicy: Never
  rules:
  - apiGroups:
    - ""
    apiVersions:
    - v1
    operations:
    - CREATE
    resources:
    - pods
    scope: '*'
  sideEffects: None
  timeoutSeconds: 30
```

获取配置

```shell
# kubectl get cm istio-sidecar-injector -o yaml -n istio-system
      {{- if gt .ProxyConfig.Concurrency 0 }}
        - --concurrency
        - "{{ .ProxyConfig.Concurrency }}"
      {{- end -}}
```

这里执行失败了

[ProxyConfig](https://istio.io/latest/docs/reference/config/istio.mesh.v1alpha1/#ProxyConfig)

>  concurrency - The number of worker threads to run. Default value is number of cores on the machine. 

目前删除这几句可以回复正常，问题在于为何拿不到配置?

```shell
# kubectl get cm istio -o yaml -n istio-system
apiVersion: v1
data:
  mesh: |-
    accessLogEncoding: TEXT
    accessLogFile: /dev/stdout
    accessLogFormat: ""
    defaultConfig:
      concurrency: 2
```

[常见 inject 问题](https://istio.io/latest/docs/ops/common-problems/injection/)

### 3.  x509: certificate has expired or is not yet valid

```sh
# 获取部署 
kubectl get deployment
#然后 describe deployment
#获取replicaset，这里有具体的错误信息
kubectl get replicaset
kubectl describe replicaset xxx
```

```yaml
Name:           eduiot-platform-v1-956d589bc
Namespace:      tj-edu-iot
Selector:       app=eduiot-platform,pod-template-hash=956d589bc,version=v1
Labels:         app=eduiot-platform
                pod-template-hash=956d589bc
                version=v1
Annotations:    deployment.kubernetes.io/desired-replicas: 1
                deployment.kubernetes.io/max-replicas: 2
                deployment.kubernetes.io/revision: 1
                kubernetes.io/change-cause:
                  kubectl apply --filename=./eduiot-platform_kube_1597047540.yaml --namespace=tj-edu-iot --kubeconfig=/root/.kube/config --record=true
Controlled By:  Deployment/eduiot-platform-v1
Replicas:       0 current / 1 desired
Pods Status:    0 Running / 0 Waiting / 0 Succeeded / 0 Failed
Pod Template:
  Labels:  app=eduiot-platform
           pod-template-hash=956d589bc
           version=v1
  Containers:
   eduiot-platform:
    Image:        ccr.ccs.tencentyun.com/release-prod/eduiot-platform:P009M000C001B018SP002
    Ports:        8090/TCP, 8080/TCP
    Host Ports:   0/TCP, 0/TCP
    Environment:  <none>
    Mounts:
      /data/log/eduiot-platform from eduiot-platform-log-dir (rw)
      /data/service/eduiot-platform/conf/app.yaml from eduiot-platform-conf-vol (rw)
  Volumes:
   eduiot-platform-log-dir:
    Type:    EmptyDir (a temporary directory that shares a pod's lifetime)
    Medium:
   eduiot-platform-conf-vol:
    Type:      ConfigMap (a volume populated by a ConfigMap)
    Name:      eduiot-platform-v1
    Optional:  false
Conditions:
  Type             Status  Reason
  ----             ------  ------
  ReplicaFailure   True    FailedCreate
Events:
  Type     Reason        Age                 From                   Message
  ----     ------        ----                ----                   -------
  Warning  FailedCreate  15m (x20 over 54m)  replicaset-controller  Error creating: Internal error occurred: failed calling admission webhook "sidecar-injector.istio.io": Post https://istio-sidecar-injector.istio-system.svc:443/inject?timeout=30s: x509: certificate has expired or is not yet valid
```

重点看Event事件

>`x509: certificate signed by unknown authority` errors are typically caused by an empty `caBundle` in the webhook configuration.

> Verify the `caBundle` in the `mutatingwebhookconfiguration` matches the root certificate mounted in the `istiod` pod.



```sh
kubectl get mutatingwebhookconfiguration istio-sidecar-injector -o yaml -o jsonpath='{.webhooks[0].clientConfig.caBundle}' | md5sum
kubectl -n istio-system get secret istio-galley-service-account-token-hwxw4 -o jsonpath='{.data.ca\.crt}' | md5sum
```



```sh
# kubectl describe  deployment istio-sidecar-injector -n istio-system

Pod Template:
  Labels:           app=sidecarInjectorWebhook
                    chart=sidecarInjectorWebhook
                    heritage=Tiller
                    istio=sidecar-injector
                    release=istio
  Annotations:      sidecar.istio.io/inject: false
  Service Account:  istio-sidecar-injector-service-account
  Containers:
   sidecar-injector-webhook:
    Image:      docker.io/istio/sidecar_injector:1.1.3
    Port:       <none>
    Host Port:  <none>
    Args: # istio/pilot/cmd/sidecar-injector/main.go 中定义
      --caCertFile=/etc/istio/certs/root-cert.pem
      --tlsCertFile=/etc/istio/certs/cert-chain.pem
      --tlsKeyFile=/etc/istio/certs/key.pem
      --injectConfig=/etc/istio/inject/config
      --meshConfig=/etc/istio/config/mesh
      --healthCheckInterval=2s
      --healthCheckFile=/health
    Requests:
      cpu:        10m
    Liveness:     exec [/usr/local/bin/sidecar-injector probe --probe-path=/health --interval=4s] delay=4s timeout=1s period=4s #success=1 #failure=3
    Readiness:    exec [/usr/local/bin/sidecar-injector probe --probe-path=/health --interval=4s] delay=4s timeout=1s period=4s #success=1 #failure=3
    Environment:  <none>
    Mounts:
      /etc/istio/certs from certs (ro)
      /etc/istio/config from config-volume (ro)
      /etc/istio/inject from inject-config (ro)
  Volumes:
   config-volume:
    Type:      ConfigMap (a volume populated by a ConfigMap)
    Name:      istio
    Optional:  false
   certs:
    Type:        Secret (a volume populated by a Secret)
    SecretName:  istio.istio-sidecar-injector-service-account
    Optional:    false
   inject-config:
    Type:      ConfigMap (a volume populated by a ConfigMap)
    Name:      istio-sidecar-injector
    Optional:  false
```

它证书是挂载的 istio.istio-sidecar-injector-service-account

```sh
# kubectl describe  deployment istio-galley  -n istio-system
Pod Template:
  Labels:           app=galley
                    chart=galley
                    heritage=Tiller
                    istio=galley
                    release=istio
  Annotations:      sidecar.istio.io/inject: false
  Service Account:  istio-galley-service-account
  Containers:
   galley:
    Image:       docker.io/istio/galley:1.1.3
    Ports:       443/TCP, 15014/TCP, 9901/TCP
    Host Ports:  0/TCP, 0/TCP, 0/TCP
    Command:
      /usr/local/bin/galley
      server
      --meshConfigFile=/etc/mesh-config/mesh
      --livenessProbeInterval=1s
      --livenessProbePath=/healthliveness
      --readinessProbePath=/healthready
      --readinessProbeInterval=1s
      --insecure=true
      --validation-webhook-config-file
      /etc/config/validatingwebhookconfiguration.yaml
      --monitoringPort=15014
    Requests:
      cpu:        10m
    Liveness:     exec [/usr/local/bin/galley probe --probe-path=/healthliveness --interval=10s] delay=5s timeout=1s period=5s #success=1 #failure=3
    Readiness:    exec [/usr/local/bin/galley probe --probe-path=/healthready --interval=10s] delay=5s timeout=1s period=5s #success=1 #failure=3
    Environment:  <none>
    Mounts:
      /etc/certs from certs (ro)
      /etc/config from config (ro)
      /etc/mesh-config from mesh-config (ro)
  Volumes:
   certs:
    Type:        Secret (a volume populated by a Secret)
    SecretName:  istio.istio-galley-service-account
    Optional:    false
   config:
    Type:      ConfigMap (a volume populated by a ConfigMap)
    Name:      istio-galley-configuration
    Optional:  false
   mesh-config:
    Type:      ConfigMap (a volume populated by a ConfigMap)
    Name:      istio
    Optional:  false
```

它证书挂在 istio.istio-galley-service-account

```sh
#查看他们的证书
kubectl get Secret istio.istio-galley-service-account -n istio-system -o jsonpath='{.data.root-cert\.pem}'
kubectl get Secret istio.istio-sidecar-injector-service-account -n istio-system -o jsonpath='{.data.root-cert\.pem}'
```

找了一圈，结果是istio自己的bug

```sh
kubectl delete Secret istio-ca-secret -n istio-system
```

官网原因在这里[Extending Self-Signed Certificate Lifetime](https://istio.io/latest/docs/ops/configuration/security/root-transition/)

阿里的一位哥们遇到了同样的事情 https://developer.aliyun.com/article/728018

>在Istio比较早期的版本中，自签名Ca证书有效期只有一年时间，如果使用老版本Istio超过一年，就会遇到这个问题。当证书过期之后，我们创建新的虚拟服务或者pod，都会因为CA证书过期而失败。而这时如果Citadel重启，它会读取过期证书并验证其有效性，就会出现以上Cidatel不能启动的问题。

> 这个Ca证书在K8s集群中，是以istio-ca-secret命名的secret，我们可以使用openssl解码证书来查看有效期。这个问题比较简单的处理方法，就是删除这个Secret，并重启Citadel，这时Citadel会走向新建和验证自签名Ca证书的逻辑并刷新Ca证书。或者参考以下官网处理方式。