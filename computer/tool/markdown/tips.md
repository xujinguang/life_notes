# 写ppt
使用插件
[Marp for VS Code](https://zhuanlan.zhihu.com/p/149521766)

## 修改全局属性
```
<style>
    :root {
        --color-background: #101010 !important;
	      --color-foreground: #FFFFFF !important;
    }
</style>
```

## 页面背景
```
<!-- _backgroundColor:  -->
```
去掉下划线就是从当前页到末尾

## 字体颜色
```
<!-- color:  -->
```

https://marpit.marp.app/theme-css?id=tweak-style-through-markdown

## 只设置当前页面
```
<style scoped>
  section li {
    font-size:20px;
  }
</style>
```
https://www.jianshu.com/p/7702cddafca0