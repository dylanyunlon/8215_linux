package com.hcn.media.base;

import androidx.annotation.StringDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * 事件类型
 * @see IEventCallback
 *
 * @author 65821
 */
@Retention(RetentionPolicy.SOURCE)
@StringDef({
        IEvent.MEDIA_EXTEND_EVENT,
        IEvent.CONNECTION_STATE,
        IEvent.MUSIC_PLAY_INFO,
        IEvent.MUSIC_PLAY_STATE,
        IEvent.MUSIC_PLAY_TIME,
        IEvent.MUSIC_LYRICS_INFO,
        IEvent.MEDIA_APP_WILL_EXIT
})

public @interface IEvent {

    /**
     * 媒体扩展事件
     * <pre>
     *    可以自由发挥，尽可能不用；
     *    一般我们用来做测试调试用；
     * <pre>
     */
    String MEDIA_EXTEND_EVENT = "extend-event";

    /**
     * 媒体连接状态
     * <p> idle、 connected、disconnected...
     *
     * @see IConnectionState
     */
    String CONNECTION_STATE = "connection-state";

    /**
     * 音乐播放信息
     * <pre>
     *    可以定义特定的格式信息；
     *    参见 {@link com.hcn.media.api.IMediaCallback#onEvent}
     * </pre>
     */
    String MUSIC_PLAY_INFO = "music-play-info";

    /**
     * 音乐播放状态
     * <pre>
     *    附加参数 arg0 传递当前播放状态（playing、pause、stop）;
     *    参见 {@link com.hcn.media.api.IMediaCallback#onEvent}
     * </pre>
     */
    String MUSIC_PLAY_STATE = "music-play-state";

    /**
     * 音乐播放事件
     * <pre>
     *    附加参数 arg0 / arg1（当前播放时间 / 持续时间）
     *    参见 {@link com.hcn.media.api.IMediaCallback#onEvent}
     * </pre>
     */
    String MUSIC_PLAY_TIME = "music-play-time";

    /**
     * 音乐播放歌词
     * <pre>
     *    附加参数 arg0 (歌词文件路径)
     *    参见 {@link com.hcn.media.api.IMediaCallback#onEvent}
     * </pre>
     */
    String MUSIC_LYRICS_INFO = "music-lyrics-info";

    /**
     * 媒体进程将要退出
     * <pre>
     *    这是退出前回调事件，客户端需要解除绑定的服务状态；
     *    如果媒体退出后客户端不解除 bind，系统又会把媒体拉起来；
     * </pre>
     */
    String MEDIA_APP_WILL_EXIT = "media-app-will-exit";
}
