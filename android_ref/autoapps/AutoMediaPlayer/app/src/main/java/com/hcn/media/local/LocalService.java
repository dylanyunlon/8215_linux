package com.hcn.media.local;

import android.Configures.HConfig;
import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.bluetooth.BluetoothProfile;
import android.carstatus.CarStatus;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.media.AudioAttributes;
import android.media.AudioFocusRequest;
import android.media.AudioManager;
import android.media.AudioManager.OnAudioFocusChangeListener;
import android.os.Binder;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.RemoteViews;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.core.app.NotificationCompat;
import androidx.lifecycle.Lifecycle;

import com.hcn.AutoMediaPlayer.BuildConfig;
import com.hcn.AutoMediaPlayer.R;
import com.hcn.auto.AutoStatus;
import com.hcn.auto_compat.bluetooth.BluetoothCompat;
import com.hcn.common.misc.HBusUtils;
import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HHandler;
import com.hcn.config.Feature;
import com.hcn.media.R3;
import com.hcn.media.base.IMedia;
import com.hcn.media.base.Preferences;
import com.hcn.media.base.xbus.IBusTag;
import com.hcn.media.extend.base.IExtend;
import com.hcn.media.folder.MediaFilePathScan;
import com.hcn.media.local.base.MediaService;
import com.hcn.media.local.observer.MediaServiceObserver;
import com.hcn.media.local.observer.MusicProviderObserver;
import com.hcn.media.main.MusicUI;
import com.hcn.media_base.HMediaConfig;
import com.hcn.media_base.IAutoEvent;
import com.hcn.media_base.IMediaBroadcast;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_base.impl.MediaEvent;
import com.hcn.media_common.HBroadcastEx.SpecialChain;
import com.hcn.media_common.HEventBus;
import com.hcn.media_common.HMessage;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.thread.HTaskRunnable;
import com.hcn.media_common.utils.MediaID3Util;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.ListSceneManager;
import com.hcn.media_data.MusicRegInfo;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_data.debug.MediaDebugger;
import com.hcn.media_data.folder.AbcFolderUtils;
import com.hcn.media_data.folder.FilePathScanManager;
import com.hcn.media_data.storage.IStorageDevice;
import com.hcn.media_data.storage.StorageDeviceEx;
import com.hcn.media_data.ui.base.PageDataKV;
import com.hcn.media_model.MediaModel;
import com.hcn.media_model.base.IPlayerModel;
import com.hcn.media_model.eq.EQMediaController;
import com.hcn.media_theme.Argument;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.plugin.ApkClassLoaderEx;
import com.hcn.skinx.SkinX;

import java.io.File;
import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * 媒体本地服务组件
 * <pre>
 *    绑定多媒体数据提供服务，获取数据列表；
 *    支持后台播放，接受处理系统广播事件等；
 * </pre>
 *
 * @author 65821
 */
public class LocalService extends MediaService implements IMediaBroadcast {
    private static final String SINGLE_TAG = "SingleTask";

    /**
     * 媒体通知菜单信息标记
     * <p> NotificationManager
     */
    private static final String MUSIC_PLAYER_NOTIFY_TAG = "HMedia";
    private static final int MUSIC_PLAYER_NOTIFY_ID = 10;

    private static final int SEEK_STEP = 5000;

    /**
     * 当前服务 Notification 信道的 ID
     * <pre>
     *     信道 ID 必须是唯一的；
     *     主要是为了设置前台服务，保活使用；
     * </pre>
     */
    private final String NOTIFICATION_CHANNEL_ID = "MM_PLAY_2023-0307-1630";

    /** 当前服务观察者对象 **/
    MediaServiceObserver mServiceObserver = null;

    /** 音乐信息观察者对象 */
    MusicProviderObserver mProviderObserver = null;

    /** 接收处理跨进程广播 **/
    private ExternalActionReceiver mExternalActionReceiver = null;
    private VoiceControlReceiver mVoiceControlReceiver = null;

    /** 音频焦点处理监听对象 **/
    protected AudioFocusChangeListener mAudioFocusListener = null;
    private final AudioHandler mAudioHandler = new AudioHandler(Looper.getMainLooper());
    private AudioFocusRequest mAudioFocusRequest = null;

    private Handler mUpdateTimeHandler = null;
    private Handler mSmartControlHandler = null;

    /** 处理 ID3 异步任务 **/
    private AsyncHandler mAsyncHandler = null;
    private ExecutorService mSyncID3ThreadPool = null;

    /**
     * 是否在接收到倒车打开广播状态
     * <pre>
     *    注意这个和在倒车状态有些差别，它只代表当前是在收到 ReverseOn 和 ReverseOff 状态广播之间；
     *    可能已经在倒车状态，但是广播可能还没收到（有延时）；
     * </>
     */
    private boolean mInReverseOnBroadcast = false;

    /** 定时更新当前播放信息时间戳 **/
    private final Runnable mTimeRunnable = new Runnable() {

        @Override
        public void run() {
            int tmpCurrentTime = mAppData.mPlayTimeInfo.mCurrentTime;

            if (mAppData.mPlayTimeInfo.mTotalTime <= 0) {
                mAppData.mPlayTimeInfo.mTotalTime =
                        MediaModel.call().playerModel().getTotalTime();
            }

            int currentPosition =
                    MediaModel.call()
                            .playerModel().getCurrentPosition();
            if (tmpCurrentTime != 0 && currentPosition <= 0) {
                // currentPosition <= 0 都是无效的值（会触发显示错误）
                LogUtil.e(TAG, ">>>  gets the current playback position: 0.");
            } else {
                mAppData.mPlayTimeInfo.setCurrentTime(
                        currentPosition, true, "timer");
            }

            onMediaEvent(IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME, null, null);
            mUpdateTimeHandler.postDelayed(this, 1010);
        }
    };

    /**
     * Smart 控制事件
     * <p> 通过消息队列做延时处理用；
     */
    private final Runnable mSmartControlRunnable = () -> {
        // 取消控制事件
        onMediaEvent(
                IMediaEvent.EVENT_CANCEL_SMART_CONTROL,
                null,
                null);
    };

    /** 通知栏状态显示 **/
    private Notification mMusicNotification = null;
    private RemoteViews mMusicRemoteViews = null;
    private NotificationCompat.Builder mNotifyBuilder = null;
    private NotificationManager mNotifyManager = null;

    /** 是否在执行退出 Application 任务 **/
    private boolean mExecutingExitAppTask = false;

    /**
     * 默认无参构造函数
     **/
    public LocalService() {
        super();

        // 创建对外 Binder 接口对象
        mMediaBinder = new MediaBinder(this);
    }

    /**
     * 关联应用上下文
     * <pre>
     *    注意：服务的构造、attach()、onCreate() 函数都是在同一个方法中调用的；
     *    ActivityThread::handleCreateService(CreateServiceData data);
     *    在 ActivityThread::H::CREATE_SERVICE 消息处理（bindService）
     * </pre>
     *
     * @param newBase 上下文环境
     */
    @Override
    protected void attachBaseContext(Context newBase) {
        // 支持检查扩展皮肤包（逻辑扩展）
        initAndCheckServiceExtend();

        super.attachBaseContext(newBase);
    }

    /**
     * 创建观察者
     * <pre>
     *    可以在此处扩展服务功能；
     *    用来简洁当前服务代码使用，避免代码过于膨胀；
     * </pre>
     *
     * @see android.app.Service#attachBaseContext(Context);
     */
    @Override
    protected void onCreateObserver() {

        // 创建音乐信息观察者
        if (Objects.isNull(mProviderObserver)) {
            mProviderObserver = new MusicProviderObserver(this);
            // 非配置版本，不启动观察
            if (BuildConfig.SUPPORT_MUSIC_PROVIDER) {
                mProviderObserver.startProviderObserver(new MusicProviderObserver.IProviderCallback() {
                    @Override
                    public void onPlayModeUpdate(int playMode) {
                        doChangeRepeatMode(playMode);
                    }
                });
            }
        }

        // 避免被重复创建
        if (!Objects.isNull(mServiceObserver)) {
            return;
        }

        // 创建服务观察者
        mServiceObserver = new MediaServiceObserver(this, this, null);
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

    @RequiresApi(api = Build.VERSION_CODES.R)
    @Override
    public void onCreate() {
        super.onCreate();
        LogUtil.e(TAG, ">>>>> onCreate ");

        // 创建消息处理器
        mUpdateTimeHandler = new Handler(Looper.getMainLooper());
        mSmartControlHandler = new Handler(Looper.getMainLooper());

        // 异步任务线程池
        if (HMediaConfig.USE_THREAD_POOL_SYNC_ID3) {
            mAsyncHandler = new AsyncHandler(getMainLooper());
            mSyncID3ThreadPool = new ThreadPoolExecutor(0, 1,
                    30, TimeUnit.SECONDS, new LinkedBlockingQueue<Runnable>());
        } else {
            // 处理异步任务的 Handler
            HandlerThread handlerThread = new HandlerThread("LocalService");
            handlerThread.start();
            mAsyncHandler = new AsyncHandler(handlerThread.getLooper());
        }

        // 注册广播接收者
        localOtherRegisterReceiver();
        localVoiceRegisterReceiver();

        // 检查初始化服务扩展类
        initAndCheckServiceExtend(true, true);
    }

    /**
     * 初始化并检查扩展逻辑包
     * <p> 注意扩展业务逻辑都在皮肤包中；
     * @see #initAndCheckServiceExtend(boolean, boolean)
     */
    private void initAndCheckServiceExtend() {
        initAndCheckServiceExtend(false, false);
    }

    /**
     * 初始化并检查扩展逻辑包
     * <p> 注意扩展业务逻辑都在皮肤包中；
     *
     * @param checked 是否是检查调用
     * @param delayed 延迟处理
     */
    private void initAndCheckServiceExtend(boolean checked, boolean delayed) {
        if (mServiceExtend != null) {
            return;
        } else {
            if (checked && delayed) {
                H0.sendEmptyUniqueMessageDelayed(
                        MsgEx.MSG_INIT_SERVICE_EXTEND, 120);
                return;
            }
        }

        // 等待皮肤包加载完成（阻塞）
        SkinX.instance().waitLoadCompleted(checked? 10: 80);

        // 支持检查扩展皮肤包（逻辑扩展）
        String pageExtendResConfigName = "local_service_extend";
        if (xBoolean(pageExtendResConfigName)) {
            ApkClassLoaderEx classLoader = xClassLoader();
            if (!Objects.isNull(classLoader)) {
                String serviceExtendClassName =
                        IExtend.MEDIA_PACKAGE_NAME + ".LocalServiceExtend";
                mServiceExtend = classLoader.newServiceExtendInterface(serviceExtendClassName, this);
            }

            LogUtils.iTag(TAG, mServiceExtend != null?
                    "Has LocalServiceExtend class.": "No LocalServiceExtend class.");
        }

        // 是检查后触发（让服务先跑起来）
        if (mServiceExtend != null) {
            mServiceExtend.onInitialized(getLifecycle().getCurrentState());
        }
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

    /** 当前服务内部消息事件定义 **/
    private interface MsgEx extends H {
        int MSG_UPDATE_NOTIFICATION = MSG_LAST + 1;
        int MSG_CANCEL_NOTIFICATION = MSG_LAST + 2;

        int MSG_UNSUPPORT_VIDEO_CODE = MSG_LAST + 3;
        int MSG_UNSUPPORT_AUDIO_CODE = MSG_LAST + 4;
        int MSG_UNSUPPORT_SEEKABLE = MSG_LAST + 5;

        int MSG_UNKNOWN_ERROR = MSG_LAST + 6;
        int MSG_ERROR_FILE_NOT_EXIST = MSG_LAST + 7;
        int MSG_GOTO_NEXT_MEDIA = MSG_LAST + 8;

        int MSG_HANDLE_ACC_STATUS = MSG_LAST + 9;
        int MSG_POLLING_ACC_STATUS = MSG_LAST + 10;

        int MSG_INIT_SERVICE_EXTEND = MSG_LAST + 11;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        LogUtil.e(TAG, ">>>>> onStartCommand: AccStatus = " + !mIsPowerOff);

        // [每 6 秒查询一次 ACC 状态]
        H0.sendEmptyUniqueMessageDelayed(
                MsgEx.MSG_POLLING_ACC_STATUS, 6);

        return super.onStartCommand(intent, flags, startId);
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
            case IMediaEvent.EVENT_MUSIC_PLAYER_PREPARING:
            case IMediaEvent.EVENT_VIDEO_PLAYER_PREPARING:
            case IMediaEvent.EVENT_REQUEST_MEDIA_PAUSE:
                break;
            default:
                return;
        }

        // 打印事件名称（非关键事件只打印 ID 值）
        LogUtil.v(TAG, "ServiceObserver/event: " + MediaEvent.name(event));

        switch (event) {
            // 音乐成功触发切曲动作
            case IMediaEvent.EVENT_MUSIC_PLAYER_PREPARING:
                sendLocalBroadcast(event);
                break;
            case IMediaEvent.EVENT_VIDEO_PLAYER_PREPARING:
                onVideoPlayerPreparing(obj1);
                break;
            case IMediaEvent.EVENT_REQUEST_MEDIA_PAUSE:
                doShouldPauseEvent(false);
                break;
            case IMediaEvent.EVENT_NONE:
            default:
                break;
        }
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
                    .setContentTitle(getResources().getText(
                            R3.string.media_notification_title))
                    .setContentText(getResources().getText(
                            R3.string.media_notification_info));
        } else {
            builder = new Notification.Builder(this)
                    .setSmallIcon(R.drawable.ic_media_24)
                    .setContentTitle(getResources().getText(
                            R3.string.media_notification_title))
                    .setContentText(getResources().getText(
                            R3.string.media_notification_info));
        }

        return builder.build();
    }

    /**
     * 服务自带的消息处理函数
     * <p> 只处理 {@link H} 接口定义的消息；
     *
     * @param msg 消息对象
     */
    @Override
    protected boolean onHandleMessage(@NonNull Message msg) {
        switch (msg.what) {
            case MsgEx.MSG_UPDATE_NOTIFICATION:
                onUpdateNotification();
                return true;
            case MsgEx.MSG_CANCEL_NOTIFICATION:
                cancelNotification();
                return true;
            case MsgEx.MSG_UNSUPPORT_VIDEO_CODE:
                onToastText(R3.string.tip_unsupport_video);
                return true;
            case MsgEx.MSG_UNSUPPORT_AUDIO_CODE:
                onToastText(R3.string.tip_unsupport_audio);
                return true;
            case MsgEx.MSG_UNSUPPORT_SEEKABLE:
                onToastText(R3.string.tip_unsupport_seek);
                return true;
            case MsgEx.MSG_UNKNOWN_ERROR:
                onToastText(R3.string.tip_unsupport_file);
                return true;
            case MsgEx.MSG_ERROR_FILE_NOT_EXIST:
                onToastText(R3.string.tip_file_not_exist);
                return true;
            case MsgEx.MSG_GOTO_NEXT_MEDIA:
                onGotoNextMediaEvent();
                return true;
            case MsgEx.MSG_POLLING_ACC_STATUS:
                onMsgPollingAccStatus();
                return true;
            // [过时的接口]
            case MsgEx.MSG_HANDLE_ACC_STATUS:
                writeCurrentMediaTime(true, 9);
                return true;
            case MsgEx.MSG_INIT_SERVICE_EXTEND:
                initAndCheckServiceExtend(true, false);
                return true;
            default:
                break;
        }

        return super.onHandleMessage(msg);
    }

    /** 处理播放结束自动切换媒体对象事件 **/
    private void onGotoNextMediaEvent() {
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
            onLocalMusicPlayControl(IMusicState.PLAY_CMD_NEXT);
        } else if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            if (mAppData.mIsMediaPlayerLocked) {
                LogUtil.d(TAG, "   onGotoNextMediaEvent: mIsMediaPlayerLocked!");
                H0.sendEmptyUniqueMessageDelayed(MsgEx.MSG_GOTO_NEXT_MEDIA, 250);
            } else {
                onLocalVideoPlayControl(IMusicState.PLAY_CMD_NEXT);
            }
        }
    }

    /**
     * 本地服务 dump 参数说明
     * <p> dumpsys activity service ${class name} -h
     *
     * @param fout 您应该将状态 dump 到的 PrintWriter。您返回后这里将为您关闭。
     */
    private void dumpServiceUsage(@NonNull PrintWriter fout) {
        fout.println(
                "usage: \n" +
                "    dumpsys activity service com.hcn.media.local.LocalService [ARGS] \n" +
                "        -h: shows this help\n" +
                "        -d: debug level\n" +
                "        -e media play control event\n" +
                "        ...\n");
    }

    /**
     * 倾倒信息到终端
     * <p> dumpsys activity service com.hcn.media.local.LocalService
     *
     *@param fd dump 要发送到的原始文件描述符。
     *@param fout 您应该将状态 dump 到的 PrintWriter。您返回后这里将为您关闭。
     *@param args dump 请求的其他参数。
     */
    @Override
    protected void dump(@NonNull FileDescriptor fd,
                        @NonNull PrintWriter fout,
                        @Nullable String[] args) {
        if (mContext.checkCallingOrSelfPermission(
                Manifest.permission.DUMP) != PackageManager.PERMISSION_GRANTED) {
            fout.println("Permission Denial: can't" +
                    " 'dumpsys activity service com.hcn.media.local.LocalService' " +
                    "from from pid=" + Binder.getCallingPid() + ", uid=" + Binder.getCallingUid());
            return;
        }

        int opti = 0;
        while (opti < args.length) {
            String opt = args[opti];
            if (opt == null || opt.length() == 0 || opt.charAt(0) != '-') {
                break;
            }

            opti++;
            if ("-h".equals(opt)) {
                dumpServiceUsage(fout);
                return;
            }

            if ("-d".equals(opt)) {
                int args_length = args.length - opti + 1;
                args_length = Math.min(args_length, 3);
                String[] debug_args = new String[args_length];

                System.arraycopy(args, opti - 1, debug_args, 0, args_length);
                MediaDebugger.dumpDebugEvent(fout, debug_args);
                return;
            } else if ("-e".equals(opt)) {
                int args_length = args.length - opti + 1;
                args_length = Math.min(args_length, 3);
                String[] debug_args = new String[args_length];

                System.arraycopy(args, opti - 1, debug_args, 0, args_length);
                MediaDebugger.dumpMediaEvent(fout, debug_args);
                return;
            }
        }

        fout.println("CurrentDevice:");
        fout.println("    device = " + mAppData.mCurrentDevice.getFilePath());
        fout.println("    music size = " + mAppData.mCurrentDevice.getMusicInfoList().size());
        fout.println("    video size = " + mAppData.mCurrentDevice.getMusicInfoList().size());

        fout.println("MediaType = " + mAppData.mediaType());

        fout.println("MediaInfo:");
        if (Objects.isNull(mAppData.currentMediaInfo())) {
            fout.println("    playback information (null)");
            return;
        }

        fout.println("    file = " + mAppData.currentMediaInfo().mFilePath);

        // 如果是音乐显示 ID3 信息
        if (mAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
            fout.println("    artist = " + mAppData.currentMediaInfo().mArtist);
            fout.println("    album = " + mAppData.currentMediaInfo().mAlbum);
            fout.println("    title = " + mAppData.currentMediaInfo().mTitle);
        }

        fout.println("    duration = " + mAppData.mPlayTimeInfo.mTotalTime);
        fout.println("    state = " + mAppData.mediaPlayState());
    }

    @Override
    protected void onStopForeground() {
        // 停止前提服务，移除通知。
        stopForeground(true);
    }

    @Override
    protected void onDestroyObserver() {
        if (mServiceObserver != null) {
            getLifecycle().removeObserver(mServiceObserver);
            mServiceObserver = null;
        }
        if (mProviderObserver != null) {
            mProviderObserver.stopProviderObserver();
        }
    }

    @Override
    public void onDestroy() {
        LogUtil.e(TAG, ">>>>> onDestroy ");
        super.onDestroy();

        mUpdateTimeHandler.removeCallbacksAndMessages(null);
        mSmartControlHandler.removeCallbacksAndMessages(null);
        mControlHandler.removeCallbacksAndMessages(null);

        onClearAudioEvent();
        cancelNotification();

        localOtherUnregisterReceiver();
        localVoiceUnregisterReceiver();
        EQMediaController.instance().onPauses();
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        super.onTaskRemoved(rootIntent);
        LogUtil.v(TAG, "onTaskRemoved.");
    }

    /**
     * 处理媒体事件
     * @see MediaBinder
     *
     * @param eventId {@link IMediaEvent}
     * @param wParam 附加参数 1
     * @param lParam 附加参数 2
     */
    @Override
    public void onMediaEvent(int eventId, Object wParam, Object lParam) {
        super.onMediaEvent(eventId, wParam, lParam);

        // 过滤打印信息
        if (eventId != IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME) {
            LogUtil.low_i(TAG, ">>>>> onMediaEvent, event: " + eventId);
        }

        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_COMPLETION: {
                mAppData.mMediaPlayState = IMusicState.E_PLAY_STATE_STOP;
                eventId = IMediaEvent.EVENT_CHANGE_PLAY_STATE;
                mUpdateTimeHandler.removeCallbacksAndMessages(null);

                // [播放完了: 那么进度需要重置, 下次记忆如果还是播放这个文件, 那么就从头开始]
                if (null != mAppData.mCurrentMediaInfo) {
                    writeMediaTime(mAppData.mMediaType,
                            mAppData.mCurrentMediaInfo.mFilePath,
                            0,
                            102);

                    // [重置 mAppData.mCurrentMediaInfo, 避免 onSetSeektimeZero() 重复写]
                    mAppData.mCurrentMediaInfo = null;
                }

                H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);
                H0.sendEmptyMessageDelayed(MsgEx.MSG_GOTO_NEXT_MEDIA, 1000);
                MediaModel.call().playerModel().onSetSeekTimeZero();
                break;
            }

            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_PLAY: {
                mAppData.mMediaPlayState = IMusicState.E_PLAY_STATE_PLAY;
                if (lParam instanceof String) {
                    if ("only-state".equals(lParam)) {
                        // [只需要更新状态，不做其他处理]
                        return;
                    }
                }

                eventId = IMediaEvent.EVENT_CHANGE_PLAY_STATE;
                mUpdateTimeHandler.removeCallbacksAndMessages(null);
                mUpdateTimeHandler.post(mTimeRunnable);

                if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
                    H0.sendEmptyMessage(MsgEx.MSG_UPDATE_NOTIFICATION);
                }
                break;
            }

            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_PAUSE: {
                mAppData.mMediaPlayState = IMusicState.E_PLAY_STATE_PAUSE;
                eventId = IMediaEvent.EVENT_CHANGE_PLAY_STATE;

                mUpdateTimeHandler.removeCallbacksAndMessages(null);
                if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
                    H0.sendEmptyMessage(MsgEx.MSG_UPDATE_NOTIFICATION);
                }
                break;
            }

            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_STOP: {
                mAppData.mMediaPlayState = IMusicState.E_PLAY_STATE_STOP;
                eventId = IMediaEvent.EVENT_CHANGE_PLAY_STATE;

                mUpdateTimeHandler.removeCallbacksAndMessages(null);
                if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
                    H0.sendEmptyMessage(MsgEx.MSG_UPDATE_NOTIFICATION);
                }
                return;
            }

            case IMediaEvent.EVENT_CANCEL_NOTIFICATION: {
                H0.sendEmptyMessage(MsgEx.MSG_CANCEL_NOTIFICATION);
                return;
            }

            case IMediaEvent.EVENT_UNSUPPORT_VIDEO_CODE: {
                H0.sendEmptyMessage(MsgEx.MSG_UNSUPPORT_VIDEO_CODE);
                return;
            }

            case IMediaEvent.EVENT_UNSUPPORT_AUDIO_CODE: {
                H0.sendEmptyMessage(MsgEx.MSG_UNSUPPORT_AUDIO_CODE);
                return;
            }

            case IMediaEvent.EVENT_UNSUPPORT_SEEKABLE: {
                H0.sendEmptyMessage(MsgEx.MSG_UNSUPPORT_SEEKABLE);
                return;
            }

            case IMediaEvent.EVENT_CODE_UNSUPPORT: {
                H0.sendEmptyMessage(MsgEx.MSG_UNKNOWN_ERROR);
                return;
            }

            case IMediaEvent.EVENT_ERROR_FILE_NOT_EXIST: {
                H0.sendEmptyMessage(MsgEx.MSG_ERROR_FILE_NOT_EXIST);

                mAppData.mFileNotExistCount++;
                if (mAppData.mFileNotExistCount < 3) {
                    // [连续3次播放发现不存在的文件，将会触发扫描线程]
                    return;
                }

                if (null != wParam) {
                    if (!(wParam instanceof String)) {
                        // 非法的类型
                        return;
                    }

                    StorageDeviceEx storageDevice = getStorageDevice((String) wParam);
                    if (null != storageDevice) {
                        requestScanStorageDevice(storageDevice);
                    }
                }
                break;
            }

            default:
                break;
        }

        sendLocalBroadcast(eventId);
    }

    @SuppressLint("UnspecifiedRegisterReceiverFlag")
    private void localOtherRegisterReceiver() {
        mExternalActionReceiver = new ExternalActionReceiver();

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(SpecialChain.ACTION_MESSAGE_CALLBACK);

        intentFilter.addAction(IConstant.ACTION_NOTIFICATION_PREV);
        intentFilter.addAction(IConstant.ACTION_NOTIFICATION_PLAYPAUSE);
        intentFilter.addAction(IConstant.ACTION_NOTIFICATION_NEXT);
        intentFilter.addAction(IConstant.ACTION_NOTIFICATION_SHOW);
        intentFilter.addAction(IConstant.ACTION_NOTIFICATION_CANCEL);

        intentFilter.addAction(ACTION_VOICE_EVENT_PLAY);
        intentFilter.addAction(ACTION_VOICE_EVENT_PAUSE);
        intentFilter.addAction(ACTION_VOICE_EVENT_VIDEO_PLAY);
        intentFilter.addAction(ACTION_VOICE_EVENT_VIDEO_PAUSE);
        intentFilter.addAction(ACTION_VOICE_EVENT_NEXT);
        intentFilter.addAction(ACTION_VOICE_EVENT_PREV);
        intentFilter.addAction(ACTION_VOICE_EVENT_MODE_LOOP_ALL);
        intentFilter.addAction(ACTION_VOICE_EVENT_MODE_LOOP_ONE);
        intentFilter.addAction(ACTION_VOICE_EVENT_MODE_RANDOM);
        intentFilter.addAction(ACTION_VOICE_EVENT_REWIND);
        intentFilter.addAction(ACTION_VOICE_EVENT_FAST_FORWARD);

        intentFilter.addAction(ACTION_EVENT_K_SCROLL_R);
        intentFilter.addAction(ACTION_EVENT_K_SCROLL_L);
        intentFilter.addAction(ACTION_EVENT_K_ENTER);

        // 刷新路径广播
        intentFilter.addAction(ACTION_REFRESH_PATH);

        intentFilter.addAction(CarStatus.ACTION_ACC);
        intentFilter.addAction(CarStatus.ACTION_PARKING);
        intentFilter.addAction(CarStatus.ACTION_REVSTATUS);

        // 监听蓝牙音频状态改变广播
        intentFilter.addAction(BluetoothCompat.HeadsetClient.ACTION_AUDIO_STATE_CHANGED);

        registerReceiver(mExternalActionReceiver, intentFilter);
    }

    private void localOtherUnregisterReceiver() {
        unregisterReceiver(mExternalActionReceiver);
    }

    /**
     * 存储设备卸载
     * <p> IMediaEvent.EVENT_MEDIA_UNMOUNTED
     *
     * @param storageDevice
     */
    private void onUnloadMediaPathEvent(StorageDeviceEx storageDevice) {
        if (Objects.isNull(storageDevice)) {
            return;
        }

        LogUtil.d(TAG, ">>>>> onUnloadMediaPathEvent path: " + storageDevice.mFilePath);

        // 重置存储设备数据状态
        storageDevice.updateMounted(isMounted(storageDevice.mFilePath));
        storageDevice.mMusicInfoList.clear();
        storageDevice.mVideoInfoList.clear();
        storageDevice.mAlbumListMap.clear();
        storageDevice.mArtistListMap.clear();
        storageDevice.mPathListMap.clear();

        // 清空音乐播放外部任务
        if (!TextUtils.isEmpty(mAppData.mSingleMusicFilePath)
                && mAppData.mSingleMusicFilePath.startsWith(storageDevice.getFilePath())) {
            LogUtil.v(TAG, ">>>>> onUnloadMediaPathEvent"
                    + " mAppData.mSingleMusicFilePath: " + mAppData.mSingleMusicFilePath);
            mAppData.mSingleMusicPlay = false;
            mAppData.mSingleMusicFilePath = "";
        }

        // 清空视频播放外部任务
        if (!TextUtils.isEmpty(mAppData.mSingleVideoFilePath)
                && mAppData.mSingleVideoFilePath.startsWith(storageDevice.getFilePath())) {
            LogUtil.v(TAG, ">>>>> onUnloadMediaPathEvent"
                    + " mAppData.mSingleVideoFilePath: " + mAppData.mSingleVideoFilePath);

            mAppData.mSingleVideoPlay = false;
            mAppData.mSingleVideoFilePath = "";
        }

        // 不是当前播放设备卸载，并且是在 ACC-OFF 状态
        if (storageDevice != mAppData.mCurrentDevice || mIsPowerOff) {
            // ACC-OFF 会暂停播放任务，但是不会停止播放任务；
            return;
        }

        // 清理当前播放设备状态
        writeCurrentMediaTime(true, 1);
        MediaModel.call()
                .playerModel()
                .onPlayControlEvent(IMusicState.PLAY_CMD_STOP, 6);

        // 校准当前播放选择设备（当前设备被移除）
        if (!mAppData.mCurrentDevice.isMounted()
                || !mAppData.mCurrentDevice.isLoading()) {
            // 跳转到一个有效存储设备
            mAppData.mCurrentDevice = getValidStorageDevice();
            mAppData.mSelectedDevice = mAppData.mCurrentDevice;
        }

        // 检查更新当前存储设备（播放）
        mAppData.mAllowResumePlay = false;
        updateCurrentManager();
    }

    // 只有扫描结束后调用
    private boolean onChangeCurrentStorage() {
        if (!checkoutStorage(mAppData.mSelectedDevice)) {
            mAppData.mSelectedDevice = getValidStorageDevice();
            mAppData.mCurrentDevice = getValidStorageDevice();

            updateCurrentManager();
            return true;
        }

        return false;
    }

    /**
     * 处理媒体扫描数据进程的状态广播
     * @see SpecialChain#ACTION_MESSAGE_CALLBACK:
     *
     * @param eventId {@link IMediaEvent} 事件 ID
     * @param strPath 事件参数
     */
    private void onActionMessageCallback(int eventId, String strPath) {
        LogUtil.i(TAG, ">>>> onMessageEvent eventId: " + eventId + " strPath: " + strPath);

        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_MOUNTED:
                onMediaStorageDeviceMounted(strPath);
                break;

            case IMediaEvent.EVENT_MEDIA_UNMOUNTED:
                onMediaStorageDeviceUnmounted(strPath);
                break;

            case IMediaEvent.EVENT_MEDIA_LOADING_START:
                onStartLoadingStorageDevice(strPath);
                break;

            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
                onStorageDeviceLoadingCompleted(strPath);
                break;

            case IMediaEvent.EVENT_ID3_SCAN_FINISHED:
                onMediaId3ScanFinished(strPath);
                break;

            case IMediaEvent.EVENT_DEEP_SLEEP_STATUS:
                onEnterDeepSleepStatus();
                break;

            // 远端不发送这 2 个消息（HMediaService）
            case IMediaEvent.EVENT_CHANGE_REPEAT_MODE:
            case IMediaEvent.EVENT_UPDATE_AUTO_BRAKE_STATUS:
            default:
                break;
        }
    }

    /**
     * 媒体存储设备挂载事件
     * @see IMediaEvent#EVENT_MEDIA_MOUNTED
     *
     * @param strPath 存储设备路径
     */
    private void onMediaStorageDeviceMounted(String strPath) {
        LogUtil.i(TAG, " -- EVENT_MEDIA_MOUNTED: " + strPath);
        sendNotifyMediaState();
        sendLocalBroadcast(IMediaEvent.EVENT_MEDIA_MOUNTED, strPath);
    }

    /**
     * 媒体存储设备卸载事件
     * @see IMediaEvent#EVENT_MEDIA_UNMOUNTED
     *
     * @param strPath 存储设备路径
     */
    private void onMediaStorageDeviceUnmounted(String strPath) {
        LogUtil.i(TAG, " -- EVENT_MEDIA_UNMOUNTED: " + strPath);

        if (strPath.contains(IConstant.PATH_USB_PREFIX) && isMounted(IConstant.PATH_USB)) {
            if (mAppData.mCurrentMediaInfo != null
                    && mAppData.mCurrentMediaInfo.mFilePath.contains(strPath)) {
                // 移除当前正在播放的 USB 盘符触发停止播放
                int command = IMusicState.PLAY_CMD_STOP;
                switch (mAppData.mMediaType) {
                    case IMusicState.MEDIA_TYPE_MUSIC:
                        onLocalMusicPlayControl(command);
                        break;
                    case IMusicState.MEDIA_TYPE_VIDEO:
                        onLocalVideoPlayControl(command);
                        break;
                    default:
                        break;
                }
            }

            return;
        }

        onUnloadMediaPathEvent(getStorageDevice(strPath));
        sendNotifyMediaState();
        sendLocalBroadcast(IMediaEvent.EVENT_MEDIA_UNMOUNTED, strPath);
    }

    /**
     * 远程数据服务通知开始扫描目标存储设备
     * @see IMediaEvent#EVENT_MEDIA_LOADING_START
     *
     * @param strPath 存储设备路径
     */
    private void onStartLoadingStorageDevice(String strPath) {
        LogUtil.i(TAG, " -- EVENT_MEDIA_LOADING_START: " + strPath);

        StorageDeviceEx fileManager = getStorageDevice(strPath);
        fileManager.updateLoading(isLoading(strPath));
        fileManager.updateMounted(isMounted(strPath));

        if (fileManager != mUsbDevice) {
            fileManager.mMusicInfoList.clear();
            fileManager.mVideoInfoList.clear();
        }

        fileManager.mAlbumListMap.clear();
        fileManager.mArtistListMap.clear();
        fileManager.mMusicFavoriteList.clear();
        fileManager.mFileScanState.mIsLoadFinished = getFileScanState(fileManager.mFilePath);
        fileManager.mID3ParseState.mIsLoadFinished = getID3ParseState(fileManager.mFilePath);

        // [当前播放设备没有数据, 更换目标存储设备]
        if (!checkoutStorage(mAppData.mCurrentDevice)) {
            mAppData.mCurrentDevice = getValidStorageDevice();
            mAppData.mSelectedDevice = mAppData.mCurrentDevice;
        }

        sendLocalBroadcast(IMediaEvent.EVENT_MEDIA_LOADING_START, strPath);
    }

    /**
     * 远程数据服务通知目标存储设备扫描完成
     * @see IMediaEvent#EVENT_MEDIA_LOADING_COMPLETE
     * @param strPath 存储设备路径
     */
    private void onStorageDeviceLoadingCompleted(String strPath) {
        StorageDeviceEx fileManager = getStorageDevice(strPath);

        if (fileManager != null) {
            // 扫描完成或移除设备，需要同步数据状态
            fileManager.updateLoading(isLoading(strPath));
            fileManager.updateMounted(isMounted(strPath));

            fileManager.mFileScanState.mIsLoadFinished =
                    getFileScanState(fileManager.mFilePath);
            fileManager.mID3ParseState.mIsLoadFinished =
                    getID3ParseState(fileManager.mFilePath);

            // [同步列表数据] MOUNTED 和 UNMOUNTED 完成都会同步一次
            getMusicInfoList(fileManager.mFilePath, fileManager.mMusicInfoList);
            getVideoInfoList(fileManager.mFilePath, fileManager.mVideoInfoList);

            if (null != mAppData.mCurrentDevice) {
                LogUtil.d(TAG, " -- EVENT_MEDIA_LOADING_COMPLETE: "
                        + fileManager.getFilePath() + ", Current: "
                        + mAppData.mCurrentDevice.getFilePath());

                LogUtil.i(TAG, " -- EVENT_MEDIA_LOADING_COMPLETE: "
                        + "mIsMounted = " + mAppData.mCurrentDevice.isMounted()
                        + ", mIsLoading = " + mAppData.mCurrentDevice.isLoading());
            }

            // 检查外部任务设备是否还有效, 避免移除情况下不退出
            // [updateCurrentManager()] 触发播放会检查外部任务路径
            checkSingleMediaInfo();

            // 检查并等待记忆盘符扫描结束
            if (mAppData.mCurrentDevice != null
                    && mAppData.mCurrentDevice.isMounted()
                    && mAppData.mCurrentDevice.isLoading()) {
                // mAppData.mCurrentManager.mIsLoading 说明：ACC OFF的时候就强制设置了 true;
                // 例如：记忆设备是 USB，当前上报 Flash, 就会在这里被过滤

                // [USB: 当前播放存储设备在扫描中, 但是又通知扫描结束]
                if (fileManager.isUsb() && fileManager == mAppData.mCurrentDevice) {
                    // [无有效数据且在扫描中场景: 当前播放的 USB 被移除，且还存在一个 USB 在扫描]
                    if (!mAppData.mCurrentDevice.existValidMediaInfo()) {
                        updateCurrentManager(); // 会更新切换 Loading 界面
                    }
                }
            } else {
                // 记忆盘符扫描完成或者等待挂载超时，检查记忆盘符，匹配设备来播放
                assert mAppData.mCurrentDevice != null;
                if (checkoutStorage(mAppData.mCurrentDevice)) {
                    if (fileManager == mAppData.mCurrentDevice) {
                        updateCurrentManager();
                    } else {
                        // 保留不处理[是否存在BUG，待测试]
                    }
                } else if (onChangeCurrentStorage()) {
                    // 调整设备播放： 记忆设备无媒体文件，就根据优先级选有效设备
                    LogUtil.i(TAG, " -- onPathScanFinished, onChangeCurrentStorage");
                } else {
                    LogUtil.i(TAG, " -- onPathScanFinished," +
                            " mAppData.mMediaType: " + mAppData.mMediaType);
                }
            }
        }

        // 给 UI 刷新用
        sendLocalBroadcast(IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE, strPath);
    }

    /**
     * 远程服务通知媒体 ID3 扫描任务完成
     * @see IMediaEvent#EVENT_ID3_SCAN_FINISHED
     *
     * @param strPath 存储设备路径
     */
    private void onMediaId3ScanFinished(String strPath) {
        LogUtil.i(TAG, " -- EVENT_ID3_SCAN_FINISHED: " + strPath);
        StorageDeviceEx storageDevice = getStorageDevice(strPath);

        if (storageDevice != null) {
            storageDevice.mFileScanState.mIsLoadFinished =
                    getFileScanState(storageDevice.mFilePath);
            storageDevice.mID3ParseState.mIsLoadFinished =
                    getID3ParseState(storageDevice.mFilePath);

            // [ID3 扫描完又同步一次，为了ID3分类]
            getMusicInfoList(storageDevice.mFilePath, storageDevice.mMusicInfoList);
            getVideoInfoList(storageDevice.mFilePath, storageDevice.mVideoInfoList);

            // [扫描线程已经分类过了] ID3 分类: 专辑分类、艺术家分类
            if (storageDevice.mID3ParseState.mIsLoadFinished) {
                classifyMediaInfoList(storageDevice);
            }

            // 更新ID3信息到播放列表
            if (IMusicState.MEDIA_TYPE_MUSIC == mAppData.mLastMediaType) {
                if (storageDevice == mAppData.mCurrentDevice) {
                    // 音乐播放列表不是空，且当前存储设备音乐信息不是空
                    if (!mAppData.musicPlaylist().isEmpty()
                            && !storageDevice.mMusicInfoList.isEmpty()) {
                        if (mAsyncHandler.hasMessages(
                                AsyncHandler.MSG_SYNC_ID3INFO_2_MUSIC_PLAYLIST)) {
                            mAsyncHandler.removeMessages(
                                    AsyncHandler.MSG_SYNC_ID3INFO_2_MUSIC_PLAYLIST);
                        }

                        Message msg = Message.obtain();
                        msg.what = AsyncHandler.MSG_SYNC_ID3INFO_2_MUSIC_PLAYLIST;
                        msg.obj = storageDevice.mFilePath;
                        mAsyncHandler.sendMessage(msg);
                    }
                }
            }
        }

        sendLocalBroadcast(IMediaEvent.EVENT_ID3_SCAN_FINISHED, strPath);
    }

    /**
     * 远程服务通知将进入深度休眠状态
     * @see IMediaEvent#EVENT_DEEP_SLEEP_STATUS
     */
    private void onEnterDeepSleepStatus() {
        mIsDeepSleep = true;
        H0.removeMessages(MsgEx.MSG_POLLING_ACC_STATUS);

        MediaModel.call()
                .playerModel()
                .onPlayControlEvent(IMusicState.PLAY_CMD_STOP, 7);

        // [视频 UI 在后台 (按了 Home 按键)]
        if (MediaModel.call()
                .uiModel()
                .isVideoActivityBackground()) {
            // [进入休眠状态: 是否需要干掉后台播放视频]
            if (Argument.sleepExitVideoBackground()) {
                MediaModel.call()
                        .uiModel()
                        .finishVideoUI(-2);
            }
        }

        // 清除音乐播放列表
        mAppData.musicPlaylist().clear();
        mAppData.musicRandomPositionList().clear();

        // 清除视频播放列表
        mAppData.videoPlaylist().clear();
        mAppData.videoRandomPositionList().clear();

        // 清除单任务播放信息
        onClearSingleMediaInfo();

        // 清除当前播放对象
        mAppData.mCurrentMediaInfo = null;
        H0.sendEmptyMessage(MsgEx.MSG_CANCEL_NOTIFICATION);

        if (mAppData.mCurrentDevice != null) {
            // 深度休眠的时候，避免唤醒起来跳盘符，强制设置为true，保证 RCheck 20S.
            mAppData.mCurrentDevice.updateLoading(true);
            mAppData.mCurrentDevice.updateMounted(true);
            mAppData.mCurrentDevice.clear();
            LogUtil.d(TAG, " -- EVENT_DEEP_SLEEP_STATUS: "
                    + mAppData.mCurrentDevice.getFilePath());
        }

        // [刷新 UI 元素]
        sendLocalBroadcast(IMediaEvent.EVENT_MEDIA_LOADING_FILE);
    }

    /**
     * 通知远程服务 扫描路径下的媒体文件
     * @param path 扫描路径
     */
    private void onRefreshMusicPlaylist(String path) {
        long currentTime = System.currentTimeMillis();
        Message msg = mAsyncHandler.obtainMessage();
        msg.what = AsyncHandler.MSG_REFRESH_MUSIC_PLAYLIST;
        msg.obj = path;

        mAsyncHandler.removeMessages(AsyncHandler.MSG_REFRESH_MUSIC_PLAYLIST);

        if (currentTime - mAsyncHandler.lastExecutionTime > AsyncHandler.EXECUTION_DELAY_TIME) {
            // 超过 5 秒，立即执行
            mAsyncHandler.lastExecutionTime = currentTime;
            mAsyncHandler.sendMessage(msg);
        } else {
            // 5 秒内的请求，延迟执行
            mAsyncHandler.sendMessageDelayed(msg, AsyncHandler.EXECUTION_DELAY_TIME);
        }
    }


    // 异步 Handler 处理
    private class AsyncHandler extends Handler {

        public static final int MSG_NONE = -1;
        public static final int MSG_SYNC_ID3INFO_2_MUSIC_PLAYLIST = 1;
        public static final int MSG_REFRESH_MUSIC_PLAYLIST = 2;

        // 刷新列表标准延迟时间
        public static final int EXECUTION_DELAY_TIME = 5000;

        // 记录上次执行刷新列表的时间
        public long lastExecutionTime = 0;


        public AsyncHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_SYNC_ID3INFO_2_MUSIC_PLAYLIST: {
                    if (HMediaConfig.USE_THREAD_POOL_SYNC_ID3) {
                        // Main Looper
                        if (msg.obj instanceof String) {
                            String path = (String) msg.obj;
                            mSyncID3ThreadPool.execute(new SyncID3Runnable(path));
                        }
                    } else {
                        // HandlerThread Looper
                        syncId3info2MusicPlaylist();
                    }
                    break;
                }

                case MSG_REFRESH_MUSIC_PLAYLIST:
                    // 外部应用通知媒体刷新路径的广播，当外部列表发生变化时使用
                    if (mRemoteService != null) {
                        try {
                            mRemoteService.onLoadMediaPathEvent((String) msg.obj);
                            Log.w(TAG, "handleMessage: MSG_REFRESH_MUSIC_PLAYLIST : " + msg.obj);
                        } catch (Exception ignored) {
                        }
                    }
                    break;

                case MSG_NONE:
                default: {
                    break;
                }
            }
        }

        // 同步 ID3 信息到播放列表
        private void syncId3info2MusicPlaylist() {
            if (null == mAppData.mCurrentDevice) {
                return;
            }

            try {
                int type = mAppData.mCurrentDevice.storageType();
                List<MusicInfo> musicInfoList = mAppData.mCurrentDevice.mMusicInfoList;

                for (MusicInfo info : mAppData.musicPlaylist()) {
                    if (info.mID3Type != MusicInfo.ID3_TYPE_NONE) {
                        continue;
                    }

                    for (MusicInfo newInfo : musicInfoList) {
                        if (type != mAppData.mCurrentDevice.storageType()) {
                            return;
                        }

                        if (null == newInfo) {
                            return;
                        }

                        // [需要使用文件名全路径匹配（暂时使用文件名）]
                        // [使用文件名问题：如果是文件夹重命名，会导致歌曲不存在]
                        if (!TextUtils.isEmpty(info.mFileName) &&
                                MiscUtils.reverseEquals(info.mFileName, newInfo.mFileName)) {
                            info.mID3Type = newInfo.mID3Type;
                            info.mIndex = newInfo.mIndex;
                            info.mTotalTime = newInfo.mTotalTime;
                            info.mTotalSize = newInfo.mTotalSize;
                            info.mTitle = newInfo.mTitle;
                            info.mArtist = newInfo.mArtist;
                            info.mAlbum = newInfo.mAlbum;
                            info.mFilePath = newInfo.mFilePath;
                            info.mFileName = newInfo.mFileName;
                            break;
                        }
                    }
                }

                sendLocalBroadcast(IMediaEvent.EVENT_UPDATE_MUSIC_LIST);
            } catch (Exception e) {
                LogUtil.w(TAG, "MSG_SYNC_ID3INFO_2_MUSIC_PLAYLIST:" + e);
            }
        }
    }

    private class SyncID3Runnable implements Runnable {
        private Reference<String> mStoragePathRef = null;

        public SyncID3Runnable(String path) {
            mStoragePathRef = new WeakReference<String>(path);
        }

        @Override
        public void run() {
            LogUtil.i(TAG, "[Enter][Thread]SyncID3Runnable.");

            String storagePath = mStoragePathRef.get();
            if (!TextUtils.isEmpty(storagePath)) {
                mAsyncHandler.syncId3info2MusicPlaylist();
            }

            LogUtil.i(TAG, "[Leave][Thread]SyncID3Runnable.");
        }
    }

    // [处理 SMART 按键事件]
    @Override
    protected void handleSmartKeyEvent(int nCommand) {
        // 只有音频才需要处理 SMART 按键
        if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_MUSIC) {
            return;
        }

        // 存在高优先级情况下不处理该事件
        if (existsHighPriorityEvent()) {
            return;
        }

        // 当前音乐播放列表不为空
        if (!mAppData.musicPlaylist().isEmpty()) {
            if (nCommand == IMusicState.PLAY_CMD_SMART_ENTER) {
                if (!mAppData.mIsControlPage || -1 == mAppData.musicSelectPosition()) {
                    onPlayControl(IMusicState.PLAY_CMD_PLAY_PAUSE);
                } else if (mAppData.musicSelectPosition() == mAppData.musicPlayPosition()) {
                    onPlayControl(IMusicState.PLAY_CMD_PLAY_PAUSE);
                } else {
                    onMediaEvent(
                            IMediaEvent.EVENT_CONTROL_SMART_ENTER,
                            null,
                            null);
                }
            } else if (nCommand == IMusicState.PLAY_CMD_SMART_CW && mAppData.mIsControlPage) {
                mSmartControlHandler.removeCallbacksAndMessages(null);
                mSmartControlHandler.postDelayed(mSmartControlRunnable, 5000);

                onMediaEvent(
                        IMediaEvent.EVENT_CONTROL_SMART_CW,
                        null,
                        null);
            } else if (nCommand == IMusicState.PLAY_CMD_SMART_CCW && mAppData.mIsControlPage) {
                mSmartControlHandler.removeCallbacksAndMessages(null);
                mSmartControlHandler.postDelayed(mSmartControlRunnable, 5000);

                onMediaEvent(
                        IMediaEvent.EVENT_CONTROL_SMART_CCW,
                        null,
                        null);
            }
        }
    }

    /**
     * 处理播放模式改变事件
     * @param random 播放模式
     * @deprecated 过时的设计接口，未使用；
     */
    @Deprecated
    private void onChangeRepeatMode(boolean random) {
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
            if (random) {
                if (mAppData.isMusicRepeatMode(IMusicState.REPEAT_MODE_QUEUE)) {
                    mAppData.setMusicRepeatMode(IMusicState.REPEAT_MODE_RANDOM);
                } else {
                    mAppData.setMusicRepeatMode(IMusicState.REPEAT_MODE_QUEUE);
                }
            } else {
                if (mAppData.isMusicRepeatMode(IMusicState.REPEAT_MODE_QUEUE)) {
                    mAppData.setMusicRepeatMode(IMusicState.REPEAT_MODE_ONE);
                } else {
                    mAppData.setMusicRepeatMode(IMusicState.REPEAT_MODE_QUEUE);
                }
            }

            updateMusicPlayRepeatMode();
            sendLocalBroadcast(IMediaEvent.EVENT_CHANGE_REPEAT_MODE);
        } else if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            if (random) {
                if (mAppData.isVideoRepeatMode(IMusicState.REPEAT_MODE_QUEUE)) {
                    mAppData.setVideoRepeatMode(IMusicState.REPEAT_MODE_RANDOM);
                } else {
                    mAppData.setVideoRepeatMode(IMusicState.REPEAT_MODE_QUEUE);
                }
            } else {
                if (mAppData.isVideoRepeatMode(IMusicState.REPEAT_MODE_QUEUE)) {
                    mAppData.setVideoRepeatMode(IMusicState.REPEAT_MODE_ONE);
                } else {
                    mAppData.setVideoRepeatMode(IMusicState.REPEAT_MODE_QUEUE);
                }
            }

            savePlayRepeatMode(IMusicState.MEDIA_TYPE_VIDEO);
            sendLocalBroadcast(IMediaEvent.EVENT_CHANGE_REPEAT_MODE);
        }
    }

    /**
     * 更新当前播放状态（会触发播放）
     * <p> 这个函数名称取的有些问题，待修改；
     */
    private void updateCurrentManager() {
        if (mAppData.mCurrentDevice.isLoading()) {
            LogUtil.i(TAG, ">>> updateCurrentManager loading.");

            // 清除当前音乐播放列表信息
            mAppData.updateMusicPlayPosition(0);
            mAppData.musicPlaylist().clear();
            mAppData.musicRandomPositionList().clear();

            // 清除当前视频播放列表信息
            mAppData.updateVideoPlayPosition(0);
            mAppData.videoPlaylist().clear();
            mAppData.videoRandomPositionList().clear();

            // 通知 Loading 媒体文件中
            if (!MediaModel.call()
                    .playerModel()
                    .existsValidMediaPlayer()) {
                H0.sendEmptyMessage(MsgEx.MSG_CANCEL_NOTIFICATION);
                sendLocalBroadcast(IMediaEvent.EVENT_MEDIA_LOADING_FILE);
            }
        } else if (mAppData.mLastMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
            LogUtil.i(TAG, ">>> updateCurrentManager music.");

            if (!mAppData.mCurrentDevice.mMusicInfoList.isEmpty()) {
                // 用户列表操作场景
                int actionScene = ListSceneManager.getInstance().readActionScene();
                Log.d(TAG, "actionScene = " + actionScene);
                switch (actionScene) {
                    case PageDataKV.ActionSceneValue.FOLDER:
                        tryPlayUsbFolderMusic();
                        break;
                    case PageDataKV.ActionSceneValue.NORMAL:
                    default:
                        playAvailableMusic();
                }
            } else {
                // [mCurrentManager 没有媒体文件]
                mAppData.updateMusicPlayPosition(0);
                mAppData.musicPlaylist().clear();
                mAppData.musicRandomPositionList().clear();

                H0.sendEmptyMessage(MsgEx.MSG_CANCEL_NOTIFICATION);
                sendLocalBroadcast(IMediaEvent.EVENT_MEDIA_NO_MUSIC_FILE);


                MediaModel.call()
                        .playerModel()
                        .onPlayControlEvent(IMusicState.PLAY_CMD_STOP, 8);
            }
        } else if (mAppData.mLastMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            LogUtil.i(TAG, ">>>> updateCurrentManager-video");

            // 视频播放列表不为空，触发播放
            if (!mAppData.mCurrentDevice.mVideoInfoList.isEmpty()) {
                // [是否能后台播放: 休眠唤醒起来、插拔 USB等]
                boolean canBackgroundPlay =
                        MediaModel.call()
                                .uiModel()
                                .isVideoActivityBackground();
                canBackgroundPlay = canBackgroundPlay && Argument.isBackgroundPlayMode();

                if (mAppData.mVideoUiShow || canBackgroundPlay) {
                    // [存在外部触发任务的时候不处理]没有外部触发任务，执行记忆播放
                    if (TextUtils.isEmpty(mAppData.mSingleVideoFilePath)) {
                        int position = getLastMediaInfoPosition(
                                mAppData.mCurrentDevice.mVideoInfoList);
                        tryUpdateVideoPlaylist(position, mAppData.mCurrentDevice.mVideoInfoList);
                    } else {
                        LogUtil.i(TAG, ">>>> updateCurrentManager-video, has Single Video!");
                    }
                }
            } else {
                if (mAppData.mSingleVideoPlay) {
                    LogUtil.i(TAG, ">>>> updateCurrentManager-video," +
                            " [not in the list]has Single Video!");
                    return;
                }

                // 当前存储设备没有视频文件
                mAppData.updateVideoPlayPosition(0);
                mAppData.videoPlaylist().clear();
                mAppData.videoRandomPositionList().clear();

                H0.sendEmptyMessage(MsgEx.MSG_CANCEL_NOTIFICATION);
                sendLocalBroadcast(IMediaEvent.EVENT_MEDIA_NO_MUSIC_FILE);

                MediaModel.call()
                        .playerModel()
                        .onPlayControlEvent(IMusicState.PLAY_CMD_STOP, 9);
            }
        }
    }

    private void playAvailableMusic() {
        // [播放检查]
        if (TextUtils.isEmpty(mAppData.mSingleMusicFilePath)) {
            if (mAppData.mSelectedDevice.storageType() != -1) {
                int position = getLastMediaInfoPosition(
                        mAppData.mCurrentDevice.mMusicInfoList);
                tryUpdateMusicPlaylist(position, mAppData.mCurrentDevice.mMusicInfoList);
            }
        } else {
            if (mAppData.mSelectedDevice.storageType() != -1) {
                int position = getMusicAssignPosition(mAppData.mSingleMusicFilePath);
                position = Math.max(position, 0);
                tryUpdateMusicPlaylist(position, mAppData.mCurrentDevice.mMusicInfoList);
            }
        }
    }

    private void tryPlayUsbFolderMusic() {
        String filepath = ListSceneManager.getInstance().readFolderPath();
        Log.d(TAG, "tryPlayUsbFolderMusic filepath = " + filepath);
        if (filepath != null && !filepath.isEmpty()
                && new File(filepath).exists()
                && filepath.contains(IConstant.USB_PATH_MARK)
                && IStorageDevice.STORAGE_TYPE_USB == mAppData.mCurrentDevice.storageType()) {

            FilePathScanManager fileManager = mAppData.mFilePathScanManager;
            fileManager.mFilePath = filepath;
            fileManager.mIsLoading = true;
            fileManager.mMediaPathState.mLoadingIndex.incrementAndGet();
            fileManager.mMediaPathState.mIsLoadFinished = false;

            MediaFilePathScan.getInstance(MediaFilePathScan.ALL_STORAGE_MODE)
                    .loadMediaPathList(fileManager.mFilePath,
                            fileManager.mMediaPathState,
                            fileManager.mMediaPathState.mLoadingIndex.get(),
                            fileManager.SCAN_MUSIC_FILE_TYPE,
                            new MediaFilePathScan.IMediaFilePathScanCallBack() {
                                @Override
                                public void onPathScanFinishedEx(FilePathScanManager fileManager, String path) {
                                    Log.d(TAG, "onPathScanFinishedEx");
                                    if (Objects.isNull(mAppData.mFilePathScanManager) || fileManager == null || fileManager.mMusicOnlyList.isEmpty()) {
                                        playAvailableMusic();
                                        return;
                                    }

                                    FilePathScanManager manager = mAppData.mFilePathScanManager;
                                    manager.mIsLoading = false;
                                    manager.mObjectTag = fileManager.mObjectTag;
                                    manager.mMediaPathState.mIsLoadFinished = true;

                                    manager.mMusicInfoList.clear();
                                    manager.mMusicOnlyList.clear();
                                    manager.mMediaFolderList.clear();

                                    manager.mMusicInfoList.addAll(fileManager.mMediaFolderList);
                                    manager.mMusicInfoList.addAll(fileManager.mMusicOnlyList);

                                    manager.mMusicOnlyList.addAll(fileManager.mMusicOnlyList);
                                    manager.mMediaFolderList.addAll(fileManager.mMediaFolderList);

                                    int position = getLastMediaInfoPosition(
                                            manager.mMusicOnlyList);
                                    tryUpdateMusicPlaylist(IPlaylistType.FOLDER_LIST, position, manager.mMusicOnlyList, true);


                                    // 同步更新 ID3 信息
                                    long nowMillis = SystemClock.elapsedRealtime();
                                    AbcFolderUtils.updateFolderListId3Info(
                                            fileManager.getObjectTag(),
                                            fileManager.mMusicOnlyList,
                                            true,
                                            new HTaskRunnable.OnCompletionListener() {
                                                @Override
                                                public void onCompletion(Object result) {
                                                    // TODO: reserved
                                                }

                                                @Override
                                                public void onCompletion(long taskTag, Object result) {
                                                    Log.d(TAG, "updateFolderListId3Info onCompletion");
                                                    // 有效数据才需要处理更新
                                                    if (!(result instanceof ArrayList)) {
                                                        return;
                                                    }

                                                    @SuppressWarnings("unchecked")
                                                    ArrayList<MusicInfo> list = (ArrayList<MusicInfo>) result;

                                                    // 如果列表信息个数都不一样，肯定不是一个任务了。
                                                    FilePathScanManager manager = mAppData.mFilePathScanManager;
                                                    if (taskTag == manager.getObjectTag()
                                                            && list.size() == manager.mMusicOnlyList.size()) {
                                                        manager.mMusicOnlyList.clear();
                                                        manager.mMusicOnlyList.addAll(list);
                                                    }

                                                    long deltaTime = SystemClock.elapsedRealtime() - nowMillis;
                                                    LogUtils.vTag(TAG, "updateFolderListId3Info:" +
                                                            " size = " + list.size() + ", execution time = " + deltaTime);
                                                }
                                            });
                                }
                            });
        } else {
            playAvailableMusic();
        }
    }

    /** 清除单例播放任务 **/
    private void onClearSingleMediaInfo() {
        clearSingleMusicInfo();
        clearSingleVideoInfo();
    }

    private void clearSingleMusicInfo() {
        mAppData.mSingleMusicPlay = false;
        mAppData.mSingleMusicFilePath = "";
    }

    private void clearSingleVideoInfo() {
        mAppData.mSingleVideoPlay = false;
        mAppData.mSingleVideoFilePath = "";
    }

    // [USB 比较特殊，移除的时候不一定好判断卸载状态]
    private void checkSingleMediaInfo() {
        // 检查任务路径是否还有效
        String musicPath = mAppData.mSingleMusicFilePath;
        String videoPath = mAppData.mSingleVideoFilePath;

        // [注意 mAppData.mSingleMusicFilePath 有可能是 null]
        if (!TextUtils.isEmpty(musicPath)) {
            File file = new File(musicPath);

            if (!file.exists() || !file.canRead()) {
                clearSingleMusicInfo();
            } else {
                StorageDeviceEx storageDevice = getStorageDevice(musicPath);

                if (storageDevice != null) {
                    if (!storageDevice.isMounted()) {
                        clearSingleMusicInfo();
                    } else {
                        // [USB 比较特殊，需要判定它是否存在比较严格]
                        if (storageDevice.isUsb() && (null != mRemoteService)) {
                            try {
                                boolean isMounted = mRemoteService.isUsbDeviceMounted(musicPath);
                                if (!isMounted) {
                                    clearSingleMusicInfo();
                                }
                            } catch (Exception ignored) {
                            }
                        }
                    }
                } else {
                    clearSingleMusicInfo();
                }
            }
        }

        // [注意 mAppData.mSingleVideoFilePath 有可能是 null]
        if (!TextUtils.isEmpty(videoPath)) {
            File file = new File(videoPath);

            if (!file.exists() || !file.canRead()) {
                clearSingleVideoInfo();
            } else {
                StorageDeviceEx storageDevice = getStorageDevice(videoPath);

                if (storageDevice != null) {
                    if (!storageDevice.isMounted()) {
                        clearSingleVideoInfo();
                    } else {
                        // [USB 比较特殊，需要判定它是否存在比较严格]
                        if (storageDevice.isUsb() && (null != mRemoteService)) {
                            try {
                                boolean isMounted = mRemoteService.isUsbDeviceMounted(videoPath);
                                if (!isMounted) {
                                    clearSingleVideoInfo();
                                    // 打印的作用是看什么时候能捕获到这种情况
                                    LogUtil.i(TAG, ">>> [Catch] -------- USB Removed!");
                                }
                            } catch (Exception ignored) {
                            }
                        }
                    }
                } else {
                    clearSingleVideoInfo();
                }
            }
        }
    }

    /**
     * 播放指定列表位置的音乐信息
     * @see #tryUpdateMusicPlaylist(int, List, boolean);
     */
    private boolean tryUpdateMusicPlaylist(int position,
                                           List<MusicInfo> infoList) {
        return tryUpdateMusicPlaylist(
                position, infoList, false);
    }

    /**
     * 由列表触发播放 [例: 点击歌曲]
     * @see #tryUpdateMusicPlaylist(int, int, List, boolean);
     */
    private boolean tryUpdateMusicPlaylist(int position,
                                   List<MusicInfo> infoList,
                                   boolean clickListItem) {
        return tryUpdateMusicPlaylist(
                IPlaylistType.DEVICE_LIST, position, infoList, false);
    }

    /**
     * 由列表触发播放 [例: 点击歌曲]
     *
     * @param playlistType 播放列表类型
     * @param position 播放索引
     * @param infoList 目标播放列表
     * @param clickListItem 是点击列表触发
     * @return 执行成功返回 true，反之失败；
     */
    boolean tryUpdateMusicPlaylist(int playlistType,
                                   int position,
                                   List<MusicInfo> infoList,
                                   boolean clickListItem) {
        LogUtil.d(TAG, ">>> tryUpdateMusicPlaylist:" +
                " playlistType = " + playlistType +
                " pos/size = " + position + "/" + infoList.size());

        // 清除外部临时播放任务信息
        onClearSingleMediaInfo();

        // 有效性检查，需要在非 ACC-OFF 状态
        if (position < infoList.size() && !mIsPowerOff) {
            // 更新当前音乐播放列表
            mAppData.updateMusicPlaylist(playlistType, infoList);
            mAppData.updateMusicRandomPositionList();

            // 更新当前播放索引标签
            mAppData.updateMusicPlayPosition(position);
            mAppData.mMusicPlayIndex = infoList.get(position).mIndex;

            // 请求执行音乐播放任务
            requestExecuteMusicPlayTask(mAppData.musicPlayPosition());
            mAppData.updateMusicSelectPosition(-1);

            // 通知更新显示信息
            if (clickListItem) {
                sendLocalBroadcast(
                        IMediaEvent.EVENT_CHANGE_MUSIC_LIST, "clickListItem");
            } else {
                sendLocalBroadcast(
                        IMediaEvent.EVENT_CHANGE_MUSIC_LIST);
            }

            return true;
        }

        return false;
    }

    /**
     * 尝试检查更新音乐播放列表
     * <pre>
     *    只处理正在播放的音乐列表内容需要改变的场景
     *    例如：收藏播放列表内容改变（取消/添加收藏）；
     * </pre>
     *
     * @param listType 列表类型
     * @param list 期望更新的列表
     * @return 更新成功/更新失败
     */
    boolean tryUpdateMusicPlaylistEx(@IPlaylistType int listType, List<MusicInfo> list) {
        // 是否匹配当前音乐播放列表类型
        if (mAppData.musicPlayListType() != listType) {
            LogUtil.w(TAG, "tryUpdateMusicPlaylistEx," +
                    " Does not match the current music playlist type!");
            return false;
        }

        // 列表的有效性检查
        if (Objects.isNull(list) || list.isEmpty()) {
            LogUtil.w(TAG, "tryUpdateMusicPlaylistEx, list empty!");
            return false;
        }

        // 计算需要播放的位置信息
        int listSize = list.size();
        int position = mAppData.musicPlayPosition();
        if (!BaseMediaData.isValidIndex(list, position)) {
            position = 0;
        }

        // 如果正在切换下一曲的间隙
        if (H0.hasMessages(MsgEx.MSG_GOTO_NEXT_MEDIA)) {
            H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);
        } else {
            // 暂时强制切换到下一曲
        }

        return tryUpdateMusicPlaylist(listType, position, list, true);
    }

    /**
     * 由列表触发播放 [例: 点击视频]
     * @see #tryUpdateVideoPlaylist(int, List, boolean);
     */
    private void tryUpdateVideoPlaylist(int position, List<MusicInfo> infoList) {
        tryUpdateVideoPlaylist(position, infoList, false);
    }

    /**
     * 由列表触发播放[例: 点击视频]
     *
     * @param position 播放索引
     * @param infoList 目标播放列表
     * @param clickListItem 是点击列表触发
     * @return 执行成功返回 true，反之失败；
     */
    void tryUpdateVideoPlaylist(int position, List<MusicInfo> infoList, boolean clickListItem) {
        LogUtil.d(TAG, ">>> tryUpdateVideoPlaylist, position: " + position);

        onClearSingleMediaInfo();

        if (position < infoList.size() && !mIsPowerOff) {
            // 更新当前视频播放列表
            mAppData.updateVideoPlaylist(infoList);
            mAppData.updateVideoRandomPositionList();

            // 更新视频播放索引标签
            mAppData.updateVideoPlayPosition(position);
            mAppData.mVideoPlayIndex = infoList.get(position).mIndex;

            // 请求执行视频播放任务
            requestExecuteVideoPlayTask(mAppData.videoPlayPosition());

            if (clickListItem) {
                sendLocalBroadcast(
                        IMediaEvent.EVENT_CHANGE_VIDEO_LIST, "clickListItem");
            } else {
                sendLocalBroadcast(
                        IMediaEvent.EVENT_CHANGE_VIDEO_LIST);
            }
        }
    }

    // [只处理音乐, 函数名称和功能不匹配]
    @Override
    public void onPlayControl(int command) {
        LogUtil.d(TAG, ">>> onPlayControl, command: " + command);
        if (existsHighPriorityEvent()) {
            return;
        }

        // [过滤事件]
        switch (command) {
            case IMusicState.PLAY_CMD_PREV:
            case IMusicState.PLAY_CMD_NEXT: {
                if (mControlHandler.hasMessages(ControlHandler.EVENT_KEY_PREV_NEXT_FILTER)) {
                    LogUtil.d(TAG, ">>> onPlayControl, Filter PREV/NEXT KEY!");
                    return;
                }
                break;
            }

            default: {
                break;
            }
        }

        // [分发事件]
        boolean handled = false;
        switch (mAppData.mMediaType) {
            case IMusicState.MEDIA_TYPE_MUSIC:
                handled = onLocalMusicPlayControl(command);
                break;

            case IMusicState.MEDIA_TYPE_VIDEO:
                handled = onLocalVideoPlayControl(command);
                break;

            default:
                break;
        }

        // [过滤事件]
        if (!handled) {
            // 事件没有被处理，无须重置处理过滤器
            return;
        }

        switch (command) {
            case IMusicState.PLAY_CMD_PREV:
            case IMusicState.PLAY_CMD_NEXT:
                mControlHandler.sendEmptyMessageDelayed(
                        ControlHandler.EVENT_KEY_PREV_NEXT_FILTER, 500);
                break;

            default:
                break;
        }
    }

    /**
     * 处理音乐播放控制
     *
     * @param command
     * @return
     */
    private boolean onLocalMusicPlayControl(int command) {
        LogUtil.d(TAG, ">>> onLocalPlayControl, nCommand: " + command
                + " mIsMediaPlayerLocked: " + mAppData.mIsMediaPlayerLocked
                + " mAppData.mMusicInfoList.size(): " + mAppData.musicPlaylist().size());

        if (mAppData.mIsMediaPlayerLocked) {
            // [这个变量有问题，非常不严谨, 有机会需要测底用严谨的方式替代]
            return false;
        }

        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
            if (mAppData.musicPlaylist().isEmpty()
                    && command != IMusicState.PLAY_CMD_STOP) {
                return false;
            }
        }

        switch (command) {
            case IMusicState.PLAY_CMD_PLAY:
            case IMusicState.PLAY_CMD_PAUSE:
            case IMusicState.PLAY_CMD_STOP:
            case IMusicState.PLAY_CMD_PLAY_PAUSE: {
                if (command == IMusicState.PLAY_CMD_STOP) {
                    writeCurrentMediaTime(true, 2);
                }

                MediaModel.call()
                        .playerModel()
                        .onPlayControlEvent(command, 10);
                break;
            }

            case IMusicState.PLAY_CMD_NEXT: {
                mAppData.mCurrentMediaInfo = null;
                H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);

                mAppData.adjustMusicPlayPosition(true);
                requestExecuteMusicPlayTask(mAppData.musicPlayPosition());
                break;
            }

            case IMusicState.PLAY_CMD_PREV: {
                mAppData.mCurrentMediaInfo = null;
                H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);

                mAppData.adjustMusicPlayPosition(false);
                requestExecuteMusicPlayTask(mAppData.musicPlayPosition());
                break;
            }

            case IMusicState.PLAY_CMD_SMART_CCW: {
                handleSmartKeyEvent(IMusicState.PLAY_CMD_SMART_CCW);
                break;
            }

            case IMusicState.PLAY_CMD_SMART_CW: {
                handleSmartKeyEvent(IMusicState.PLAY_CMD_SMART_CW);
                break;
            }

            case IMusicState.PLAY_CMD_SMART_ENTER: {
                handleSmartKeyEvent(IMusicState.PLAY_CMD_SMART_ENTER);
                break;
            }

            case KeyEvent.KEYCODE_MEDIA_REWIND: {
                onSeekRewind();
                break;
            }

            case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD: {
                onFastForward();
                break;
            }

            default: {
                return false;
            }
        }

        return true;
    }

    /**
     * 处理视频播放控制
     *
     * @param command
     * @return
     */
    private boolean onLocalVideoPlayControl(int command) {
        LogUtil.d(TAG, ">> onLocalVideoPlayControl, nCommand: " + command);

        // 播放组件初始流程锁
        if (mAppData.mIsMediaPlayerLocked) {
            LogUtil.d(TAG, "   onLocalVideoPlayControl: mIsMediaPlayerLocked!");
            return false;
        }

        // 空列表无法播放检查
        if (mAppData.videoPlaylist().isEmpty()
                && command != IMusicState.PLAY_CMD_STOP) {
            LogUtil.d(TAG, "   onLocalVideoPlayControl: list isEmpty!");

            // 如果无外部触发的单任务视频在播放
            if (!mAppData.mSingleVideoPlay || mAppData.mSingleVideoFilePath.isEmpty()) {
                LogUtil.d(TAG, "   onLocalVideoPlayControl: single video: "
                        + mAppData.mSingleVideoPlay + ", " + mAppData.mSingleVideoFilePath);
                return false;
            }

            // 如果在播放单任务视频文件，又触发了上下曲切换
            if (command == IMusicState.PLAY_CMD_NEXT
                    || command == IMusicState.PLAY_CMD_PREV) {
                MusicInfo singleInfo = getSingleMediaInfo(mAppData.mSingleVideoFilePath);
                if (null != singleInfo) {
                    // 把单任务加到视频播放列表中（此时列表是空）
                    mAppData.videoPlaylist().add(singleInfo);
                    mAppData.updateVideoPlayPosition(0);
                    mAppData.mVideoPlayIndex = -1;
                }
            }
        }

        switch (command) {
            case IMusicState.PLAY_CMD_PLAY:
            case IMusicState.PLAY_CMD_PAUSE:
            case IMusicState.PLAY_CMD_STOP:
            case IMusicState.PLAY_CMD_PLAY_PAUSE: {
                if (IMusicState.PLAY_CMD_STOP == command) {
                    writeCurrentMediaTime(true, 3);
                }

                MediaModel.call()
                        .playerModel()
                        .onPlayControlEvent(command, 2);
                break;
            }

            case IMusicState.PLAY_CMD_NEXT: {
                mAppData.mCurrentMediaInfo = null;
                H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);

                // 更新下一个播放索引，并请求播放
                mAppData.adjustVideoPlayPosition(true);
                requestExecuteVideoPlayTask(mAppData.videoPlayPosition());
                break;
            }

            case IMusicState.PLAY_CMD_PREV: {
                mAppData.mCurrentMediaInfo = null;
                H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);

                // 更新下一个播放索引，并请求播放
                mAppData.adjustVideoPlayPosition(false);
                requestExecuteVideoPlayTask(mAppData.videoPlayPosition());
                break;
            }

            case KeyEvent.KEYCODE_MEDIA_REWIND: {
                onSeekRewind();
                break;
            }

            case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD: {
                onFastForward();
                break;
            }

            default: {
                return false;
            }
        }

        return true;
    }

    // [这个方法会更新: mCurrentTime]
    void trySeekToTime(int nTime) {
        if (mAppData.mIsMediaPlayerLocked) {
            // 说明 MediaPlayer 还没准备好，触发拖动无意义。
            return;
        }

        MediaModel.call()
                .playerModel()
                .seekToTime(nTime);

        // [毫无意义, SeekTo(time) 是异步执行的]
        mAppData.mPlayTimeInfo.setCurrentTime(
                MediaModel.call().playerModel().getCurrentPosition(),
                true, "trySeekToTime");
        onMediaEvent(IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME, null, null);
    }

    /**
     * 请求执行音乐播放任务（上下曲切换）
     * <p> 这个函数不会改变播放列表（对应的随机信息列表也不会改变）；
     *
     * @param position 期望的播放媒体对象索引
     */
    private void requestExecuteMusicPlayTask(int position) {
        LogUtil.d(TAG, ">>>> requestExecuteMusicPlayTask, position: " + position);

        // 播放目标索引是否在当前播放列表中
        if (BaseMediaData.isValidIndex(mAppData.musicPlaylist(), position)) {
            if (!mAppData.mSingleMusicPlay) {
                mAppData.mMusicPlayIndex = mAppData.musicPlaylist().get(position).mIndex;
            }

            // 从随机列表中移除索引
            mAppData.removeFromMusicRandomPositionList(position);

            // 请求播放指定的媒体对象
            requestPlayDataSource(mAppData.musicPlaylist().get(position));
            H0.sendEmptyMessage(MsgEx.MSG_UPDATE_NOTIFICATION);

            // 对外广播当前播放信息
            sendNotifyMediaState();
        }

        onMediaEvent(IMediaEvent.EVENT_CHANGE_MUSIC_ITEM, null, null);

        mProviderObserver.updateAllMusicData();
    }

    /**
     * 请求执行视频播放任务（上下曲切换）
     * <p> 这个函数不会改变播放列表（对应的随机信息列表也不会改变）；
     *
     * @param position 期望的播放媒体对象索引
     */
    private void requestExecuteVideoPlayTask(int position) {
        LogUtil.d(TAG, ">>> requestExecuteVideoPlayTask, position: " + position);

        if (BaseMediaData.isValidIndex(mAppData.videoPlaylist(), position)) {
            // 外部触发任务没有在播放
            if (!mAppData.mSingleVideoPlay) {
                mAppData.mVideoPlayIndex = mAppData.videoPlaylist().get(position).mIndex;
            }

            mAppData.removeFromVideoRandomPositionList(position);
            requestPlayDataSource(mAppData.videoPlaylist().get(position));
            sendNotifyMediaState();
        }

        onMediaEvent(IMediaEvent.EVENT_CHANGE_VIDEO_ITEM, null, null);
    }

    /**
     * 请求音频焦点
     * <p> 焦点请求成功后会再次注册媒体按键事件处理器；
     */
    void onRequestAudioFocus() {
        LogUtil.d(TAG, ">>> onRequestAudioFocus ");

        if (null != mAudioManager) {
            int status = AudioManager.AUDIOFOCUS_GAIN;

            // 请求音频焦点
            if (null == mAudioFocusListener) {
                mAudioFocusListener = new AudioFocusChangeListener();

                // 高版本焦点请求支持延时机制
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                    AudioAttributes audioAttributes = new AudioAttributes.Builder()
                            .setUsage(AudioAttributes.USAGE_MEDIA)
                            .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                            .build();

                    mAudioFocusRequest = new AudioFocusRequest.Builder(AudioManager.AUDIOFOCUS_GAIN)
                            .setAudioAttributes(audioAttributes)
                            .setAcceptsDelayedFocusGain(true)
                            .setWillPauseWhenDucked(true)
                            .setOnAudioFocusChangeListener(mAudioFocusListener)
                            .build();

                    status = mAudioManager.requestAudioFocus(mAudioFocusRequest);
                } else {
                    status = mAudioManager.requestAudioFocus(mAudioFocusListener,
                            AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN);
                }

                mAudioFocusListener.focusState = status;
                LogUtil.d(TAG, "requestAudioFocus, " +
                        "status: " + (status == AudioManager.AUDIOFOCUS_GAIN) + " status = " + status);
            }

            // 请求失败返回
            if (status != AudioManager.AUDIOFOCUS_GAIN) {
                return;
            }

            // 注册 Media Button 事件
            registerMediaButtonEvent();
        }
    }

    /**
     * 释放音频焦点
     * <pre>
     *    1、进程退出（exitMediaApp）前调用；
     *    2、服务销毁{@link this#onDestroy()} 时候调用；
     * </pre>
     */
    private void onClearAudioEvent() {
        if (null != mAudioManager) {
            if (null != mAudioFocusListener) {
                // 高版本音频焦点释放处理
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                    if (mAudioFocusRequest != null) {
                        mAudioManager.abandonAudioFocusRequest(mAudioFocusRequest);
                        mAudioFocusRequest = null;
                    }
                }

                // 低版本音频焦点释放处理
                if (Objects.isNull(mAudioFocusRequest)) {
                    mAudioManager.abandonAudioFocus(mAudioFocusListener);
                }

                mAudioFocusListener.focusState = AudioManager.AUDIOFOCUS_NONE;
                mAudioFocusListener = null;
            }
        }

        mAudioHandler.removeCallbacksAndMessages(null);
    }

    // [保存状态]
    private void writeCurrentMediaTime(boolean stop, int reason) {
        if (mAppData.mCurrentMediaInfo != null) {
            Log.d(TAG, "writeCurrentMediaTime:"
                    + " stop = " + stop + ", reason = " + reason
                    + " file = " + mAppData.mCurrentMediaInfo.mFilePath
                    + ", time = " + mAppData.mPlayTimeInfo.mCurrentTime);

            // 保存当前状态
            writeMediaTime(
                    mAppData.mLastMediaType,
                    mAppData.mCurrentMediaInfo.mFilePath,
                    mAppData.mPlayTimeInfo.mCurrentTime,
                    105);

            // 新的播放任务需要清除相关数据
            if (stop) {
                mAppData.mCurrentMediaInfo = null;
                mAppData.mPlayTimeInfo.setCurrentTime(
                        0, true, "stop");
                mAppData.mPlayTimeInfo.mTotalTime = 0;
            }
        }
    }


    // [函数名取的有问题，这里不一定会播放: 取决于 mShouldPlay 条件]
    void doShouldPlayEvent() {
        doShouldPlayEvent("default");
    }

    /**
     * 触发恢复播放事件处理
     * <p> 一个非常无聊的函数，完全失败的设计；
     *
     * @param reason 调用原因
     */
    private void doShouldPlayEvent(String reason) {
        if (!mAppData.mAllowResumePlay) {
            // 不需要恢复播放检查
            LogUtil.d(TAG, "doShouldPlayEvent: mShouldPlay = false.");
            return;
        }

        if (existsHighPriorityEvent()) {
            // 没有权限恢复播放状态
            LogUtil.d(TAG, "doShouldPlayEvent: existsHighPriorityEvent.");
            return;
        }

        // [如果是在硬解码失败切换软解的衔接时间段: mCurrentMediaInfo 可能为 null.]
        if (mAppData.mCurrentMediaInfo != null) {
            LogUtil.d(TAG, "doShouldPlayEvent: " + mAppData.mCurrentMediaInfo.mFileName);
        }

        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
            // [触发播放后重置 mShouldPlay]
            mAppData.mAllowResumePlay = false;
            onLocalMusicPlayControl(IMusicState.PLAY_CMD_PLAY);
        } else if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            if (isCanPlayVideo()) {
                if (null == mAppData.mCurrentMediaInfo) {
                    // [触发播放后重置 mShouldPlay]
                    mAppData.mAllowResumePlay = false;
                    onLocalVideoPlayControl(IMusicState.PLAY_CMD_PLAY);
                } else {
                    if (!BaseMediaData.isValidIndex(
                            mAppData.videoPlaylist(), mAppData.videoPlayPosition())) {
                        mAppData.updateVideoPlayPosition(0);
                    }

                    requestExecuteVideoPlayTask(mAppData.videoPlayPosition());
                }
            } else if (isCanPlayVideoLowest()
                    && "reverse-off".equals(reason)) {
                // 在视频列表页面倒车结束可以恢复播放
                mAppData.mAllowResumePlay = false;
                onLocalVideoPlayControl(IMusicState.PLAY_CMD_PLAY);
            }
        }
    }

    // [状态记忆暂停函数, 因为某些事件导致需要临时暂停]
    private void doShouldPauseEvent(boolean is_stop) {
        doShouldPauseEvent(is_stop, 0);
    }

    /**
     * 应用暂停事件
     *
     * @param is_stop
     * @param reason  { 如果值 (200, ~) 表示由视频调用，如果值(100 ~ 200) 表示音乐调用 }
     */
    void doShouldPauseEvent(boolean is_stop, int reason) {
        LogUtil.d(TAG,
                "doShouldPauseEvent: mShouldPlay = " + mAppData.mAllowResumePlay
                        + ", mPlayState = " + mAppData.mMediaPlayState + ", reason = " + reason);

        // [特殊情况不处理]
        switch (reason) {
            case 201: {
                // 例如: 视频模式通过外部直接切换到音频，导致音乐暂停。
                if (IMusicState.MEDIA_TYPE_MUSIC == mAppData.mMediaType) {
                    LogUtil.i(TAG, "current media source: Music, no handle video event!");
                    return;
                }
                break;
            }
            case -1:
            default:
                break;
        }

        // 播放状态记忆，恢复播放状态使用
        if (!mAppData.mAllowResumePlay) {
            mAppData.mAllowResumePlay =
                    (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY));
        }

        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
            onLocalMusicPlayControl(IMusicState.PLAY_CMD_PAUSE);
        } else if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            if (is_stop) {
                mAppData.mAllowResumePlay = true;
                onLocalVideoPlayControl(IMusicState.PLAY_CMD_STOP);
                writeCurrentMediaTime(true, 4);
            } else {
                onLocalVideoPlayControl(IMusicState.PLAY_CMD_PAUSE);
                writeCurrentMediaTime(false, 5);
            }
        }
    }

    // 是否能够播放视频(最低要求)
    private boolean isCanPlayVideoLowest() {
        if (Argument.isCanPlayVideoBack()) {
            return isCanWatchVideo();
        } else {
            return isCanWatchVideo()
                    && mAppData.mVideoUiShow;
        }
    }

    /**
     * 媒体播放检查入口
     * <pre>
     *    这个函数有些复杂和臃肿，后续需要调整；
     *    当前播放存储设备与播放列表在此确认和更新；
     * </pre>
     */
    void trySwitchMediaTypeTask() {
        // [Media/检查重置播放数据]
        tryResetMediaPlayDataInfo();

        // [尝试开始更新播放媒体任务]
        tryUpdateAndPlayMediaTask();
    }

    /**
     * 尝试重置媒体播放相关的数据信息
     * <pre>
     *    参见：{@caller onChangeMediaType()} 函数
     *    函数名称没啥实际意义，仅仅为了把历史遗留代码拆分，方便阅读；
     * </pre>
     */
    private void tryResetMediaPlayDataInfo() {
        // 检查是否由视频占用模式切换到音频占用模式
        if (IMusicState.MEDIA_TYPE_MUSIC == mAppData.mMediaType) {
            LogUtil.d(TAG, ">>>> onChangeMediaType MEDIA_TYPE_MUSIC");

            // 内存中如果存在则不需要去偏好读取（避免冲掉模式信息）
            if (needReadPlayMode(IMusicState.MEDIA_TYPE_MUSIC)) {
                mAppData.setMusicRepeatMode(
                        Preferences.readPlayRepeatMode(
                                mContext, IMusicState.MEDIA_TYPE_MUSIC));
            }

            // 如果是 VIDEO --> MUSIC, 重置 MUSIC 播放列表
            if (mAppData.mLastMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
                mAppData.musicPlaylist().clear();
                mAppData.musicRandomPositionList().clear();

                // 清除上一个模式的任务
                H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);
                MediaModel.call()
                        .playerModel()
                        .onPlayControlEvent(IMusicState.PLAY_CMD_STOP, 3);

                // 保存与重置工作, 媒体类型变换时候才重置
                writeCurrentMediaTime(true, 6);
            }

            mAppData.mLastMediaType = IMusicState.MEDIA_TYPE_MUSIC;
        } /// [MusicState.MEDIA_TYPE_MUSIC == mAppData.mMediaType]

        // 检查是否由音频占用模式切换到视频占用模式
        if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType) {
            LogUtil.d(TAG, ">>>> onChangeMediaType MEDIA_TYPE_VIDEO");
            // 内存中如果存在则不需要去偏好读取（避免冲掉模式信息）
            if (needReadPlayMode(IMusicState.MEDIA_TYPE_VIDEO)) {
                mAppData.setVideoRepeatMode(
                        Preferences.readPlayRepeatMode(
                                mContext, IMusicState.MEDIA_TYPE_VIDEO));
            }

            // 视频模式不再显示音乐的通知菜单
            H0.sendEmptyMessage(MsgEx.MSG_CANCEL_NOTIFICATION);

            // 如果是 MUSIC --> VIDEO, 重置 VIDEO 播放列表
            if (mAppData.mLastMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
                mAppData.videoPlaylist().clear();
                mAppData.videoRandomPositionList().clear();

                // 清除上一个模式的任务
                H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);
                MediaModel.call()
                        .playerModel()
                        .onPlayControlEvent(IMusicState.PLAY_CMD_STOP, 4);

                // 保存与重置工作
                writeCurrentMediaTime(true, 7);
            }

            mAppData.mLastMediaType = IMusicState.MEDIA_TYPE_VIDEO;
        } /// [MusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType]
    }

    /** 尝试更新和播放媒体任务 **/
    private void tryUpdateAndPlayMediaTask() {
        switch (mAppData.mMediaType) {
            // [Music/开始音频任务检查]
            case IMusicState.MEDIA_TYPE_MUSIC:
                executeUpdateAndPlayMusicTask();
                break;
            // [Video/开始视频任务检查]
            case IMusicState.MEDIA_TYPE_VIDEO:
                executeUpdateAndPlayVideoTask();
                break;
            default:
                LogUtil.w(TAG, "tryUpdateAndPlayMediaTask, fail!");
                break;
        }
    }

    /**
     * 更新播放音乐
     * <pre>
     *    只有当前媒体类型是音乐时才处理；
     *    更新播放存储设备与播放列表，并执行播放触发动作；
     * </pre>
     */
    private void executeUpdateAndPlayMusicTask() {
        // 不是音乐播放模式
        if (IMusicState.MEDIA_TYPE_MUSIC != mAppData.mMediaType) {
            return;
        }

        // 清除视频的外部任务信息
        mAppData.mSingleVideoPlay = false;
        mAppData.mSingleVideoFilePath = "";

        if (mAppData.mSingleMusicPlay) {
            // [保留][就没有地方设置过变量 mAppData.mSingleMusicPlay = true]
            return;
        }

        // 外部触发播放事件检查[如果目标媒体文件存在]
        MusicInfo singleInfo = getSingleMediaInfo(mAppData.mSingleMusicFilePath);
        if (singleInfo != null) {
            StorageDeviceEx manager = getStorageDevice(mAppData.mSingleMusicFilePath);
            if (null == manager || !manager.isMounted()) {
                // 理论上不应该出现这样的问题，否则会跑飞。
                LogUtil.d(TAG, "[SingleMusic]onChangeMediaType, invalid target!");
                return;
            }

            // 改变播放存储设备，不管是否加载中，是否有列表信息
            mAppData.mCurrentDevice = manager;

            // [不理解，为什么要再修正一次，感觉是其它地方有BUG，在这里做了修正]
            if (manager.mMusicInfoList.isEmpty()) {
                initStorageDevice(manager);
            }

            // 歌曲本身如果不是被支持的格式，会存在问题。
            int position = getMusicAssignPosition(mAppData.mSingleMusicFilePath);
            if (-1 == position) {
                // 不在列表中的数据处理[BUG][需要处理]
                LogUtil.d(SINGLE_TAG, "[not in the list][no support format]target file:"
                        + mAppData.mSingleMusicFilePath);

                // 判断文件后缀格式是否在当前多媒体支持序列
                boolean isSupport = false;
                String suffix = getSuffix(mAppData.mSingleMusicFilePath);

                // [注意]服务可能不一定响应，需要考虑它可能存在异常[如果服务异常，就当不支持处理]
                if (null != mRemoteService) {
                    try {
                        isSupport = mRemoteService.isSupportMediaFile(
                                IMusicState.MEDIA_TYPE_MUSIC, suffix);
                    } catch (Exception ignored) {
                    }

                    // 是支持的格式，但文件不在列表中，需要单独处理[重新扫描]
                    if (isSupport) {
                        // 需要移除消息队列中的播放任务，因为扫描后会开始新任务
                        H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);

                        // 跳转当作点击处理，修正 SelectedManager 指向
                        mAppData.mSelectedDevice = mAppData.mCurrentDevice;

                        try {
                            mRemoteService.onLoadMediaPathEvent(mAppData.mSingleMusicFilePath);
                        } catch (Exception ignored) {
                        }
                    }
                }

                // 不支持的格式跳转，需要添加提示信息
                if (!isSupport) {
                    onToastText(R3.string.tip_unsupport_audio);

                    if (null == mAppData.mCurrentMediaInfo) {
                        // 播放当前存储设备列表第一首
                        if (!tryUpdateMusicPlaylist(
                                0, mAppData.mCurrentDevice.mMusicInfoList)) {
                            // 过滤重复的更新事件，如果执行成功函数本身就会调用通知事件
                            sendLocalBroadcast(IMediaEvent.EVENT_CHANGE_MUSIC_LIST);
                        } else {
                            // 新播放任务触发成功，需要移除消息队列中的播放任务
                            H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);
                        }
                    }
                }
            } else {
                // 找到目标，执行播放流程
                LogUtil.d(SINGLE_TAG,
                        "[in the list]target file:" + mAppData.mSingleMusicFilePath);

                // 跳转当作点击处理，修正 SelectedManager 指向
                mAppData.mSelectedDevice = mAppData.mCurrentDevice;

                if (!tryUpdateMusicPlaylist(
                        position, mAppData.mCurrentDevice.mMusicInfoList)) {
                    // 过滤重复的更新事件，如果执行成功函数本身就会调用通知事件
                    sendLocalBroadcast(IMediaEvent.EVENT_CHANGE_MUSIC_LIST);
                } else {
                    // 新播放任务触发成功，需要移除消息队列中的播放任务
                    H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);
                }
            }

            return;
        }

        // 根据外部触发参数匹配媒体信息
        boolean bFlag = onFindMusicRegInfoEvent(mAppData.mMusicRegInfo);
        if (bFlag) {
            mAppData.mSingleMusicPlay = false;
            mAppData.mSingleMusicFilePath = "";
            LogUtil.d(TAG, "onFindMusicRegInfoEvent, bFlag: true!");
            return;
        }

        // 当前音乐播放列表为空，找个有效的存储设备播放
        if (mAppData.musicPlaylist().isEmpty()) {
            // 先读记忆播放信息还在不在，在就播放
            String filePath = readMediaPath(mAppData.mLastMediaType);
            if (checkoutPlayFilePath(filePath)) {
                return;
            }

            // 找不到记忆源信息，找优先级高的存储设备播放
            mAppData.mCurrentDevice = getValidStorageDevice();
            updateCurrentManager();
        } else {
            LogUtil.d(TAG, ">>>> onChangeMediaType MEDIA_TYPE_MUSIC Something Other");

            if (null == mAppData.mCurrentMediaInfo) {
                if (!BaseMediaData.isValidIndex(
                        mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
                    mAppData.updateMusicPlayPosition(0);
                }

                requestExecuteMusicPlayTask(mAppData.musicPlayPosition());
            }
        }
    }

    /**
     * 尝试更新播放视频
     * <pre>
     *    只有当前媒体类型是视频时才处理；
     *    更新播放存储设备与播放列表，并执行播放触发动作；
     * </pre>
     */
    private void executeUpdateAndPlayVideoTask() {
        // 不在视频播放模式
        if (IMusicState.MEDIA_TYPE_VIDEO != mAppData.mMediaType) {
            return;
        }

        // 清除音频的外部任务信息
        mAppData.mSingleMusicPlay = false;
        mAppData.mSingleMusicFilePath = "";

        if (mAppData.mSingleVideoPlay) {
            Log.v(TAG, "   onChangeMediaType, mSingleVideoPlay == true!");
            return;
        }

        MusicInfo singleInfo = getSingleMediaInfo(mAppData.mSingleVideoFilePath);
        if (null != singleInfo) {
            StorageDeviceEx manager = getStorageDevice(mAppData.mSingleVideoFilePath);

            // 同步播放存储设备
            mAppData.mCurrentDevice = manager;
            mAppData.mSelectedDevice = manager;

            // 清除视频播放列表
            mAppData.videoPlaylist().clear();
            mAppData.videoRandomPositionList().clear();

            // 创建新的播放列表
            mAppData.videoPlaylist().add(singleInfo);
            mAppData.updateVideoPlayPosition(0);
            mAppData.mVideoPlayIndex = -1;
            mAppData.mSingleVideoPlay = true;

            requestExecuteVideoPlayTask(mAppData.videoPlayPosition());

            H0.removeMessages(MsgEx.MSG_GOTO_NEXT_MEDIA);
            sendLocalBroadcast(IMediaEvent.EVENT_CHANGE_VIDEO_LIST);
            return;
        }

        // 先检查播放列表是否为空
        if (mAppData.videoPlaylist().isEmpty()) {
            // 播放列表空，优先恢复记忆播放
            String filePath = readMediaPath(mAppData.mLastMediaType);
            if (checkoutPlayFilePath(filePath)) {
                return;
            }

            // 无记忆播放源，按 USB > SD > Flash 选择有源存储设备播放
            mAppData.mCurrentDevice = getValidStorageDevice();
            updateCurrentManager();
        } else {
            LogUtil.d(TAG, ">>>> onChangeMediaType MEDIA_TYPE_VIDEO Something Other");

            // 当前无播放信息，需要触发播放信息
            if (null == mAppData.mCurrentMediaInfo) {
                if (!BaseMediaData.isValidIndex(
                        mAppData.videoPlaylist(), mAppData.videoPlayPosition())) {
                    mAppData.updateVideoPlayPosition(0);
                }

                requestExecuteVideoPlayTask(mAppData.videoPlayPosition());
            }
        }
    }

    private MusicInfo getSingleMediaInfo(String filePath) {
        MusicInfo info = null;
        if (TextUtils.isEmpty(filePath)) {
            return info;
        }

        File file = new File(filePath);
        if (file.exists()) {
            info = new MusicInfo();

            info.mFilePath = file.getPath();
            info.mFileName = file.getName();

            // 视频不解析 ID3
            if (AppGlobalData.call()
                    .isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
                MediaID3Util.retrieveTargetID3Info(info);
            } else {
                info.mTitle = info.mFileName;
                info.mAlbum = "<Unknown>";
                info.mArtist = "<Unknown>";
                info.mTotalTime = 0;
                info.mID3Type = MusicInfo.ID3_TYPE_ERROR;
            }
        }

        return info;
    }

    // 这个函数会触发播放流程[吐槽: 这函数名称取的有问题]
    private boolean checkoutPlayFilePath(String filePath) {
        LogUtil.d(TAG, ">>>> checkoutPlayFilePath filePath: " + filePath);
        boolean result = false;

        if (!TextUtils.isEmpty(filePath)) {
            StorageDeviceEx manager = getStorageDevice(filePath);

            switch (mAppData.mLastMediaType) {
                case IMusicState.MEDIA_TYPE_MUSIC: {
                    mAppData.musicPlaylist().clear();
                    mAppData.musicRandomPositionList().clear();
                    break;
                }

                case IMusicState.MEDIA_TYPE_VIDEO: {
                    mAppData.videoPlaylist().clear();
                    mAppData.videoRandomPositionList().clear();
                    break;
                }

                default:
                    break;
            }

            if (checkoutStorage(manager)) {
                result = true;
                mAppData.mCurrentDevice = manager;
                mAppData.mSelectedDevice = manager;

                updateCurrentManager();
            }
        }

        return result;
    }

    /**
     * 设置播放数据源时间
     * <pre>
     *    这是一个历史对外 aidl 接口的实现，现已关闭，不要再调用它；
     *    由于函数名称和 {@link IPlayerModel} 的个别功能接口相近，容易导致代码理解问题；
     *    替换接口 {@link #requestPlayDataSource(MusicInfo)} 建议也少用；
     * </pre>
     *
     * @param info
     * @deprecated 过时的接口，禁止再调用它；
     */
    @Deprecated
    private void onSetDataSourceEvent(MusicInfo info) {
        requestPlayDataSource(info);
    }

    /**
     * 播放指定媒体对象
     * <pre>
     *    1、外部 UI 直接触发调用（建议少用）；
     *    2、播放列表改变后触发调用；
     *    这个接口对外是不安全的，由于历史原因，暂时还不能彻底关闭它；
     * </pre>
     *
     * @param info {@link MusicInfo}
     */
    void requestPlayDataSource(MusicInfo info) {
        LogUtil.d(TAG, ">>>> onSetDataSourceEvent, path: " + info.mFilePath);

        // 无效类型
        if (IMusicState.MEDIA_TYPE_IDLE == mAppData.mMediaType) {
            mAppData.mCurrentMediaInfo = null;
            return;
        }

        // [触发播放后重置 mShouldPlay]
        mAppData.mAllowResumePlay = false;

        // 选择文件命中当前播放文件，发送播放即刻。
        if (mAppData.mCurrentMediaInfo != null
                && MiscUtils.reverseEquals(mAppData.mCurrentMediaInfo.mFilePath, info.mFilePath)
                && MediaModel.call().playerModel().existsValidMediaPlayer()) {
            if (!mAppData.mIsMediaPlayerLocked) {
                boolean triggerPlay = true;

                // [例: 避免在刹车警告界面, 插拔USB触发播放.]
                if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType) {
                    if (!isCanPlayVideo()) {
                        triggerPlay = false;
                        mAppData.mAllowResumePlay = true;
                    }
                }

                if (triggerPlay) {
                    MediaModel.call()
                            .playerModel()
                            .onPlayControlEvent(IMusicState.PLAY_CMD_PLAY, 5);
                }
            }

            return;
        }

        // [这里做法非常暴力，感觉就是不管三七二十一，发个停止总错不了]
        if (!mAppData.mIsMediaPlayerLocked) {
            MediaModel.call()
                    .playerModel()
                    .onPlayControlEvent(IMusicState.PLAY_CMD_STOP, 5);
        }

        mAppData.mCurrentMediaInfo = info;
        mUpdateTimeHandler.removeCallbacks(mTimeRunnable);

        if (IMusicState.MEDIA_TYPE_VIDEO == mAppData.mMediaType) {
            // [ 画中画模式: 软解码切换硬解码, 会闪烁主界面背景, 需要遮挡;
            //   闪烁的原因: 硬解码 setDisplay(SurfaceHolder) 存在锁问题 ]
            sendLocalBroadcast(IMediaEvent.EVENT_VIDEO_SHOW_BLACK_PAGE);

            // [下一曲时间间隔非常重要，这里涉及到软解释放资源的缓冲时间]
            // [例如:在还未 Prepared 成功的情况下去 release，这将是灾难]
            mControlHandler.removeCallbacksAndMessages(null);
            mControlHandler.sendEmptyMessageDelayed(
                    ControlHandler.EVENT_SET_DATA_SOURCE, 250);
        } else if (IMusicState.MEDIA_TYPE_MUSIC == mAppData.mMediaType) {
            mControlHandler.removeCallbacksAndMessages(null);
            mControlHandler.sendEmptyMessageDelayed(
                    ControlHandler.EVENT_SET_DATA_SOURCE, 700);
        }

        // [数据存储]
        mAppData.mPlayTimeInfo.setCurrentTime(
                readMediaTime(mAppData.mMediaType, info.mFilePath),
                true, "requestPlayDataSource");
        mAppData.mPlayTimeInfo.mTotalTime = info.mTotalTime;
        writeMediaTime(mAppData.mMediaType,
                info.mFilePath, mAppData.mPlayTimeInfo.mCurrentTime, 106);

        // 通知 UI 更新媒体播放时间信息
        onMediaEvent(
                IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME,
                null,
                null);
    }

    /** 存储与更新音乐播放模式 **/
    void updateMusicPlayRepeatMode() {
        // 更新 provider
        mProviderObserver.updatePlayMode(mAppData.musicRepeatMode());

        // 存储播放模式
        savePlayRepeatMode(IMusicState.MEDIA_TYPE_MUSIC);
    }

    /** 切换音乐播放模式 **/
    void switchMusicRepeatMode() {
        // 切换播放循环模式
        switch (mAppData.musicRepeatMode()) {
            case IMusicState.REPEAT_MODE_QUEUE:
                mAppData.setMusicRepeatMode(IMusicState.REPEAT_MODE_ONE);
                break;
            case IMusicState.REPEAT_MODE_ONE:
                mAppData.setMusicRepeatMode(IMusicState.REPEAT_MODE_RANDOM);
                break;
            case IMusicState.REPEAT_MODE_RANDOM:
            default:
                mAppData.setMusicRepeatMode(IMusicState.REPEAT_MODE_QUEUE);
                break;
        }

        updateMusicPlayRepeatMode();
    }

    /** 切换音视频放模式 **/
    void switchVideoRepeatMode() {
        switch (mAppData.videoRepeatMode()) {
            case IMusicState.REPEAT_MODE_QUEUE:
                mAppData.setVideoRepeatMode(IMusicState.REPEAT_MODE_ONE);
                break;
            case IMusicState.REPEAT_MODE_ONE:
                mAppData.setVideoRepeatMode(IMusicState.REPEAT_MODE_RANDOM);
                break;
            case IMusicState.REPEAT_MODE_RANDOM:
            default:
                mAppData.setVideoRepeatMode(IMusicState.REPEAT_MODE_QUEUE);
                break;
        }

        savePlayRepeatMode(IMusicState.MEDIA_TYPE_VIDEO);
    }

    /**
     * 循环更新通知信息菜单显示
     * <p> 需要确认是否一直需要 initMusicPlayerNotification 吗？
     */
    private void updateMusicNotification() {
        if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_MUSIC) {
            return;
        }

        // 判断是否为紧凑型布局
        boolean isCompact = Feature.instance().hasFeature(Feature.BIT.COMPACT_NOTIFICATION);

        initMusicPlayerNotification(isCompact);

        if (null != mMusicRemoteViews) {
            // 当前音乐播放索引是有效的
            if (BaseMediaData.isValidIndex(
                    mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
                MusicInfo info = mAppData.musicPlayPositionInfo();
                mMusicRemoteViews.setTextViewText(R.id.tvMusicTitle, info.mFileName);
                mMusicRemoteViews.setTextViewText(R.id.tvMusicArtist,
                        info.mArtist.equals("<Unknown>") ?
                                mContext.getString(R3.string.text_unknown) :
                                info.mArtist);
            }

            if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                mMusicRemoteViews.setImageViewResource(R.id.btnPlayPause,
                        isCompact ? R.drawable.btn_pause_bg : R.drawable.notify_pause_bg);
            } else {
                mMusicRemoteViews.setImageViewResource(R.id.btnPlayPause,
                        isCompact ? R.drawable.btn_play_bg : R.drawable.notify_play_bg);
            }
        }

        if (null != mNotifyManager && null != mMusicNotification) {
            mMusicNotification.flags = Notification.FLAG_ONGOING_EVENT;
            mNotifyManager.notify(MUSIC_PLAYER_NOTIFY_TAG, MUSIC_PLAYER_NOTIFY_ID, mMusicNotification);
            mMusicNotification = null;
        }
    }

    @SuppressLint({"ObsoleteSdkInt", "LaunchActivityFromNotification"})
    private void initMusicPlayerNotification(Boolean isCompact) {
        mNotifyManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

        // 判断是否为紧凑型布局
        if (isCompact) {
            mMusicRemoteViews = new RemoteViews(this.getPackageName(), R.layout.notification_music_compact);
        } else {
            mMusicRemoteViews = new RemoteViews(this.getPackageName(), R.layout.notification_music);
        }

        Intent intentPrev = new Intent(IConstant.ACTION_NOTIFICATION_PREV);

        // 适配高版本 Android 系统接口
        int flags = PendingIntent.FLAG_UPDATE_CURRENT;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            flags |= PendingIntent.FLAG_IMMUTABLE;
        }

        @SuppressLint("UnspecifiedImmutableFlag")
        PendingIntent piPrev = PendingIntent.getBroadcast(this, 0, intentPrev, flags);
        mMusicRemoteViews.setOnClickPendingIntent(R.id.btnPrev, piPrev);

        Intent intentNext = new Intent(IConstant.ACTION_NOTIFICATION_NEXT);
        @SuppressLint("UnspecifiedImmutableFlag")
        PendingIntent piNext = PendingIntent.getBroadcast(this, 0, intentNext, flags);
        mMusicRemoteViews.setOnClickPendingIntent(R.id.btnNext, piNext);

        Intent intentPlay = new Intent(IConstant.ACTION_NOTIFICATION_PLAYPAUSE);
        @SuppressLint("UnspecifiedImmutableFlag")
        PendingIntent piPlay = PendingIntent.getBroadcast(this, 0, intentPlay, flags);
        mMusicRemoteViews.setOnClickPendingIntent(R.id.btnPlayPause, piPlay);

        Intent intentShow = new Intent(IConstant.ACTION_NOTIFICATION_SHOW);
        @SuppressLint("UnspecifiedImmutableFlag")
        PendingIntent pIntent = PendingIntent.getBroadcast(this, 0, intentShow, flags);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            String id = "channel_001";
            String name = "media";
            NotificationChannel mChannel = new NotificationChannel(id, name,
                    NotificationManager.IMPORTANCE_LOW);
            mNotifyManager.createNotificationChannel(mChannel);
            mMusicNotification = new Notification.Builder(this)
                    .setChannelId(id)
                    .setOngoing(true)
                    .setSmallIcon(R.drawable.music_notify_icon)
                    .setTicker(getString(R3.string.app_music_label))
                    .setContent(mMusicRemoteViews).setContentIntent(pIntent)
                    .build();
        } else {
            mNotifyBuilder = new NotificationCompat.Builder(getApplicationContext());
            mMusicNotification = mNotifyBuilder.setOngoing(true)
                    .setSmallIcon(R.drawable.music_notify_icon)
                    .setTicker(getString(R3.string.app_music_label))
                    .setContent(mMusicRemoteViews)
                    .setContentIntent(pIntent)
                    .build();
        }

        mMusicNotification.bigContentView = mMusicRemoteViews;
    }

    private void cancelNotification() {
        if (mNotifyManager != null) {
            mNotifyManager.cancel(MUSIC_PLAYER_NOTIFY_TAG, MUSIC_PLAYER_NOTIFY_ID);
            mMusicNotification = null;
        }
    }

    /**
     * 请求退出当前进程
     * @param reason
     */
    void requestExitApplication(int reason) {
        LogUtil.d(TAG, "requestExitApp, reason: " + reason);

        // 是在执行退出应用任务
        if (mExecutingExitAppTask) {
            return;
        }

        // 标记退出任务状态
        mExecutingExitAppTask = true;
        mAppData.mMediaType = IMusicState.MEDIA_TYPE_IDLE;

        // 通知正常退出应用消息（阻塞调用）
        HBusUtils.post(IBusTag.MEDIA_APP_WILL_EXIT);

        // 历史数据存储动作
        writeCurrentMediaTime(true, 8);

        // 清理声音和通知事件
        onClearAudioEvent();
        cancelNotification();

        // 执行数据库存储动作
        MediaModel.call()
                .dataModel()
                .closeDataModel();

        // 延时 128ms 退出
        H0.postDelayed(() -> {
            android.os.Process.killProcess(android.os.Process.myPid());
            System.exit(0);
        }, 128);
    }

    private void localVoiceRegisterReceiver() {
        mVoiceControlReceiver = new VoiceControlReceiver();

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_MUSIC_SINGLE_MODEL);
        intentFilter.addAction(ACTION_MUSIC_RANDOM_MODEL);
        intentFilter.addAction(ACTION_MUSIC_ALL_LOOP_MODEL);
        intentFilter.addAction(ACTION_MUSIC_PLAY);
        intentFilter.addAction(ACTION_MUSIC_PAUSE);
        intentFilter.addAction(ACTION_NOTIFICATION_PREV);
        intentFilter.addAction(ACTION_NOTIFICATION_NEXT);

        registerReceiver(mVoiceControlReceiver, intentFilter);
    }

    private void localVoiceUnregisterReceiver() {
        unregisterReceiver(mVoiceControlReceiver);
    }

    /**
     * 播放模式改变事件
     * @param mode
     */
    private void onChangeRepeatMode(int mode) {
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
            mAppData.setMusicRepeatMode(mode);
            updateMusicPlayRepeatMode();
            sendLocalBroadcast(IMediaEvent.EVENT_CHANGE_REPEAT_MODE);
        } else if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            mAppData.setVideoRepeatMode(mode);
            savePlayRepeatMode(IMusicState.MEDIA_TYPE_VIDEO);
            sendLocalBroadcast(IMediaEvent.EVENT_CHANGE_REPEAT_MODE);
        }
    }

    /**
     * 匹配指定媒体信息并执行播放
     * <pre>
     *    处理语言识别结果；
     *    匹配的效率不是一般的低（还是在主线程干活）；
     * </pre>
     *
     * @param info 媒体信息包
     * @return
     */
    private boolean onFindMusicRegInfoEvent(MusicRegInfo info) {
        if (!info.mEnable) {
            return false;
        }

        info.mEnable = false;

        String title = info.mTitle;
        String artist = info.mArtist;
        String album = info.mAlbum;

        if (TextUtils.isEmpty(title)) {
            if (!TextUtils.isEmpty(artist)) {
                return onFindMusicListByArtist(artist);
            } else if (!TextUtils.isEmpty(album)) {
                return onFindMusicListByAlbum(album);
            }

            return false;
        } else if (!TextUtils.isEmpty(artist)) {
            return onFindMusicListByParam(title, artist, album, 1);
        } else if (!TextUtils.isEmpty(album)) {
            return onFindMusicListByParam(title, artist, album, 2);
        }

        return onFindMusicListByParam(title, artist, album, 0);
    }

    /**
     * 播放指定列表位置的歌曲
     *
     * @param position 位置
     * @param infoList 列表
     */
    private void onFindMusicListEvent(int position, List<MusicInfo> infoList) {
        tryUpdateMusicPlaylist(position, infoList);
    }

    private boolean onFindMusicListByArtist(String artist) {
        List<MusicInfo> infoList;

        if (mAppData.mCurrentDevice != null) {
            infoList = mAppData.mCurrentDevice.mArtistListMap.get(artist);
            if (infoList != null) {
                onFindMusicListEvent(0, infoList);
                return true;
            }
        }

        for (StorageDeviceEx manager : mStorageDeviceList) {
            if (manager != mAppData.mCurrentDevice) {
                infoList = manager.mArtistListMap.get(artist);
                if (infoList != null) {
                    mAppData.mCurrentDevice = manager;
                    onFindMusicListEvent(0, infoList);
                    return true;
                }
            }
        }

        return false;
    }

    private boolean onFindMusicListByAlbum(String album) {
        List<MusicInfo> infoList;

        if (mAppData.mCurrentDevice != null) {
            infoList = mAppData.mCurrentDevice.mAlbumListMap.get(album);
            if (infoList != null) {
                onFindMusicListEvent(0, infoList);
                return true;
            }
        }

        for (StorageDeviceEx manager : mStorageDeviceList) {
            if (manager != mAppData.mCurrentDevice) {
                infoList = manager.mAlbumListMap.get(album);
                if (infoList != null) {
                    mAppData.mCurrentDevice = manager;
                    onFindMusicListEvent(0, infoList);
                    return true;
                }
            }
        }

        return false;
    }

    private boolean onFindMusicListByParam(
            String title, String artist, String album, int type) {
        // 当前音乐播放列表（浅拷贝）
        List<MusicInfo> infoList = new ArrayList<>(mAppData.musicPlaylist());
        int index = onFindStorageMusicListEvent(title, artist, album, type, infoList);
        if (index >= 0) {
            // 找到匹配结果，执行播放任务
            onFindMusicListEvent(index, infoList);
            return true;
        }

        // 去当前播放设备中匹配查找
        if (mAppData.mCurrentDevice != null) {
            infoList = mAppData.mCurrentDevice.mMusicInfoList;
            index = onFindStorageMusicListEvent(title, artist, album, type, infoList);

            if (index >= 0) {
                onFindMusicListEvent(index, infoList);
                return true;
            }
        }

        // 到所有剩余存储设备中匹配查找
        for (StorageDeviceEx manager : mStorageDeviceList) {
            if (manager != mAppData.mCurrentDevice) {
                infoList = manager.mMusicInfoList;
                index = onFindStorageMusicListEvent(title, artist, album, type, infoList);

                if (index >= 0) {
                    mAppData.mCurrentDevice = manager;
                    onFindMusicListEvent(index, infoList);
                    return true;
                }
            }
        }

        return false;
    }

    /**
     * 在指定列表中匹配指定信息
     *
     * @param title 歌曲名称
     * @param artist 艺术家名称
     * @param album 专辑名称
     * @param type 匹配类型
     * @param infoList 匹配数据源
     * @return 匹配索引，返回 -1 表示没有匹配结果
     */
    private int onFindStorageMusicListEvent(
            String title, String artist, String album, int type, List<MusicInfo> infoList) {
        if (null == infoList || infoList.isEmpty()) {
            return -1;
        }

        int index;
        for (index = 0; index < infoList.size(); ++index) {
            MusicInfo info = infoList.get(index);

            if (type == 0) {
                // 歌曲名称关键字匹配
                if (info.mFileName.contains(title) || info.mTitle.contains(title)) {
                    break;
                }
            } else if (type == 1) {
                // 艺术家匹配 + 歌曲名称关键字匹配
                if ((info.mFileName.contains(title) || info.mTitle.contains(title))
                        && info.mArtist.equals(artist)) {
                    break;
                }
            } else if (type == 2) {
                // 专辑名匹配 + 歌曲名称关键字匹配
                if ((info.mFileName.contains(title) || info.mTitle.contains(title))
                        && info.mAlbum.equals(album)) {
                    break;
                }
            }
        }

        if (index < infoList.size()) {
            return index;
        }

        return -1;
    }

    /**
     * 更新播放信息到通知栏
     * <pre>
     *    播放状态改变的时候都需要更新；
     *    焦点被抢走的时候就不需要更新了；
     * </pre>
     */
    private void onUpdateNotification() {
        // 无焦点情况下关闭通知菜单
        if (mAudioFocusListener != null) {
            int focusState = mAudioFocusListener.focusState;
            if (focusState == AudioManager.AUDIOFOCUS_LOSS
                    || focusState == AudioManager.AUDIOFOCUS_NONE) {
                cancelNotification();
                return;
            }
        }

        // 音乐播放列表不为空
        if (!mAppData.musicPlaylist().isEmpty()) {
            updateMusicNotification();
            sendNotifyMediaState();
        }
    }

    /**
     * [轮询 ACC de 状态]
     * <pre>
     *    主要是避免休眠唤醒时候的各种未知状态；
     *    例如: 休眠状态时候，接收到系统的 ACC-OFF 广播太慢，导致没有停止播放，被 vold 进程 kill 掉；
     *         只有系统 AMS 阻塞或者异常情况下，才会出现系统广播接收延迟的问题；
     * </pre>
     */
    private void onMsgPollingAccStatus() {
        // [检查 ACC ON 状态]
        if (!mIsPowerOff) {
            boolean isAccOff = !AutoStatus.isRealtimeAccON();
            if (isAccOff) {
                Log.d(TAG, "onMsgPollingAccStatus: ACC-OFF.");
                onAccOffBroadcastEvent();
            }
        }

        H0.sendEmptyUniqueMessageDelayed(
                MsgEx.MSG_POLLING_ACC_STATUS, 6000);
    }

    /** 控制过滤类事件处理器 **/
    private final Handler mControlHandler = new ControlHandler(Looper.getMainLooper());

    @SuppressLint("HandlerLeak")
    private class ControlHandler extends HHandler {
        // 消息定义（触发播放设置、过滤上下曲按键事件等）
        private static final int EVENT_SET_DATA_SOURCE = 0;
        private static final int EVENT_KEY_PREV_NEXT_FILTER = 1;
        private static final int EVENT_REVERSE_STATUS = 5;

        public ControlHandler(@NonNull Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_REVERSE_STATUS: {
                    boolean isRev = (Boolean) msg.obj;
                    if (isRev) {
                        doShouldPauseEvent(false);
                    } else {
                        doShouldPlayEvent("reverse-off");
                    }
                    break;
                }

                case EVENT_SET_DATA_SOURCE: {
                    if (mAppData.mCurrentMediaInfo != null) {
                        LogUtil.d(TAG, " -- EVENT_SET_DATA_SOURCE");
                        MediaModel.call()
                                .playerModel()
                                .onLocalSetDataSourceEvent(mAppData.mCurrentMediaInfo);
                    }
                    break;
                }

                default:
                    break;
            }

            super.handleMessage(msg);
        }
    }

    /**
     * 外部事件广播：语音、ACC等
     * <p> 这里接收处理的都是跨进程广播，命名建议和 Local 广播区分开来；
     */
    private class ExternalActionReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            LogUtil.i(TAG, "ExternalActionReceiver: " + action);

            assert action != null;
            switch (action) {
                // HMediaService[RemoteService] Event
                case SpecialChain.ACTION_MESSAGE_CALLBACK:
                    int eventID = intent.getIntExtra(
                            SpecialChain.EXTRA_CALLBACK_TYPE, -1);
                    String strPath = intent.getStringExtra(
                            SpecialChain.EXTRA_CALLBACK_DATA);

                    if (mRemoteService != null) {
                        onActionMessageCallback(eventID, strPath);
                    }
                    break;
                case IConstant.ACTION_NOTIFICATION_PREV:
                    if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
                        // 当前音乐播放列表不为空
                        if (!mAppData.musicPlaylist().isEmpty()) {
                            MediaModel.call()
                                    .playerModel()
                                    .onSetSeekTimeZero();
                            onPlayControl(IMusicState.PLAY_CMD_PREV);
                        }
                    }
                    break;
                case IConstant.ACTION_NOTIFICATION_NEXT:
                    if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
                        // 当前音乐播放列表不为空
                        if (!mAppData.musicPlaylist().isEmpty()) {
                            MediaModel.call()
                                    .playerModel()
                                    .onSetSeekTimeZero();
                            onPlayControl(IMusicState.PLAY_CMD_NEXT);
                        }
                    }
                    break;
                case IConstant.ACTION_NOTIFICATION_PLAYPAUSE:
                    if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC
                            || mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
                        mAppData.mAllowResumePlay = true;
                        onPlayControl(IMusicState.PLAY_CMD_PLAY_PAUSE);
                    }
                    break;
                case IConstant.ACTION_NOTIFICATION_SHOW:
                    Intent newIntent = new Intent(context, MusicUI.sAliasClass);
                    newIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    context.startActivity(newIntent);
                    break;
                case IConstant.ACTION_NOTIFICATION_CANCEL:
                    H0.sendEmptyMessage(MsgEx.MSG_CANCEL_NOTIFICATION);
                    break;
                case ACTION_VOICE_EVENT_PLAY:
                case ACTION_VOICE_EVENT_VIDEO_PLAY:
                    doMediaKeyEvent(IMusicState.PLAY_CMD_PLAY);
                    break;
                case ACTION_VOICE_EVENT_PAUSE:
                case ACTION_VOICE_EVENT_VIDEO_PAUSE:
                    doMediaKeyEvent(IMusicState.PLAY_CMD_PAUSE);
                    break;
                case ACTION_VOICE_EVENT_NEXT:
                    MediaModel.call()
                            .playerModel()
                            .onSetSeekTimeZero();
                    doMediaKeyEvent(IMusicState.PLAY_CMD_NEXT);
                    break;
                case ACTION_VOICE_EVENT_PREV:
                    MediaModel.call()
                            .playerModel()
                            .onSetSeekTimeZero();
                    doMediaKeyEvent(IMusicState.PLAY_CMD_PREV);
                    break;
                case ACTION_VOICE_EVENT_MODE_LOOP_ALL:
                    doChangeRepeatMode(IMusicState.REPEAT_MODE_QUEUE);
                    break;
                case ACTION_VOICE_EVENT_MODE_LOOP_ONE:
                    doChangeRepeatMode(IMusicState.REPEAT_MODE_ONE);
                    break;
                case ACTION_VOICE_EVENT_MODE_RANDOM:
                    doChangeRepeatMode(IMusicState.REPEAT_MODE_RANDOM);
                    break;
                case ACTION_EVENT_K_SCROLL_L:
                    handleSmartKeyEvent(IMusicState.PLAY_CMD_SMART_CW);
                    break;
                case ACTION_EVENT_K_SCROLL_R:
                    handleSmartKeyEvent(IMusicState.PLAY_CMD_SMART_CCW);
                    break;
                case ACTION_EVENT_K_ENTER:
                    handleSmartKeyEvent(IMusicState.PLAY_CMD_SMART_ENTER);
                    break;
                case CarStatus.ACTION_ACC:
                    // [ACC-ON: true, ACC-OFF: false]
                    onActionAccStatus(intent);
                    break;
                case CarStatus.ACTION_PARKING:
                    // 刹车状态检测[BRAKE]
                    onActionParkingStatus(intent);
                    break;
                case CarStatus.ACTION_REVSTATUS:
                    // 只有视频才处理，因为视频在倒车的时候如果不暂停，可能导致系统处理不过来倒车画面的渲染，导致倒车画面卡顿；
                    onActionReverseStatus(intent);
                    break;
                case BluetoothCompat.HeadsetClient.ACTION_AUDIO_STATE_CHANGED:
                    onActionBTAudioStateChanged(intent);
                    break;
                case ACTION_VOICE_EVENT_REWIND:
                    doMediaKeyEvent(KeyEvent.KEYCODE_MEDIA_REWIND);
                    break;
                case ACTION_VOICE_EVENT_FAST_FORWARD:
                    doMediaKeyEvent(KeyEvent.KEYCODE_MEDIA_FAST_FORWARD);
                    break;
                case ACTION_REFRESH_PATH:
                    onRefreshMusicPlaylist(intent.getStringExtra(SpecialChain.EXTRA_CALLBACK_DATA));
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * 处理 Parking 状态
     * @param intent 广播意图
     */
    private void onActionParkingStatus(@NonNull Intent intent) {
        mInBrakingState = intent.getBooleanExtra(CarStatus.EXTRA_PARKING, false);

        if (!mAppData.mDrivingWatchVideoEnable) {
            // 发送状态到 UI 界面[驾驶警告页面提示切换]
            sendLocalBroadcast(IMediaEvent.EVENT_UPDATE_AUTO_BRAKE_STATUS);
        }

        // 行车中不可以观看视频（安全法规）
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            if (isCanWatchVideo()) {
                H0.removeCallbacks(mExecuteParkingPauseTask);
                H0.postUniqueDelayed(
                        mUpdateParkingStateTask, 1000);
            } else {
                H0.removeCallbacks(mUpdateParkingStateTask);
                H0.postUniqueDelayed(
                        mExecuteParkingPauseTask, 600);
            }
        }
    }

    /**
     * 更新刹车状态任务
     * <pre>
     *    主要是为了避免快速频繁改变刹车状态而导致的播放状态不一致问题；
     *    如果当前在视频界面，且刹车状态改变能观看视频，我们需要检查是否可以恢复视频播放；
     * </pre>
     */
    private final Runnable mUpdateParkingStateTask = () -> {
        // 只处理视频模式场景
        if (!mAppData.isMediaType(
                IMusicState.MEDIA_TYPE_VIDEO)) {
            return;
        }

        // 能看视频且在视频播放界面
        if (isCanWatchVideo()
                && mAppData.mInVideoPlayUi) {
            // 检查是否可以恢复视频播放
            doShouldPlayEvent("update-parking");
        }
    };

    /**
     * 执行行车中不可观看视频任务
     * <pre>
     *    如果因为刹车状态改变需要暂停视频播放，我们需要延时一点点后再执行，
     *    主要是为了避免先看到暂停状态后，才看到驾驶警告页面（做显示同步）；
     * </pre>
     */
    private final Runnable mExecuteParkingPauseTask = () -> {
        // 只处理视频模式场景
        if (!mAppData.isMediaType(
                IMusicState.MEDIA_TYPE_VIDEO)) {
            return;
        }

        if (!isCanWatchVideo()) {
            doShouldPauseEvent(false);
        }
    };

    /**
     * 处理 ACC 广播事件
     * @param intent 广播意图
     */
    private void onActionAccStatus(@NonNull Intent intent) {
        if (intent.getBooleanExtra(CarStatus.EXTRA_ACC, true)) {
            Log.d(TAG, "LocalOtherReceiver: ACC-ON.");
            onAccOnBroadcastEvent();
        } else {
            Log.d(TAG, "LocalOtherReceiver: ACC-OFF.");
            onAccOffBroadcastEvent();
        }
    }

    /**
     * 处理倒车广播事件
     * @param intent 广播意图
     */
    private void onActionReverseStatus(@NonNull Intent intent) {
        // 获取倒车状态
        boolean isReverseState = intent.getBooleanExtra(CarStatus.EXTRA_REVSTATUS, false);
        mInReverseOnBroadcast = isReverseState;

        // 音乐模式不处理
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
            return;
        }

        // 在蓝牙通话状态
        if (mCallState) {
            Log.d(TAG, "onActionReverseStatus, is BT call state!");
            return;
        }

        Message msg = new Message();
        msg.what = ControlHandler.EVENT_REVERSE_STATUS;
        msg.obj = isReverseState;
        mControlHandler.removeMessages(msg.what);

        if (isReverseState) {
            mControlHandler.sendMessage(msg);
        } else {
            mControlHandler.sendMessageDelayed(msg, 500);
        }
    }

    /**
     * 蓝牙 Audio 状态改变
     * @param intent 广播意图
     */
    private void onActionBTAudioStateChanged(@NonNull Intent intent) {
        // 高版本收不到该广播，暂不需要特殊处理
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            return;
        }

        int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE, -1);
        switch (state) {
            case BluetoothProfile.STATE_CONNECTING:
                Log.d(TAG, "onActionBTAudioStateChanged: state = " + "STATE_CONNECTING");
                break;
            case BluetoothProfile.STATE_CONNECTED:
                mCallState = true;
                Log.d(TAG, "onActionBTAudioStateChanged: state = " + "STATE_CONNECTED");
                break;
            case BluetoothProfile.STATE_DISCONNECTING:
                Log.d(TAG, "onActionBTAudioStateChanged: state = " + "STATE_DISCONNECTING");
                break;
            case BluetoothProfile.STATE_DISCONNECTED:
                mCallState = false;
                Log.d(TAG, "onActionBTAudioStateChanged: state = " + "STATE_DISCONNECTED");
                Log.d(TAG, "onActionBTAudioStateChanged: = " + !existsHighPriorityEvent()
                        + ", " + isClientConnected() + "," + isRemoteConnected() + "," + Objects.isNull(mAudioFocusListener));

                // 不存在高优先级事件/且有音频焦点；
                if (!existsHighPriorityEvent()
                        && isClientConnected()
                        && isRemoteConnected()
                        && mAudioFocusListener != null) {
                    Log.d(TAG, "onActionBTAudioStateChanged: focusState = " + mAudioFocusListener.focusState);
                    if (mAudioFocusListener.focusState == AudioManager.AUDIOFOCUS_GAIN) {
                        if (mAudioHandler.hasMessages(AudioHandler.WHAT_AUDIO_FOCUS_CHANGE)) {
                            return;
                        }

                        // 尝试恢复媒体播放状态
                        mAudioHandler.obtainMessage(
                                AudioHandler.WHAT_AUDIO_FOCUS_CHANGE,
                                AudioManager.AUDIOFOCUS_GAIN, 0).sendToTarget();

                    } else if (mAudioFocusListener.focusState == AudioManager.AUDIOFOCUS_NONE) {
                        // 播放蓝牙音乐时蓝牙来/去电，通话时车机端切换音乐或者视频模式界面后挂断电话，蓝牙音乐声音与音乐/视频模式混音
                        // 当蓝牙挂断、当前应用没有音频焦点并处于前台，尝试请求焦点
                        if (MiscUtils.isForegroundApp(mContext, IMedia.MEDIA_SERVICE_PACKAGE_NAME)) {
                            int status = mAudioManager.requestAudioFocus(mAudioFocusListener,
                                    AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN);
                            mAudioFocusListener.focusState = status;

                            // 请求失败返回
                            if (status != AudioManager.AUDIOFOCUS_GAIN) {
                                return;
                            }

                            // 注册 Media Button 事件
                            registerMediaButtonEvent();
                        }
                    }
                }
                break;
            default:
                break;
        }
    }

    /** [处理 ACC-ON 广播事件] **/
    private void onAccOnBroadcastEvent() {
        if (mIsPowerOff) {
            mIsPowerOff = false;

            if (!mIsDeepSleep) {
                doShouldPlayEvent();
            }

            mIsDeepSleep = false;
            H0.sendEmptyUniqueMessageDelayed(
                    MsgEx.MSG_POLLING_ACC_STATUS, 6000);
        }

        // 通知 UI 当前 ACC 状态
        HEventBus.post(
                HMessage.obtain(
                        IAutoEvent.EVENT_ACC_STATUS_ON));
    }

    /** [处理 ACC-OFF 广播事件] **/
    private void onAccOffBroadcastEvent() {
        if (!mIsPowerOff) {
            mIsPowerOff = true;

            doShouldPauseEvent(false);
            writeCurrentMediaTime(false, 10);

            mIsDeepSleep = false;
            H0.removeMessages(MsgEx.MSG_POLLING_ACC_STATUS);
        }

        // 通知 UI 当前 ACC 状态
        HEventBus.post(
                HMessage.obtain(
                        IAutoEvent.EVENT_ACC_STATUS_OFF));
    }

    /**
     * 音频焦点监听
     * <p> 在后装车载系统中当丢失焦点，一般表示切源了；
     */
    private final class AudioFocusChangeListener implements OnAudioFocusChangeListener {
        public int focusState = AudioManager.AUDIOFOCUS_NONE;

        @Override
        public void onAudioFocusChange(int arg0) {
            focusState = arg0;
            mAudioHandler.obtainMessage(
                    AudioHandler.WHAT_AUDIO_FOCUS_CHANGE, arg0, 0).sendToTarget();
        }
    }

    // handler
    @SuppressLint("HandlerLeak")
    private final class AudioHandler extends Handler {
        public static final int WHAT_AUDIO_FADE_DOWN = 1;
        public static final int WHAT_AUDIO_FADE_UP = 2;
        public static final int WHAT_AUDIO_FOCUS_CHANGE = 3;

        public AudioHandler(@NonNull Looper looper) {
            super(looper);
        }

        public void OnAudioFocusChangerEvent(int focusType) {
            if (mIsPowerOff) {
                LogUtil.d(TAG, "current state is Acc don't anything");
                switch (focusType) {
                    case AudioManager.AUDIOFOCUS_GAIN:
                        // [对策可能出现的休眠唤醒起来没声音现象]
                        if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_IDLE) {
                            MediaModel.call()
                                    .playerModel()
                                    .requestSetVolume(1.0f);
                        }
                        break;
                    case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                    case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                    case AudioManager.AUDIOFOCUS_LOSS:
                    default:
                        break;
                }
                return;
            }

            switch (focusType) {
                case AudioManager.AUDIOFOCUS_LOSS: {
                    LogUtil.d(TAG, ">>>> OnAudioFocusChangerEvent AUDIOFOCUS_LOSS");

                    // 释放 MediaButton
                    unregisterMediaButtonEvent();

                    // 暂停播放、保持播放信息
                    if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
                        cancelNotification();
                        onLocalMusicPlayControl(IMusicState.PLAY_CMD_PAUSE);
                    } else if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
                        onLocalVideoPlayControl(IMusicState.PLAY_CMD_PAUSE);

                        // 视频丢失焦点可直接退出
                        requestExitApplication(4);
                        return;
                    } else {
                        return;
                    }

                    mAppData.mAllowResumePlay = true;
                    writeCurrentMediaTime(true, 11);
                    break;
                }

                // frameworks/base/service/.../audio/FocusRequester.java
                // mSdkTarget <= MediaFocusControl.DUCKING_IN_APP_SDK_LEVEL 才能收到。
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK: {
                    LogUtil.d(TAG, ">>>> OnAudioFocusChangerEvent AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK");
                    if (IMusicState.MEDIA_TYPE_IDLE == mAppData.mMediaType) {
                        return;
                    }

                    // [如果系统有混音比例定义开关]
                    int audioBackgroundVolume = Settings.System.getInt(
                            getContentResolver(), HConfig.audio_background_volume, -1);
                    if (audioBackgroundVolume == -1) {
                        // [如果没有该配置，强制压低到 50 % 的音量]
                        MediaModel.call()
                                .playerModel()
                                .requestSetVolume(0.5f);
                    } else if (audioBackgroundVolume >= 0 && audioBackgroundVolume <= 100) {
                        // [如果存在配置，则在 90% 的基准上做混音]
                        MediaModel.call()
                                .playerModel()
                                .requestSetVolume(0.9f * audioBackgroundVolume / 100);
                    }
                    break;
                }

                // [短暂丢失焦点]
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT: {
                    LogUtil.d(TAG, ">>>> OnAudioFocusChangerEvent AUDIOFOCUS_LOSS_TRANSIENT");

                    if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
                        doShouldPauseEvent(true);
                    } else if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
                        doShouldPauseEvent(false);
                        MediaModel.call()
                                .playerModel()
                                .requestSetVolume(0.0f);
                    }
                    break;
                }

                case AudioManager.AUDIOFOCUS_GAIN: {
                    LogUtil.d(TAG, ">>>> OnAudioFocusChangerEvent AUDIOFOCUS_GAIN");
                    if (IMusicState.MEDIA_TYPE_IDLE == mAppData.mMediaType) {
                        return;
                    }

                    // 注册 Media Button 事件
                    registerMediaButtonEvent();

                    // [视频模式/如果又在倒车状态，不处理]
                    if (mInReverseOnBroadcast
                            && mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
                        MediaModel.call()
                                .playerModel()
                                .requestSetVolume(1.0f);
                        return;
                    }

                    doShouldPlayEvent();
                    MediaModel.call()
                            .playerModel()
                            .requestSetVolume(1.0f);
                    break;
                }

                default:
                    break;
            }
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            switch (msg.what) {
                case WHAT_AUDIO_FOCUS_CHANGE:
                    OnAudioFocusChangerEvent(msg.arg1);
                    break;

                case WHAT_AUDIO_FADE_DOWN:
                case WHAT_AUDIO_FADE_UP:
                default:
                    break;
            }
        }
    }

    /** [语音控制类触发事件] **/
    private class VoiceControlReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            LogUtil.i(TAG, ">>>>> action:" + action);

            assert action != null;
            switch (action) {
                case ACTION_MUSIC_SINGLE_MODEL:
                    onChangeRepeatMode(IMusicState.REPEAT_MODE_ONE);
                    break;
                case ACTION_MUSIC_RANDOM_MODEL:
                    onChangeRepeatMode(IMusicState.REPEAT_MODE_RANDOM);
                    break;
                case ACTION_MUSIC_ALL_LOOP_MODEL:
                    onChangeRepeatMode(IMusicState.REPEAT_MODE_QUEUE);
                    break;
                case ACTION_MUSIC_PLAY:
                    doMediaKeyEvent(IMusicState.PLAY_CMD_PLAY);
                    mAppData.mAllowResumePlay = true;
                    break;
                case ACTION_MUSIC_PAUSE:
                    doMediaKeyEvent(IMusicState.PLAY_CMD_PAUSE);
                    break;
                case ACTION_NOTIFICATION_PREV:
                    MediaModel.call()
                            .playerModel()
                            .onSetSeekTimeZero();
                    doMediaKeyEvent(IMusicState.PLAY_CMD_PREV);
                    break;
                case ACTION_NOTIFICATION_NEXT:
                    MediaModel.call()
                            .playerModel()
                            .onSetSeekTimeZero();
                    doMediaKeyEvent(IMusicState.PLAY_CMD_NEXT);
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * 快退
     */
    private void onSeekRewind() {
        if (null == mAppData.mPlayTimeInfo || mAppData.mPlayTimeInfo.mTotalTime <= 0) {
            LogUtil.d(TAG, "onSeekNext do nothing mPlayTimeInfo or mTotalTime is null");
            return;
        }

        int nTime = mAppData.mPlayTimeInfo.mCurrentTime - SEEK_STEP;
        if (nTime < 0) {
            nTime = 0;
        }
        trySeekToTime(nTime);
    }

    /**
     * 快进
     */
    private void onFastForward() {
        if (null == mAppData.mPlayTimeInfo || mAppData.mPlayTimeInfo.mTotalTime <= 0) {
            LogUtil.d(TAG, "onSeekNext do nothing mPlayTimeInfo or mTotalTime is null");
            return;
        }

        int nTime = mAppData.mPlayTimeInfo.mCurrentTime + SEEK_STEP;
        if (nTime > mAppData.mPlayTimeInfo.mTotalTime) {
            nTime = mAppData.mPlayTimeInfo.mTotalTime;
        }
        trySeekToTime(nTime);
    }
}
