#### [<font color="#eeccaa">Update Time:</font> 2023-08-17 09:19:11]

### <font color="#eeccaa">媒体接口说明</font><br/>

#### <font color="#00cccc">导入 api</font><br/>
```
// build.gradle
implementation files('libs/media-api-release.aar')
```

#### <font color="#00cccc">清单权限</font><br/>
```
// 添加服务访问权限
<uses-permission android:name="media.permission.ACCESS_RECEPTION_SERVICE" />
```

#### <font color="#00cccc">接口使用</font><br/>
```
// 首先构建 api 对象
HMediaApi mediaApi = HMediaApi.createMediaApi(getApplicationContext(), "launcher");

// 再初始化 api 配置
mediaApi.init()
        .setDebug(-1, true)
        .setDebug(Log.VERBOSE, true)
        .registerListener(new IConnectionState() {
            @Override
            public void onConnected() {
                // 表示连接上媒体服务了
                // 建议维护下连接状态，非连接状态不要调用操作接口；
                // 非连接状态点击直接调用 mediaApi.requestStartApp();
            }
            
            @Override
            public void onDisconnected() {
                // 媒体异常退出了
                // 不用处理，下次启动会自动连接
            }
            
            @Override
            public void onDied() {
                // 媒体进程包在线发生了升级
                // 暂时不用处理（保留）
            }
        });

// 启动媒体 app（会触发自动播放）
mediaApi.requestStartApp();

// 启动媒体 app（只建立连接，不自动播放）
mediaApi.requestBindApp();

// 退出媒体 app (调用者决定)
mediaApi.requestExitApp();

// 控制相关接口
Interface Description {
    // 播放暂停音乐
    mediaApi.musicPlayPause();
    
    // 播放音乐
    mediaApi.musicPlay();
    
    // 暂停音乐
    mediaApi.musicPause();
    
    // 切换音乐下一曲
    mediaApi.musicPlayNext();
    
    // 切换音乐上一曲
    mediaApi.musicPlayPrev();
    
    // 切换音乐播放模式
    mediaApi.musicSwitchPlayMode();
}

// 反注册监听接口 (与 registerListener 成对出现)
mediaApi.unregisterListener(object);

// 反初始化函数（与 init 成对出现）
// 如果调用 uninit，就无需调用 unregisterListener 接口了
mediaApi.uninit();
```
