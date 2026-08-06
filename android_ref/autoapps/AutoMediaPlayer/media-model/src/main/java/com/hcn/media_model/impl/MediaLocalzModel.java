package com.hcn.media_model.impl;

import android.annotation.SuppressLint;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.audiofx.Visualizer;
import android.os.Build;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.UserHandle;
import android.util.Log;

import androidx.annotation.NonNull;

import com.hcn.auto_compat.PlatformUtils;
import com.hcn.common.misc.HBusUtils;
import com.hcn.media.base.IConnectionState;
import com.hcn.media.utils.LogUtils;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_model.MediaModel;
import com.hcn.media_model.base.ui.BaseMediaActivity;
import com.hcn.media_common.HBroadcastEx;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_model.eq.EQMediaController;
import com.hcn.media_model.eq.SharePreferencesTools;
import com.hcn.media_theme.Argument;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media.IMediaPlayerService;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_model.base.BaseModel;
import com.hcn.media_model.eq.EqChangeListener;
import com.hcn.media_model.base.ILocalzModel;
import com.hcn.media_model.base.IUiModel;
import com.hcn.mediaservice.data.MusicInfo;
import com.orhanobut.logger.Logger;

import java.util.List;
import java.util.Objects;

/**
 * 提供访问 LocalService 接口
 * <pre>
 *    主要是为 UI 组件提供访问服务的接口；
 *    注意：它不可以直接访问 IPlayerModel 的接口；
 *    View/ViewModel ----> ILocalzModel ----> LocalService
 *    Player ----> IPlayerModel ----> ILocalzModel ----> LocalService
 * </pre>
 *
 * @author 65821
 */
final class MediaLocalzModel extends BaseModel
        implements ILocalzModel {
    private static final String TAG = MediaLocalzModel.class.getSimpleName();

    /** Model 必须是唯一实例设计 **/
    private static MediaLocalzModel sInstance = null;

    /** LocalzModel 对外接口实例 **/
    public static MediaLocalzModel instance() {
        if (Objects.isNull(sInstance)) {
            throw new RuntimeException(
                    "Please initialize [MediaLocalzModel] Object!");
        }

        return sInstance;
    }

    /**
     * 初始化 LocalzModel 实例
     * <pre>
     *    LocalzModel 可以访问 UiModel；
     *    注意 LocalzModel 绝对不可以访问 PlayerModel；
     * </pre>
     *
     * @param context 上下文环境
     * @param uiModel {@link IUiModel} 用来访问 ui 状态；
     */
    public static void init(@NonNull Context context, @NonNull IUiModel uiModel) {
        if (Objects.isNull(sInstance)) {
            sInstance = new MediaLocalzModel(context, uiModel);
        } else {
            throw new RuntimeException(
                    "[MediaLocalzModel] already initialized!");
        }
    }

    /** MediaUiModel/查询 Ui 状态的桥梁 **/
    private IUiModel mUiModel;

    /**
     * 本地服务对象
     * <pre>
     *    业务逻辑分成使用，区分播放逻辑和 UI 显示逻辑；
     *    用来禁止 Service 和 Activity 之间直接互相调用方法；
     * </pre>
     */
    private IMediaPlayerService mLocalService = null;

    /**
     * 本地服务连接器
     * <pre>
     *     理论上应该遵守 C-S 模式，通过连接器返回的 IBinder 对象访问服务;
     *     但是由于历史原因，我们的工程师偷懒了，导致这里其实并不美观，可以优化；
     * </pre>
     */
    private ServiceConnection mConnection = new LocalServiceConnection();

    /** 本地服务连接状态 **/
    private String mConnectionState = IConnectionState.IDLE;

    /**
     * 媒体本地服务连接器类
     * <pre>
     *    为灵活绑定提供支持（保留扩展需求）；
     *    用来确保每次 bind 都是不一样的连接对象；
     * </pre>
     */
    private final class LocalServiceConnection implements ServiceConnection {

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "onServiceDisconnected.");

            // 服务端异常退出才会触发（理论上还在 bind 状态）
            mConnectionState = IConnectionState.DISCONNECTED;

            // 我们还是强制解除绑定的好
            unbindLocalService();
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "onServiceConnected.");
            if (!isConnectionState(IConnectionState.CONNECTING)) {
                // 假如服务组件被系统复活了（服务端与客户端同进程下不存在这种情况）
                Log.d(TAG, "onServiceConnected, Restore connection.");
            }

            // 连接成功，进入本地服务连接状态
            mConnectionState = IConnectionState.CONNECTED;
            H0.removeMessages(H.MSG_BIND_SERVICE_TIMEOUT);
            mLocalService = IMediaPlayerService.Stub.asInterface(service);

            // [连接上服务后，如果存在 UI 模块，可直接请求音频焦点]
            if (!Objects.isNull(mUiModel.getMusicActivity())
                    || !Objects.isNull(mUiModel.getVideoActivity())) {
                onRequestAudioFocus();
            }

            // 远程数据服务是否也已经绑定成功
            if (isServiceReadyState()) {
                HBroadcastEx.sendLocalBroadcast(
                        mContextRef.get(), IMediaEvent.EVENT_SERVICE_INITIALIZED);
            }
        }

        @Override
        public void onBindingDied(ComponentName name) {
            ServiceConnection.super.onBindingDied(name);
            // 如果被绑定的服务应用程序已更新，则会发生这种情况。
            Log.d(TAG, "onBindingDied.");

            // 已经解除绑定，不再处理；
            if (Objects.isNull(mLocalService)) {
                return;
            }

            // 服务进程异常退出
            mConnectionState = IConnectionState.DIED;
            unbindLocalService();
        }

        @Override
        public void onNullBinding(ComponentName name) {
            ServiceConnection.super.onNullBinding(name);
            Log.d(TAG, "onNullBinding.");

            // 已经解除绑定，不再处理；
            if (Objects.isNull(mLocalService)) {
                return;
            }

            // 服务进程异常退出
            mConnectionState = IConnectionState.DIED;
            unbindLocalService();
        }
    };

    /** 禁止构造无参对象 **/
    private MediaLocalzModel() {
        super(null, null);
        throw new RuntimeException(
                "Prohibit the construction of parameterless objects");
    }

    /**
     * MediaModel 构造函数
     * <p> 禁止在外部直接访问它；
     *
     * @param context 当前应用上下文环境
     * @param uiModel {@link IUiModel}
     */
    private MediaLocalzModel(@NonNull Context context, @NonNull IUiModel uiModel) {
        super(context, null);
        mUiModel = uiModel;

        // 绑定本地服务
        tryBindLocalService();
    }

    /**
     * 低内存的时候调用
     * @param reason 原因
     */
    @Override
    public void onLowMemory(int reason) {
        // 确保正常退出进程
        if (Objects.isNull(mLocalService)) {
            onMsgBindMediaServiceTimeout();
            return;
        }

        requestExitApp(reason);
    }

    /**
     * 判断媒体服务连接状态
     * @param state {@link IConnectionState}
     * @return 是期望状态/否
     */
    public boolean isConnectionState(@NonNull String state) {
        return mConnectionState == state;
    }

    /**
     * 尝试绑定本地服务
     * <pre>
     *    连接成功反馈大概需要 500ms ~ 800ms 之间；
     *    本地服务处理数据状态监听、以及播放逻辑处理；
     * </pre>
     */
    @SuppressLint("NewApi")
    private void tryBindLocalService() {
        // 已经绑定成功（过滤）
        if (!Objects.isNull(mLocalService)) {
            LogUtils.w("bindLocalServiceEvent:" +
                    " Is media local service bind state!");
            return;
        }

        // 禁止重复执行绑定动作
        if (isConnectionState(IConnectionState.CONNECTED)
                || isConnectionState(IConnectionState.CONNECTING)) {
            LogUtils.w("bindLocalServiceEvent:" +
                    " Is media local service " + mConnectionState + " state!");
            return;
        }

        // 构建绑定服务意图
        Intent intent = new Intent("HMEDIAPLAYER.ACTION.LOCALSERVICE");
        intent.setClassName("com.hcn.AutoMediaPlayer",
                "com.hcn.media.local.LocalService");

        // 构建本地服务连接器
        if (Objects.isNull(mConnection)) {
            mConnection = new LocalServiceConnection();
        }

        // 执行绑定服务动作
        boolean bindResult;
        Context context = mContextRef.get();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            bindResult = context.bindServiceAsUser(intent,
                    mConnection, Context.BIND_AUTO_CREATE,
                    UserHandle.getUserHandleForUid(BaseMediaData.UID));
        } else {
            bindResult = context.bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        }

        // 绑定执行成功（等待结果）
        if (bindResult) {
            LogUtil.d(TAG, "execute/bindService.");
            mConnectionState = IConnectionState.CONNECTING;

            // 如果 bindService 后 5s 都不返回结果，我们认为绑定异常
            H0.sendEmptyMessageDelayed(
                    H.MSG_BIND_SERVICE_TIMEOUT, 5000);
        } else {
            LogUtils.e("bindLocalServiceEvent: bindService return false!");
        }
    }

    /**
     * 解除本地服务绑定
     * <pre>
     *    暂时还没有这种需求；
     *    出了低内存退出的情况下回收；
     * </pre>
     */
    private void unbindLocalService() {
        // 无绑定操作，无需解除绑定
        if (isConnectionState(IConnectionState.IDLE)) {
            return;
        }

        // 是否需要解除 bind 状态
        if (mLocalService != null
                && mConnection != null) {
            Context context = mContextRef.get();
            context.unbindService(mConnection);
            mLocalService = null;
            mConnection = null;
        }

        // 更新连接状态到 IDLE
        mConnectionState = IConnectionState.IDLE;
        H0.removeMessages(H.MSG_BIND_SERVICE_TIMEOUT);
    }

    /** 消息定义 **/
    private interface H {
        int MSG_NONE = -1;

        /** bindService 连接状态超时 **/
        int MSG_BIND_SERVICE_TIMEOUT = 1;
    }

    @Override
    protected void onHandleMessage(@NonNull Message msg) {
        super.onHandleMessage(msg);

        switch (msg.what) {
            case H.MSG_BIND_SERVICE_TIMEOUT:
                onMsgBindMediaServiceTimeout();
                break;
            case H.MSG_NONE:
            default:
                break;
        }
    }

    /**
     * 绑定 Local 媒体服务超时
     * <pre>
     *    如果 bindService 执行成功后，超过 5s 得不到连接状态反馈，我们可以重置连接状态；
     *    系统设备是否存在有这种情况，还是待观察（预留处理，为了测试覆盖率）；
     * </pre>
     * @see H#MSG_BIND_SERVICE_TIMEOUT
     */
    private void onMsgBindMediaServiceTimeout() {
        LogUtil.e(TAG, "onMsgBindMediaServiceTimeout.");

        // 超时也需要解除绑定
        unbindLocalService();

        // 执行数据库存储动作
        MediaModel.call()
                .dataModel()
                .closeDataModel();

        // 这里是认知外的情况（退出进程）
        H0.postDelayed(() -> {
            android.os.Process.killProcess(android.os.Process.myPid());
            System.exit(0);
        }, 256);
    }

    /**
     * 本地服务是连接的
     * <p> LocalService 绑定成功；
     *
     * @return 是/否
     */
    @Override
    public boolean isLocalConnected() {
        return mLocalService != null;
    }

    public IMediaPlayerService getLocalService() {
        return mLocalService;
    }

    @Override
    public void registerMediaButton() {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.registerMediaButton();
        } catch (RemoteException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    /**
     * 是 Ready 状态
     * <pre>
     *    表示数据服务已经连接上；
     *    Model 和 Service 都已经准备就绪；
     * </pre>
     *
     * @return  是/否
     */
    @Override
    public boolean isServiceReadyState() {
        if (Objects.isNull(mLocalService)) {
            return false;
        }

        try {
            return mLocalService.isRemoteConnected();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return false;
    }

    public void doShouldPauseEvent(boolean stop) {
        doShouldPauseEvent(stop,0);
    }

    @Override
    public void doShouldPauseEvent(boolean stop, int reason) {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.doShouldPauseEvent(stop, reason);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public void doShouldPlayEvent() {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.doShouldPlayEvent();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public boolean isSdcardMounted() {
        if (Objects.isNull(mLocalService)) {
            return false;
        }

        try {
            return mLocalService.getSDState();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return false;
    }

    @Override
    public boolean isSdCard2Mounted() {
        if (Objects.isNull(mLocalService)) {
            return false;
        }

        try {
            return mLocalService.getSD2State();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return false;
    }

    @Override
    public boolean isUsbMounted() {
        if (Objects.isNull(mLocalService)) {
            return false;
        }

        try {
            return mLocalService.getUSBState();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return false;
    }

    @Override
    public boolean targetStorageMounted(String filePath) {
        if (Objects.isNull(mLocalService)) {
            return false;
        }

        try {
            return mLocalService.targetStorageMounted(filePath);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return false;
    }

    @Override
    public boolean isCanWatchVideo() {
        if (Objects.isNull(mLocalService)) {
            return false;
        }

        try {
            return mLocalService.isCanWatchVideo();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return false;
    }

    @Override
    public boolean isCanPlayVideo() {
        if (Objects.isNull(mLocalService)) {
            return false;
        }

        try {
            return mLocalService.isCanPlayVideo();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return false;
    }

    @Override
    public boolean existsHighPriorityEvent() {
        if (Objects.isNull(mLocalService)) {
            return false;
        }

        try {
            return mLocalService.existsHighPriorityEvent();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return false;
    }

    /** UI 启动触发播放流程必经函数 **/
    @Override
    public void requestSwitchMediaType() {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        LogUtil.d(TAG, "mLastMediaType = "
                + sAppData.mLastMediaType + ", MediaType = " + sAppData.mMediaType);

        try {
            // 触发播放流程
            mLocalService.requestSwitchMediaType();

            // 切换音乐干掉视频资源
            BaseMediaActivity videoActivity = mUiModel.getVideoActivity();
            if (videoActivity != null
                    && sAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
                videoActivity.hidePresentationDisplay();

                if (videoActivity.isInPictureInPictureMode()) {
                    mUiModel.finishVideoUI(-1);
                }
            }
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public void onRequestAudioFocus() {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.onRequestAudioFocus();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    /** 请求播放期望的音乐信息[改变播放列表] **/
    @Override
    public void requestPlayMusicInfo(@IPlaylistType int type,
                                     List<MusicInfo> infoList,
                                     int position) {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.requestPlayMusiclist(type, infoList, position);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    /** 请求播放期望的视频信息[改变播放列表] **/
    @Override
    public void requestPlayVideoInfo(@IPlaylistType int playlistType,
                                     List<MusicInfo> infoList,
                                     int position) {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.requestPlayVideolist(playlistType, infoList, position);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    /** 请求更新音乐播放列表[触发播放任务] **/
    @Override
    public boolean requestUpdateMusicPlaylist(@IPlaylistType int playlistType,
                                              @NonNull List<MusicInfo> infoList) {
        if (Objects.isNull(mLocalService)) {
            return false;
        }

        try {
            return mLocalService.requestUpdateMusicPlaylist(playlistType, infoList);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return false;
    }

    /** 请求更新视频播放列表[触发播放任务] **/
    @Override
    public boolean requestUpdateVideoPlaylist(@IPlaylistType int playlistType,
                                              @NonNull List<MusicInfo> infoList) {
        Objects.isNull(mLocalService);
        return false;
    }

    /** 请求下发播放控制命令[播放、暂停、上一曲、下一曲等] **/
    @Override
    public void requestPlayControl(int nCommand) {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.requestPlayControl(nCommand);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    /**
     * 改变指定媒体类型的播放循环模式
     * @param mediaType {@link IMusicState#MEDIA_TYPE_MUSIC/VIDEO}
     */
    @Override
    public void requestSwitchRepeatMode(int mediaType) {
        switch (mediaType) {
            case IMusicState.MEDIA_TYPE_MUSIC:
                switchMusicRepeatMode();
                break;
            case IMusicState.MEDIA_TYPE_VIDEO:
                switchVideoRepeatMode();
                break;
            default:
                break;
        }
    }

    /** 切换音乐播放器播放模式 **/
    private void switchMusicRepeatMode() {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.switchMusicRepeatMode();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    /** 切换视频播放器播放模式 **/
    private void switchVideoRepeatMode() {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.switchVideoRepeatMode();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public void requestScanTargetPath(String filePath) {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.requestScanTargetPath(filePath);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public int readMediaTime(int type, String path) {
        if (Objects.isNull(mLocalService)) {
            return -1;
        }

        try {
            return mLocalService.readMediaTime(type, path);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return -1;
    }

    @Override
    public void writeMediaTime(int type, String path, int nTime, int reason) {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.writeMediaTime(type, path, nTime, reason);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public void requestPlayDataSource(MusicInfo info) {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.requestPlayDataSource(info);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public void requestExitApp(int reason) {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.requestExitApp(reason);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public boolean inAccOnState() {
        if (Objects.isNull(mLocalService)) {
            return true;
        }

        try {
            return mLocalService.inAccON();
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        return true;
    }

    /**
     * 请求对外广播当前音乐播放信息
     * <pre>
     *    用于通知外部应用更新当前音乐播放显示信息；
     *    如果当前不在音乐模式（则暂不处理，保留）；
     * </pre>
     */
    @Override
    public void requestBroadcastMusicPlayInfo() {
        dispatchMusicEvent(
                IMediaEvent.EVENT_REQUEST_BROADCAST_MUSIC_PLAY_INFO, null, null);
    }

    /**
     * 保存视频显示尺寸类型
     * @param type 1:1/16:9/4:3/Fullscreen
     */
    @Override
    public void writeVideoScaleType(int type) {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            mLocalService.writeVideoScaleType(type);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    /**
     * 多媒体事件分发
     * <p> 这是 Player 类组件上报媒体事件到后台 Service 的通道；
     *
     * @param eventId 媒体事件 {@link IMediaEvent}
     * @param wParam 附加参数 1
     * @param lParam 附加参数 2
     */
    @Override
    public void dispatchMusicEvent(int eventId, Object wParam, Object lParam) {
        if (Objects.isNull(mLocalService)) {
            return;
        }

        try {
            String strParam = null;
            int nParam = -1;

            switch (eventId) {
                case IMediaEvent.EVENT_ERROR_FILE_NOT_EXIST: {
                    if (null != wParam) {
                        if (wParam instanceof MusicInfo) {
                            MusicInfo info = (MusicInfo) wParam;
                            strParam = info.mFilePath;
                        }
                    }
                    break;
                }

                // 播放器触发退出进程事件
                case IMediaEvent.EVENT_EXIT_PROCESS: {
                    int reason = -9;
                    if (wParam instanceof Integer) {
                        reason = (int) wParam;
                    }

                    mLocalService.requestExitApp(reason);
                    return;
                }

                case IMediaEvent.EVENT_NONE:
                default:
                    break;
            }

            mLocalService.dispatchMusicEvent(eventId, strParam, nParam);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
    }

    /**
     * 声音相关成员变量
     * <pre>
     *    AudioManger 相关的处理；
     *    进程本身自带 EQ 效果调节；
     * </pre>
     */
    private Visualizer mVisualizer = null;
    private EqChangeListener mEqChangeListener;
    private int mAudioSessionId = 0;

    public void setEqChangeListener(EqChangeListener qChangeListener) {
        mEqChangeListener = qChangeListener;
    }

    @Override
    public Visualizer getVisualizer() {
        return mVisualizer;
    }

    @Override
    public int getAudioSessionId() {
        return mAudioSessionId;
    }

    @Override
    public void setAudioSessionId(int Id) {
        long startTime = SystemClock.elapsedRealtime();

        try {
            mAudioSessionId = Id;
            if (mAudioSessionId <= 0) {
                return;
            }

            if (null != mVisualizer) {
                mVisualizer.release();
                mVisualizer = null;
            }

            // uis8581 创建不成功，暂时跳过
            if (Build.HARDWARE.contains("uis8581")
                    && PlatformUtils.noExistExternalDSPChip()) {
                Logger.t(TAG).w("The current platform is" +
                        " uis8581/no external dsp chip, unable to create Visualizer Object!");
                return;
            }

            // 音效处理（频谱数据依靠它来拿音频数据）
            mVisualizer = new Visualizer(0x00);

            // 这里的调试表示，外部挂了音频处理芯片，也可能配置成使用内置 DSP。
            Context context = mContextRef.get();
            if (PlatformUtils.usedInternalDSP(context)) {
                SharePreferencesTools spt = SharePreferencesTools.getSharePreferencesTools();
                spt.init(context);

                if (null == mEqChangeListener) {
                    setEqChangeListener(EQMediaController.instance());
                } else {
                    // [内置音效] 初始化后，测试音频指标 “分离度” 会过不了的，建议用酷我去测试；
                    // [属性控制] 也可以设置 persist.sys.internal.eq.disable 属性来控制其不初始化内置 EQ 相关设置；
                    if (!Argument.isAudioIndexTestEnable()) {
                        mEqChangeListener.onEqChange(spt);
                    }
                }
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }

        long consumingTime = SystemClock.elapsedRealtime() - startTime;
        if (consumingTime > 20) {
            LogUtil.v(TAG, ">>> setAudioSessionId: " + consumingTime + "ms");
        }
    }
}
