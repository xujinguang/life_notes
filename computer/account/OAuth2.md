## OAuth

```mermaid
sequenceDiagram
    zw ->> +zs: 拉取回放视频列表
    zs -->> -zw: 视频列表
    zw->> +zs: 点击"去剪辑"
    zs->>+ks: name, vodid云API创建project
    ks-->>ks: cme创建项目
    ks-->>-zs: projectid
    zs-->>-zw: projectid 
    zw->>  +kw: 跳转剪辑视频[projectid] 
    kw ->> +ks: 发起签名请求
    ks-->> -kw: token失败或者鉴权失败
    kw ->> -zw: 设定callback, 向招生通发起授权
    zw-->> +zs: 授权
    zs-->> -ks: 携带code， callback 
    ks ->> +zs: 获取accesstoken
    zs -->> -ks: accesstoken
    ks ->> +zs: 获取用户资料
    zs -->> -ks: 用户基础资料
    ks ->> +ks:- 签发token
    ks -->> - kw: 重定向快剪页面 + token
    kw ->> +ks: token+请求签名
    ks-->> -kw: 签名
    kw ->> +cme: webapi打开项目
    cme-->> -kw: success
    kw->>kw: 剪辑视频
```



<<<<<<< HEAD

=======
```
 kw->> +ks:token, projectid, vodid创建
    ks->>cme:创建项目
    cme-->>ks: projectid
    ks-->>kw: 创建成功
    kw->>kw: 剪辑视频
    ks-->>-zs: 回调告知zs, porjectid创建成功或失败
    zs->>zs: 修改项目状态，或者删除记录
```
>>>>>>> f07a145de0894bf4f24b41c5910edf7ba23e02e3

