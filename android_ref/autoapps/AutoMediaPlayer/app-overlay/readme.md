#### RRO(Runtime Resource Overlay)语音包

- <font color="#0faff0">语言包的资源组成</font><br/>
```gradle
srcDirs += "../app-language/src/main/res-common"
srcDirs += "../app-language/src/main/res-pool"
srcDirs += "../app-language/src/skinx/res-ext"
srcDirs += "../app-language/src/skinx/res-ext-pool"
```

- <font color="#0faff0">语言包实现原理</font><br/>
> 保留文档（后续补充）

- <font color="#0faff0">当前模块注意事项</font><br/>
> <font color="#ef00f0">注意：不要添加任何源代码，以及 drawable、layout 相关资源；<br/>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
> 我们约定 overlay 模块只能添加 values/strings.xml 资源；</font><br/>