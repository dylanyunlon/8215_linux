package com.hcn.media.base;

import androidx.annotation.NonNull;

import com.hcn.media_view.lyrics.LyricsRow;

import java.util.List;

/**
 * 媒体代理接口
 * @author 65821
 */
public interface IMediaAgent {
    
    /**
     * 设置客户端的名字
     * @param name 字符串名字
     */
    void setClientName(@NonNull String name);

    /**
     * 设置媒体事件回调接口
     * @param callback 监听者
     */
    void setMediaCallback(IEventCallback callback);

    /**
     * 判断当前连接状态
     * <p> 如果当前在连接状态，说明媒体进程已启动；
     *
     * @param state {@link IConnectionState#CONNECTED ...}
     * @return 是否符合预期
     */
    boolean isConnectionState(String state);

    /**
     * 多媒体连接状态
     * @return {@link IConnectionState}
     */
    String connectionState();

    /**
     * 获取指定歌曲歌詞信息
     * <pre>
     *    接口説明（收到如下信息后可以使用）：
     *      IEvent.MUSIC_PLAY_INFO
     *      IEvent.MUSIC_LYRICS_INFO
     * </pre>
     *
     * @param path 歌曲文件路径
     *
     * @return {@link LyricsRow}
     */
    List<LyricsRow> getLyricsRowInfo(String path);

    /**
     * 请求执行音乐 api 接口
     * @param mediaApi {@link IMediaApi}
     */
    int requestExecuteMusicApi(@IMediaApi String mediaApi);

    /**
     * 请求启动媒体播放器进程
     * @param reason 启动原因
     */
    void requestStartApp(String reason);

    /**
     * 请求 Bind 媒体播放器服务
     * @param reason 绑定原因
     */
    void requestBindApp(String reason);

    /**
     * 请求退出媒体播放器进程
     * @param reason 退出原因字符串描述
     */
    void requestExitApp(String reason);
}
