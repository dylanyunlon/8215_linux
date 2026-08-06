package com.hcn.media.base.xbus;

/**
 * 总线配置信息
 * <p> HBusUtils 只支持无参 TAG 和 带一个参数的 TAG；
 *
 * @author 65821
 */
public interface IBusTag {
    /**
     * 更新音乐播放信息
     * <pre>
     *    带一个参数对象；
     *    默认：{@link com.hcn.mediaservice.data.MusicInfo};
     * </pre>
     */
    String UPDATE_MUSIC_PLAY_INFO = "music-play-info";

    /**
     * 更新音乐播放状态
     * <pre>
     *    带一个参数对象；
     *    默认：String / 保留扩展;
     * </pre>
     */
    String UPDATE_MUSIC_PLAY_STATE = "music-play-state";

    /**
     * 更新音乐播放时间
     * <pre>
     *    带一个参数对象；
     *    默认：MediaTimeInfo / 保留扩展;
     * </pre>
     */
    String UPDATE_MUSIC_PLAY_TIME = "music-play-time";

    /**
     * 更新音乐播放歌词
     * <pre>
     *    带一个参数对象；
     *    默认：String / 文件路径;
     * </pre>
     */
    String UPDATE_MUSIC_LYRICS_INFO = "music-lyrics-info";

    /**
     * 当前媒体进程将要退出
     * <pre>
     *    这个总线事件不需要带参数；
     *    这个事件需要运行在主线程，需要阻塞调用；
     * </pre>
     */
    String MEDIA_APP_WILL_EXIT = "media-app-will-exit";
}
