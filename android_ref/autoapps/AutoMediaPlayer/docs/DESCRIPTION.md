### <font color="#eeccaa">媒体工程构成说明</font><br/>

#### <font color="#00cccc">[ module : <font color="#66cc66">app</font> ]</font><br/>
```
主模块
  所有 UI 的业务逻辑都在此实现
  播放器的后台服务组件代码也在这里
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">app-language</font> ]</font><br/>
```
多语言模块
  主 app 和所有皮肤包都加载它实现语言共享
  主 app 不需要添加语言资源（应该是禁止添加）；
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">app-overlay</font> ]</font><br/>
```
运行时语言覆盖模块
  可以直接安装的语言 apk  
  通过 Android RRO(Runtime Resource Overlay) OverlayManagerService 实现语音覆盖；
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">api</font> ]</font><br/>
```
对外接口模块
  对外提供访问播放的接口模块
  为跨进程的客户端提供多媒体 SDK 操作代理；
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">auto-compat</font> ]</font><br/>
```
版本兼容模块
  提供不同 Android 版本和平台项目差异处理
  系统接口兼容、蓝牙接口兼容等
  设备平台工具等
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">base</font> ]</font><br/>
```
媒体基础模块
  媒体相关常量定义
  多媒体内部业务交互的事件定义
  页面类型定义、各类监听接口
  内部外部广播定义
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">common</font> ]</font><br/>
```
媒体工具模块
  所有因为多媒体项目衍生出来的工具封装
  主要是为多媒体编码提供工具支持（后续可以抽离）
  缩略图缓存、调试打印控制、ID3 解析、通讯等
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">data</font> ]</font><br/>
```
数据模块
  多媒体所有关键的数据结构在此定义
  运行中的全局数据状态也存储在此
  数据库与媒体元素表在此定义
    ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">model</font> ]</font><br/>
```
模式模块
  整个业务逻辑接口都在此处定义
  它为 UI 组件以及 Service 组件提供交互接口
  这里有核心播放组件、播放业务逻辑处理
  所有的数据库的业务也在此处理
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">skinx</font> ]</font><br/>
```
换肤支持模块
  外部扩展主题包加载业务逻辑
  为换肤提供统一的接口支持；
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">theme</font> ]</font><br/>
```
主题定义模块
  所有皮肤代号定义与说明
  外部参数暂时存放于此
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">view</font> ]</font><br/>
```
自定义视图模块
  所有因为当前项目衍生出来的自定义视图封装
  给提供给所有 UI 开发者使用
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">metadata-native</font> ]</font><br/>
```
音视频元数据解析模块
  为音视频提供元数据解码支持
  是一个基于 ffmpeg 的开源库接口包
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">ijk</font> ]</font><br/>
```
软解码模块
  为音视频提供软解码支持
  是一个基于 ffmpeg 的开源库接口包
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">vitamio</font> ]</font><br/>
```
软解码模块
  为音视频提供软解码支持
  是一个基于 ffmpeg 的闭源库接口包
  ...
```

#### <font color="#00cccc">[ module : <font color="#66cc66">transformer</font> ]</font><br/>
```
软解码模块
  为音视频提供软解码支持
  是一个基于 ffmpeg 的闭源库接口包
  ...
```


