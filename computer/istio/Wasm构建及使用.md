## Wasm构建及使用2

这篇文章是上一篇文章的补充。上一篇文章阐述了wasm的构建和使用是通过wasme工具搞的，而没有使用istio原生态方式。原因就是觉得wasme工具的思想和设计比较合理的。它借鉴了Docker的构建分发思想，以及插件在节点的缓存处理，这对用户都是比较友好的。但是这个东西冷却下来了，维护人员已经很久没有更新了，而且有些问题尚未修复，看来还需要时间慢慢成长。避免依赖，本文就是要补全手动安装wasm这一块。自己动手丰衣足食。

其实也不复杂。只需要将wasme封装成自动化的东西换成自己手撸即可。内容包括：

1. 使用sdk构建wasm
2. 编写EnvoyFilter配置文件
3. 手动分发wasm文件
4. 注册插件
5. 重启pod

### 1. SDK构建wasm

这个就不赘述了，参考这两篇文章。

1. [emsdk安装与使用](https://emscripten.org/docs/getting_started/downloads.html)
2. [emsdk Docker 镜像及其使用](https://hub.docker.com/r/emscripten/emsdk)

根据自己喜好选择使用安装还是Docker镜像。个人推荐使用docker镜像。构建命令上面链接文档里面都有说明。

同时还有第三种方式，在线IDE

1. 在线IDE [webassembly studio](https://webassembly.studio/)
2. 在线体验 [WebAssembly Explorer](https://mbebenita.github.io/WasmExplorer/)

由于构建Envoy的插件而不是web的插件。因此，第三种方式不方便添加依赖库。因为不论选择哪种方式吧，成功构建xxx.wasm即可。

### 2. 编写EnvoyFilter配置文件

避免直接从理论到实践的难理解性，先从实践开始，然后到理论，最后再回到实践中去。这样路线最容易理解。所以上先上代码。

```yaml
apiVersion: networking.istio.io/v1alpha3
kind: EnvoyFilter #istio的CRD API资源类型
metadata:
  name: filter-example #filter资源名称
  namespace: istio-system #所属的ns
spec: #描述状态
  configPatches:
  - applyTo: HTTP_FILTER 
  #Envoy支持的地方包括：LISTENER、FILTER_CHAIN、NETWORK_FILTER、HTTP_FILTER、
  # ROUTE_CONFIGURATION、VIRTUAL_HOST、HTTP_ROUTE、CLUSTER
    match: # 匹配条件，包括：context，proxy，listener，routeConfiguration，cluster
      context: GATEWAY 
      #支持网关上下文GATEWAY，边车进入流量SIDECAR_INBOUND，边车出站流量SIDECAR_OUTBOUND以及ANY
      listener:
        filterChain: 
          filter: #名称匹配 envoy的 http_connection_manager 下的 router
            name: envoy.http_connection_manager
            subFilter:
              name: envoy.router
    patch: # Patch specifies how the selected object should be modified
      operation: INSERT_BEFORE #如何将 patch 应用于所选配置（上面configPatch)
      #MERGE, ADD,REMOVE,INSERT_BEFORE,INSERT_AFTER,INSERT_FIRST
      value: #这个对象就是 Envoy插件的配置内容
        typed_config:
          '@type': type.googleapis.com/udpa.type.v1.TypedStruct
          type_url: type.googleapis.com/envoy.extensions.filters.http.wasm.v3.Wasm
          value:
            config:
              name: myfilter
              root_id: myfilter
              vm_config:
                code:
                  local:
                    filename: /data/wasm-filter/_wasm_test.wasm
                runtime: envoy.wasm.runtime.v8
                vm_id: myfilter
              configuration:
                '@type': type.googleapis.com/google.protobuf.StringValue
                value: |
                    {
                        "key": value
                    }
        name: envoy.filters.http.wasm
  workloadSelector: #指定此资源应用的工作负载
    labels: #通过标签选择负载
      app: istio-ingressgateway
```

说明几点：

1. EnvoyFilter 是istio提供的CRD，用于自定义 Istio Pilot 生成的Envoy的配置
2. 用于修改特定字段的值，添加特定的过滤器，甚至添加全新的监听器、集群等
3. 小心使用，不正确的配置可能会破坏整个网格的稳定
4. 当多个 EnvoyFilters 绑定到给定名称空间中的相同工作负载时，所有patch将按创建时间顺序处理。如果多个 EnvoyFilter 配置之间发生冲突，行为未定义。
5. 如果对系统中所有工作负载应用 EnvoyFilter， 则不要指定 workloadSelector

[Envoy Filters](https://www.envoyproxy.io/docs/envoy/latest/api-v2/config/filter/filter)

其中patch.value字段是envoy的配置内容了。它的定义参考[这里](https://www.envoyproxy.io/docs/envoy/latest/api-v2/config/filter/network/http_connection_manager/v2/http_connection_manager.proto#config-filter-network-http-connection-manager-v2-httpfilter)，它包括三个成员name，config和typed_config，其中后面两个成员都是一个对象结构，typed_config 的具体内容要看具体的过滤器。config和typed_config只能设置其中一个。

#### 2.1 CRD资源定义

```shell
# kubectl get crd envoyfilters.networking.istio.io -o yaml
```

这个文件比较大，不粘贴这里了。它是EnvoyFilter CRD的定义文件。yaml层级结构看着不是很舒服，换成json会好一些。从中可以发现，patch只定义到了value之后就没有再定义了其结构了

```json
"patch":{
  "description":"The patch to apply along with the operation.",
  "properties":{
    "operation":{
      "description":"Determines how the patch should be applied.",
      "enum":[
        "INVALID",
        "MERGE",
        "ADD",
        "REMOVE",
        "INSERT_BEFORE",
        "INSERT_AFTER",
        "INSERT_FIRST"
      ],
      "type":"string"
    },
    "value":{
      "description":"The JSON config of the object being patched.", //here
      "type":"object"
    }
  },
  "type":"object"
}
```

那么value结构难道自由发挥不成？当然不是，上面已经提到它要根据具体的过滤器而定。这个借口定义在envoy的相关proto文件里。已wasm为例，就是在api/envoy/extensions/wasm/v3/wasm.proto中



```sh
kubectl get envoyfilter auth-wasm -n istio-system -o yaml
istioctl pc listener istio-ingressgateway-689dd678b4-bmsxp  --address 0.0.0.0 --port 15001 -o json
kubectl scale deployment/istio-ingressgateway --replicas=0 -n istio-system
```



## 探索

1.7 网关端口变更 https://istio.io/latest/news/releases/1.7.x/announcing-1.7/upgrade-notes/

```shell
2020-08-26T10:41:49.596907Z     debug   envoy wasm      WasmVm created envoy.wasm.runtime.null now active
2020-08-26T10:41:49.596918Z     debug   envoy wasm      Thread-Local Wasm created 12 now active
2020-08-26T10:41:49.640016Z     debug   envoy wasm      WasmVm created envoy.wasm.runtime.v8 now active
2020-08-26T10:41:49.640434Z     debug   envoy wasm      Base Wasm created 13 now active
2020-08-26T10:41:49.821640Z     trace   envoy wasm      Failed to load Wasm module due to a missing import: env.proxy_continue_stream
2020-08-26T10:41:49.821726Z     error   envoy wasm      Wasm VM failed Wasm module is missing malloc function.
2020-08-26T10:41:49.821739Z     error   envoy wasm      Wasm VM failed Failed to initialize Wasm code
2020-08-26T10:41:49.821745Z     debug   envoy wasm      ~Wasm 12 remaining active
2020-08-26T10:41:49.827734Z     debug   envoy wasm      ~WasmVm envoy.wasm.runtime.v8 12 remaining active
2020-08-26T10:41:49.827865Z     trace   envoy wasm      Unable to create Wasm
2020-08-26T10:41:49.877958Z     debug   envoy init      init manager RDS local-init-manager http.80 destroyed
2020-08-26T10:41:49.877987Z     debug   envoy init      target RdsRouteConfigSubscription local-init-target http.80 destroyed
2020-08-26T10:41:49.877991Z     debug   envoy init      RDS local-init-watcher http.80 destroyed
```

