## 私有化部署价值
1. 和工作环境融为一体，提高工作效率
3. 完全免费，节约一丢丢成本
4. 虽然不是满血模型，同样可以感受deepseek的优势

## Mac 部署
1. 使用傻瓜集成平台ollama，ollama平台依赖于docker，Mac的M3/M4 因doesn’t have VT-X/AMD-v enabled 所以VB,VM无法使用，导致docker无法启动，需要使用docker的替代品podman， 它完全兼容docker
```shell
brew install podman
```

2. 安装ollama
```shell
brew install --cask ollama 
```
3. 拉取deepseek
```shell
# r1聊天模型
ollama pull deepseek-r1:1.5b
# 或者
ollama pull deepseek-r1:8b
# 编码版
ollama pull deepseek-coder
```
更多版本参见
https://ollama.com/library/deepseek-r1/tags
4. 终端运行
```shell
ollama run deepseek-r1:1.5b
```
5. 集成到vscode

可以快乐的玩耍了！

## Linux部署
应该也比较简单，没有验证，命令应该是这个
```shell
curl -fsSL https://ollama.com/install.sh | sh ollama run deepseek-r1:7b
```