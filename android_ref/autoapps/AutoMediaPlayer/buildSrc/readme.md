### 编译管理模块

- BuildConfig
```
  管理所有外部引入的库文件版本；
  工程相关模块、插件管理...
```

- ApiDocumentPlugin
```
  它是一个简单的 API文档导出插件，把 media-api 的文档导出到 rootProject/docs 目录下；
  使用方法（Terminal 窗口）：
    PS $Path/AutoMediaPlayer> ./gradlew documentTask
```