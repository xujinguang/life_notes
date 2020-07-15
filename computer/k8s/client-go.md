1.12版本这里用到time的Milliseconds，这是1.13的版本库
升级到go1.13,又报错这个runtime.Scheme的s.DefaultConvert是1.12的能力，1.13版本时候这个能力给删除了。
降低client-go版本，它又跟自己的api版本不兼容。。。

按照官方指引，还是要使用0.x.y版本，然后切换到go.1.14版本
