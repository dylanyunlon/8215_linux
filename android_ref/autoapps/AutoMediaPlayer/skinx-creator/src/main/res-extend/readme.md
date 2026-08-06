### <font color="#eeccaa">资源目录 res-extend 说明</font><br/>

#### <font color="#00cccc">[ values : <font color="#77ff77">config.xml</font> ]</font><br/>
- <font color="#dd66dd">用来配置指定组件是否存在对应的扩展 java 类</font><br/>
```
这里我们对扩展类命名规则做强制约定，格式如下：

Activity 命名
    MusicUI.java 的扩展类为 MusicUiExtend.java

Fragment 命名
    VideoInfoFragment.java 的扩展类为 VideoInfoPageExtend.java
    
Service 命名
    MediaService.java 的扩展类为 MediaServiceExtend.java
    
注意: 所有组件的扩展类类名按规则约定且不可以修改
```