package com.hcn.media.local.base;

import android.Configures.HConfig;
import android.annotation.SuppressLint;
import android.carstatus.CarStatus;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.AudioManager;
import android.os.Build;
import android.os.Environment;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.UserHandle;
import android.provider.Settings;
import android.support.v4.media.session.MediaSessionCompat;
import android.support.v4.media.session.PlaybackStateCompat;
import android.text.TextUtils;
import android.view.KeyEvent;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import com.google.common.util.concurrent.ServiceManager;
import com.hcn.auto_compat.os.ServiceManagerCompat;
import com.hcn.common.misc.LogUtils;
import com.hcn.media.IMediaClientThread;
import com.hcn.media.base.IMedia;
import com.hcn.media.base.Preferences;
import com.hcn.media.base.service.BaseService;
import com.hcn.media.local.event.MediaSessionCallback;
import com.hcn.media.local.event.MusicIntentReceiver;
import com.hcn.media.local.event.VehicleConfigEx;
import com.hcn.media.local.observer.MediaEventObserver;
import com.hcn.media.local.utils.FileStorageState;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.key.IKeyEvent;
import com.hcn.media_base.key.KeyEventExt;
import com.hcn.media_common.HBroadcastEx;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.debug.MediaDebug;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_data.notify.NotifyMediaState;
import com.hcn.media_data.storage.StorageDeviceEx;
import com.hcn.media_model.MediaModel;
import com.hcn.media_theme.Argument;
import com.hcn.mediaservice.api.IMediaObserver;
import com.hcn.mediaservice.api.IRemotePlayerService;
import com.hcn.mediaservice.data.MusicInfo;

import java.io.File;
import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Objects;

/**
 * 媒体后台服务
 * <pre>
 *    这个抽象类主要是为了拆解 LocalService.java 文件；
 *    把媒体远程数据服务的绑定和操作接口封装到这里；
 * </pre>
 *
 * @author 65821
 */
public abstract class MediaService extends BaseService
        implements IMediaEventListener, IKeyEvent {
    protected static final String TAG = "MediaService";

    /** 内容提供者解析器 **/
    protected ContentResolver mContentResolver;

    /** 存储对外通知信息数据 **/
    protected final NotifyMediaState mNotifyMediaState = new NotifyMediaState();

    /**
     * 车辆相关状态成员变量
     * <pre>
     *    刹车状态、倒车状态...
     *    车速相关状态与事件（前装兼容）
     * </pre>
     */
    protected boolean mInBrakingState = false;
    protected final CarStatus mCarsStatus = new CarStatus();
    protected final VehicleConfigEx mVehicleConfig;

    /** 存储车机系统相关临时状态 **/
    protected boolean mIsPowerOff = false;
    protected boolean mIsDeepSleep = false;
    protected boolean mCallState = false;

    /**
     * Media Button 接受者
     * <p> 接收处理外部按键事件；
     */
    protected AudioManager mAudioManager = null;
    protected ComponentName mMediaButtonReceiver;
    protected final PlaybackStateCompat.Builder mPlaybackStateBuilder;
    protected MediaSessionCompat mMediaSessionCompat;

    /** 存储设备相关数据状态对象 **/
    protected FileStorageState mFileStorageState = null;
    protected StorageDeviceEx mFlashDevice = null;
    protected StorageDeviceEx mSdDevice = null;
    protected StorageDeviceEx mUsbDevice = null;
    protected List<StorageDeviceEx> mStorageDeviceList = null;

    /** 媒体事件观察者（分发器） */
    protected final IEventObserver mEventObserver = new MediaEventObserver();

    /** 本地服务 Binder 对象 **/
    protected IBinder mMediaBinder = null;
    protected boolean mIsClientConnected;

    /** 远程服务 Binder 接口 **/
    protected IRemotePlayerService mRemoteService = null;
    protected final MediaClientThread mMediaClientThread;
    protected IMediaObserver mMediaObserver = null;

    /**
     * 远程数据扫描服务连接器
     * <pre>
     *    参考 IRemotePlayerService 服务接口定义；
     *    远程服务是一个独立的进程（HMediaService）；
     * </pre>
     */
    private final ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceDisconnected(ComponentName name) {
            LogUtil.d(TAG, ">>>> RemoteService/onServiceDisconnected.");
            mRemoteService = null;

            // 重置服务端的观察者对象
            synchronized (mMediaClientThread) {
                mMediaObserver = null;
            }
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            LogUtil.d(TAG, ">>>> RemoteService/onServiceConnected.");
            mRemoteService = IRemotePlayerService.Stub.asInterface(service);

            try {
                mRemoteService.setClientBinder(mMediaBinder);
                mRemoteService.attachMediaClient("HMediaPlayer", mMediaClientThread);

                //恢复服务记忆的状态
                mAppData.mVideoScaleType = mRemoteService.readVideoScaleType();
            } catch (RemoteException ex) {
                ex.printStackTrace();
            }

            // 初始化存储设备（取数据列表）
            for (StorageDeviceEx device : mStorageDeviceList) {
                initStorageDevice(device);
            }

            // 按优先级获取一个有效的存储设备（默认是 Flash 存储设备）
            if (mAppData.mCurrentDevice == mFlashDevice) {
                mAppData.mCurrentDevice = getValidStorageDevice();
            }

            // 选择存储设备默认同当前存储设备
            mAppData.mSelectedDevice = mAppData.mCurrentDevice;

            // 通知远程服务初始化完成
            if (isClientConnected()) {
                sendLocalBroadcast(IMediaEvent.EVENT_SERVICE_INITIALIZED);
            }
        }
    };

    /**
     * 媒体客户端代理
     * <p> 客户端代理对象，attach 到远程媒体数据服务；
     */
    private static class MediaClientThread extends IMediaClientThread.Stub {
        private final Reference<MediaService> mOwnerRef;

        public MediaClientThread(MediaService service) {
            mOwnerRef = new WeakReference<>(service);
        }

        @Override
        public void bindMediaClient(IMediaObserver iMediaObserver) {
            MediaService s = mOwnerRef.get();
            if (s != null) {
                Message msg = Message.obtain(
                        s.H0, H.MSG_BIND_MEDIA_CLIENT, iMediaObserver);
                s.H0.removeMessages(H.MSG_BIND_MEDIA_CLIENT);
                s.H0.sendMessage(msg);
            }
        }

        @Override
        public void scheduleTask(String task, String arg) {
            LogUtils.dTag(TAG, "scheduleTask: " + task);
        }
    }

    protected MediaService() {
        super();

        // 车辆相关配置扩展对象
        mVehicleConfig = new VehicleConfigEx(this);

        // 媒体客户端 Binder
        mMediaClientThread = new MediaClientThread(this);

        // 播放状态构建器（监听）
        mPlaybackStateBuilder = new PlaybackStateCompat.Builder()
                .setActions(PlaybackStateCompat.ACTION_PLAY |
                        PlaybackStateCompat.ACTION_PAUSE |
                        PlaybackStateCompat.ACTION_PLAY_PAUSE |
                        PlaybackStateCompat.ACTION_STOP |
                        PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS |
                        PlaybackStateCompat.ACTION_SKIP_TO_NEXT |
                        PlaybackStateCompat.ACTION_FAST_FORWARD |
                        PlaybackStateCompat.ACTION_REWIND);
    }

    @SuppressLint("NewApi")
    @Override
    public void onCreate() {
        super.onCreate();

        // 内容解析器
        mContentResolver = getContentResolver();

        // AudioFocus/MediaButton
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);

        // 存储状态工具
        mFileStorageState = new FileStorageState(getApplicationContext());

        // 又重搞一份[就很离谱]
        mFlashDevice = mAppData.mFlashStorage;
        mSdDevice = mAppData.mSdStorage;
        mUsbDevice = mAppData.mUsbStorage;

        // 按循序加入列表（不要打乱）
        mStorageDeviceList = mAppData.mStorageDeviceList;
        mStorageDeviceList.add(mUsbDevice);
        mStorageDeviceList.add(mSdDevice);
        mStorageDeviceList.add(mFlashDevice);

        // 行车中观看视频监听
        mAppData.mDrivingWatchVideoEnable = (1 == Settings.System.getInt(
                mContentResolver, HConfig.driving_disable_video, 0));
        mIsPowerOff = mCarsStatus.getAccStatus() == 0;

        // 绑定远程媒体数据服务
        onBindServiceEvent();

        // 添加到 ServiceManager 中（需要 selinux 权限）
        // ServiceManagerCompat.addService("media.HMediaPlayer", mMediaBinder);
    }

    /** 当前 ACC 状态 **/
    public boolean inAccON() {
        return !mIsPowerOff;
    }

    /** 远程数据服务连接状态 **/
    public boolean isRemoteConnected() {
        return mRemoteService != null;
    }

    /** 绑定媒体远程数据服务 **/
    @RequiresApi(api = Build.VERSION_CODES.R)
    private void onBindServiceEvent() {
        Intent intent = new Intent("com.hcn.mediaservice.service.MediaPlayerService");
        intent.setClassName("com.hcn.mediaservice",
                "com.hcn.mediaservice.service.MediaPlayerService");
        intent.putExtra(IMedia.START_REASON_EXTRA_KEY, "BIND_MEDIAPLAYER");

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            bindServiceAsUser(intent, mConnection,
                    Context.BIND_AUTO_CREATE, UserHandle.getUserHandleForUid(BaseMediaData.UID));
        } else {
            bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        }
    }

    /**
     * 解绑媒体远程数据服务
     * <p> 现阶段基本不可能使用到；
     */
    public void onUnbindServiceEvent() {
        if (mRemoteService != null) {
            unbindService(mConnection);
            mRemoteService = null;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        super.onBind(intent);
        LogUtil.d(TAG, ">>>>> onBind.");
        mIsClientConnected = true;
        return mMediaBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        LogUtil.d(TAG, ">>>>> onUnbind.");
        mIsClientConnected = false;
        return super.onUnbind(intent);
    }

    @Override
    public void onRebind(Intent intent) {
        super.onRebind(intent);
        mIsClientConnected = true;
        LogUtil.d(TAG, ">>>>> onRebind.");
    }

    /** 是本地客户端连接状态 **/
    public boolean isClientConnected() {
        return mIsClientConnected;
    }

    /**
     * 注册媒体按键事件
     * <pre>
     *    监听外部 MediaButton 事件；
     *    注意：不请求音频焦点就不要去注册 MediaButton 事件，否则其它模块会注册失败；
     * </pre>
     */
    @SuppressLint("ObsoleteSdkInt")
    public void registerMediaButtonEvent() {
        // Audio Service
        if (null == mAudioManager) {
            mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        }
        List<MediaSessionCallback> mediaSessionCallbackList =new ArrayList<>();
        mediaSessionCallbackList.(add)
        MediaSessionCallback mediaSessionCallback;
        mediaSessionCallback=
        // 高版本不在使用过时的接口
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            if (mMediaSessionCompat == null) {
                mMediaSessionCompat = new MediaSessionCompat(this, MediaDebug.TAG);
                mMediaSessionCompat.setMediaButtonReceiver(null);
                mMediaSessionCompat.setPlaybackState(mPlaybackStateBuilder.build());
                mMediaSessionCompat.setCallback(
                        new MediaSessionCallback(this, this));
            }

            // 避免重复激活调用
            if (!mMediaSessionCompat.isActive()) {
                mMediaSessionCompat.setActive(true);
            }
        } else {
            if (mMediaButtonReceiver == null) {
                mMediaButtonReceiver = new ComponentName(
                        getPackageName(), MusicIntentReceiver.class.getName());
            }

            mAudioManager.registerMediaButtonEventReceiver(mMediaButtonReceiver);
        }
    }

    /**
     * 取消媒体按键事件监听
     * <p> 如果不取消，其它模式如果注册将注册请求失败；
     *
     * @see #registerMediaButtonEvent()
     */
    @SuppressLint("ObsoleteSdkInt")
    protected void unregisterMediaButtonEvent() {
        // 高版本不在使用过时的接口
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            if (mMediaSessionCompat != null) {
                mMediaSessionCompat.release();
                mMediaSessionCompat = null;
            }
        } else {
            if (mMediaButtonReceiver != null) {
                mAudioManager.unregisterMediaButtonEventReceiver(mMediaButtonReceiver);
                mMediaButtonReceiver = null;
            }
        }
    }

    /**
     * 处理服务反向 bind 媒体客户端事件
     * @see H#MSG_BIND_MEDIA_CLIENT
     *
     * @param obj {@link IMediaObserver}
     */
    private void onMsgBindMediaClient(Object obj) {
        synchronized (mMediaClientThread) {
            if (!(obj instanceof IMediaObserver)) {
                return;
            }

            mMediaObserver = (IMediaObserver) obj;
        }
    }

    /**
     * 视频播放准备事件
     * <p> 可以用来更新当前正在播放的视频 Title 信息；
     *
     * @param obj {@link MusicInfo}
     * @see IMediaEvent#EVENT_VIDEO_PLAYER_PREPARING
     */
    protected void onVideoPlayerPreparing(Object obj) {
        if (!(obj instanceof MusicInfo)) {
            return;
        }

        synchronized (mMediaClientThread) {
            if (Objects.isNull(mMediaObserver)) {
                return;
            }

            try {
                MusicInfo info = (MusicInfo) obj;
                if (info != mAppData.mCurrentMediaInfo) {
                    return;
                }

                // 我们现在只对外发送软解吗的视频
                if (MediaModel.call()
                        .playerModel()
                        .isMediaPlayerValid()) {
                    // 硬解码媒体直接返回
                    return;
                }

                mMediaObserver.updateVideoPlayInfo(info.mFilePath, "paused", 0, 0);
            } catch (RemoteException e) {
                LogUtils.wTag(TAG, "updateVideoPlayInfo: " + e.getMessage());
            }
        }
    }

    /**
     * 分发媒体事件
     * <p> 转发处理关心的媒体事件，例如：对外部客户端发布媒体事件；
     *
     * @param eventId {@link IMediaEvent}
     * @param wParam 附加参数 1
     * @param lParam 附加参数 2
     */
    public void dispatchMediaEvent(int eventId, Object wParam, Object lParam) {
        // 暂时只处理音频模式的事件信息
        if (mAppData.isMediaType(IMusicState.MEDIA_TYPE_VIDEO)) {
            return;
        }

        int mediaEventId = eventId;
        Object wParamObj = wParam;
        Object lParamObj = lParam;

        switch(eventId) {
            case IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME:
                wParamObj = mAppData.mPlayTimeInfo;
                break;
            case IMediaEvent.EVENT_UPDATE_MUSIC_ID3:
                wParamObj = mAppData.mCurrentMediaInfo;
                break;
            case IMediaEvent.EVENT_MEDIA_COMPLETION:
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_PLAY:
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_PAUSE:
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_STOP:
                mediaEventId = IMediaEvent.EVENT_CHANGE_PLAY_STATE;
                wParamObj = BaseMediaData.call().mediaPlayState();
                break;
            case IMediaEvent.EVENT_NONE:
            default:
                break;
        }

        mEventObserver.dispatchMediaEvent(mediaEventId, wParamObj, lParamObj);
    }

    /**
     * 处理媒体事件
     * <p> 拦截并处理关心的媒体事件；
     *
     * @param eventId {@link IMediaEvent}
     * @param wParam 附加参数 1
     * @param lParam 附加参数 2
     */
    @Override
    public void onMediaEvent(int eventId, Object wParam, Object lParam) {
        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_COMPLETION:
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_PLAY:
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_PAUSE:
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE_STOP:
                onMediaPlayStateChange(100);
                break;
            case IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME:
                onMediaPlayStateChange(1500);
                break;
            default:
                break;
        }

        // 播放时间信息需要特殊处理（对外分发时间信息的好处）
        dispatchMediaEvent(eventId, wParam, lParam);
    }

    /**
     * 媒体播放状态改鬓
     * <p> 不要直接处理，先入一次消息队列；
     *
     * @param delayMillis 更新延时（ms）
     */
    protected void onMediaPlayStateChange(int delayMillis) {
        // 暂时只处理视频模式的播放状态
        if (mAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
            return;
        }

        // 高版本才有 hasCallbacks 接口
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            if (H0.hasCallbacks(mUpdatePlayStateRunnable)) {
                return;
            }
        } else {
            delayMillis = Math.min(delayMillis, 1000);
        }

        H0.postUniqueDelayed(
                mUpdatePlayStateRunnable, delayMillis);
    }

    /**
     * 更新播放状态
     * <p> 对外更新当前播放器的播放状态信息；
     */
    private final Runnable mUpdatePlayStateRunnable = new Runnable() {
        @Override
        public void run() {
            synchronized (mMediaClientThread) {
                if (Objects.isNull(mMediaObserver)) {
                    return;
                }

                try {
                    MusicInfo info = mAppData.currentMediaInfo();
                    if (Objects.isNull(info)) {
                        return;
                    }

                    String playState = "stopped";
                    switch (mAppData.mMediaPlayState) {
                        case IMusicState.E_PLAY_STATE_PLAY:
                            playState = "start";
                            break;
                        case IMusicState.E_PLAY_STATE_PAUSE:
                            playState = "paused";
                            break;
                        case IMusicState.E_PLAY_STATE_STOP:
                        default:
                            break;
                    }

                    // 更新视频播放状态
                    if (mAppData.isMediaType(IMusicState.MEDIA_TYPE_VIDEO)) {
                        // 我们现在只对外发送软解吗的视频
                        if (MediaModel.call()
                                .playerModel()
                                .isMediaPlayerValid()) {
                            // 硬解码媒体直接返回
                            return;
                        }

                        mMediaObserver.updateVideoPlayInfo(
                                info.mFilePath,
                                playState,
                                mAppData.mPlayTimeInfo.mCurrentTime,
                                mAppData.mPlayTimeInfo.mTotalTime);
                    }
                } catch (RemoteException e) {
                    LogUtils.wTag(TAG, "updateVideoPlayInfo: " + e.getMessage());
                }
            }
        }
    };

    /**
     * 对外广播媒体播放状态信息
     * <p> 历史设计接口，建议暂时不要删除；
     */
    protected void sendNotifyMediaState() {
        MusicInfo info = null;
        int repeatMode = 0;

        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
            repeatMode = mAppData.musicRepeatMode();

            if (BaseMediaData.isValidIndex(
                    mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
                info = mAppData.musicPlayPositionInfo();
                mNotifyMediaState.mCurrentIndex = mAppData.musicPlayPosition();
                mNotifyMediaState.mTotalNum = mAppData.musicPlaylist().size();
            }
        } else if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            repeatMode = mAppData.videoRepeatMode();

            if (BaseMediaData.isValidIndex(
                    mAppData.videoPlaylist(), mAppData.videoPlayPosition())) {
                info = mAppData.videoPlayPositionInfo();
                mNotifyMediaState.mCurrentIndex = mAppData.videoPlayPosition();
                mNotifyMediaState.mTotalNum = mAppData.videoPlaylist().size();
            }
        }

        mNotifyMediaState.mMediaType = mAppData.mMediaType;
        if (null == info) {
            mNotifyMediaState.mCurrentIndex = 0;
            mNotifyMediaState.mTotalNum = 0;
        }

        switch (repeatMode) {
            case IMusicState.REPEAT_MODE_QUEUE:
            case IMusicState.REPEAT_MODE_ALL:
                mNotifyMediaState.mPlayMode = 0;
                break;

            case IMusicState.REPEAT_MODE_ONE:
                mNotifyMediaState.mPlayMode = 1;
                break;

            case IMusicState.REPEAT_MODE_RANDOM:
                mNotifyMediaState.mPlayMode = 4;
                break;

            default:
                break;
        }

        Intent intent = new Intent(HBroadcastEx.SpecialChain.ACTION_OTHER_CALLBACK);
        intent.putExtra("TotalNum", mNotifyMediaState.mTotalNum);
        intent.putExtra("CurrentIndex", mNotifyMediaState.mCurrentIndex);
        sendBroadcastAsUser(intent, UserHandle.getUserHandleForUid(BaseMediaData.UID));
    }

    /**
     * 播放控制接口
     * <pre>
     *    {@link IMusicState#PLAY_CMD_PLAY}
     *    {@link IMusicState#PLAY_CMD_PAUSE}
     *    {@link IMusicState#PLAY_CMD_PLAY_PAUSE}
     *    {@link IMusicState#PLAY_CMD_STOP}
     *    {@link IMusicState#PLAY_CMD_PREV}
     *    {@link IMusicState#PLAY_CMD_NEXT}
     * </pre>
     *
     * @param command {@link IMusicState}
     */
    protected abstract void onPlayControl(int command);

    /**
     * 处理 Smart 按键事件
     * <pre>
     *    {@link IMusicState#PLAY_CMD_SMART_CW}
     *    {@link IMusicState#PLAY_CMD_SMART_CCW}
     *    {@link IMusicState#PLAY_CMD_SMART_ENTER}
     * </pre>
     *
     * @param command {@link IMusicState}
     */
    protected abstract void handleSmartKeyEvent(int command);

    /** [存在高优先级状态] **/
    public boolean existsHighPriorityEvent() {
        return mCallState || mIsPowerOff;
    }

    /**
     * 是否能够观看视频
     * <pre>
     *    1、行车中可以观看视频开关；
     *    2、车辆速度视频状态（低速可观看视频）；
     *    3、当前车辆刹车状态（后装就是驻车状态）；
     *    4、当前车辆是否在驻车状态（后装就是检测刹车线）；
     * </pre>
     *
     * @return 能/不能看视频
     */
    public boolean isCanWatchVideo() {
        LogUtil.d(TAG, "isCanWatchVideo"
                + " mAppData.mIsCanWatchVideo: " + mAppData.mDrivingWatchVideoEnable);

        // 检查低速是否可以观看视频
        boolean canWatchVideo = mAppData.mDrivingWatchVideoEnable;
        if (!canWatchVideo) {
            canWatchVideo = mVehicleConfig.vehicleCanWatchVideo();
        }

        return (canWatchVideo || mInBrakingState
                || (mCarsStatus.getParkingStatus() != 0));
    }

    /**
     * 是否能够播放视频
     * @return 能/不能播放
     */
    public boolean isCanPlayVideo() {
        if (Argument.isCanPlayVideoBack()) {
            return isCanWatchVideo();
        } else {
            return isCanWatchVideo()
                    && mAppData.mVideoUiShow
                    && mAppData.mInVideoPlayUi;
        }
    }

    /**
     * 保存播放循环模式到记忆体
     * <p> 避免一改变就马上保存，建议做延时再保存，避免压测写频率过高；
     *
     * @param type 媒体类型
     */
    protected void savePlayRepeatMode(int type) {
        switch (type) {
            case IMusicState.MEDIA_TYPE_MUSIC:
            case IMusicState.MEDIA_TYPE_VIDEO:
                // 延时保存（确保每 5S 只保存一次）
                if (!H0.hasMessages(H.MSG_SAVE_PLAY_REPEAT_MODE, type)) {
                    Message msg = H0.obtainMessage(
                            H.MSG_SAVE_PLAY_REPEAT_MODE, type);
                    H0.sendMessageDelayed(msg, 5000);
                }
                break;
            default:
                break;
        }
    }

    /**
     * 处理外部通知修改播放模式广播活动
     * @param repeatMode 播放模式
     */
    protected void doChangeRepeatMode(int repeatMode) {
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
            mAppData.setMusicRepeatMode(repeatMode);
            savePlayRepeatMode(IMusicState.MEDIA_TYPE_MUSIC);
            sendLocalBroadcast(IMediaEvent.EVENT_CHANGE_REPEAT_MODE);
        } else if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            mAppData.setVideoRepeatMode(repeatMode);
            savePlayRepeatMode(IMusicState.MEDIA_TYPE_VIDEO);
            sendLocalBroadcast(IMediaEvent.EVENT_CHANGE_REPEAT_MODE);
        }
    }

    /**
     * 直接保存播放模式到记忆体
     * @see H#MSG_SAVE_PLAY_REPEAT_MODE
     *
     * @param type 媒体类型
     */
    private void onMsgSavePlayRepeatMode(int type) {
        switch (type) {
            case IMusicState.MEDIA_TYPE_MUSIC:
                Preferences.writePlayRepeatMode(
                        mContext, type, mAppData.musicRepeatMode());
                break;
            case IMusicState.MEDIA_TYPE_VIDEO:
                Preferences.writePlayRepeatMode(
                        mContext, type, mAppData.videoRepeatMode());
                break;
            default:
                break;
        }
    }

    /**
     * 处理媒体默认按键事件
     * <p> 只处理播放、暂停、停止、上下曲；
     *
     * @param command 播放控制命令
     *
     */
    protected void doMediaKeyEvent(int command) {
        mAppData.mAllowResumePlay = false;

        switch (mAppData.mMediaType) {
            case IMusicState.MEDIA_TYPE_MUSIC: {
                // 当前音乐播放列表是空
                if (mAppData.musicPlaylist().isEmpty()) {
                    return;
                }
                onPlayControl(command);
                break;
            }

            case IMusicState.MEDIA_TYPE_VIDEO:
                // 当前视频播放列表是空
                if (mAppData.videoPlaylist().isEmpty()) {
                    return;
                }
                if (isCanPlayVideo()) {
                    onPlayControl(command);
                }
                break;

            default:
                break;
        }
    }

    /**
     * 处理外部按键事件
     * <p> 我们建议统一在此处理（别这里一点那里一点）；
     *
     * @param keyCode {@link com.hcn.media_base.key.KeyEventExt}
     */
    @Override
    public void onKeyEvent(int keyCode) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_MEDIA_PLAY:
                doMediaKeyEvent(IMusicState.PLAY_CMD_PLAY);
                break;
            case KeyEvent.KEYCODE_MEDIA_PAUSE:
            case KeyEvent.KEYCODE_MEDIA_STOP:
                doMediaKeyEvent(IMusicState.PLAY_CMD_PAUSE);
                break;
            case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
                if (!mAppData.isMediaType(IMusicState.MEDIA_TYPE_IDLE)) {
                    onPlayControl(IMusicState.PLAY_CMD_PLAY_PAUSE);
                }
                break;
            case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
                MediaModel.call()
                        .playerModel()
                        .onSetSeekTimeZero();
                doMediaKeyEvent(IMusicState.PLAY_CMD_PREV);
                break;
            case KeyEvent.KEYCODE_MEDIA_NEXT:
                MediaModel.call()
                        .playerModel()
                        .onSetSeekTimeZero();
                doMediaKeyEvent(IMusicState.PLAY_CMD_NEXT);
                break;
            case KeyEventExt.KEYCODE_MEDIA_RANDOM_ALL:
                doChangeRepeatMode(IMusicState.REPEAT_MODE_RANDOM);
                break;
            case KeyEventExt.KEYCODE_MEDIA_REPEAT_ALL:
                doChangeRepeatMode(IMusicState.REPEAT_MODE_QUEUE);
                break;
            case KeyEventExt.KEYCODE_MEDIA_SMART_DECREMENT:
                handleSmartKeyEvent(IMusicState.PLAY_CMD_SMART_CW);
                break;
            case KeyEventExt.KEYCODE_MEDIA_SMART_INCREMENT:
                handleSmartKeyEvent(IMusicState.PLAY_CMD_SMART_CCW);
                break;
            case KeyEventExt.KEYCODE_MEDIA_SMART_ENTER:
                handleSmartKeyEvent(IMusicState.PLAY_CMD_SMART_ENTER);
                break;
            case KeyEvent.KEYCODE_MEDIA_REWIND:
                doMediaKeyEvent(KeyEvent.KEYCODE_MEDIA_REWIND);
                break;
            case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
                doMediaKeyEvent(KeyEvent.KEYCODE_MEDIA_FAST_FORWARD);
                break;
            default:
                break;
        }
    }

    /** 当前服务内部消息事件定义 **/
    protected interface H {
        int MSG_NONE = -1;

        int MSG_SAVE_PLAY_REPEAT_MODE = 1;
        int MSG_BIND_MEDIA_CLIENT = 2;

        int MSG_LAST = MSG_BIND_MEDIA_CLIENT + 1;
    }

    @Override
    protected boolean onHandleMessage(@NonNull Message msg) {
        switch (msg.what) {
            case H.MSG_SAVE_PLAY_REPEAT_MODE:
                onMsgSavePlayRepeatMode((Integer) msg.obj);
                return true;
            case H.MSG_BIND_MEDIA_CLIENT:
                onMsgBindMediaClient(msg.obj);
                return true;
            case H.MSG_NONE:
            default:
                break;
        }

        return false;
    }

    /**
     * 是否需要读取播放模式信息
     * <pre>
     *    由于播放模式信息需要 5 秒才会存储到持久化偏好配置文件中；
     *    播放模式信息如果已经在内存，则不需要去偏好文件再读取（避免冲掉最新状态）；
     * </pre>
     *
     * @param mediaType 音乐模式/视频模式
     *        {@link IMusicState#MEDIA_TYPE_MUSIC,IMusicState#MEDIA_TYPE_VIDEO}
     * @return 需要/不需要
     */
    protected boolean needReadPlayMode(int mediaType) {
        return !H0.hasMessages(H.MSG_SAVE_PLAY_REPEAT_MODE, mediaType);
    }

    /**
     * 显示 Toast 提示信息
     * <p> 少用这个接口，不是很美观；
     *
     * @param resId 文本资源
     */
    protected void onToastText(int resId) {
        if (!mAppData.mShowToast) {
            return;
        }

        String text = mContext.getResources().getString(resId);
        if (mToast == null) {
            mToast = Toast.makeText(mContext, text, Toast.LENGTH_SHORT);
            mToast.show();
        } else {
            mToast.setText(text);
            mToast.setDuration(Toast.LENGTH_SHORT);
            mToast.show();
        }
    }

    /**
     * 初始化存储设备
     * <p> 清除数据状态，同步更新远程数据；
     *
     * @param device 存储设备对象
     */
    protected void initStorageDevice(StorageDeviceEx device) {
        String strPath = device.getFilePath();

        // 校准内置存储的路径，兼容历史平台
        String flashPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        if (strPath.startsWith(IConstant.PATH_FLASH)) {
            device.mFilePath = strPath = flashPath;
        }

        // 同步状态，并清除数据
        device.updateLoading(isLoading(strPath));
        device.updateMounted(mFileStorageState.queryMediaState(strPath));
        device.mMusicInfoList.clear();
        device.mVideoInfoList.clear();
        device.mAlbumListMap.clear();
        device.mArtistListMap.clear();
        device.mMusicFavoriteList.clear();

        // [后面的条件多此一举][Flash 100% 是 MOUNTED 状态]
        if (device.isMounted() || MiscUtils.reverseEquals(flashPath, strPath)) {
            device.mFileScanState.mIsLoadFinished = getFileScanState(device.mFilePath);
            device.mID3ParseState.mIsLoadFinished = getID3ParseState(device.mFilePath);

            // 同步列表数据
            getMusicInfoList(device.mFilePath, device.mMusicInfoList);
            getVideoInfoList(device.mFilePath, device.mVideoInfoList);

            if (device.mID3ParseState.mIsLoadFinished) {
                classifyMediaInfoList(device);
            }
        }
    }

    /**
     * 同步 ID3 信息
     * <p> 按专辑和艺术家分类到指定列表；
     *
     * @param storageDevice
     */
    protected void classifyMediaInfoList(StorageDeviceEx storageDevice) {
        if (null == storageDevice) {
            return;
        }

        storageDevice.mAlbumListMap.clear();
        storageDevice.mArtistListMap.clear();
        storageDevice.mPathListMap.clear();

        for (MusicInfo info : storageDevice.mMusicInfoList) {
            List<MusicInfo> albumList = storageDevice.mAlbumListMap.get(info.mAlbum);
            List<MusicInfo> artistList = storageDevice.mArtistListMap.get(info.mArtist);

            String path = Objects.requireNonNull(
                    new File(info.mFilePath).getParentFile()).getAbsolutePath();
            List<MusicInfo> musicFileList = storageDevice.mPathListMap.get(path);

            if (null == albumList) {
                albumList = new ArrayList<>();
                storageDevice.mAlbumListMap.put(info.mAlbum, albumList);
            }

            if (null == artistList) {
                artistList = new ArrayList<>();
                storageDevice.mArtistListMap.put(info.mArtist, artistList);
            }

            if (null == musicFileList) {
                musicFileList = new ArrayList<>();
                storageDevice.mPathListMap.put(path, musicFileList);
            }

            albumList.add(info);
            artistList.add(info);
            musicFileList.add(info);
        }
    }

    /** 获取文件名后缀 **/
    protected String getSuffix(String filePath) {
        if (TextUtils.isEmpty(filePath)) {
            return null;
        }

        int pos = filePath.lastIndexOf('.');
        if (-1 == pos) {
            return null;
        }

        String strSuffix = filePath.substring(pos) + ".";
        strSuffix = strSuffix.toLowerCase(Locale.getDefault());

        return strSuffix;
    }

    /**
     * 那优先级获取当前有效的存储设备
     * <p> 检查顺序：USB -- SD -- FLASH
     *
     * @return {@link StorageDeviceEx} 存储设备对象
     */
    protected StorageDeviceEx getValidStorageDevice() {
        LogUtil.i(TAG, ">>> getValidStorageDevice");

        // 清除音乐播放列表相关信息
        mAppData.updateMusicPlayPosition(0);
        mAppData.musicPlaylist().clear();
        mAppData.musicRandomPositionList().clear();

        // 清除视频播放列表相关信息
        mAppData.updateVideoPlayPosition(0);
        mAppData.videoPlaylist().clear();
        mAppData.videoRandomPositionList().clear();

        // 按优先级轮询有效存储设备
        for (StorageDeviceEx manager : mStorageDeviceList) {
            if (checkoutStorage(manager)) {
                return manager;
            }
        }

        return mFlashDevice;
    }

    /** 接口有些奇怪，限制了特定情况下的单一播放事件，过于鸡肋 **/
    protected boolean checkoutStorage(StorageDeviceEx manager) {
        if (manager == null) {
            return false;
        }

        LogUtil.low_i(TAG, ">>>> checkoutStorage filePath: " + manager.getFilePath());

        if (manager.isMounted()) {
            if (manager.isLoading()
                    || (mAppData.mLastMediaType == IMusicState.MEDIA_TYPE_MUSIC && !manager.mMusicInfoList.isEmpty())
                    || (mAppData.mLastMediaType == IMusicState.MEDIA_TYPE_VIDEO && !manager.mVideoInfoList.isEmpty())) {
                return true;
            }
        }

        LogUtil.low_i(TAG, "checkoutStorage," +
                " manager.isLoading: " + manager.isLoading() +
                " manager.mMusicInfoList.size: " + manager.mMusicInfoList.size());

        return false;
    }

    protected boolean isLoading(String filePath) {
        try {
            if (mRemoteService != null) {
                return mRemoteService.isLoading(filePath);
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }

        return false;
    }

    public boolean isMounted(String filePath) {
        try {
            if (mRemoteService != null) {
                return mRemoteService.isMounted(filePath);
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }

        return false;
    }

    /** [比系统的 MEDIA_MOUNTED 广播可靠] **/
    public boolean targetStorageMounted(String filePath) {
        StorageDeviceEx storage = getStorageDevice(filePath);

        if (null != storage) {
            // 正常工作情况下, FLASH 不存在被卸载的问题
            if (storage.isFlash()) {
                return true;
            }

            String storagePath = storage.mFilePath;
            if (storage.isUsb()) {
                int length = IConstant.PATH_USB_PREFIX.length();

                // 分离存储设备盘符路径
                int index = filePath.indexOf(File.separatorChar, length);
                if (-1 == index) {
                    // 找不到说明是 IConstant.PATH_USB_PREFIX + <Number> 格式
                    storagePath = filePath.substring(0, length + 1);
                } else {
                    // 找到了说明是 USB-Hub 或者是多分区存储设备
                    storagePath = filePath.substring(0, index);
                }
            }

            return mFileStorageState.isMounted(storagePath);
        }

        return false;
    }


    /**
     * 请求扫描存储设备
     * <pre>
     *    1、清除当前存储设备信息和状态；
     *    2、通知远程数据服务开始扫描指定设备；
     * </pre>
     *
     * @param fileManager
     */
    public void requestScanStorageDevice(StorageDeviceEx fileManager) {
        if (mIsPowerOff) {
            LogUtil.d(TAG, ">>> requestScanStorageDevice: mIsPowerOff = true");
            return;
        }

        LogUtil.d(TAG, ">>> requestScanStorageDevice: path = " + fileManager.mFilePath);

        String strPath = fileManager.getFilePath();
        fileManager.updateLoading(true);
        fileManager.updateMounted(isMounted(strPath));
        fileManager.mFileScanState.mLoadingIndex.incrementAndGet();
        fileManager.mFileScanState.mIsLoadFinished = false;
        fileManager.mID3ParseState.mLoadingIndex.incrementAndGet();
        fileManager.mID3ParseState.mIsLoadFinished = false;
        fileManager.mMusicInfoList.clear();
        fileManager.mVideoInfoList.clear();
        fileManager.mAlbumListMap.clear();
        fileManager.mArtistListMap.clear();
        fileManager.mMusicFavoriteList.clear();

        try {
            if (mRemoteService != null) {
                mRemoteService.onLoadMediaPathEvent(strPath);
            }
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    protected boolean getFileScanState(String filePath) {
        try {
            if (mRemoteService != null) {
                return mRemoteService.getFileScanState(filePath);
            }
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return false;
    }

    protected boolean getID3ParseState(String filePath) {
        try {
            if (mRemoteService != null) {
                return mRemoteService.getID3ParseState(filePath);
            }
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return false;
    }

    protected void getMusicInfoList(String filePath, List<MusicInfo> infoList) {
        try {
            if (mRemoteService != null) {
                infoList.clear();

                int i = 0;
                List<MusicInfo> tempList = mRemoteService.getMusicInfoList(filePath, i);
                infoList.addAll(tempList);

                // 每次最多能取 1000 条数据
                while (!tempList.isEmpty() && (0 == tempList.size() % 1000)) {
                    i++;
                    tempList = mRemoteService.getMusicInfoList(filePath, i);
                    infoList.addAll(tempList);
                }
            }
        } catch (RemoteException ignored) {
        }
    }

    protected void getVideoInfoList(String filePath, List<MusicInfo> infoList) {
        try {
            if (mRemoteService != null) {
                infoList.clear();
                List<MusicInfo> tempList = mRemoteService.getVideoInfoList(filePath);
                infoList.addAll(tempList);
            }
        } catch (RemoteException ignored) {
        }
    }

    /**
     * 读取播放记忆的媒体路径
     * @param type 多媒体类型 {@link IMusicState#MEDIA_TYPE_MUSIC/VIDEO}
     * @return 媒体文件路径
     */
    protected String readMediaPath(int type) {
        String path = "";

        if (null == mRemoteService) {
            LogUtil.d(TAG, "writeMediaTime: <mRemoteService == null>.");
            return path;
        }

        try {
            path = mRemoteService.readMediaPath(type);
        } catch (Exception ignored) {
        }

        return path;
    }

    public int readMediaTime(int type, String path) {
        int time = 0;

        try {
            time = mRemoteService.readTempMediaTime(type, path);
        } catch (Exception ignored) {
        }

        return time;
    }

    public void writeMediaTime(int type, String path, int nTime, int reason) {
        if (null == mRemoteService) {
            LogUtil.d(TAG, "writeMediaTime: <mRemoteService == null>.");
            return;
        }

        try {
            mRemoteService.writeTempMediaTime(type, path, nTime, reason);
        } catch (RemoteException e) {
            LogUtil.d(TAG, "writeMediaTime: RemoteException");
        } catch (Exception e) {
            LogUtil.d(TAG, "writeMediaTime: Exception");
            e.printStackTrace();
        }
    }

    public int readVideoScaleType() {
        int scaleType = 0;
        try {
            scaleType = mRemoteService.readVideoScaleType();
        } catch (Exception ignored) {
        }

        return scaleType;
    }

    public void writeVideoScaleType(int type) {
        try {
            mRemoteService.writeVideoScaleType(type);
        } catch (Exception ignored) {
        }
    }

    /**
     * 由目标存储路径获取存储设备
     * <pre>
     *    先到内置存储设备中匹配；
     *    再到外置 SDCard 存储设备中匹配；
     *    最后到 USB 存储设备中匹配；
     *    默认返回是内置存储设备；
     * </pre>
     *
     * @param strPath 目标路径
     * @return {@link StorageDeviceEx}
     */
    public StorageDeviceEx getStorageDevice(String strPath) {
        if (TextUtils.isEmpty(strPath)) {
            return mFlashDevice;
        }

        // 这个判断很奇怪
        if (!mStorageDeviceList.isEmpty()) {
            if (strPath.startsWith(IConstant.PATH_FLASH)) {
                return mFlashDevice;
            }

            if (strPath.startsWith(IConstant.PATH_SD)) {
                return mSdDevice;
            }

            // [必须以 PATH_USB_PREFIX 开头才能匹配 USB]
            if (strPath.startsWith(IConstant.PATH_USB_PREFIX)
                    || MiscUtils.reverseEquals(strPath, mUsbDevice.mFilePath)) {
                return mUsbDevice;
            }
        }

        return mFlashDevice;
    }


    // 查找歌曲在列表中的位置，用位置来触发播放本身设计不合理，导致可播放事件被局限了
    // 返回 -1 表示在列表中没找到对应的歌曲
    protected int getMusicAssignPosition(String path) {
        int position = -1;
        StorageDeviceEx manager = getStorageDevice(path);

        if (!manager.mMusicInfoList.isEmpty()) {
            for (position = 0; position < manager.mMusicInfoList.size(); ++position) {
                MusicInfo info = manager.mMusicInfoList.get(position);
                if (MiscUtils.reverseEquals(info.mFilePath, path)) {
                    break;
                }
            }

            if (position >= manager.mMusicInfoList.size()) {
                position = -1;
            }
        }

        return position;
    }

    /**
     * 获取上一次播放的媒体文件位置
     * <p> 实测试耗时[循环 1000 次耗时 9ms 不到]
     *
     * @param infoList 目标列表
     * @return 记忆对象在当前目标列表中的位置
     */
    protected int getLastMediaInfoPosition(List<MusicInfo> infoList) {
        int position = 0;

        if (!infoList.isEmpty()) {
            String path = readMediaPath(mAppData.mLastMediaType);

            for (position = 0; position < infoList.size(); ++position) {
                MusicInfo info = infoList.get(position);
                if (MiscUtils.reverseEquals(info.mFilePath, path)) {
                    break;
                }
            }

            if (position >= infoList.size()) {
                position = 0;
            }
        }

        return position;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        // 销毁显示资源
        if (mToast != null) {
            mToast.cancel();
        }

        // 解绑远程服务
        onUnbindServiceEvent();
    }
}
