// IReceptionService.aidl
package com.hcn.media.api;

import com.hcn.media.api.IMediaCallback;
import com.hcn.media_view.lyrics.LyricsRow;

interface IReceptionService {
    /**
     * 设置客户端 Binder 对象
     * <pre>
     *    主要是用来监听客户端绑定信息；
     *    如果客户端挂掉了，释放对应资源
     * </pre>
     */
    void setClientBinder(String name, IBinder client);

    /**
     * 设置多媒体事件回调
     * <pre>
     *    1、通知播放信息改变；
     *    2、通知播放状态改变；
     *    3、通知播放进度改变；
     * </pre>
     */
    void setMediaCallback(IMediaCallback callback);

    /**
     * 获取当前媒体类型
     * <pre>
     *    int MEDIA_TYPE_IDLE = -1;
     *    int MEDIA_TYPE_MUSIC = 0;
     *    int MEDIA_TYPE_VIDEO = 1;
     * </pre>
     */
    int getCurrentMediaType();

    /**
     * 请求播放音乐模式
     * <pre>
     *    如果不在视频前台，可以强制播放音乐；
     *    如果本身已经在音乐播放状态，不处理；
     * </pre>
     * @return -1 请求失败 / 0 请求成功;
     */
    int requestPlayMusicMode(String reason);

    /**
     * 请求执行音乐 api 接口
     * <p> 当前如果不在音乐模式不可以执行；
     *
     * @param mediaApi {@link IMediaApi}
     * @param reason 触发原因
     */
    void requestExecuteMusicApi(String mediaApi, String reason);

    /**
     * 请求退出媒体播放器进程
     * @param reason 退出原因字符串描述
     */
    void requestExitApp(String reason);

    /**
     * 获取当前媒体歌词信息
     *
     * @param path 歌曲路径
     * @return 歌词行信息列表（空标识无歌词）
     */
    List<LyricsRow> getLyricsInfo(String path);
}