package com.hcn.media.external;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.lifecycle.Lifecycle;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.auto_compat.PlatformUtils;
import com.hcn.common.misc.HBusUtils;
import com.hcn.media.base.IEvent;
import com.hcn.media.base.IMedia;
import com.hcn.media.base.IMedia.TriggerReason;
import com.hcn.media.base.IMediaApi;
import com.hcn.media.base.xbus.IBusTag;
import com.hcn.media.base.service.BaseService;
import com.hcn.media.R3;
import com.hcn.media.external.debug.BroadcastApi;
import com.hcn.media.external.debug.BroadcastApi.ITypeValue;
import com.hcn.media.external.observer.ReceptionServiceObserver;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.impl.MediaEvent;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_model.MediaModel;
import com.hcn.media_model.base.ILocalzModel;
import com.hcn.media_view.lyrics.LyricsRow;
import com.hcn.mediaservice.data.MediaTimeInfo;
import com.hcn.mediaservice.data.MusicInfo;
import com.orhanobut.logger.Logger;

import java.util.List;
import java.util.Objects;

/**
 * 前台接待服务
 * <pre>
 *    处理外部请求进程事件（实现后台播放）；
 *    这里相当于是一个接待处，后续外部所有请求和访问都先到这里沟通；
 *    不需要设置为前台服务（被回收也无所谓）；
 *    通过 {@link ILocalzModel} 访问播放服务；
 * </pre>
 *
 * @author 65821
 */
public class ReceptionService
        extends BaseService implements TriggerReason {
    public static final String TAG = ReceptionService.class.getSimpleName();

    /**
     * 当前服务 Notification 信道的 ID
     * <pre>
     *     信道 ID 必须是唯一的；
     *     主要是为了设置前台服务，保活使用；
     * </pre>
     */
    private final String NOTIFICATION_CHANNEL_ID = "MM_PLAY_2023-0407-1755";

    /** 当前服务观察者对象 **/
    ReceptionServiceObserver mServiceObserver = null;

    /**
     * 是媒体服务初始化完成
     * <pre>
     *    只有媒体服务初始化完成了才可以触发播放动作；
     *    具体参见 {@link IMediaEvent#EVENT_SERVICE_INITIALIZED}；
     * </pre>
     */
    private boolean mMediaInitCompleted;

    /**
     * 本服务 Binder 对象；
     * <p> 对外提供访问媒体接待服务的接口；
     */
    private final IBinder mReceptionBinder;
    private boolean mHasClientConnected;

    /** 标记请求播放音乐的原因 **/
    private static String mRequestPlayMusicReason;

    /** 默认无参构造函数 **/
    public ReceptionService() {
        super();

        // 创建对外 Binder 接口对象
        mReceptionBinder = new ReceptionBinder(this);
    }

    @SuppressLint("ObsoleteSdkInt")
    @Override
    public void onCreate() {
        super.onCreate();
        Logger.t(TAG).v("onCreate...");

        // 注册媒体总线
        HBusUtils.register(this);
    }

    @Override
    protected void onCreateObserver() {
        // 避免被重复创建
        if (!Objects.isNull(mServiceObserver)) {
            return;
        }

        // 创建服务观察者
        mServiceObserver = new ReceptionServiceObserver(
                this, this, null);
        getLifecycle().addObserver(mServiceObserver);

        // 订阅观察者事件
        mCompositeDisposable.add(
                mServiceObserver.eventRelay().subscribe(msg -> {
                    if (!mServiceObserver.isState(Lifecycle.State.STARTED)) {
                        // TODO: 约束只处理 started 状态下的事件
                        return;
                    }

                    onServiceObserverEvent(msg.what, msg.arg0, msg.obj0);
                }));
    }

    @SuppressLint("ObsoleteSdkInt")
    @Override
    protected void onStartForeground() {
        // Android 8.0 需要创建通知信道
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationManager notificationManager =
                    (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            NotificationChannel channel = new NotificationChannel(
                    NOTIFICATION_CHANNEL_ID,
                    getResources().getText(R3.string.media_notification_name),
                    NotificationManager.IMPORTANCE_LOW);
            notificationManager.createNotificationChannel(channel);
        }

        // 启动前台服务，并通知状态栏显示状态。
        startForeground(1711, getNotification());
    }

    /**
     * 获取一个 Notification 对象
     * <p> 通知系统状态栏显示服务状态信息使用, 兼容 Android 低版本。</p>
     */
    @SuppressLint("ObsoleteSdkInt")
    private Notification getNotification() {
        // 为后续兼容低版本考虑
        Notification.Builder builder;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            builder = new Notification.Builder(this, NOTIFICATION_CHANNEL_ID)
                    .setSmallIcon(R.drawable.ic_media_24)
                    .setContentTitle(getResources().getText(R3.string.media_notification_title))
                    .setContentText(getResources().getText(R3.string.media_notification_info));
        } else {
            builder = new Notification.Builder(this)
                    .setSmallIcon(R.drawable.ic_media_24)
                    .setContentTitle(getResources().getText(R3.string.media_notification_title))
                    .setContentText(getResources().getText(R3.string.media_notification_info));
        }

        return builder.build();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // 如未收到初始化事件（主动查询）
        if (!mMediaInitCompleted) {
            // 媒体服务初始化状态
            mMediaInitCompleted = MediaModel.call()
                    .localzModel().isServiceReadyState();

            // 尝试请求切换到音乐模式
            if (mMediaInitCompleted) {
                Message msg = H0.obtainMessage(
                        H.MSG_TRY_REQUEST_ENTER_MUSIC_MODE,
                        IMedia.TriggerReason.START_SERVICE);

                H0.removeMessages(msg.what);
                H0.sendMessageDelayed(msg, 10);
            }
        }

        // 确保服务被 kill 后不会重启；
        super.onStartCommand(intent, flags, startId);
        return START_NOT_STICKY;
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        super.onBind(intent);
        LogUtil.v(TAG, "onBind.");

        mHasClientConnected = true;
        mMediaInitCompleted = MediaModel.call()
                .localzModel().isServiceReadyState();

        return mReceptionBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        LogUtil.v(TAG, "onUnbind.");

        mHasClientConnected = false;
        return super.onUnbind(intent);
    }

    @Override
    public void onRebind(Intent intent) {
        LogUtil.v(TAG, "onRebind.");

        mHasClientConnected = true;
        super.onRebind(intent);
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        LogUtil.v(TAG, "onTaskRemoved.");
        super.onTaskRemoved(rootIntent);
    }

    /**
     * 是否有客户端和当前服务在连接状态
     * @return 有连接/无连接
     */
    public boolean hasClientConnected() {
        return mHasClientConnected;
    }

    /**
     * 读取当前媒体类型
     * @return {@link com.hcn.media.base.IMedia.Type}
     */
    /* public */ int currentMediaType() {
        return AppGlobalData.getInstance().mMediaType;
    }

    /**
     * 处理服务观察者事件
     * <p> 禁止手动在非订阅观察对象中调用这个函数；
     *
     * @param event 事件类型
     * @param arg0 附加数据 1
     * @param obj1 附加数据 2
     */
    private void onServiceObserverEvent(int event, int arg0, Object obj1) {
        // 事件检查（只处理关心的）
        switch (event) {
            case IMediaEvent.EVENT_SERVICE_INITIALIZED:
            case IMediaEvent.EVENT_PRE_NORMAL_EXIT_APP:
            case IMediaEvent.EVENT_DEBUG_MEDIA_API:
                break;
            default:
                return;
        }

        // 打印事件名称（非关键事件只打印 ID 值）
        LogUtil.v(TAG, "ServiceObserver/event: " + MediaEvent.name(event));

        switch (event) {
            case IMediaEvent.EVENT_SERVICE_INITIALIZED:
                onEventServiceInitialized();
                break;
            case IMediaEvent.EVENT_DEBUG_MEDIA_API:
                onEventDebugMediaApi(arg0, obj1);
                break;
            case IMediaEvent.EVENT_PRE_NORMAL_EXIT_APP:
                onEventPreNormalExitApp();
                break;
            case IMediaEvent.EVENT_NONE:
            default:
                break;
        }
    }

    /**
     * 多媒体服务初始化完成
     * @see IMediaEvent#EVENT_SERVICE_INITIALIZED
     */
    private void onEventServiceInitialized() {
        // 过滤重复事件
        if (mMediaInitCompleted) {
            LogUtil.v(TAG, "onEventServiceInitialized, Initialized");
            return;
        }

        mMediaInitCompleted = true;

        // 当前不在任何播放模式（触发后台播放）
        if (mAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
            tryRequestEnterMusicMode(
                    MediaEvent.name(IMediaEvent.EVENT_SERVICE_INITIALIZED));
        }
    }

    /**
     * 调试多媒体 API 接口
     * @see IMediaEvent#EVENT_DEBUG_MEDIA_API
     * @param type api 接口类型值
     * @param obj 附加数据对象
     */
    private void onEventDebugMediaApi(int type, Object obj) {
        // 未初始化完成不处理 Api 请求
        if (!mMediaInitCompleted) {
            // 标记请求播放的原因
            if (BroadcastApi.isPlayTypeValue(type)) {
                mRequestPlayMusicReason = (String) obj;
            }

            return;
        }

        // 根据不同的 api 类型值处理请求
        switch (type) {
            case ITypeValue.START_BACKGROUND_PLAYBACK:
                tryRequestEnterMusicMode(obj);
                break;
            case ITypeValue.TRIGGER_PLAY_PAUSE_TRACK:
                tryTriggerMusicPlayPause(obj);
                break;
            case ITypeValue.TRIGGER_PLAY_TRACK:
                tryTriggerMusicPlay(obj);
                break;
            case ITypeValue.TRIGGER_PAUSE_TRACK:
                tryTriggerMusicPause(obj);
                break;
            case ITypeValue.TRIGGER_SWITCH_PREV_TRACK:
                tryTriggerMusicPrevTrack(obj);
                break;
            case ITypeValue.TRIGGER_SWITCH_NEXT_TRACK:
                tryTriggerMusicNextTrack(obj);
                break;
            default:
                break;
        }
    }

    /**
     * 处理应用退出前事件
     * <p> 正常停止当前服务，避免退出后还被系统拉起；
     * @see IMediaEvent#EVENT_PRE_NORMAL_EXIT_APP
     */
    private void onEventPreNormalExitApp() {
        if (!hasClientConnected()) {
            stopSelf();
            LogUtil.v(TAG, "onPreNormalExitApp/stopSelf.");
            return;
        }

        // 通知对应的绑定者，进程将要退出了，请解除绑定；
        if (mReceptionBinder instanceof ReceptionBinder) {
            ReceptionBinder binder = (ReceptionBinder) mReceptionBinder;
            binder.postMediaEvent(IEvent.MEDIA_APP_WILL_EXIT, null, null);
        }
    }

    /**
     * 获取当前歌词信息
     * <p> 当前必须是在音乐播放模式才会有歌词信息；
     *
     * @param path 歌曲文件路径
     * @return 歌词行信息集合
     */
    public List<LyricsRow> getCurrentPlayLyricsInfo(String path) {
        if (TextUtils.isEmpty(path)) {
            return null;
        }

        if (!mAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
            return null;
        }

        return MediaModel.call()
                .playerModel()
                .getLyricsInfo(path);
    }

    /**
     * 尝试请求进入音乐播放模式
     * <pre>
     *    如果不在视频前台，可以强制播放音乐；
     *    如果本身已经在音乐播放状态，不处理；
     * </pre>
     *
     * @param obj 附加参数/Reason
     * @return -1 请求失败 / 0 请求成功
     */
    private int tryRequestEnterMusicMode(Object obj) {
        // 播放接口触发的原因
        String reason = IMedia.TriggerReason.EMPTY;
        if (obj instanceof String) {
            reason = (String) obj;
            LogUtil.d(TAG, "tryRequestEnterMusicMode: " + reason);
        } else {
            LogUtil.d(TAG, "tryRequestEnterMusicMode." );
        }

        // 检查当前进程多媒体类型（过滤场景）
        switch (mAppData.mMediaType) {
            case IMusicState.MEDIA_TYPE_VIDEO:
                // 视频显示状态不处理
                if (mAppData.mVideoUiShow) {
                    Log.d(TAG, "video is visible!");
                    return -1;
                }
            case IMusicState.MEDIA_TYPE_MUSIC:
                // 当前在播放状态，不处理
                if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    Log.d(TAG, "music playing!");
                    return 0;
                }
            default:
                break;
        }

        // 根据不同的原因过滤处理
        switch (reason) {
            case TriggerReason.USER_OPERATION:
            case TriggerReason.BROADCAST_API:
                mRequestPlayMusicReason = reason;
                break;
            case TriggerReason.START_SERVICE:
            default:
                break;
        }

        // 是否允许触发播放（需要初始化完成且请求原因不能为空）
        if (mMediaInitCompleted
                && !TextUtils.isEmpty(mRequestPlayMusicReason)) {
            // 后台只容许播放音乐文件
            mAppData.mMediaType = IMusicState.MEDIA_TYPE_MUSIC;

            // 请求焦点并开始后台播放
            PlatformUtils.requestPlayAudio(mContext);
            MediaModel.call().localzModel().onRequestAudioFocus();
            MediaModel.call().localzModel().requestSwitchMediaType();
        }

        return 0;
    }

    /**
     * 尝试请求进入音乐播放模式
     * <pre>
     *    如果是 binder（外部连接）触发的，不要直接调用；
     *    跨进程调用栈中使用反射接口，会导致反射接口执行失败；
     * </pre>
     *
     * @param obj 附加参数/Reason
     * @param isBinder 是 binder 触发调用（外部连接）；
     * @return -1 请求失败 / 0 请求成功 / 1 转发请求到消息队列
     * @see #tryRequestEnterMusicMode(Object)
     */
    /* public */ int tryRequestEnterMusicMode(Object obj, boolean isBinder) {
        if (isBinder) {
            Message msg = H0.obtainMessage(
                    H.MSG_TRY_REQUEST_ENTER_MUSIC_MODE, obj);

            H0.removeMessages(msg.what);
            H0.sendMessageDelayed(msg, 0);
            return 1;
        }

        return tryRequestEnterMusicMode(obj);
    }

    /**
     * 是否允许执行指定的媒体 api 接口；
     *
     * @param mediaApi {@link IMediaApi}
     * @return 允许执行/不允许
     */
    private boolean allowExecuteMediaApi(@IMediaApi String mediaApi) {
        LogUtil.v(TAG, "allowExecuteMediaApi: " + mediaApi);

        // 非音乐模式不处理
        if (!mAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
            Log.d(TAG, "not in music mode!");
            return false;
        }

        // 检查初始化状态（初始化完成才允许播放）
        if (mMediaInitCompleted) {
            // 如果外部请求原因为空（可能是异常退出后被系统拉起还没有焦点）
            if (TextUtils.isEmpty(mRequestPlayMusicReason)) {
                PlatformUtils.requestPlayAudio(mContext);
                MediaModel.call().localzModel().onRequestAudioFocus();
            }
            return true;
        }

        // 过滤处理播放相关的接口
        switch (mediaApi) {
            case IMediaApi.PLAY:
            case IMediaApi.PAUSE:
            case IMediaApi.PLAY_PAUSE:
            case IMediaApi.NEXT:
            case IMediaApi.PREV:
                // 标记请求播放原因是 mediaApi 触发
                mRequestPlayMusicReason = mediaApi;
                return false;
            default:
                break;
        }

        // 非播放接口（允许执行）
        return true;
    }

    /**
     * 尝试请求音乐播放信息
     * <p> 当前已经在音乐模式才处理；
     *
     * @param obj 附加参数/Reason
     */
    /* public */ void tryRequestMusicPlayInfo(Object obj) {
        if (!allowExecuteMediaApi(IMediaApi.PLAY_INFO)) {
            return;
        }

        MediaModel.call()
                .localzModel()
                .requestBroadcastMusicPlayInfo();
    }

    /**
     * 尝试触发音乐播放暂停事件
     * <p> 当前已经在音乐模式才处理；
     *
     * @param obj 附加参数/Reason
     */
    /* public */ void tryTriggerMusicPlayPause(Object obj) {
        if (!allowExecuteMediaApi(IMediaApi.PLAY_PAUSE)) {
            return;
        }

        // 暂停状态触发播放，播放状态触发暂停
        if (mAppData.isPlayState(IMusicState.PLAY_CMD_PLAY)) {
            MediaModel.call()
                    .localzModel()
                    .requestPlayControl(IMusicState.PLAY_CMD_PAUSE);
        } else {
            MediaModel.call()
                    .localzModel()
                    .requestPlayControl(IMusicState.PLAY_CMD_PLAY);
        }
    }

    /**
     * 尝试触发音乐播放事件
     * <p> 当前已经在音乐模式才处理；
     *
     * @param obj 附加参数/Reason
     */
    /* public */ void tryTriggerMusicPlay(Object obj) {
        if (!allowExecuteMediaApi(IMediaApi.PLAY)) {
            return;
        }

        MediaModel.call()
                .localzModel()
                .requestPlayControl(IMusicState.PLAY_CMD_PLAY);
    }

    /**
     * 尝试触发音乐暂停事件
     * <p> 当前已经在音乐模式才处理；
     *
     * @param obj 附加参数/Reason
     */
    /* public */ void tryTriggerMusicPause(Object obj) {
        if (!allowExecuteMediaApi(IMediaApi.PAUSE)) {
            return;
        }

        MediaModel.call()
                .localzModel()
                .requestPlayControl(IMusicState.PLAY_CMD_PAUSE);
    }

    /**
     * 尝试触发音乐上一曲事件
     * <p> 当前已经在音乐模式才处理；
     *
     * @param obj 附加参数/Reason
     */
    /* public */ void tryTriggerMusicPrevTrack(Object obj) {
        if (!allowExecuteMediaApi(IMediaApi.PREV)) {
            return;
        }

        // 切曲的时候清理掉播放进度记忆信息
        MediaModel.call()
                .playerModel()
                .onSetSeekTimeZero();
        MediaModel.call()
                .localzModel()
                .requestPlayControl(IMusicState.PLAY_CMD_PREV);
    }

    /**
     * 尝试触发音乐下一曲事件
     * <p> 当前已经在音乐模式才处理；
     *
     * @param obj 附加参数/Reason
     */
    /* public */ void tryTriggerMusicNextTrack(Object obj) {
        if (!allowExecuteMediaApi(IMediaApi.NEXT)) {
            return;
        }

        // 切曲的时候清理掉播放进度记忆信息
        MediaModel.call()
                .playerModel()
                .onSetSeekTimeZero();
        MediaModel.call()
                .localzModel()
                .requestPlayControl(IMusicState.PLAY_CMD_NEXT);
    }

    /**
     * 尝试触发音乐下一曲事件
     * <p> 当前已经在音乐模式才处理；
     *
     * @param obj 附加参数/Reason
     */
    /* public */ void tryTriggerMusicPlayMode(Object obj) {
        if (!allowExecuteMediaApi(IMediaApi.PLAY_MODE)) {
            return;
        }

        MediaModel.call()
                .localzModel()
                .requestSwitchRepeatMode(IMusicState.MEDIA_TYPE_MUSIC);
    }

    /**
     * 请求退出当前进程
     * @param reason 遗言
     */
    /* public */ void requestExitApp(String reason) {
        LogUtil.w(TAG, "requestExitApp: " + reason);

        MediaModel.call()
                .localzModel()
                .requestExitApp(-1);
    }

    /** 当前服务内部消息事件定义 **/
    private interface H {
        int MSG_NONE = -1;

        // 尝试请求进入音乐模式
        int MSG_TRY_REQUEST_ENTER_MUSIC_MODE = 1;
    }

    @Override
    protected boolean onHandleMessage(@NonNull Message msg) {
        switch (msg.what) {
            case H.MSG_TRY_REQUEST_ENTER_MUSIC_MODE:
                tryRequestEnterMusicMode(msg.obj);
                return true;
            case H.MSG_NONE:
            default:
                break;
        }

        return false;
    }

    /**
     * 让事件任务入队列（非及时执行）
     * <p> 事件处理转换机制，避免某些事件被直接调用；
     *
     * @param runnable 任务对象
     */
    /* public */ void postRunnable(@NonNull Runnable runnable) {
        if (Objects.isNull(H0)) {
            return;
        }

        H0.post(runnable);
    }

    /**
     * 更新音乐播放信息
     * @param info 参数对象
     */
    @HBusUtils.HBus(tag = IBusTag.UPDATE_MUSIC_PLAY_INFO)
    public void onUpdateMusicPlayInfo(@NonNull MusicInfo info) {
        // Binder 类型检查
        if (mReceptionBinder instanceof ReceptionBinder) {
            ReceptionBinder binder = (ReceptionBinder) mReceptionBinder;
            String id3Info = "@ID3:title:#:artist:#:album";

            // 歌曲名
            id3Info = id3Info.replaceFirst("title",
                    TextUtils.isEmpty(info.mTitle)? "null": info.mTitle);

            // 艺术家
            id3Info = id3Info.replaceFirst("#:artist",
                    TextUtils.isEmpty(info.mArtist)? "#:null": "#:" + info.mArtist);

            // 专辑名
            id3Info = id3Info.replaceFirst("#:album",
                    TextUtils.isEmpty(info.mAlbum)? "#:null": "#:" + info.mAlbum);

            LogUtil.v(TAG, "onUpdateMusicPlayInfo, " + info.mFileName + " " + id3Info);
            binder.postMediaEvent(IEvent.MUSIC_PLAY_INFO, info.mFilePath, id3Info);
        }
    }

    /**
     * 更新音乐播放状态
     * @param param 参数对象
     */
    @HBusUtils.HBus(tag = IBusTag.UPDATE_MUSIC_PLAY_STATE)
    public void onUpdateMusicPlayState(String param) {
        LogUtil.v(TAG, "onUpdateMusicPlayState: " + param);

        // Binder 类型检查/事件分发
        if (mReceptionBinder instanceof ReceptionBinder) {
            ReceptionBinder binder = (ReceptionBinder) mReceptionBinder;
            binder.postMediaEvent(IEvent.MUSIC_PLAY_STATE, param, null);
        }
    }

    /**
     * 更新音乐播放时间
     * @param param 参数对象
     */
    @HBusUtils.HBus(tag = IBusTag.UPDATE_MUSIC_PLAY_TIME)
    public void onUpdateMusicPlayTime(MediaTimeInfo param) {
        // adb install app-release.apk
        if (BaseMediaData.UID > 10000) {
            LogUtil.v(TAG, "onUpdateMusicPlayTime: "
                    + param.mCurrentTime + "/" + param.mTotalTime);
        }

        // Binder 类型检查/事件分发
        if (mReceptionBinder instanceof ReceptionBinder) {
            ReceptionBinder binder = (ReceptionBinder) mReceptionBinder;
            binder.postMediaEvent(IEvent.MUSIC_PLAY_TIME,
                    String.valueOf(param.mCurrentTime), String.valueOf(param.mTotalTime));
        }
    }

    /**
     * 更新音乐播放歌词
     * @param param 参数对象
     */
    @HBusUtils.HBus(tag = IBusTag.UPDATE_MUSIC_LYRICS_INFO)
    public void onUpdateMusicPlayLyrics(Object param) {
        LogUtil.v(TAG, "onUpdateMusicPlayLyrics: " + param);

        // 如果是 String 就是返回的歌词路径，否则就是歌词封装对象
        String lyricsFilePath = null;
        if (param instanceof String) {
            lyricsFilePath = (String) param;
        }

        // Binder 类型检查/事件分发
        if (mReceptionBinder instanceof ReceptionBinder) {
            ReceptionBinder binder = (ReceptionBinder) mReceptionBinder;
            binder.postMediaEvent(IEvent.MUSIC_LYRICS_INFO, lyricsFilePath, null);
        }
    }

    /**
     * 当前媒体进程就要退出了
     * <pre>
     *    这个总线事件接收函数必须运行在主线程，确保客户端接收即使；
     *    IEvent.MEDIA_APP_WILL_EXIT 的事件也需要直接阻塞分发；
     * </pre>
     */
    @HBusUtils.HBus(
            tag = IBusTag.MEDIA_APP_WILL_EXIT,
            threadMode = HBusUtils.ThreadMode.MAIN)
    public void onMediaAppWillExit() {
        LogUtil.v(TAG, "onMediaAppWillExit.");

        // Binder 类型检查/事件分发
        if (mReceptionBinder instanceof ReceptionBinder) {
            ReceptionBinder binder = (ReceptionBinder) mReceptionBinder;
            binder.postMediaEvent(IEvent.MEDIA_APP_WILL_EXIT, null, null);
        }
    }

    @Override
    protected void onDestroyObserver() {
        if (mServiceObserver != null) {
            getLifecycle().removeObserver(mServiceObserver);
            mServiceObserver = null;
        }
    }

    @Override
    protected void onStopForeground() {
        // 停止前提服务，移除通知。
        stopForeground(true);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        LogUtil.d(TAG, "onDestroy.");

        // 取消注册媒体总线
        HBusUtils.unregister(this);
    }
}
