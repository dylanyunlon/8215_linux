package com.hcn.media.api;

import android.app.Application;
import android.content.Context;
import android.graphics.Typeface;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import com.hcn.media.base.IConnectionState;
import com.hcn.media.base.IEvent;
import com.hcn.media.base.IMediaAgent;
import com.hcn.media.base.IMediaApi;
import com.hcn.media.base.IPlayInfoChanged;
import com.hcn.media.base.MediaPlayInfo;
import com.hcn.media.impl.Instrumentation;
import com.hcn.media.utils.LogUtils;
import com.hcn.media.utils.UtilsEx;
import com.hcn.media_view.lyrics.LyricsRow;
import com.hcn.media_view.lyrics.LyricsView;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import java.io.File;

/**
 * 多媒体对外 API 接口；
 * @author 65821
 */
public class HMediaApi extends Handler {
    private static final String TAG = HMediaApi.class.getSimpleName();

    /**
     * 唯一实例接口
     * <p> 确保每个客户端进程只初始化一次媒体接口；
     */
    private static HMediaApi sMediaApi = null;

    /** 功能特征标签 **/
    private String mFeatureLabels = null;

    /** 存储当前播放信息 */
    private final MediaPlayInfo mMediaPlayInfo;

    /** 当前歌词行信息集合 */
    private final List<LyricsRow> mLyricsRowInfos = new ArrayList<>();

    /**
     * 创建媒体 API 接口
     * @param context 上下文环境对象（建议传递 Application 对象）
     * @param name 客户端名称
     *
     * @return {@link HMediaApi}
     */
    public static HMediaApi createMediaApi(@NonNull Context context, @NonNull String name) {
        if (sMediaApi == null) {
            sMediaApi = new HMediaApi(context, name);
        }

        return sMediaApi;
    }

    public static HMediaApi getMediaApi() {
        if (sMediaApi == null) {
            throw new UnsupportedOperationException(
                    "Please call the createMediaApi interface in Application!");
        } else {
            return sMediaApi;
        }
    }

    /** 客户端上下文环境对象 **/
    private final Reference<Context> mContextRef;

    /** 客户端名称（调试用） **/
    private final String mClientName;

    /** 媒体播放代理对象接口 **/
    private IMediaAgent mMediaAgent;

    /** 媒体连接状态监听管理 **/
    private final List<IConnectionState> mConnectionStateListeners;

    /** 媒体连接状态监听管理 **/
    private final List<IPlayInfoChanged> mPlayInfoChangedListeners;

    /** 经典歌词显示视图对象 */
    private Reference<LyricsView> mLyricsView = null;

    /** 禁止实例化无参数对象 **/
    private HMediaApi() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    /** 当前正在请求音乐 API **/
    private boolean mCurrentBeRequestingMusicMode;

    /**
     * 私有的构造函数
     * <p> 请确保只会被构造一次；
     *
     * @param context 上下文环境
     * @param name 客户端名称
     */
    private HMediaApi(@NonNull Context context, @NonNull String name) {
        super(Looper.getMainLooper());

        mContextRef = new WeakReference<>(context);
        mClientName = name;

        mConnectionStateListeners = new ArrayList<>();
        mPlayInfoChangedListeners = new ArrayList<>();

        mMediaPlayInfo = new MediaPlayInfo();
    }

    /**
     * 设置打印等级开关
     * <p> 当 level 等于 -1 的时候，表示设置全局开关；
     *
     * @param level {@link Log#VERBOSE,Log#DEBUG ...}
     * @param debug 调试开关
     */
    public HMediaApi setDebug(int level, boolean debug) {
        // 全局打印开关
        if (level == -1) {
            LogUtils.setConfig(debug, null);
        }

        // 调试等级开关
        LogUtils.setDebug(level, debug);
        return this;
    }

    /**
     * 返回当前关联的 {@link Context}
     *
     * @throws IllegalStateException 如果当前未与上下文相关联。
     * @see #mContextRef
     */
    @NonNull
    public final Context requireContext() {
        Context context = mContextRef.get();
        if (context == null) {
            throw new IllegalStateException("HMediaApi " + this + " not attached to a context.");
        }
        return context;
    }

    /**
     * Api 初始化函数
     * <pre>
     *    非线程安全，请在主线程调用；
     *    必须调用初始化函数，否则操作类接口调用都会报错；
     * </pre>
     *
     * @return {@link HMediaApi}
     */
    public HMediaApi init() {
        // 避免重复构建
        if (!Objects.isNull(mMediaAgent)) {
            return this;
        }

        // 构建媒体代理类
        mMediaAgent = Instrumentation.buildAgent(requireContext());

        // 设置媒体代理事件监听信息
        mMediaAgent.setClientName(mClientName);
        mMediaAgent.setMediaCallback((event, obj0, obj1) -> {
            switch (event) {
                case IEvent.CONNECTION_STATE:
                    onConnectionStateChanged(obj0);
                    break;
                case IEvent.MUSIC_PLAY_INFO:
                case IEvent.MUSIC_PLAY_STATE:
                case IEvent.MUSIC_PLAY_TIME:
                case IEvent.MUSIC_LYRICS_INFO:
                    onPlayInfoChanged(event, obj0, obj1);
                default:
                    break;
            }
        });

        // 下一个消息处理
        postDelayed(() -> {
            // 首次构建，清除所有消息任务事件；
            removeCallbacksAndMessages(null);

            // 如果是连接状态需要通知更新状态
            String state = IConnectionState.CONNECTED;
            if (mMediaAgent.isConnectionState(state)) {
                onConnectionStateChanged(state);
            }
        }, 10);

        return this;
    }

    /**
     * Api 反初始化函数
     * <pre>
     *    如需要释放 api 相关资源可以调用它；
     *    uninit 后不可以再调用操作类函数（将报异常）；
     * </pre>
     */
    public void uninit() {
        mConnectionStateListeners.clear();
        mPlayInfoChangedListeners.clear();
        removeCallbacksAndMessages(null);

        if (mMediaAgent != null) {
            mMediaAgent.setMediaCallback(null);
            mMediaAgent = null;
        }
    }

    /**
     * 检查接口调用有效性
     * <p> 因为是 SDK 接口类，需要约束指引使用者正确调用；
     */
    private void checkCallValidity() {
        if (Objects.isNull(mMediaAgent)) {
            throw new UnsupportedOperationException(
                    "Media Api interface object not initialized!");
        }
    }

    /**
     * 注册监听连接状态对象
     * <p> 非线程安全，请在主线程调用；
     *
     * @param listener 监听者
     * @return {@link HMediaApi}
     */
    public HMediaApi registerListener(@NonNull IConnectionState listener) {
        checkCallValidity();

        // 不要重复添加同一个对象
        if (mConnectionStateListeners.contains(listener)) {
            return this;
        }

        // 添加到连接状态监听管理
        mConnectionStateListeners.add(listener);

        // 新添加的监听对象，需要补发连接状态消息
        postDelayed(() -> {
            // 如果回调是有效的
            if (mMediaAgent != null
                    && mConnectionStateListeners.contains(listener)) {

                if (!mMediaAgent.isConnectionState(
                        IConnectionState.CONNECTED)) {
                    return;
                }

                listener.onConnected();
            }
        }, 0);

        return this;
    }

    /**
     * 取消目标对象对连接状态的监听
     * <p> 非线程安全，请在主线程调用；
     *
     * @see #registerListener(IConnectionState)
     * @param listener 监听者
     */
    public void unregisterListener(IConnectionState listener) {
        if (Objects.isNull(listener)) {
            return;
        }

        checkCallValidity();
        mConnectionStateListeners.remove(listener);
    }

    /**
     * 媒体连接状态改变
     * @see IEvent#CONNECTION_STATE
     * @param obj 当前连接状态
     */
    private void onConnectionStateChanged(Object obj) {
        // 无状态监听者
        if (mConnectionStateListeners.isEmpty()) {
            return;
        }

        // 参数类型检查
        if (!(obj instanceof String)
                || TextUtils.isEmpty((CharSequence) obj)) {
            Log.w(TAG, "CONNECTION_STATE/Invalid state parameter!");
            return;
        }

        // 分发媒体连接状态事件
        for (IConnectionState listener : mConnectionStateListeners) {
            if (listener == null) {
                continue;
            }

            String state = (String) obj;
            switch (state) {
                case IConnectionState.CONNECTED:
                    listener.onConnected();
                    break;
                case IConnectionState.DISCONNECTED:
                    listener.onDisconnected();
                    break;
                case IConnectionState.DIED:
                    listener.onDied();
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * 注册监听播放信息改变
     * <p> 非线程安全，请在主线程调用；
     *
     * @param listener 监听者
     * @return {@link HMediaApi}
     */
    public HMediaApi registerListener(@NonNull IPlayInfoChanged listener) {
        checkCallValidity();

        // 不要重复添加同一个对象
        if (mPlayInfoChangedListeners.contains(listener)) {
            return this;
        }

        // 添加到连接状态监听管理
        mPlayInfoChangedListeners.add(listener);

        // 新添加的监听对象，需要补发播放信息消息
        postDelayed(() -> {
            // 如果回调是有效的
            if (mMediaAgent != null
                    && mPlayInfoChangedListeners.contains(listener)) {

                if (!mMediaAgent.isConnectionState(
                        IConnectionState.CONNECTED)) {
                    return;
                }

                listener.onPlayInfoChanged(IEvent.MUSIC_PLAY_INFO, mMediaPlayInfo);
            }
        }, 0);

        return this;
    }

    /**
     * 取消目标对象对播放信息的监听
     * <p> 非线程安全，请在主线程调用；
     *
     * @see #registerListener(IPlayInfoChanged)
     * @param listener 监听者
     */
    public void unregisterListener(IPlayInfoChanged listener) {
        if (Objects.isNull(listener)) {
            return;
        }

        checkCallValidity();
        mPlayInfoChangedListeners.remove(listener);
    }

    /**
     * 媒体播放信息改变
     * @see com.hcn.media.base.IEventCallback
     * <pre>
     *    事件类型：{@link IEvent#MUSIC_PLAY_INFO}
     * </pre>
     *
     * @param event 事件类型
     * @param obj0 附加参数 1
     * @param obj1 附加参数 2
     */
    private void onPlayInfoChanged(@IEvent String event, Object obj0, Object obj1) {
        // 解析数据类型
        switch (event) {
            case IEvent.MUSIC_PLAY_INFO: {
                if (!(obj0 instanceof String)) {
                    Log.w(TAG, "MUSIC_PLAY_INFO/Invalid parameter!");
                    return;
                }

                // 更新播放信息
                mMediaPlayInfo.setFilePath((String) obj0);
                if (!(obj1 instanceof String)) {
                    mMediaPlayInfo.setArtist(null);
                    mMediaPlayInfo.setAlbum(null);

                    File file = new File(mMediaPlayInfo.getFilePath());
                    mMediaPlayInfo.setTitle(file.exists()? file.getName(): null);

                    onUpdatePlayInfo(event);
                    Log.w(TAG, "MUSIC_PLAY_INFO/No ID3 Info!");
                    return;
                }

                // 更新 ID3 = "@ID3:title:#:artist:#:album"
                String id3Info = (String) obj1;
                if (!TextUtils.isEmpty(id3Info)
                        && id3Info.startsWith("@ID3:")) {
                    id3Info = id3Info.substring(5);
                    String[] id3Array = id3Info.split(":#:");
                    if (id3Array.length >= 3) {
                        mMediaPlayInfo.setTitle(id3Array[0]);
                        mMediaPlayInfo.setArtist(id3Array[1]);
                        mMediaPlayInfo.setAlbum(id3Array[2]);
                    }
                }
                break;
            }
            case IEvent.MUSIC_PLAY_STATE: {
                if (!(obj0 instanceof String)) {
                    Log.w(TAG, "MUSIC_PLAY_STATE/Invalid parameter!");
                    return;
                }

                // 更新播放状态
                mMediaPlayInfo.setState((String) obj0);
                break;
            }
            case IEvent.MUSIC_PLAY_TIME: {
                if (!(obj0 instanceof String)
                        || !(obj1 instanceof String)) {
                    Log.w(TAG, "MUSIC_PLAY_TIME/Invalid parameter!");
                    return;
                }

                String current = (String) obj0;
                String duration = (String) obj1;
                if (TextUtils.isEmpty(current)
                        || TextUtils.isEmpty(duration)) {
                    Log.w(TAG, "MUSIC_PLAY_TIME/Invalid parameter!");
                    return;
                }

                int currentInt = Integer.parseInt(current);
                int durationInt = Integer.parseInt(duration);

                // 更新播放进度
                mMediaPlayInfo.setCurrentPosition(currentInt);
                mMediaPlayInfo.setDuration(durationInt);

                // 更新歌词显示信息
                checkUpdateLyricsViewInfo();
                break;
            }
            case IEvent.MUSIC_LYRICS_INFO: {
                if (!(obj0 instanceof String)) {
                    Log.w(TAG, "MUSIC_LYRICS_INFO/Invalid parameter!");
                    mLyricsRowInfos.clear();
                    mMediaPlayInfo.setLyricsFile(null);
                    checkSyncLyricsInfo2View();

                    onUpdatePlayInfo(event);
                    return;
                }

                // 更新歌词行信息
                mLyricsRowInfos.clear();
                String mediaFilePath = mMediaPlayInfo.getFilePath();
                if (!TextUtils.isEmpty(mediaFilePath)) {
                    List<LyricsRow> list = mMediaAgent.getLyricsRowInfo(mediaFilePath);
                    if (!Objects.isNull(list) && !list.isEmpty()) {
                        mLyricsRowInfos.addAll(list);
                    }
                }
                // 同步更新显示
                checkSyncLyricsInfo2View();

                // 歌词文件路径
                mMediaPlayInfo.setLyricsFile((String) obj0);
                break;
            }
            default:
                break;
        }

        // 无状态监听者
        if (mPlayInfoChangedListeners.isEmpty()) {
            return;
        }

        onUpdatePlayInfo(event);
    }

    /**
     * 分发播放信息改变事件
     * @param event {@link IEvent}
     */
    private void onUpdatePlayInfo(String event) {
        // 分发媒体连接状态事件
        for (IPlayInfoChanged listener : mPlayInfoChangedListeners) {
            if (listener == null) {
                continue;
            }

            listener.onPlayInfoChanged(event, mMediaPlayInfo);
        }
    }

    /**
     * 获取歌词信息
     * <p> 注意并不是所有歌曲都有歌词文件；
     *
     * @param path 歌曲文件
     * @return 歌词信息集合
     */
    public List<LyricsRow> getLyricsInfo(String path) {
        checkCallValidity();

        // 非启动状态不处理；
        if (!mMediaAgent.isConnectionState(
                IConnectionState.CONNECTED)) {
            return null;
        }

        return mMediaAgent.getLyricsRowInfo(path);
    }

    /**
     * 检查并同步歌词信息到歌词显示视图
     * <p> 只有配置了 LyricsView 的时候才会更新；
     */
    private void checkSyncLyricsInfo2View() {
        // 非启动状态不处理；
        if (!mMediaAgent.isConnectionState(
                IConnectionState.CONNECTED)) {
            return;
        }

        // 没有配置 LyricsView 不处理；
        if (mLyricsView == null || mLyricsView.get() == null) {
            return;
        }

        // 更新歌词显示信息
        LyricsView view = mLyricsView.get();
        if (view != null) {
            view.setLrcRows(mLyricsRowInfos);
        }
    }

    /**
     * 更新歌词显示视图
     * <p> 只有配置了 LyricsView 的时候才会更新；
     */
    private void checkUpdateLyricsViewInfo() {
        // 非启动状态不处理；
        if (!mMediaAgent.isConnectionState(
                IConnectionState.CONNECTED)) {
            return;
        }

        // 没有配置 LyricsView 不处理；
        if (mLyricsView == null || mLyricsView.get() == null) {
            return;
        }

        // 更新歌词显示信息
        LyricsView view = mLyricsView.get();
        if (view != null && mMediaPlayInfo.isValid()) {
            view.seekTo(mMediaPlayInfo.getCurrentPosition(), true, false);
        }
    }

    /**
     * 媒体 api 特征接口
     * <p> 当前调用者包名；
     *
     * @return 特征字符串
     */
    private String reason() {
        // 避免重复校验特征；
        if (!TextUtils.isEmpty(mFeatureLabels)) {
            return mFeatureLabels;
        }

        // 获取当前应用包名；
        mFeatureLabels = "media-api";
        Application app = UtilsEx.getApplication();
        if (app != null) {
            mFeatureLabels = app.getPackageName();
        }
        return mFeatureLabels;
    }

    /**
     * 请求启动媒体应用
     * <pre>
     *    如果没有启动，则后台启动播放音乐；
     *    如果已经启动，且未播放（非视频前台），触发音乐播放；
     * </pre>
     */
    public void requestStartApp() {
        checkCallValidity();

        // 是启动状态不处理；
        if (mMediaAgent.isConnectionState(
                IConnectionState.CONNECTED)) {
            // 需要强制触发播放
            onExecuteMusicApi(IMediaApi.PLAY);
            return;
        }

        mMediaAgent.requestStartApp(reason());
    }

    /**
     * 调用音乐 API
     * <p>
     *     执行音乐功能操作，当返回 -2 表示当前不是音乐模式，延迟 0.1 秒保证切换成功后重新调用功能
     * </p>
     * @param musicMode
     */
    private void onExecuteMusicApi(@IMediaApi String musicMode) {
        if (mCurrentBeRequestingMusicMode) {
            return;
        }

        if (-2 == mMediaAgent.requestExecuteMusicApi(musicMode)) {
            mCurrentBeRequestingMusicMode = true;
            postDelayed(() -> {
                mMediaAgent.requestExecuteMusicApi(musicMode);
                mCurrentBeRequestingMusicMode = false;
            }, 100);
        }
    }

    /**
     * 请求绑定媒体应用（不自动触发播放）
     * <pre>
     *    如果没有启动，则后台启动媒体服务；
     *    不会自动触发播放动作（绑定后如要播放，需要调用 musicPlay()）
     * </pre>
     */
    public void requestBindApp() {
        checkCallValidity();

        // 是启动状态不处理；
        if (mMediaAgent.isConnectionState(
                IConnectionState.CONNECTED)) {
            return;
        }

        mMediaAgent.requestBindApp(reason());
    }

    /**
     * 请求推出媒体应用
     * <p> 强制退出多媒体进程；
     */
    public void requestExitApp() {
        checkCallValidity();
        mMediaAgent.requestExitApp(reason());
    }

    /**
     * 音乐播放暂停
     * <p> 自适应暂停播放，非音乐模式不执行；
     */
    public void musicPlayPause() {
        checkCallValidity();
        onExecuteMusicApi(IMediaApi.PLAY_PAUSE);
    }

    /**
     * 音乐播放
     * <p> 非音乐模式不执行；
     */
    public void musicPlay() {
        checkCallValidity();
        onExecuteMusicApi(IMediaApi.PLAY);
    }

    /**
     * 音乐暂停
     * <p> 非音乐模式不执行；
     */
    public void musicPause() {
        checkCallValidity();
        onExecuteMusicApi(IMediaApi.PAUSE);
    }

    /**
     * 音乐下一曲
     * <p> 非音乐模式不执行；
     */
    public void musicPlayNext() {
        checkCallValidity();
        onExecuteMusicApi(IMediaApi.NEXT);
    }

    /**
     * 音乐上一曲
     * <p> 非音乐模式不执行；
     */
    public void musicPlayPrev() {
        checkCallValidity();
        onExecuteMusicApi(IMediaApi.PREV);
    }

    /**
     * 切换音乐播放模式
     * <p> 非音乐模式不执行；
     */
    public void musicSwitchPlayMode() {
        checkCallValidity();
        mMediaAgent.requestExecuteMusicApi(IMediaApi.PLAY_MODE);
    }

    /**
     * 请求播放信息
     * <pre>
     *    非连接模式不可以执行（暂支持音乐模式使用）
     *    会触发 {@link IPlayInfoChanged} 回调；
     * </pre>
     */
    public void requestPlayInfo() {
        checkCallValidity();

        // 非启动状态不处理；
        if (!mMediaAgent.isConnectionState(
                IConnectionState.CONNECTED)) {
            return;
        }

        mMediaAgent.requestExecuteMusicApi(IMediaApi.PLAY_INFO);
    }

    /**
     * 获取当前媒体播放信息（内存）
     * <pre>
     *   非连接模式不可以执行（暂支持音乐模式使用）
     *   获取的是内存中的数据，如果需要最新数据请调用 {@link #requestPlayInfo()} 更新；
     * </pre>
     *
     * @return {@link MediaPlayInfo}
     */
    public MediaPlayInfo mediaPlayInfo() {
        checkCallValidity();

        // 非启动状态不处理；
        if (!mMediaAgent.isConnectionState(
                IConnectionState.CONNECTED)) {
            return null;
        }

        return mMediaPlayInfo;
    }

    /**
     * 获取当前媒体歌词信息集合（内存）
     * <pre>
     *   非连接模式不可以执行（暂支持音乐模式使用）
     *   在接收到回调 {@link IPlayInfoChanged} 事件 {@link IEvent#MUSIC_LYRICS_INFO} 时可以调用；
     *   接口用来获取歌词信息，如果需要最新数据请调用 {@link #requestPlayInfo()} 更新；
     *   主要用来给需要自定义特定歌词显示视图的需求使用；
     * </pre>
     *
     * @return {@link LyricsRow}
     */
    public List<LyricsRow> mediaLyricsRowInfo() {
        checkCallValidity();

        // 非启动状态不处理；
        if (!mMediaAgent.isConnectionState(
                IConnectionState.CONNECTED)) {
            return null;
        }

        return mLyricsRowInfos;
    }

    /**
     * 设置歌词显示视图
     * <pre>
     *    我们这里支持直接在 LyricsView 显示歌词；
     *    如果没有显示需求建议设置为 null（正常弱引用）；
     *    单个进程中，最后一次设置有效（不支持多个视图）；
     *    如果 LyricsView 不能满足需求，请使用接口 {@link #getLyricsInfo(String)}；
     * </pre>
     *
     * @param view 歌词显示视图对象
     */
    public void setLyricsView(@NonNull LyricsView view) {
        checkCallValidity();
        mLyricsView = new WeakReference<>(view);

        // 非启动状态不处理；
        if (!mMediaAgent.isConnectionState(
                IConnectionState.CONNECTED)) {
            return;
        }

        view.SetPainTypeface(Typeface.SERIF);
        if (!mLyricsRowInfos.isEmpty()) {
            view.setLrcRows(mLyricsRowInfos);
        }
    }
}
