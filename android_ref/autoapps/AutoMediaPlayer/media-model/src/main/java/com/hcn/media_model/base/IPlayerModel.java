package com.hcn.media_model.base;

import com.hcn.media_base.IMediaEventListener;
import com.hcn.media_model.player.base.IMediaPlayer;
import com.hcn.media_view.lyrics.LyricsRow;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.List;

/**
 * 播放器模型接口
 * <p> 提供访问 Player 模型的接口；
 *
 * @author 65821
 */
public interface IPlayerModel extends IMediaEventListener {
    /**
     * 获取当前视频平台解码组件
     * @return {@link IMediaPlayer}
     */
    IMediaPlayer corePlayer();

    /**
     * 获取当前视频软解码组件
     * @return {@link IMediaPlayer}
     */
    IMediaPlayer vitamioPlayer();

    /**
     * 设置多媒体播放源
     * <pre>
     *    这些类似的接口放这里有些莫名其妙；
     *    理论上播放组件（HCorePlayer/VitamioPlayer）对象都不能放 Application 初始化;
     * </pre>
     *
     * @param info 播放信息对象
     */
    void onLocalSetDataSourceEvent(MusicInfo info);

    /**
     * 在软解播放高清视频
     * <p> 该状态用来前台提高当前进程优先级使用；
     * @return 是/否
     */
    boolean inSoftDecodingHDVideo();

    /**
     * 调整当前播放到指定位置
     * @param time 事件进度位置（ms）
     */
    void seekToTime(int time);

    /**
     * 获取当前播放目标的总时长
     * @return 媒体总时长
     */
    int getTotalTime();

    /**
     * 获取当前播放目标进度
     * @return 当前播放位置
     */
    int getCurrentPosition();

    List<LyricsRow> getLyricsInfo(String path);

    /** 重置播放时间并更新保存播放信息 **/
    void onSetSeekTimeZero();

    /** 更新平台播放显示 SurfaceHolder **/
    void updateCoreSurfaceHolder();

    /** 更新软解播放显示 SurfaceHolder **/
    void updateVitamioSurfaceHolder();

    /** 更新后台播放显示 SurfaceHolder **/
    void updateRearSurfaceHolder();

    /**
     * 下发播放控制事件
     *
     * @param nCommand 播放/暂停/上下曲/...
     * @param reason 调用调试原因
     */
    void onPlayControlEvent(int nCommand, int reason);

    /**
     * 是否存在有效的播放对象
     * @return 存在/不存在
     */
    boolean existsValidMediaPlayer();

    /**
     * 平台播放组件是否有效
     * @return 有效/无效
     */
    boolean isMediaPlayerValid();

    /**
     * Vitamio 播放组件是否有效
     * @return 有效/无效
     */
    boolean isVitamioPlayerValid();

    /**
     * 设当前播放组件 Track 的音量大小
     * @param volume 音量比例
     */
    void requestSetVolume(float volume);
}
