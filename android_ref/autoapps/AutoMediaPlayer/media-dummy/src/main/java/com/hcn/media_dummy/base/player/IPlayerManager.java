package com.hcn.media_dummy.base.player;

import android.content.Context;
import android.os.Message;

import com.hcn.media_dummy.base.cache.ICacheManager;
import com.hcn.media_dummy.base.model.MediaOptionModel;

import java.util.List;

import tv.danmaku.ijk.media.player.IMediaPlayer;

/**
 * 播放器差异管理接口
 * <p> 播放内核的对外接口；
 *
 * @author 65821
 */
public interface IPlayerManager {

    /**
     * 获取当前使用的播放器对象
     * @return 播放接口对象
     */
    IMediaPlayer getMediaPlayer();

    /**
     * 初始化播放内核
     *
     * @param context 上下文
     * @param message 播放器所需初始化内容
     * @param optionModelList 配置信息
     * @param cacheManager 缓存管理
     */
    void initMediaPlayer(Context context,
                         Message message,
                         List<MediaOptionModel> optionModelList,
                         ICacheManager cacheManager);

    /**
     * 设置渲染显示
     * @param msg 传递 Surface 对象
     */
    void showDisplay(Message msg);

    /**
     * 是否静音
     * @param needMute 静音状态
     */
    void setNeedMute(boolean needMute);

    /**
     * 设置播放音量
     * <pre>
     *    单独设置 setVolume 和 setNeedMute 互斥
     *    取值范围 [.0 - 1.0]
     * </pre>
     *
     * @param left 左通道声音
     * @param right 右通道声音
     */
    void setVolume(float left, float right);

    /**
     * 释放渲染
     */
    void releaseSurface();

    /**
     * 释放内核
     */
    void release();

    /**
     * 缓存进度
     * @return 缓存百分比
     */
    int getBufferedPercentage();

    /**
     * 网络速度
     * @return 网速
     */
    long getNetSpeed();

    /**
     * 设置倍速播放速度（播放中设置）
     *
     * @param speed 播放速度(0.5、1.0、1.5、2.0、3.0)
     * @param soundTouch 变调播放
     */
    void setSpeedPlaying(float speed, boolean soundTouch);

    /**
     * Surface 是否支持外部 lockCanvas
     * <pre>
     *    可用来自定义暂停时的绘制画面；
     *    exoplayer目前不支持，因为外部 lock 后，切换 surface 会导致异常；
     * </pre>
     *
     * @return 支持与否
     */
    boolean isSurfaceSupportLockCanvas();

    /**
     * 设置倍速播放速度（播放前设置）
     *
     * @param speed 播放速度(0.5、1.0、1.5、2.0、3.0)
     * @param soundTouch 变调播放
     */
    void setSpeed(float speed, boolean soundTouch);

    /** 开始播放 */
    void start();

    /** 停止播放 */
    void stop();

    /** 暂停播放 */
    void pause();

    /**
     * 视频宽度
     * @return 宽
     */
    int getVideoWidth();

    /**
     * 视频高度
     * @return 高
     */
    int getVideoHeight();

    /**
     * 是播放中
     * @return 是/否
     */
    boolean isPlaying();

    /**
     * 指定播放位置
     * @param time 毫秒
     */
    void seekTo(long time);

    /**
     * 获取当前播放位置
     * @return 毫秒
     */
    long getCurrentPosition();

    /**
     * 获取播放源总时长
     * @return 毫秒
     */
    long getDuration();
}
