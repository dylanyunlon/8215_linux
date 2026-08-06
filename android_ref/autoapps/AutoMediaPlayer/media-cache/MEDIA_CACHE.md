### <font color="#eeccaa">媒体缓存模块说明</font><br/>
- <font color="#00cccc">主要是做媒体缓存管理功能，支持边下边播，离线播放和缓存管理等。</font><br/>
```
    MediaCache 通过代理的策略将我们的网络请求代理到本地服务，本地服务决定数据是从缓存获取还是发起网络请求，如果需要发起网
络请求就先向本地写入数据，再从本地服务获取数据给VideoView，从而做到数据的复用。
```

- <font color="#cc66cc">这个库最开始是 danikula 大神写的，利用 socket 开启一个本机的代理服务器，实现流媒体边下边播。</font><br/>
```
    由于 AndroidVideoCache 项目已经不再维护，且 github 上面挂了 2k+ 的 issues，我们只能把源码移植过来，根据实际需求改写并自己维护；
```
### <font color="#eeccaa">为什么要创建 media-cache 模块?</font><br/>
因为在流媒体播放时大量下载媒体文件是没有意义的！</br>
这里 `media-cache` 允许将缓存支持添加到 `VideoView/MediaPlayer` 组件。

### <font color="#eeccaa">模块特征</font><br/>
- 在加载流时缓存至本地中;
- 缓存资源离线工作;
- 部分加载;
- 自定义缓存限制;
- 同一个 url 多客户端支持;

该项目仅支持直接 url 媒体文件，并不支持如 DASH，SmoothStreaming，HLS 等流媒体;

### <font color="#eeccaa">如何上手</font><br/>
只需要添加依赖项：
```
dependencies {
    compile 'com.hcn:media_cache:2.1.1'
}
```

并使用来自代理的 url 而不是原始 url 来添加缓存：
```java
@Override
protected void onCreate(Bundle savedInstanceState) {
super.onCreate(savedInstanceState);

    HttpProxyCacheServer proxy = getProxy();
    String proxyUrl = proxy.getProxyUrl(VIDEO_URL);
    videoView.setVideoPath(proxyUrl);
}

private HttpProxyCacheServer getProxy() {
    // should return single instance of HttpProxyCacheServer shared for whole app.
}
```

为了保证正常工作，您应该为整个应用程序使用 `HttpProxyCacheServer` 的单个实例。<br/>
例如，您可以将共享代理存储在 `Application` 中：
```java
public class App extends Application {

    private HttpProxyCacheServer proxy;

    public static HttpProxyCacheServer getProxy(Context context) {
        App app = (App) context.getApplicationContext();
        return app.proxy == null ? (app.proxy = app.newProxy()) : app.proxy;
    }

    private HttpProxyCacheServer newProxy() {
        return new HttpProxyCacheServer(this);
    }
}
```

### <font color="#eeccaa">磁盘缓存限制说明</font><br/>
默认情况下 `HttpProxyCacheServer` 使用 512Mb 缓存文件，您可以更改此值：
```java
private HttpProxyCacheServer newProxy() {
    return new HttpProxyCacheServer.Builder(this)
            .maxCacheSize(1024 * 1024 * 1024)       // 1 Gb for cache
            .build();
}
```    

或者甚至实现您自己的 `DiskUsage` 策略：
```java
private HttpProxyCacheServer newProxy() {
    return new HttpProxyCacheServer.Builder(this)
            .maxCacheFilesCount(20)
            .build();
}
```

or even implement your own `DiskUsage` strategy:
```java
private HttpProxyCacheServer newProxy() {
    return new HttpProxyCacheServer.Builder(this)
            .diskUsage(new MyCoolDiskUsageStrategy())
            .build();
}
```

### <font color="#eeccaa">侦听缓存进度</font><br/>
- 使用 `HttpProxyCacheServer.registerCacheListener(CacheListener listener)` 方法设置具有回调: 
  <br>`onCacheAvailable(File cacheFile，String url，int percentsAvailable)` 的侦听器，以了解当前媒体源的缓存进度。
- 注意不要忘记借助 `HttpProxyCacheServer.unregisterCacheListener(CacheListener listener)`方法取消订阅 listener，以避免内存泄漏。
- 使用 `HttpProxyCacheServer.isCached(String url)` 方法检查 url 的内容是否已完全缓存到文件中。

### <font color="#eeccaa">提供缓存文件的名称</font><br/>
默认情况下 `media-cache` 使用媒体源 url 的 MD5 作为文件名。但在某些情况下，url 并不稳定，它可能包含一些生成的部分（例如会话令牌）。<br/>
在这种情况下，缓存机制将被破坏。要修复它，您必须提供自己的 `FileNameGenerator`;
``` java
public class MyFileNameGenerator implements FileNameGenerator {

    // Urls contain mutable parts (parameter 'sessionToken') and stable video's id (parameter 'videoId').
    // e. g. http://example.com?videoId=abcqaz&sessionToken=xyz987
    public String generate(String url) {
        Uri uri = Uri.parse(url);
        String videoId = uri.getQueryParameter("videoId");
        return videoId + ".mp4";
    }
}

...
HttpProxyCacheServer proxy = new HttpProxyCacheServer.Builder(context)
    .fileNameGenerator(new MyFileNameGenerator())
    .build()
```

### <font color="#eeccaa">添加自定义 http 标头</font><br/>
您可以在 "HeadersInjector" 的帮助下向请求添加自定义标头：
``` java
public class UserAgentHeadersInjector implements HeaderInjector {

    @Override
    public Map<String, String> addHeaders(String url) {
        return Maps.newHashMap("User-Agent", "Cool app v1.1");
    }
}

private HttpProxyCacheServer newProxy() {
    return new HttpProxyCacheServer.Builder(this)
            .headerInjector(new UserAgentHeadersInjector())
            .build();
}
```