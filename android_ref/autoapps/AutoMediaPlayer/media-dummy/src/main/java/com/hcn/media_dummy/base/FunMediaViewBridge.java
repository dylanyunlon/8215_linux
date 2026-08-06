package com.hcn.media_dummy.base;

import android.content.Context;
import android.view.Surface;

import com.hcn.media_dummy.base.player.IPlayerManager;
import com.hcn.media_dummy.listener.FunMediaPlayerListener;

import java.io.BufferedInputStream;
import java.io.File;
import java.util.Map;

/**
 * Manager 与 View 之间的接口
 * @author 65821
 */
public interface FunMediaViewBridge {

    /**
     * 获取播放器事件状态监听接口
     * @return {@link FunMediaPlayerListener}
     */
    FunMediaPlayerListener listener();

    /**
     * 设置播放器事件状态监听
     * @param listener 监听对象
     */
    void setListener(FunMediaPlayerListener listener);

    /**
     * 获取正常播放页面状态下的播放器事件状态监听接口
     * <p> 全屏和小屏播放视图与正常播放页面切换的时候用来保存正常播放的监听对象；
     *
     * @return {@link FunMediaPlayerListener}
     */
    FunMediaPlayerListener lastListener();

    /**
     * 保存播放器事件状态监听
     * <p> 全屏和小屏播放视图与正常播放页面切换的时候用来保存正常播放的监听对象；
     *
     * @param lastListener 监听对象
     */
    void setLastListener(FunMediaPlayerListener lastListener);

    /**
     * 标记接口
     * <p> tag 和 position 都是属于标记 flag，不参与播放器实际工作，只是用于防止错误等等
     * @return flag
     */
    String getPlayTag();

    /**
     * 标记接口
     * <p> tag 和 position 都是属于标记 flag，不参与播放器实际工作，只是用于防止错误等等
     * @param  playTag
     */
    void setPlayTag(String playTag);

    /**
     * 标记接口
     * <p> tag 和 position 都是属于标记 flag，不参与播放器实际工作，只是用于防止错误等等
     * @return flag
     */
    int getPlayPosition();

    /**
     * 标记接口
     * <p> tag 和 position 都是属于标记 flag，不参与播放器实际工作，只是用于防止错误等等
     * @param  playPosition
     */
    void setPlayPosition(int playPosition);

    /**
     * 开始准备播放
     *
     * @param url         播放url
     * @param mapHeadData 头部信息
     * @param loop        是否循环
     * @param speed       播放速度
     * @param cache       是否缓存
     * @param cachePath   缓存目录，可以为空，为空时使用默认
     */
    void prepare(final String url,
                 final Map<String, String> mapHeadData,
                 boolean loop,
                 float speed,
                 boolean cache,
                 File cachePath);

    /**
     * 开始准备播放
     *
     * @param url 播放url
     * @param mapHeadData 头部信息
     * @param loop 是否循环
     * @param speed 播放速度
     * @param cache 是否缓存
     * @param cachePath 缓存目录，可以为空，为空时使用默认
     * @param overrideExtension 是否需要覆盖拓展类型
     */
    void prepare(final String url,
                 final Map<String, String> mapHeadData,
                 boolean loop,
                 float speed,
                 boolean cache,
                 File cachePath,
                 String overrideExtension);

    /**
     * 开始准备播放
     *
     * @param videoBufferedInputStream 视频元数据输入流
     * @param mapHeadData 头部信息
     * @param loop 是否循环
     * @param speed 播放速度
     * @param cache 是否缓存
     * @param cachePath 缓存目录，可以为空，为空时使用默认
     */
    void prepare(final BufferedInputStream videoBufferedInputStream,
                 final Map<String, String> mapHeadData,
                 boolean loop,
                 float speed,
                 boolean cache,
                 File cachePath);

    /**
     * 开始准备播放
     *
     * @param videoBufferedInputStream 视频元数据输入流
     * @param mapHeadData 头部信息
     * @param loop 是否循环
     * @param speed 播放速度
     * @param cache 是否缓存
     * @param cachePath 缓存目录，可以为空，为空时使用默认
     * @param overrideExtension 是否需要覆盖拓展类型
     */
    void prepare(final BufferedInputStream videoBufferedInputStream,
                 final Map<String, String> mapHeadData,
                 boolean loop,
                 float speed,
                 boolean cache,
                 File cachePath,
                 String overrideExtension);

    /**
     * 获取当前播放内核
     * @return {@link IPlayerManager}
     */
    IPlayerManager getPlayer();

    /**
     * 针对某些内核，缓冲百分比
     * @return 缓存百分百
     */
    int getBufferedPercentage();

    /** 是否播放器 */
    void releaseMediaPlayer();

    /**
     * 设置当前视频宽
     * @param currentVideoWidth 宽
     */
    void setCurrentVideoWidth(int currentVideoWidth);

    /**
     * 设置当前视频高
     * @param currentVideoHeight 高
     */
    void setCurrentVideoHeight(int currentVideoHeight);

    /**
     * 获取当前视频宽
     * @return 宽
     */
    int getCurrentVideoWidth();

    /**
     * 获取当前视频高
     * @return 高
     */
    int getCurrentVideoHeight();

    /**
     * 设置渲染对象
     * @param holder 显示 surface
     */
    void setDisplay(Surface holder);

    /**
     * 释放渲染对象
     * @param surface 显示 surface
     */
    void releaseSurface(Surface surface);

    /**
     * 获取当前媒体最后状态
     * @return
     */
    int getLastState();

    /**
     * 设置当前媒体最后状态
     * @param lastState
     */
    void setLastState(int lastState);

    /**
     * 播放中的 url 是否已经缓存
     * @return 是/否
     */
    boolean isCacheFile();

    /**
     * 是否已经完全缓存到本地，主要用于开始播放前判断，是否提示用户
     *
     * @param context 上下文
     * @param cacheDir 缓存目录，为空是使用默认目录
     * @param url 指定 url 缓存
     * @return 是/否
     */
    boolean cachePreview(Context context, File cacheDir, String url);

    /**
     * 清除缓存
     *
     * @param context 上下文
     * @param cacheDir 缓存目录，为空是使用默认目录
     * @param url 指定 url 缓存，为空时清除所有
     */
    void clearCache(Context context, File cacheDir, String url);

    /**
     * 网络速度
     * <pre>
     *    注意，这里如果是开启了缓存，因为读取本地代理，缓存成功后还是存在速度的；
     *    再打开已经缓存的本地文件，网络速度才会回 0，因为是播放本地文件了。
     * </pre>
     * @return kb/s
     */
    long getNetSpeed();

    /**
     * 播放速度修改
     *
     * @param speed 播放速度
     * @param soundTouch
     */
    void setSpeed(float speed, boolean soundTouch);

    /**
     * 播放速度修改
     *
     * @param speed 播放速度
     * @param soundTouch
     */
    void setSpeedPlaying(float speed, boolean soundTouch);

    /**
     * 获取 Rotate 选择的 flag
     * <p> 注意目前只有 ijk 用到了；
     *
     * @return flag
     */
    int getRotateInfoFlag();

    /** 播放 */
    void start();

    /** 停止 */
    void stop();

    /** 暂停 */
    void pause();

    /**
     * 视频宽度
     * @return 宽度
     */
    int getVideoWidth();

    /**
     * 视频高度
     * @return 高度
     */
    int getVideoHeight();

    /**
     * 是播放中
     * @return 是/否
     */
    boolean isPlaying();

    /**
     * 跳到指定时间附近关键帧
     * @param time ms
     */
    void seekTo(long time);

    /**
     * 当前播放位置
     * @return ms
     */
    long getCurrentPosition();

    /**
     * 媒体源总时长
     * @return ms
     */
    long getDuration();

    /**
     * Surface 是否支持外部 lockCanvas，来自定义暂停时的绘制画面
     * @return 支持/不支持
     */
    boolean isSurfaceSupportLockCanvas();
}
