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

