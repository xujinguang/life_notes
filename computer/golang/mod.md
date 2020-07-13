mod中旧的import路径可以使用replace替代
、go mod常用命令
go build, go test：下载并添加依赖到go.mod中。
go mod tidy：整理，增加缺失的包，移除没用的包。
go mod graph：显示模块间的依赖图。
go mod why：显示为什么需要包。
go list -m all：查看所有的依赖。
go get：下载依赖并更新到go.mod中。
4、go get命令
@v0.3.2：指定tag，Release时建议此方法。
@master：master分支最新commit。
@342b2e：指定commit。go.mod中的表现格式为：v0.0.0-20190529093157-265ded9b6495。
@latest、不指定：默认行为，最新版本；有tag则最新tag，无tag则master分支最新commit。
version前使用>，>=，<，<=：大于/小于指定版本。
v0.0.1-pre：开发时版本较多时使用，不会被 @latest、不指定 自动拉取到。
