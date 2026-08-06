### <font color="#eeccaa">多语言模块说明</font><br/>

- <font color="#00cccc">目录结构</font><br/>
```
src/main/java/
    请不要在语言模块中放任何 java 文件；
    
src/main/res-common/
    这里存放海外多国语言配置（除中文外的语种）；
    
src/main/res-pool/
    这里存放特定主题的海外多国语言配置（除中英文外的语种）；
    
src/media/res-app/
    请不要在这里放除 values-zh-rCN/values-zh-rTW 之外的其它语种配置；
    
src/skinx/res-ext/
    这里存放扩展皮肤包的特定语言（只存放中文），其它语言还是放到 src/main/res-common 中；
    
src/skinx/res-pool/
    这里存放扩展皮肤包的特定主题定制语言（无限制），其它语言还是放到 src/main/res-common 中；
```

- <font color="#00cccc">组合规则</font><br/>
```
模块 media-lang-release.aar 组成：
    src/main/res/
    src/main/res-common/
    src/main/res-pool/
    
对外提供全局资源（以目录结构）的组合规则：
    src/main/res-common/
    src/main/res-pool/
    src/skinx/res-ext/
    src/skinx/res-pool/
    src/skinx/$(_other_)/
```

- <font color="#ee3333">警告约束</font><br/>
- [x] <font color="#ee7777">禁止将 media 与 skinx 组合打包或者对外提供资源组合；</font><br/>
