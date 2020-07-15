# 常见问题
## Pod长期处于Pending
```sh
kubectrl describe pod/node xxx

```
查看pod信息，查看选择器是否正确
nodeSelector/nodeName是否正确

