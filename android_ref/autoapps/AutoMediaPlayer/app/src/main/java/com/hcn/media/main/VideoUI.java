package com.hcn.media.main;

import static android.app.AppOpsManager.MODE_ALLOWED;
import static android.app.AppOpsManager.OPSTR_PICTURE_IN_PICTURE;
import static com.hcn.auto_compat.PlatformUtils.SM6225;
import static com.hcn.config.Feature.BIT.REMOTE_CONTROL_FOCUS;

import android.Configures.HConfig;
import android.Manifest;
import android.annotation.SuppressLint;
import android.app.AppOpsManager;
import android.app.PendingIntent;
import android.app.PictureInPictureParams;
import android.app.RemoteAction;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.Icon;
import android.hardware.display.DisplayManager;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Binder;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.os.UserHandle;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.WindowManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.os.HandlerCompat;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.auto_compat.PlatformUtils;
import com.hcn.auto_compat.app.WindowConfiguration;
import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HFileUtils;
import com.hcn.common.utils.HImageUtils;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.config.Feature;
import com.hcn.media_common.HBroadcastEx.SpecialChain;
import com.hcn.media_common.HBroadcastEx.PIP;
import com.hcn.media_common.file.MediaUriUtils;
import com.hcn.media_data.debug.MediaDebugger;
import com.hcn.media_data.ui.MediaPageState;
import com.hcn.media_data.ui.base.PageDataKV;
import com.hcn.media_theme.Argument;
import com.hcn.auto.AutoStatus;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.media_theme.ThemeUtilsEx;
import com.hcn.media_theme.ThemeX;
import com.hcn.media_base.IMediaBroadcast;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.fragment.IVideoPage;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.MediaActivity.VideoPlayerUiActivity;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media.local.event.MusicIntentReceiver;
import com.hcn.media.main.observer.SettingsKeyObserver;
import com.hcn.common.Utility;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media.main.base.BaseVideoActivity;
import com.hcn.media.video.common.VideoInfoFragment;
import com.hcn.media.video.common.VideoListFragment;
import com.hcn.media.video.common.VideoLoadingFragment;
import com.hcn.media.video.common.VideoParkingFragment;
import com.hcn.media.video.common.VideoSearchFragment;
import com.hcn.media.video.grid.VideoGridListFragment;
import com.hcn.common.misc.HBroadcastUtils;
import com.orhanobut.logger.Logger;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Objects;

/**
 * 视频播放器主页面入口
 * <pre>
 *    由于历史传承，这个包名类名已经不能再修改，牵扯太多项目工程：
 *    e.g. framework/carservices、HMediaService、HVoice、SystemUI、Settings等；
 * </pre>
 *
 *  @author 86158
 */
public class VideoUI extends BaseVideoActivity
        implements IMediaEventListener, SettingsKeyObserver.ICallback {

    /**
     * 视频 Activity 类对象别名
     * <p> 为兼容各平台的历史交互逻辑使用；
     */
    public static Class<?> sAliasClass = VideoPlayerUiActivity.class;

    /**
     * The arguments to be used for Picture-in-Picture mode.
     */
    private final PictureInPictureParams.Builder
            mPictureInPictureParamsBuilder = new PictureInPictureParams.Builder();

    /**
     * 当前显示页面类型
     * <p> 用来标记当前显示的是哪个 MediaFragment 页面；
     */
    private int mShowGroupType = E_GROUP_SHOW_NULL;
    private MediaFragment mCurrentFragment = null;

    /**
     * 关闭当前页面广播
     * <p> 用来标记当前关闭的是哪个页面的 action；
     */
    private final String VOICE_CLOSE_MEDIA_ACTION = "com.auto.apimediaplayer.action.VOICE_OFF_APPLICATION";

    /**
     * 视频 Activity 使用到的 Fragment 页面
     * <pre>
     *    这是由于需要兼容多套皮肤而定义的页面对象；
     *    虽然很多客户需求不一样，但是请可能复用现有的页面类，不要随意添加新的类对象；
     * </pre>
     */
    private MediaFragment mLoadingFragment = null;
    private MediaFragment mVideoInfoFragment = null;
    private MediaFragment mVideoListFragment = null;
    private MediaFragment mVideoSearchFragment = null;
    private MediaFragment mVideoParkingFragment = null;
    private MediaFragment mVideoGridListFragment = null;

    /**
     * Fragment 之间切换的动画时间
     * <p> 这是个动画持续时间，这里用来记忆上一次切换动画需要持续的时间；
     */
    public long mSetFragmentAnimTime = 0;

    /**
     * 当前机器型号特征
     * <p> 我们这里使用 FOTA 设备特征来区分；
     * e.g. 广告机需要隐藏 eq 、TF卡入口，video 需要隐藏状态栏；
     */
    private Feature mFeature;

    /**
     * 一定要是默认无参数构造函数
     * <p> 可以在构造函数中初始化部分变量；
     */
    public VideoUI() {
        super();
        Logger.t(TAG).d("Constructor.");
    }

    /**
     * 是为退出 PIP 而 Finish 的
     * </p> 在画中画模式，且按了 Recent 按钮；
     *
     * @return {@code true}: yes<br>{@code false}: no
     */
    private boolean isRecentExitPipFinish() {
        return finishReason() == 101;
    }

    /**
     * 是手动点击画中画窗口的 close 按钮触发
     * <p> 并不是所有的 pip close 操作都会触发 107 动作；
     * <p> e.g. RK3326 平台有概率触发，可能和平台性能有关系；
     *
     * @return {@code true}: yes<br>{@code false}: no
     */
    private boolean isClosePipFinish() {
        return finishReason() == 107;
    }

    /**
     * 仅仅销毁视频 UI 页面
     * <p> 用来判定是否只是销毁 UI 而不是退出进程；
     *
     * @return {@code true}: yes<br>{@code false}: no
     */
    private boolean onlyFinishVideoUI() {
        return finishReason() == 104 || finishReason() == 108;
    }

    /**
     * 是 KEYCODE_BACK 触发的销毁；
     * <p> 销毁（onDestroy）时判定原因，因条件不同处理不同；
     *
     * @return {@code true}: yes<br>{@code false}: no
     */
    private boolean isBackKeyTriggerFinish() {
        return finishReason() == 201;
    }

    /**
     * [Activity 进入 onPause 状态的原因]
     * <pre>
     *    0: 未知;
     *    1: 按 Home 按键
     *    2、按 Recent 按键；
     *    3、被其他页面遮挡；
     *    4、按 Back 按键
     * </pre>
     */
    private static final int UI_PAUSE_REASON_NONE = 0;
    private static final int UI_PAUSE_REASON_HOME = 1;
    private static final int UI_PAUSE_REASON_RECENT = 2;
    private static final int UI_PAUSE_REASON_OCCLUDE = 3;
    private static final int UI_PAUSE_REASON_BACK = 4;

    private int mUiPauseReason = UI_PAUSE_REASON_NONE;

    /**
     * [限制进入画中画模式]
     * <p> 避免点击 Menu 进入 recent 状态后，马上又点击 Home 想切换到画中画模式；
     */
    private boolean mIsRecentAppsStart = false;

    /**
     * 广播接收者
     * <p> 处理 Home/Recent 按键操作、Pip-Close 事件监听，
     */
    private final BroadcastReceiver mUIReceiver = new BroadcastReceiver() {
        public static final String SYSTEM_DIALOG_REASON_KEY = "reason";
        public static final String SYSTEM_DIALOG_REASON_RECENT_APPS = "recentapps";
        public static final String SYSTEM_DIALOG_REASON_HOME_KEY = "homekey";

        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            String action = intent.getAction();
            if (action == null) {
                return;
            }

            // [系统在每次点击 Home 按键时都会发出一个 ACTION_CLOSE_SYSTEM_DIALOGS 广播]
            if (action.equals(Intent.ACTION_CLOSE_SYSTEM_DIALOGS)) {
                String reason = intent.getStringExtra(SYSTEM_DIALOG_REASON_KEY);

                boolean isMultiWindMode = isInMultiWindowMode();
                boolean isPIPMode = isInPictureInPictureMode();
                LogUtil.d(TAG, "ACTION_CLOSE_SYSTEM_DIALOGS: reason = " + reason
                        + ", isMultiWindMode = " + isMultiWindMode + ", isPIPMode = " + isPIPMode);

                // [短按 Home Key]
                if (SYSTEM_DIALOG_REASON_HOME_KEY.equals(reason)) {
                    onSystemHomeKeyEvent();
                } else if (SYSTEM_DIALOG_REASON_RECENT_APPS.equals(reason)) {
                    onSystemRecentKeyEvent();
                } else if ("setRev".equals(reason)) {
                    // 倒车覆盖
                } else if ("voiceinteraction".equals(reason)) {
                    // google voice
                } else {
                    // [是否还有其它原因]
                    videoUiFinish(102);
                }
            } else if (PIP.ACTION_PIP.equals(action)) {
                // [PipMotionHelper.java] 发送画中画广播
                String key = intent.getStringExtra("EXTRA_KEY");
                boolean isMultiWindMode = isInMultiWindowMode();
                boolean isPIPMode = isInPictureInPictureMode();

                if (!TextUtils.isEmpty(key) && "close".equals(key)) {
                    if (isPIPMode) {
                        mViewModel.playerRelay().accept(
                                t -> t.requestShouldPauseEvent(true, 0));
                        videoUiFinish(103);
                    }
                } else {
                    // 现阶段理论上没有这个情况, SystemUI 只发了 close 状态。
                    if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO
                            && mShowGroupType == E_GROUP_SHOW_VIDEO_INFO
                            && !mIsRecentAppsStart && !isMultiWindMode && !isStopped()) {
                        enterPipMode(2);
                    }
                }
            }
        }
    };

    /**
     * 处理 Home 按键事件
     * <pre>
     *    Intent.ACTION_CLOSE_SYSTEM_DIALOGS
     *    SYSTEM_DIALOG_REASON_KEY / "homekey"
     * </pre>
     */
    private void onSystemHomeKeyEvent() {
        // 是在自由窗口模式显示状态（不处理）
        if (requestUiModel().isVideoWindowingMode(
                WindowConfiguration.WINDOWING_MODE_FREEFORM)) {
            LogUtil.v(TAG, "onSystemHomeKeyEvent: in FREEFORM windowing mode!");
            return;
        }

        boolean isMultiWindMode = isInMultiWindowMode();
        if (mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO
                && mShowGroupType == E_GROUP_SHOW_VIDEO_INFO
                && !mIsRecentAppsStart && !isMultiWindMode && !isStopped()) {
            enterPipMode(1);
        }

        mUiPauseReason = UI_PAUSE_REASON_HOME;
        requestUiModel().setVideoActivityBackground(true);
        mIsRecentAppsStart = false;
    }

    private void onSystemBackKeyEvent() {
        // 是在自由窗口模式显示状态（不处理）
        if (requestUiModel().isVideoWindowingMode(
                WindowConfiguration.WINDOWING_MODE_FREEFORM)) {
            LogUtil.v(TAG, "onSystemHomeKeyEvent: in FREEFORM windowing mode!");
            return;
        }

        mUiPauseReason = UI_PAUSE_REASON_BACK;
        requestUiModel().setVideoActivityBackground(true);
        mIsRecentAppsStart = false;
    }

    /**
     * 处理 Home 按键事件
     * <pre>
     *    Intent.ACTION_CLOSE_SYSTEM_DIALOGS
     *    SYSTEM_DIALOG_REASON_KEY / "recentapps"
     * </pre>
     */
    private void onSystemRecentKeyEvent() {
        // [短按 Menu Key]
        // SYSTEM_DIALOG_REASON_RECENT_APPS 时间可以在 onPause 后, 也可以在 onResume 的前面或者后面。
        if (!isResumedEx()) {
            mIsRecentAppsStart = true;
        }

        // [SYSTEM_DIALOG_REASON_RECENT_APPS 事件来的比 onPause() 迟]
        mUiPauseReason = UI_PAUSE_REASON_RECENT;
        if (mHandler.hasMessages(VideoUiHandler.MSG_WHAT_HANDLE_UI_PAUSE_EVENT)) {
            mHandler.removeMessages(VideoUiHandler.MSG_WHAT_HANDLE_UI_PAUSE_EVENT);
            onHandleUiPauseEvent(); // [直接调用]避免播放器暂停状态下快速操作切换前后台不触发播放。
        }

        // [画中画模式, 按 MenuKey 退出画中画]
        boolean isPIPMode = isInPictureInPictureMode();
        if (isPIPMode) {
            videoUiFinish(101);
        }
    }

    /**
     * VideoUI dump 参数说明
     * <p> dumpsys activity ${class name} -h
     *
     * @param fout 您应该将状态 dump 到的 PrintWriter。您返回后这里将为您关闭。
     */
    private void dumpActivityUsage(@NonNull PrintWriter fout) {
        fout.println(
                "usage: \n" +
                "    dumpsys activity com.hcn.MediaActivity.VideoPlayerUiActivity [ARGS] \n" +
                "        -h: shows this help\n" +
                "        -d: debug level\n" +
                "        -e: exec target event\n" +
                "        ...\n");
    }

    /**
     * 倾倒信息到终端
     * <p> dumpsys activity com.hcn.MediaActivity.VideoPlayerUiActivity
     *
     *@param fd dump 要发送到的原始文件描述符。
     *@param fout 您应该将状态 dump 到的 PrintWriter。您返回后这里将为您关闭。
     *@param args dump 请求的其他参数。
     */
    @Override
    public void dump(@NonNull String prefix,
                     @Nullable FileDescriptor fd,
                     @NonNull PrintWriter fout,
                     @Nullable String[] args) {
        if (checkCallingOrSelfPermission(
                Manifest.permission.DUMP) != PackageManager.PERMISSION_GRANTED) {
            fout.println("Permission Denial: can't" +
                    " 'dumpsys activity com.hcn.MediaActivity.VideoPlayerUiActivity' " +
                    "from from pid=" + Binder.getCallingPid() + ", uid=" + Binder.getCallingUid());
            return;
        }

        int option = 0;
        while (option < args.length) {
            String opt = args[option];
            if (opt == null || opt.length() == 0 || opt.charAt(0) != '-') {
                break;
            }

            option++;
            if ("-h".equals(opt)) {
                dumpActivityUsage(fout);
                return;
            }
            if ("-d".equals(opt)) {
                int args_length = args.length - option + 1;
                args_length = Math.min(args_length, 3);
                String[] debug_args = new String[args_length];

                System.arraycopy(args, option - 1, debug_args, 0, args_length);
                MediaDebugger.dumpDebugEvent(fout, debug_args);
                return;
            } else if ("-e".equals(opt)) {
                int args_length = args.length - option + 1;
                args_length = Math.min(args_length, 3);
                String[] debug_args = new String[args_length];

                System.arraycopy(args, option - 1, debug_args, 0, args_length);
                if (args_length == 2) {
                    if ("enter-pip".equals(debug_args[1])) {
                        enterPipMode(101);
                    }
                }
                return;
            }
        }

        fout.println("[dumpsys VideoUI info] " +
                "\n     onResume = " + isResumedEx() +
                "\n     isMultiWindMode = " + isInMultiWindowMode() +
                "\n     isPipMode = " + isInPictureInPictureMode() +
                "\n     mVideoUIShow = " + mAppData.mVideoUiShow +
                "\n     mediaPlayState = " + mAppData.mediaPlayState() +
                "\n     AccState = " + AutoStatus.isRealtimeAccON());
    }

    /**
     * 视频窗口主消息处理器
     * <p> 主要用来处理延时任务，工作在 MainThread；
     */
    private final VideoUiHandler mHandler = new VideoUiHandler(Looper.getMainLooper());

    @SuppressLint("HandlerLeak")
    private class VideoUiHandler extends Handler {
        private static final int MSG_WHAT_UPDATE_BRAKE_STATUS = 1;
        private static final int MSG_WHAT_HANDLE_UI_PAUSE_EVENT = 2;

        public VideoUiHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(android.os.Message msg) {
            switch (msg.what) {
                case MSG_WHAT_UPDATE_BRAKE_STATUS:
                    onUpdateFragmentEvent();
                    break;
                case MSG_WHAT_HANDLE_UI_PAUSE_EVENT:
                    onHandleUiPauseEvent();
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * 处理 UI 暂停事件
     * <p> 根据不同的场景暂停或者停止视频播放任务；
     *
     * @see VideoUiHandler#MSG_WHAT_HANDLE_UI_PAUSE_EVENT
     */
    private void onHandleUiPauseEvent() {
        // [以前是设置的 stop = true, 现在有 AVM 它是个 Activity.]
        boolean isStopPlay = false;
        switch (mUiPauseReason) {
            case UI_PAUSE_REASON_HOME:
            case UI_PAUSE_REASON_RECENT:
            case UI_PAUSE_REASON_BACK:
                isStopPlay = true;
                break;

            case UI_PAUSE_REASON_OCCLUDE:
            default:
                break;
        }

        // 如果是 stop, 下次触发 Play 就会强制播放.
        boolean finalIsStopPlay = isStopPlay;
        mViewModel.playerRelay().accept(
                t -> t.requestShouldPauseEvent(finalIsStopPlay, 201));
    }

    /**
     * 本地广播接受者
     * <pre>
     *     内部事件处理，通过回调机制实现;
     *     非跨进程广播，无 Binder 通讯消耗；
     * </pre>
     */
    private final BroadcastReceiver mLocalReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                return;
            }

            // 是内部 Local 广播事件
            if (action.equals(SpecialChain.ACTION_LOCAL_CALLBACK)) {
                onActionLocalCallback(intent);
            }
        }

        /**
         * 处理 Local 广播事件
         * @see SpecialChain#ACTION_LOCAL_CALLBACK
         *
         * @param intent 广播意图
         */
        private void onActionLocalCallback(@NonNull Intent intent) {
            int eventId = intent.getIntExtra(SpecialChain.EXTRA_CALLBACK_TYPE, -1);
            String data = intent.getStringExtra(SpecialChain.EXTRA_CALLBACK_DATA);

            // [onDestroy 才销毁这个广播接收者, 进入音乐它不一定会销毁]
            if (!mAppData.isMediaType(IMusicState.MEDIA_TYPE_VIDEO)) {
                // 处理 Music 相关事件
                if (mAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
                    if (eventId == IMediaEvent.EVENT_GOTO_RESUME_MUSIC_PLAYER_UI) {
                        if (isFinishing()) {
                            return;
                        }

                        if (isStopped()) {
                            // 如果当前在 Stop 状态，且没有销毁;
                            // 主要是因为分屏状态布局存在的问题，尤其是上下分屏。
                            videoUiFinish(104);
                        } else {
                            if (Objects.isNull(data)) {
                                return;
                            }

                            // 不在多窗口模式，且音乐全屏显示，退出视频。
                            if (!isInMultiWindowMode()) {
                                switch (Objects.requireNonNull(data)) {
                                    case POST_RESUME_STATE:
                                    case TOP_RESUMED_STATE:
                                        videoUiFinish(108);
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                    if (eventId == IMediaEvent.EVENT_CHANGE_PLAY_STATE && !mIsTopResumedActivity) {
                        // 音乐视频分屏情况下播放任务改变通知更改状态
                        mViewModel.pageEventRelay().accept(t -> {
                            t.onPageEvent(IMediaEvent.EVENT_SPLIT_SCREEN_UPDATE_PLAY_STATE, null, null);
                        });
                    }
                }
                return;
            }

            switch (eventId) {
                case IMediaEvent.EVENT_SERVICE_INITIALIZED:
                    // 本地服务是否连接
                    if (mViewModel.isServiceReadyState()) {
                        initFirstFragment();
                    }
                    break;

                case IMediaEvent.EVENT_NONE:
                default:
                    onLocalMediaEvent(eventId, data);
                    break;
            }
        }
    };

    @SuppressLint("ObsoleteSdkInt")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        LogUtil.i(TAG, ">>> [Enter]onCreate.");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);

        mAppData.mSingleVideoPlay = false;
        mAppData.mSingleVideoFilePath = getPathFromIntent(getIntent());
        if (!TextUtils.isEmpty(mAppData.mSingleVideoFilePath)) {
            LogUtil.i(TAG, ">>> [Enter]onCreate,"
                    + " single task: " + mAppData.mSingleVideoFilePath);
        }

        mUiPauseReason = UI_PAUSE_REASON_NONE;
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(SpecialChain.ACTION_LOCAL_CALLBACK);
        HBroadcastUtils.getInstance(this)
                .registerReceiver(mLocalReceiver, intentFilter);

        registerHomeKey();
        registerMediaButton();
        registerSettingsObserver();

        LogUtil.d(TAG, ">>> [Leave]onCreate.");
    }

    /**
     * 检查并同步背景
     * <p> {@link ThemeUtilsEx} 共享背景机制；
     */
    @Override
    protected void checkAndSyncBackground() {
        // 检查用户设置背景（自定义壁纸）
        if (xBoolean("support_wallpaper_customized")) {
            String wallPaperPath = MediaPageState.instance()
                    .readString(PageDataKV.Key.MUSIC_WALLPAPER_PATH);
            if (!TextUtils.isEmpty(wallPaperPath)) {
                if (!HFileUtils.isFileExists(wallPaperPath)) {
                    LogUtils.vTag(TAG,
                            "checkAndSyncBackground: "
                                    + wallPaperPath + " not exists");
                    return;
                }

                Bitmap bitmap = HImageUtils.getBitmap(wallPaperPath);
                if (bitmap != null) {
                    findViewById(xId(R.id.main_bg))
                            .setBackground(HImageUtils.bitmap2Drawable(bitmap));
                }
            }
        }

        Drawable wallPaper = ThemeUtilsEx.getAppShareBackground();
        if (wallPaper == null) {
            return;
        }

        boolean validBackground = true;
        if (wallPaper instanceof ColorDrawable) {
            // ColorDrawable(#20210821) 表示无共享资源
            ColorDrawable colorDrawable = (ColorDrawable) wallPaper;
            int colorValue = colorDrawable.getColor();
            if (colorValue == getColor(R.color.share_background_none)) {
                validBackground = false;
            }
        }

        // 背景是否有效
        if (validBackground) {
            findViewById(xId(R.id.main_bg)).setBackground(wallPaper);
        }
    }

    @SuppressLint("UnspecifiedRegisterReceiverFlag")
    private void registerHomeKey() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        filter.addAction(PIP.ACTION_PIP);

        this.registerReceiver(mUIReceiver, filter);
    }

    private void registerMediaButton() {
        // 高通 SM6225 是 Android 13 的版本
        if (PlatformUtils.isHardware(SM6225)) {
            return;
        }

        // 尽量不要影响低版本已经出货的软件
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.P) {
            LogUtil.d(TAG, ">>> registerMediaButton ");
            if (null == mAudioManager) {
                mAudioManager = (AudioManager) this.getSystemService(Context.AUDIO_SERVICE);
            }

            // dumpsys media_session | grep "Media button session is"
            // com.hcn.AutoMediaPlayer/MediaSessionHelper-com.hcn.AutoMediaPlayer
            ComponentName rec = new ComponentName(getPackageName(),
                    MusicIntentReceiver.class.getName());
            mAudioManager.registerMediaButtonEventReceiver(rec);
        }
    }

    @Override
    protected void onFragment2MainEvent(int event, Object obj1, Object obj2) {
        if (!isResumedEx()) {
            Logger.t(TAG).w("onFragment2MainEvent," +
                    " The event is not in a valid life cycle!");
            return;
        }

        // 处理显示视频 Fragment 页面事件
        if (event == IMediaEvent.EVENT_SHOW_VIDEO_FRAGMENT) {
            assert obj1 instanceof Integer;
            int pageEvent = (int) obj1;
            switch (pageEvent) {
                case IVideoPage.E_GROUP_SHOW_VIDEO_INFO:
                case IVideoPage.E_GROUP_SHOW_VIDEO_SEARCH:
                    onShowFragmentEvent(pageEvent);
                    break;
                default:
                    break;
            }

            return;
        }

        // 也可以传递媒体事件
        onMediaEvent(event, obj1, obj2);
    }

    @Override
    protected void onStart() {
        LogUtil.d(TAG, ">>> onStart.");
        super.onStart();
		
        mFeature = Feature.instance();
        if (mFeature.hasFeature(REMOTE_CONTROL_FOCUS)){
            mAppData.mFullScreen = false;
            setWindowFullScreen(false);
            Settings.System.putString(getContentResolver(),"current_media_source","video_not_fullscreen");
        }
		
        // 画中画必须是在 onPause 状态
        mMaybeInExitPipWindow = false;
        mIsInPictureInPictureMode = false;
    }

    /**
     * 标记 onResume 函数执行过程状态
     * <p> [配合 onResume 一起使用]
     */
    private boolean mIsUiResuming = false;

    /**
     * 恢复前处理
     * <pre>
     *    必须在 onResume 中调用；
     *    且必须在 super.onResume() 调用前；
     * </pre>
     */
    @Override
    protected void onPreResume() {
        super.onPreResume();

        // 通知 Video Player UI 进入显示模式
        enterAndResumeVideoPlayerUI();

        // Enter onResume()
        mIsUiResuming = true;

        // 画中画必须是在 onPause 状态
        mIsInPictureInPictureMode = false;
        mHandler.removeCallbacks(mClosePipFinishRunnable);

        // [重置为遮挡模式]
        mUiPauseReason = UI_PAUSE_REASON_OCCLUDE;
        mHandler.removeMessages(
                VideoUiHandler.MSG_WHAT_HANDLE_UI_PAUSE_EVENT);

        // [隐藏后台异显视图]
        hidePresentationDisplay();
    }

    @Override
    protected void onResume() {
        LogUtil.d(TAG, ">>> [Enter]onResume.");
        super.onResume();
        registerVoiceReceiver();

        // [如果是物理全屏显示, 需要重置部分变量]
        DisplayMetrics dm = getApplicationContext()
                .getResources().getDisplayMetrics();
        if (dm.widthPixels == mAppData.mVideoUiWidth
                && dm.heightPixels == mAppData.mVideoUiHeight) {
            // [重置 mIsRecentAppsStart: 否则分屏结束后不能进入画中画]
            mIsRecentAppsStart = false;
        }

        mAppData.mVideoUiShow = true;
        mAppData.mFullScreen = false;
        mAppData.isFrontVideo = true;

        setWindowFullScreen(false);
        requestUiModel().setVideoActivityBackground(false);

        // 本地服务是否连接
        if (localServiceConnected()) {
            initFirstFragment();
        }

        // Leave onResume()
        mIsUiResuming = false;
        LogUtil.d(TAG, ">>> [Leave]onResume.");
    }

    /**
     * 生命周期说明
     * <p> onResume -> onPostResume
     */
    @Override
    protected void onPostResume() {
        super.onPostResume();
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    /**
     * 当前窗口配置改变到物理全屏状态
     * <p> 由 onConfigurationChanged(Configuration newConfig) 触发；
     */
    @Override
    protected void onConfigurationToFullScreen() {
        // [重置 mIsRecentAppsStart: 否则分屏结束后不能进入画中画]
        mIsRecentAppsStart = false;
    }

    /**
     * 当前窗口配置改变，可以在此更新视图元素的位置大小；
     * <p> 由 onConfigurationChanged(Configuration newConfig) 触发；
     */
    @Override
    protected void onConfigurationToViewElement() {
        boolean isInPIPMode = isInPictureInPictureMode();
        if (isInPIPMode) {
            // 如果是画中画 newConfig.screenWidthDp 和 newConfig.screenHeightDp 其实就是窗口大小
            if (mVideoInfoFragment != null) {
                mVideoInfoFragment.doCallbackEvent(
                        IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_LAYOUT,
                        mAppData.mVideoUiWidth, mAppData.mVideoUiHeight);
            }
        } else {
            if (mVideoInfoFragment != null) {
                mVideoInfoFragment.doCallbackEvent(
                        IMediaEvent.EVENT_CONFIGURATION_CHANGED_SIZE);
            }
        }
    }

    /**
     * 特定场景下监听的语音控制类广播
     * <p> 打开音乐的时候，需要先 finish 视频页面；
     */
    private VoiceBroadcastReceiver mVoiceBroadcastReceiver = new VoiceBroadcastReceiver();

    private final class VoiceBroadcastReceiver extends BroadcastReceiver {
        private final String VOICE_CLOSE_MEDIA_INFO_KEY = "CLOSE_MEDIA";

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (IConstant.ACTION_VOICE_2_HMEDIA.equals(action)) {
                String szEvent = intent.getStringExtra(IConstant.EXTRA_MEDIA_EVENT);

                if ("open_music".equals(szEvent)) {
                    if (!isFinishing()) {
                        MiscUtils.goToHome(context);
                        videoUiFinish(105);
                    }
                }
            } else if (VOICE_CLOSE_MEDIA_ACTION.equals(action)) {
                String stringExtra = intent.getStringExtra(VOICE_CLOSE_MEDIA_INFO_KEY);
                if ("close_video".equals(stringExtra)){
                    MiscUtils.goToHome(context);
                    videoUiFinish(105);
                }
            }
        }
    }

    @SuppressLint("UnspecifiedRegisterReceiverFlag")
    private void registerVoiceReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(IConstant.ACTION_VOICE_2_HMEDIA);
        intentFilter.addAction(VOICE_CLOSE_MEDIA_ACTION);

        if (null == mVoiceBroadcastReceiver) {
            mVoiceBroadcastReceiver = new VoiceBroadcastReceiver();
        }

        registerReceiver(mVoiceBroadcastReceiver, intentFilter);
    }

    private void unregisterVoiceReceiver() {
        if (mVoiceBroadcastReceiver != null) {
            unregisterReceiver(mVoiceBroadcastReceiver);
            mVoiceBroadcastReceiver = null;
        }
    }

    @Override
    protected void onUserLeaveHint() {
        LogUtil.d(TAG, ">>> onUserLeaveHint.");
        super.onUserLeaveHint();

        // 当用户按下 Home键，onUserLeaveHint()将会被回调;
        // 但是当来电导致来电 Activity 自动占据前台，onUserLeaveHint() 将不会被回调。
    }

    @Override
    protected void onPause() {
        super.onPause();
        LogUtil.i(TAG, ">>> onPause.");

        unregisterVoiceReceiver();

        // 移除 MSG_WHAT_HANDLE_UI_PAUSE_EVENT 信息
        mHandler.removeMessages(VideoUiHandler.MSG_WHAT_HANDLE_UI_PAUSE_EVENT);

        // [是多窗口模式]<注意: 画中画也属于多窗口>
        boolean isMultiWindMode = isInMultiWindowMode();
        boolean isBackgroundPlayMode = Argument.isBackgroundPlayMode();

        // 获取 PRESENTATION 逻辑显示个数 [E.g: HDMI ...]
        Display[] presentationDisplays = mDisplayManager.getDisplays(
                DisplayManager.DISPLAY_CATEGORY_PRESENTATION);
        boolean isSupportPresentation = (presentationDisplays.length > 0);

        LogUtil.d(TAG, "[onPause] " +
                "\n     isMultiWindMode = " + isMultiWindMode +
                "\n     isBackgroundPlayMode = " + isBackgroundPlayMode +
                "\n     presentationDisplays = " + presentationDisplays.length +
                "\n     mVideoUIShow = " + mAppData.mVideoUiShow +
                "\n     AccState = " + AutoStatus.isRealtimeAccON());

        // 是否存在有效播放组件
        final boolean[] existsValidMediaPlayer = {false};
        mViewModel.playerRelay().accept(
                t -> existsValidMediaPlayer[0] =
                        t.requestQueryState(
                                IMediaAction.existsValidMediaPlayer, null));

        if (isSupportPresentation
                && !isMultiWindMode // [理论上前台显示大小不相干, 为什么这里要过滤]
                && isBackgroundPlayMode
                && existsValidMediaPlayer[0]) {
            // [需要实际需求调试: 支持多屏异显且后台播放可以进来]
            LogUtil.d(TAG, "[onPause] reason: 001.");

            mAppData.isFrontVideo = false;
            showPresentationDisplay();
        } else {
            // [画中画模式也是多窗口]
            if (!isMultiWindMode) {
                // [进入 onPause 情况下不一定已经收到 onPictureInPictureModeChanged
                //  和 onMultiWindowModeChanged 的反馈, 它们的时序不一定是一成不变的.]
                mAppData.mVideoUiShow = false;
                LogUtil.d(TAG, "[onPause] reason: 002.");
            } else {
                LogUtil.d(TAG, "[onPause] reason: 003.");
            }

            // 不能在后台播放，需要暂停播放
            if (!Argument.isCanPlayVideoBack()) {
                if (isMultiWindMode) {
                    // 是多窗口模式（分屏、画中画、自由窗口）
                    // TODO: 预留其它情况处理

                    // 如果当前是从自由窗口模式切换到 onPause() 状态
                    if (requestUiModel().isVideoWindowingMode(
                            WindowConfiguration.WINDOWING_MODE_FREEFORM)) {
                        LogUtil.d(TAG, "[onPause]: WINDOWING_MODE_FREEFORM 1");
                        onHandleUiPauseEvent();
                    }
                } else {
                    switch (mUiPauseReason) {
                        case UI_PAUSE_REASON_HOME:
                        case UI_PAUSE_REASON_BACK:
                            // [先有 SYSTEM_DIALOG_REASON_HOME_KEY 事件, 再触发 onPause()]
                            onHandleUiPauseEvent();
                            break;

                        case UI_PAUSE_REASON_RECENT:
                        case UI_PAUSE_REASON_OCCLUDE:
                        default:
                            // [参考 SYSTEM_DIALOG_REASON_RECENT_APPS 触发事件 / SystemUI]
                            // [延时尽可能不要低于 1S, 虽然是连续调用]
                            mHandler.sendEmptyMessageDelayed(
                                    VideoUiHandler.MSG_WHAT_HANDLE_UI_PAUSE_EVENT, 1000);
                            break;
                    }
                }
            } else {
                // 如果是设置了画中画、后台播放模式
                LogUtils.dTag(TAG, "isCanPlayVideoBack.");

                // 如果不是后台播放模式（设置的 PIP）
                if (!Argument.isBackgroundPlayMode()) {
                    // 如果当前是从自由窗口模式切换到 onPause() 状态
                    if (requestUiModel().isVideoWindowingMode(
                            WindowConfiguration.WINDOWING_MODE_FREEFORM)) {
                        LogUtil.d(TAG, "[onPause]: WINDOWING_MODE_FREEFORM 2");
                        onHandleUiPauseEvent();
                    }
                }
            }
        }
    }

    @Override
    protected void onStop() {
        LogUtil.d(TAG, ">>> onStop");
        super.onStop();

        if (!isFinishing()) {
            // 是支持后台播放模式
            boolean isBackgroundPlayMode = Argument.isBackgroundPlayMode();

            // 可能是主界面侧窗模式滑动隐藏视频
            if (isSplitScreenSideslipHidden() && !isBackgroundPlayMode) {
                videoUiFinish(501);
            } else {
                // 滑动画中画小窗口直接退出视频界面
                if (mIsInPictureInPictureMode && !isFinishing()) {
                    mMaybeInExitPipWindow = true;
                } else {
                    // 高版本才支持 hasCallbacks 接口；
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                        if (mHandler.hasCallbacks(mClosePipFinishRunnable)) {
                            mHandler.removeCallbacks(mClosePipFinishRunnable);
                            mClosePipFinishRunnable.run();
                        }
                    } else if (HandlerCompat.hasCallbacks(mHandler, mClosePipFinishRunnable)) {
                        mHandler.removeCallbacks(mClosePipFinishRunnable);
                        mClosePipFinishRunnable.run();
                    }
                }

                printVideoWindowInfo("onStop");
            }

            // 当前在自由窗口模式显示状态
            // 例如：小窗口切换到导航或者其他模式，视频没有真正退出
            if (requestUiModel().isVideoWindowingMode(
                    WindowConfiguration.WINDOWING_MODE_FREEFORM)) {
                mAppData.mVideoUiShow = false;
            }
        }

        // 画中画必须是在 onPause 状态
        mIsInPictureInPictureMode = false;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        LogUtil.d(TAG, ">>> onDestroy");

        if (mFeature.hasFeature(REMOTE_CONTROL_FOCUS)){
            Settings.System.putString(getContentResolver(), "current_media_source", "");
        }

        // 移除 Handler 消息
        mHandler.removeCallbacksAndMessages(null);

        unregisterReceiver(mUIReceiver);
        HBroadcastUtils.getInstance(this).unregisterReceiver(mLocalReceiver);
        unregisterSettingsObserver();

        // 是手动触发 finish() 结束窗口
        if (isFinishing()) {
            LogUtil.i(TAG, "[onDestroy]reason: call finish function!");
        } else {
            if (isRecreating()) {
                LogUtil.i(TAG, "[onDestroy]reason: is trigger recreate!");
            }

            printVideoWindowInfo("onDestroy");
            return;
        }

        // 开始退出进程检查（节省资源）
        boolean isSupportBackKeyExit = Utility.isExitOnBackKey();
        boolean isInPictureInPictureMode = isInPictureInPictureMode();

        // 画中画模式时，进入音乐，不需要退出进程
        // 分屏时，activity 也会先销毁再重建，所以不需要退出进程
        if (isSupportBackKeyExit
                && !isInPictureInPictureMode
                && !isRecreating()
                && !onlyFinishVideoUI()) {
            if (isInMultiWindowMode()) {
                LogUtil.i(TAG, "[onDestroy] isInMultiWindowMode!");
                // 避免分屏状态（视频在第二屏）按返回键出现后台播放情况；
                if (isBackKeyTriggerFinish()) {
                    mViewModel.exitApplication(3);
                }
            } else {
                mViewModel.exitApplication(2);
            }
        } else {
            // 画中画模式点击 Recent 按钮
            if (isInPictureInPictureMode
                    && mIsRecentAppsStart
                    && isRecentExitPipFinish()) {
                mViewModel.exitApplication(4);
            } else if (!isInPictureInPictureMode
                    && isClosePipFinish()) {
                mViewModel.exitApplication(5);
            }

            printVideoWindowInfo("onDestroy");
        }
    }

    /**
     * 打印视频窗口信息
     *
     * @param caller 调用者
     */
    private void printVideoWindowInfo(String caller) {
        boolean isInMultiWindMode = isInMultiWindowMode();
        boolean isInPictureInPictureMode = isInPictureInPictureMode();
        boolean isBackgroundPlayMode = Argument.isBackgroundPlayMode();

        LogUtil.i(TAG, "[" + caller + "] " +
                "\n     isMultiWindMode = " + isInMultiWindMode +
                "\n     isInPictureInPictureMode = " + isInPictureInPictureMode +
                "\n     isBackgroundPlayMode = " + isBackgroundPlayMode);
    }

    /**
     * @return whether the app associated with the given {@param packageName} is allowed to enter
     * picture-in-picture.
     */
    static boolean getEnterPipStateForPackage(Context context, int uid, String packageName) {
        final AppOpsManager appOps = context.getSystemService(AppOpsManager.class);
        return appOps.checkOpNoThrow(OPSTR_PICTURE_IN_PICTURE, uid, packageName) == MODE_ALLOWED;
    }

    private void enterPipMode(int reason) {
        // 非显示状态不处理
        if (!isResumedEx()) {
            return;
        }

        LogUtil.i(TAG, ">>> enterPipMode: reason = " + reason);

        // 别乱响应这个事件，视频才需要处理它
        if (mShowGroupType == E_GROUP_SHOW_VIDEO_INFO) {
            // 高版本需要检查是否允许画中画
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                if (!getEnterPipStateForPackage(
                        getApplicationContext(), BaseMediaData.UID, getPackageName())) {
                    LogUtils.iTag(TAG, "Not allowed to enter picture in picture!");
                    return;
                }
            }

            boolean isVideoActivityBackground =
                    requestUiModel().isVideoActivityBackground();
            if (Argument.isSupportPIP()
                    && !AppGlobalData.getInstance().isPauseStatus()
                    && !isInPictureInPictureMode()
                    // [ 这里不需要判断 ui 是否在显示，按 Home 进入画中画时，可能界
                    //   面先 onPause, 再收到进入画中画广播，导致无法进入画中画模式 ]
                    // && mAppData.mVideoUIShow
                    && !isVideoActivityBackground) {
                LogUtil.i(TAG, "\t enterPictureInPictureMode");

                // 画中画模式下，更新小窗口按钮
                updatePictureInPictureActions(2);

                if (enterPictureInPictureMode(mPictureInPictureParamsBuilder.build())) {
                    // [成功设置为画中画模式 或者 已经处于画中画模式/Activity#isInPictureInPictureMode()]
                }
            } else {
                LogUtil.w(TAG, "\t enterPipMode: Non-compliant!");
            }
        } else {
            LogUtil.w(TAG, "\t enterPIPMode: not in E_GROUP_SHOW_VIDEO_INFO!");
        }
    }

    /**
     * 画中画事件广播接收者
     * <p> 接收画中画小窗口上的点击事件广播；
     */
    private PipBroadcastReceiver mPipEventReceiver = null;

    /**
     * 可能是在尝试退出画中画窗口
     * <pre>
     *    画中画模式时，当前窗口是在 onPause() 状态的，正常从画中画回到全屏的流程是 onPause() -- onResume();
     *    如果在画中画时，窗口进入了 onStop() 状态，那大概率是要强制退出画中画模式（拖动画中画窗口直接往底部销毁）；
     * </pre>
     */
    private boolean mMaybeInExitPipWindow = false;

    /**
     * 当前是否在画中画窗口状态
     * <pre>
     *    画中画模式时，当前窗口是在 onPause() 状态，且一定会触发 onPictureInPictureModeChanged(true) 接口；
     *    注意了: 退出画中画模式，系统不一定会触发 onPictureInPictureModeChanged(false) 接口 (例如: 点击 Recent 按钮触发退出画中画)；
     * </pre>
     */
    private boolean mIsInPictureInPictureMode = false;

    /**
     * 画中画相关广播事件
     * <p> 处理画中画窗口上的按钮点击事件；
     */
    public class PipBroadcastReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (null == intent || !PIP.ACTION_PIP_MEDIA_CONTROL.equals(intent.getAction())) {
                return;
            }

            // This is where we are called back from Picture-in-Picture action items.
            final int controlType = intent.getIntExtra(PIP.EXTRA_CONTROL_TYPE, 0);
            LogUtil.d(TAG, "onReceive: " + controlType);

            switch (controlType) {
                case PIP.CONTROL_TYPE_PLAY:
                    mViewModel.playerRelay().accept(
                            BaseViewModel.IPlayer::requestShouldPlayEvent);
                    break;
                case PIP.CONTROL_TYPE_PAUSE:
                    mViewModel.playerRelay().accept(
                            t -> t.requestShouldPauseEvent(false, 0));
                    break;
                case PIP.CONTROL_TYPE_PREV:
                    mViewModel.playerRelay().accept(
                            t -> t.requestPlayControl(IMusicState.PLAY_CMD_PREV));
                    break;
                case PIP.CONTROL_TYPE_NEXT:
                    mViewModel.playerRelay().accept(
                            t -> t.requestPlayControl(IMusicState.PLAY_CMD_NEXT));
                    break;
                default:
                    break;
            }
        }
    }

    @SuppressLint("UnspecifiedRegisterReceiverFlag")
    @Override
    public void onPictureInPictureModeChanged(boolean isInPictureInPictureMode, Configuration newConfig) {
        super.onPictureInPictureModeChanged(isInPictureInPictureMode, newConfig);
        LogUtil.d(TAG, ">>> onPictureInPictureModeChanged: " + isInPictureInPictureMode);

        if (isInPictureInPictureMode) {
            // [onPictureInPictureModeChanged 可能在调用 onPause 后才执行]
            mAppData.mVideoUiShow = true;

            // [进入画中画模式: 有概率出现 onPictureInPictureModeChanged 慢
            //  于 onConfigurationChanged 触发的情况, 所以当触发该方法时需要
            //  更新 SurfaceView 布局大小, 避免 SurfaceView 大小和窗口不一致.]
            if (mVideoInfoFragment != null) {
                mVideoInfoFragment.doCallbackEvent(
                        IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_LAYOUT,
                        newConfig.screenWidthDp, newConfig.screenHeightDp);
            }

            // Starts receiving events from action items in PiP mode.
            if (null == mPipEventReceiver) {
                mPipEventReceiver = new PipBroadcastReceiver();
                getApplicationContext().registerReceiver(
                        mPipEventReceiver, new IntentFilter(PIP.ACTION_PIP_MEDIA_CONTROL));
            }

            updatePictureInPictureActions(0);
        } else {
            // We are out of PiP mode. We can stop receiving events from it.
            if (null != mPipEventReceiver) {
                getApplicationContext().unregisterReceiver(mPipEventReceiver);
                mPipEventReceiver = null;
            }

            // [快速拖动 PIP 小窗口到显示屏底部直接销毁]
            if (isStopped() && mMaybeInExitPipWindow && !isFinishing()) {
                mMaybeInExitPipWindow = false;
                videoUiFinish(106);
            }

            // [销毁画中画窗口，onPictureInPictureModeChanged 先于 onStop 被执行]
            if (!isResumedEx() && !isStopped() && !isFinishing()) {
                mHandler.removeCallbacks(mClosePipFinishRunnable);
                mHandler.postDelayed(mClosePipFinishRunnable,  250);
            }
        }

        // 保存 PIP 窗口状态标记
        mIsInPictureInPictureMode = isInPictureInPictureMode;
    }

    /**
     * 是否捕获到关闭 PIP 动作
     * <pre>
     *    onPictureInPictureModeChanged false 不一定是关闭 PIP 也可能是退出 PIP 进入全屏状态；
     *    所以我们需要延时判定才可以确定当前 PIP Mode Changed 是否是点击 close 按钮退出 PIP 动作；
     * <pre/>>
     */
    private final Runnable mClosePipFinishRunnable = () -> {
        // 如果是全屏动作直接返回
        if (isResumedEx()) {
            return;
        }

        // 如果是 Stopped 状态，说明可以销毁
        if (isStopped()
                && !isFinishing()
                && !isRecreating()) {
            videoUiFinish(107);
        }
    };

    /**
     * Update the state of pause/resume action item in Picture-in-Picture mode.
     *
     * @param iconId      The icon to be used.
     * @param title       The title text.
     * @param controlType The type of the action. either {@link PIP#CONTROL_TYPE_PLAY} or {@link
     *                    PIP#CONTROL_TYPE_PAUSE}.
     * @param requestCode The request code for the {@link PendingIntent}.
     */
    void updatePictureInPictureActions(int iconId, String title, int controlType, int requestCode) {
        final ArrayList<RemoteAction> actions = new ArrayList<>();

        // 适配高版本 Android 系统接口
        int flags = 0;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            flags |= PendingIntent.FLAG_IMMUTABLE;
        }

        // This is the PendingIntent that is invoked when a user clicks on the action item.
        // You need to use distinct request codes for play and pause, or the PendingIntent won't
        // be properly updated.
        final PendingIntent intent =
                PendingIntent.getBroadcast(
                        VideoUI.this,
                        requestCode,
                        new Intent(PIP.ACTION_PIP_MEDIA_CONTROL).putExtra(PIP.EXTRA_CONTROL_TYPE, controlType),
                        flags);

        final Icon icon = Icon.createWithResource(VideoUI.this, iconId);
        actions.add(new RemoteAction(icon, title, title, intent));

        mPictureInPictureParamsBuilder.setActions(actions);

        // This is how you can update action items (or aspect ratio) for Picture-in-Picture mode.
        // Note this call can happen even when the app is not in PiP mode. In that case, the
        // arguments will be used for at the next call of #enterPictureInPictureMode.
        setPictureInPictureParams(mPictureInPictureParamsBuilder.build());
    }

    void updatePictureInPictureActions(int reason) {
        LogUtils.iTag(TAG, "updatePictureInPictureActions: "
                + reason + ", playState = " + mAppData.mediaPlayState());
        final ArrayList<RemoteAction> actions = new ArrayList<>();

        // 适配高版本 Android 系统接口
        int flags = 0;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            flags |= PendingIntent.FLAG_IMMUTABLE;
        }

        final PendingIntent intentPrev =
                PendingIntent.getBroadcast(VideoUI.this,
                        PIP.REQUEST_PREV,
                        new Intent(PIP.ACTION_PIP_MEDIA_CONTROL)
                                .putExtra(PIP.EXTRA_CONTROL_TYPE, PIP.CONTROL_TYPE_PREV),
                        flags);

        actions.add(new RemoteAction(
                Icon.createWithResource(VideoUI.this, R.drawable.btn_pip_prev),
                "previous", "previous", intentPrev));

        // 如果当前不在暂停状态
        if (!AppGlobalData.getInstance().isPauseStatus()) {
            final PendingIntent intentPause =
                    PendingIntent.getBroadcast(VideoUI.this,
                            PIP.REQUEST_PAUSE,
                            new Intent(PIP.ACTION_PIP_MEDIA_CONTROL)
                                    .putExtra(PIP.EXTRA_CONTROL_TYPE, PIP.CONTROL_TYPE_PAUSE),
                            flags);

            actions.add(new RemoteAction(
                    Icon.createWithResource(VideoUI.this, R.drawable.btn_pip_pause),
                    "Pause", "Pause", intentPause));
        } else {
            final PendingIntent intentPlay =
                    PendingIntent.getBroadcast(VideoUI.this,
                            PIP.REQUEST_PLAY,
                            new Intent(PIP.ACTION_PIP_MEDIA_CONTROL)
                                    .putExtra(PIP.EXTRA_CONTROL_TYPE, PIP.CONTROL_TYPE_PLAY),
                            flags);

            actions.add(new RemoteAction(
                    Icon.createWithResource(VideoUI.this, R.drawable.btn_pip_play),
                    "Play", "Play", intentPlay));
        }

        final PendingIntent intentNext =
                PendingIntent.getBroadcast(VideoUI.this,
                        PIP.REQUEST_NEXT,
                        new Intent(PIP.ACTION_PIP_MEDIA_CONTROL)
                                .putExtra(PIP.EXTRA_CONTROL_TYPE, PIP.CONTROL_TYPE_NEXT),
                        flags);

        actions.add(new RemoteAction(
                Icon.createWithResource(VideoUI.this, R.drawable.btn_pip_next),
                "next", "next", intentNext));

        mPictureInPictureParamsBuilder.setActions(actions);
        setPictureInPictureParams(mPictureInPictureParamsBuilder.build());
    }

    private void updateBrightness() {
        WindowManager.LayoutParams lp = getWindow().getAttributes();
        lp.screenBrightness = mAppData.mScreenBrightness;
        getWindow().setAttributes(lp);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        LogUtil.d(TAG, ">>> onNewIntent: " + intent);

        if (intent != null && intent.getAction() != null) {
            mAppData.mSingleVideoPlay = false;
            mAppData.mSingleVideoFilePath = getPathFromIntent(intent);
        }
    }

    /**
     * 获取跳转的意图播放路径
     *
     * @param intent 意图
     * @return 路径信息
     */
    private String getPathFromIntent(Intent intent) {
        if (intent != null) {
            String action = intent.getAction();

            // 用户触发的数据载荷
            if (Intent.ACTION_VIEW.equals(action)) {
                Uri uri = intent.getData();
                if (uri != null) {
                    return MediaUriUtils.parseVideoUriFilePath(getApplicationContext(), uri);
                }
            }

            // 特定数据类型（e.g 宇通客车开机宣传片）
            if ("com.hcn.AutoMediaPlayer.action.PLAY_VIDEO".equals(action)) {
                return intent.getStringExtra("path");
            }
        }
        return null;
    }

    @SuppressLint("MissingSuperCall")
    @Override
    protected void onSaveInstanceState(@NonNull Bundle outState) {
        LogUtils.vTag(TAG, ">>> onSaveInstanceState()");

        // [recreate 时候会保存状态, 由于我们 Fragment 的用法没有处理  onCreateView(...)
        //  的 savedInstanceState 状态，这里屏蔽掉保存状态动作，避免 Fragment 状态混乱.]
        // super.onSaveInstanceState(outState);
    }

    /** 初始化第一个显示页面 **/
    private void initFirstFragment() {
        // 请求播放音频焦点
        mViewModel.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.onRequestAudioFocus, null, null));

        // 请求播放任务（如果是暂停不触发恢复播放）
        mViewModel.playerRelay().accept(
                BaseViewModel.IPlayer::requestPlayTask);

        // 检查更新页面显示
        onUpdateFragmentEvent();
    }

    /**
     * 历史遗留接口
     * <p> 为 Fragment 回调事件到 Activity 而设计；
     *
     * @param eventId 事件 ID
     * @param wParam  附加参数 1
     * @param lParam  附加阐述 2
     * @deprecated 建议有时间的时候尽可能淘汰与它关联的链路；
     */
    @Deprecated
    @Override
    public void onMediaEvent(int eventId, Object wParam, Object lParam) {
        // 不在视频模式状态不处理
        if (!mAppData.isMediaType(IMusicState.MEDIA_TYPE_VIDEO)) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_GOTO_MUSIC_LIST_PAGE:
                mAppData.mSelectedDevice = mAppData.mCurrentDevice;
                onShowFragmentEvent(E_GROUP_SHOW_VIDEO_LIST);
                return;
            case IMediaEvent.EVENT_GOTO_MUSIC_SEARCH_PAGE:
                onShowFragmentEvent(E_GROUP_SHOW_VIDEO_SEARCH);
                return;
            case IMediaEvent.EVENT_GOTO_MUSIC_INFO_PAGE:
                onShowInfoFragmentEvent();
                break;
            default:
                break;
        }

        onLocalMediaEvent(eventId, null);
    }

    /**
     * 处理页面事件
     * <p> 建议选择性处理，避免被嵌套；
     *
     * @param event 事件 ID
     * @param obj1  附加数据对象 1
     * @param obj2  附加数据对象 2
     */
    @Override
    protected void onHandlePageEvent(int event, Object obj1, Object obj2) {
        super.onHandlePageEvent(event, obj1, obj2);

        switch (event) {
            case IMediaEvent.EVENT_CHANGE_FULL_SCREEN:
                onEventSwitchFullscreen();
                break;
            case IMediaEvent.EVENT_TRIGGER_ENTER_PIP_MODE:
                // 检查是否可以进入画中画，通过配置来；
                enterPipMode(3);
                break;
            case IMediaEvent.EVENT_NONE:
            default:
                break;
        }
    }

    /**
     * 处理全屏切换事件
     * @see IMediaEvent#EVENT_CHANGE_FULL_SCREEN
     */
    private void onEventSwitchFullscreen() {
        // 非视频播放渲染页面不处理
        if (mShowGroupType != E_GROUP_SHOW_VIDEO_INFO) {
            return;
        }

        // 音视频分屏时调整窗口大小会出现窗口焦点在视频，播放类型为音乐，此时点击视频无法播放
        if (mAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)
                && isInMultiWindowMode() && mIsTopResumedActivity) {
            mAppData.mMediaType = IMusicState.MEDIA_TYPE_VIDEO;
            mViewModel.playerRelay().accept(
                    BaseViewModel.IPlayer::requestPlayTask);
        }

        mAppData.mFullScreen = !mAppData.mFullScreen;
        setWindowFullScreen(mAppData.mFullScreen);
    }

    /**
     * 设置当前窗口全屏显示状态
     * <p> 如果是在多窗口状态，不可以设置成全屏；
     *
     * @param fullScreen 全屏/非全屏
     */
    private void setWindowFullScreen(boolean fullScreen) {
        WindowManager.LayoutParams attrs = getWindow().getAttributes();

        if (fullScreen && !isInMultiWindowMode()) {
            if (WindowManager.LayoutParams.FLAG_FULLSCREEN !=
                    (attrs.flags & WindowManager.LayoutParams.FLAG_FULLSCREEN)) {
                attrs.flags |= WindowManager.LayoutParams.FLAG_FULLSCREEN;
                getWindow().setAttributes(attrs);
                LogUtil.v(TAG, ">>>> setWindowFullScreen / true");
				
                if (mFeature.hasFeature(REMOTE_CONTROL_FOCUS)) {
                    Settings.System.putString(getContentResolver(), "current_media_source", "video_fullscreen");
                }
            }
        } else {
            if (WindowManager.LayoutParams.FLAG_FULLSCREEN ==
                    (attrs.flags & WindowManager.LayoutParams.FLAG_FULLSCREEN)) {
                attrs.flags &= (~WindowManager.LayoutParams.FLAG_FULLSCREEN);
                getWindow().setAttributes(attrs);
                LogUtil.v(TAG, ">>>> setWindowFullScreen / " + fullScreen);
				
                if (mFeature.hasFeature(REMOTE_CONTROL_FOCUS)) {
                    Settings.System.putString(getContentResolver(), "current_media_source", "video_not_fullscreen");
                }
            }
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            int nShowGroupType = E_GROUP_SHOW_NULL;

            if (mShowGroupType == E_GROUP_SHOW_VIDEO_SEARCH) {
                nShowGroupType = E_GROUP_SHOW_VIDEO_LIST;
            } else if (mShowGroupType == E_GROUP_SHOW_VIDEO_LIST) {
                onShowInfoFragmentEvent();
                return true;
            }

            if (nShowGroupType != E_GROUP_SHOW_NULL) {
                onShowFragmentEvent(nShowGroupType);
                return true;
            }

            onSystemBackKeyEvent();
            videoUiFinish(201);
        } else if (keyCode == KeyEvent.KEYCODE_ENTER) {
            Intent intent = new Intent(IMediaBroadcast.ACTION_EVENT_K_ENTER);
            sendBroadcastAsUser(intent, UserHandle.getUserHandleForUid(BaseMediaData.UID));
        } else if (keyCode == KeyEvent.KEYCODE_HOME) {
            LogUtil.d(TAG, "home::" + keyCode);

            if (mShowGroupType == E_GROUP_SHOW_VIDEO_INFO) {
                videoUiFinish(202);
            }
        }
		
        if(mFeature.hasFeature(REMOTE_CONTROL_FOCUS)){
            if (keyCode != KeyEvent.KEYCODE_VOLUME_UP && keyCode != KeyEvent.KEYCODE_VOLUME_DOWN){
                if (mVideoInfoFragment != null && mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
                    mAppData.mFullScreen = false;
                    mVideoInfoFragment.doCallbackEvent(EVENT_CHANGE_FULL_SCREEN);
                }
            }
        }
		
        return super.onKeyDown(keyCode, event);
    }

    /**
     * 处理 Local 媒体事件
     * <pre>
     *    1、处理本地广播事件 {@link SpecialChain#ACTION_LOCAL_CALLBACK}
     *    2、处理 Fragment 下发的事件（这个用法很危险，建议慢慢清理掉关联的用例）；
     * </pre>
     *
     * @param eventId {@link IMediaEvent}
     * @param data 附加数据对象
     */
    private void onLocalMediaEvent(int eventId, String data) {
        if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_VIDEO) {
            LogUtil.v(TAG, "onLocalMessage Something Wrong, nEventID: " + eventId);
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_NO_MUSIC_FILE:
            case IMediaEvent.EVENT_MEDIA_LOADING_FILE: {
                onUpdateFragmentEvent();

                // 没有任何数据的时候，在画中画模式，直接退出（会直接退出当前源）
                if (isInPictureInPictureMode()) {
                    videoUiFinish(301);
                }
                break;
            }
            case IMediaEvent.EVENT_CHANGE_VIDEO_LIST: {
                if (mShowGroupType != E_GROUP_SHOW_VIDEO_LIST) {
                    onShowInfoFragmentEvent();
                } else {
                    if (!TextUtils.isEmpty(data)) {
                        if ("clickListItem".equals(data)) {
                            onShowInfoFragmentEvent();
                        }
                    }

                    LogUtil.d(TAG,
                            "EVENT_CHANGE_VIDEO_LIST: <E_GROUP_SHOW_VIDEO_LIST == "
                                    + "mShowGroupType>, data = " + data);
                }
                break;
            }
            case IMediaEvent.EVENT_UPDATE_AUTO_BRAKE_STATUS: {
                // [BUG: PIP模式在硬解码视频界面触发刹车检测后再刹车会显示黑屏]
                if (!mAppData.mVideoUiShow) {
                    break;
                }

                // [延时需要长一点, 避免抖动, 再一个 Fragment 切换动画时间有要求, 避免触发原生BUG]
                mHandler.removeMessages(VideoUiHandler.MSG_WHAT_UPDATE_BRAKE_STATUS);
                mHandler.sendEmptyMessageDelayed(
                        VideoUiHandler.MSG_WHAT_UPDATE_BRAKE_STATUS, 600);
                break;
            }
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE: {
                if (!mAppData.mVideoUiShow) {
                    break;
                }

                boolean isPIPMode = isInPictureInPictureMode();
                if (isPIPMode) {
                    switch (mAppData.mMediaPlayState) {
                        case IMusicState.E_PLAY_STATE_PLAY:
                        case IMusicState.E_PLAY_STATE_PAUSE:
                            updatePictureInPictureActions(1);
                            break;
                        default:
                            break;
                    }
                }
                break;
            }
            default:
                break;
        }

        dispatchLocalMediaEvent(eventId, data);
    }

    /** [这个函数被乱用了, 导致 onResume 恢复显示不严谨] **/
    private void onUpdateFragmentEvent() {
        mAppData.mFullScreen = false;
        setWindowFullScreen(false);

        LogUtil.d(TAG, "onUpdateFragmentEvent,"
                + " isCanWatchVideo: " + isCanWatchVideo());

        // [检查是否存在文件管理器触发的播放任务]
        if (!TextUtils.isEmpty(mAppData.mSingleVideoFilePath)) {
            if (isCanWatchVideo()) {
                onShowFragmentEvent(E_GROUP_SHOW_VIDEO_INFO);
            } else {
                onShowFragmentEvent(E_GROUP_SHOW_VIDEO_PARKING);
            }
            return;
        }

        // [检查更新当前需要显示的 Fragment 页面]
        if (mAppData.mCurrentDevice.isLoading()) {
            onShowFragmentEvent(E_GROUP_SHOW_LOADING);
        } else if (mAppData.mCurrentDevice.mVideoInfoList.isEmpty()) {
            onShowFragmentEvent(E_GROUP_SHOW_LOADING);
        } else if (isCanWatchVideo()) {
            onShowFragmentEvent(E_GROUP_SHOW_VIDEO_INFO);
        } else {
            onShowFragmentEvent(E_GROUP_SHOW_VIDEO_PARKING);
        }
    }

    private void onShowInfoFragmentEvent() {
        onUpdateFragmentEvent();
    }

    /**
     * 分发 Local 媒体事件
     * <pre>
     *    过时的接口，有时间可以替换淘汰它；
     *    分发业务逻辑模块下发的 Local 媒体事件；
     * </pre>
     *
     * @param eventId {@link IMediaEvent}
     * @param data 附加数据对象
     * @see #onLocalMediaEvent(int, String);
     * @deprecated 过时的分发处理函数
     */
    @Deprecated
    private void dispatchLocalMediaEvent(int eventId, String data) {
        if (mLoadingFragment != null) {
            mLoadingFragment.doCallbackEvent(eventId);
        }

        if (mVideoInfoFragment != null) {
            mVideoInfoFragment.doCallbackEvent(eventId);
        }

        if (mVideoListFragment != null) {
            mVideoListFragment.doCallbackEvent(eventId);
        }

        if (mVideoSearchFragment != null) {
            mVideoSearchFragment.doCallbackEvent(eventId);
        }

        if (mVideoGridListFragment != null) {
            mVideoGridListFragment.doCallbackEvent(eventId);
        }
    }

    /**
     * 判定指定名称的 Fragment 是否在回退栈中
     *
     * @param key An optional name for this back stack state
     * @return 是否在回退栈中
     */
    private boolean inFragmentBackStack(@NonNull String key) {
        int count = getSupportFragmentManager().getBackStackEntryCount();
        for (int i = 0; i < count; i++) {
            FragmentManager.BackStackEntry entry = getSupportFragmentManager().getBackStackEntryAt(i);
            String name = entry.getName();
            if (!TextUtils.isEmpty(key) && key.equals(name)) {
                return true;
            }
        }
        return false;
    }

    /**
     * 尝试构建期望类型的媒体页面
     *
     * @param type {@link IVideoPage}
     * @return {@link MediaFragment}
     */
    public MediaFragment tryBuildMediaFragment(final int type) {
        MediaFragment fragment = null;

        switch (type) {
            case E_GROUP_SHOW_LOADING:
                if (null == mLoadingFragment) {
                    mLoadingFragment = new VideoLoadingFragment();
                }
                fragment = mLoadingFragment;
                break;

            case E_GROUP_SHOW_VIDEO_INFO:
                if (null == mVideoInfoFragment) {
                    mVideoInfoFragment = new VideoInfoFragment();
                }
                fragment = mVideoInfoFragment;
                break;

            case E_GROUP_SHOW_VIDEO_LIST:
                if (E_THEME_TYPE == ThemeX.ET_GOD_201
                        || E_THEME_TYPE == ThemeX.ET_GOD_206) {
                    if (null == mVideoGridListFragment) {
                        mVideoGridListFragment = new VideoGridListFragment(this);
                    }
                    fragment = mVideoGridListFragment;
                } else {
                    if (null == mVideoListFragment) {
                        mVideoListFragment = new VideoListFragment();
                    }
                    fragment = mVideoListFragment;
                }
                break;

            case E_GROUP_SHOW_VIDEO_SEARCH:
                if (null == mVideoSearchFragment) {
                    mVideoSearchFragment = new VideoSearchFragment();
                }
                fragment = mVideoSearchFragment;
                break;

            case E_GROUP_SHOW_VIDEO_PARKING:
                if (null == mVideoParkingFragment) {
                    mVideoParkingFragment = new VideoParkingFragment();
                }
                fragment = mVideoParkingFragment;
                break;

            default:
                break;
        }

        return fragment;
    }

    /**
     * 当前 fragment 是否是视频播放页
     * <p> 扩展方法，以后可能不止一个视频播放页面；
     *
     * @param fragment 目标页对象
     * @return 是/否
     */
    private boolean isVideoInfoFragment(@NonNull Fragment fragment) {
        return mVideoInfoFragment == fragment;
    }

    /**
     * 处理期望的 Fragment 显示事件
     * <p> 这里有页面栈操作（看不懂的人请别乱加乱改）
     *
     * @param type {@link IVideoPage}
     */
    public void onShowFragmentEvent(final int type) {
        LogUtil.d(TAG, "onShowFragmentEvent: "
                + " mShowGroupType = " + mShowGroupType + ", type = " + type);

        // [如果是在 onResume 处理中, 需要允许刷新状态]
        if (type == mShowGroupType && !mIsUiResuming) {
            return;
        }

        mShowGroupType = type;
        MediaFragment fragment = tryBuildMediaFragment(type);
        if (Objects.isNull(fragment)) {
            return;
        }

        // 开始执行 Fragment 事务
        FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
        transaction.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN);

        //  [只有 Fragment 发生改变才需要设置切换动画]
        if (mCurrentFragment != fragment) {
            long elapsedTime = SystemClock.elapsedRealtime();
            long deltaTime = elapsedTime - mSetFragmentAnimTime;

            // [动画不能频繁设置, 否则会出现 Fragment 概率隐藏, 这是原生的BUG]
            // [@android:integer/config_mediumAnimTime = 400ms, 时间差大于 600 就够了]
            if (deltaTime > 600) {
                mSetFragmentAnimTime = elapsedTime;

                // [设置 Fragment 切换动画, 避免画中画的时候刹车检测切换出现放大缩小的默认动画]
                if (Build.VERSION.SDK_INT > Build.VERSION_CODES.Q
                        && isVideoInfoFragment(fragment)) {
                    transaction.setCustomAnimations(
                            0, R.anim.fragment_fade_out);
                } else {
                    transaction.setCustomAnimations(
                            R.anim.fragment_fade_in, R.anim.fragment_fade_out);
                }
            }

            // [只有 Fragment 发生改变, 当前 Fragment 才需要隐藏]
            if (mCurrentFragment != null) {
                mCurrentFragment.uninitFragment();
                transaction.hide(mCurrentFragment);
            }
        }

        // [进入或者刷新页面，需要修正全屏显示状态]
        boolean isPIPMode = isInPictureInPictureMode();
        if (!isPIPMode) {
            // PIP 模式就没必要刷新菜单了
            if (mAppData.mFullScreen) {
                mAppData.mFullScreen = false;
                setWindowFullScreen(false);
            }
        }

        LogUtil.d(TAG, "onShowFragmentEvent, fragment isAdded: " + fragment.isAdded());

        if (!fragment.isAdded()) {
            transaction.add(xId(R.id.ll_container), fragment);
        } else {
            // [不是显示的才需要更新状态]
            if (!fragment.isVisible()) {
                fragment.initFragment();
                transaction.show(fragment);
            }
        }


        // 添加事务, 旧 Fragment 添加到回退栈
        String tryAddKey = "";
        String tryPopKey = "";
        int popBackStackFlag = FragmentManager.POP_BACK_STACK_INCLUSIVE;
        switch (type) {
            case E_GROUP_SHOW_VIDEO_INFO:
                popBackStackFlag = 0;
                tryAddKey = "video_info";
                tryPopKey = "video_info";
                break;
            case E_GROUP_SHOW_VIDEO_LIST:
                tryAddKey = "video_list";
                tryPopKey = "video_search";
                break;
            case E_GROUP_SHOW_VIDEO_SEARCH:
                tryAddKey = "video_search";
                break;
            default:
                break;
        }

        // 需要添加到回退栈中
        if (!TextUtils.isEmpty(tryAddKey)) {
            // 只添加一次，避免循环回退
            if (!inFragmentBackStack(tryAddKey)) {
                transaction.addToBackStack(tryAddKey);
            }
        }

        transaction.commitAllowingStateLoss();
        mCurrentFragment = fragment;

        // 尝试从回退栈出栈
        if (!TextUtils.isEmpty(tryPopKey)) {
            boolean isStateSaved = getSupportFragmentManager().isStateSaved();
            if (inFragmentBackStack(tryPopKey) && !isStateSaved) {
                LogUtil.v(TAG, "popBackStack: " + tryPopKey + ", " + popBackStackFlag);
                getSupportFragmentManager().popBackStack(tryPopKey, popBackStackFlag);
            }
        }
    }

    /** 注册 Settings/System 键值监听 **/
    protected void registerSettingsObserver() {
        if (mObserver == null) {
            mObserver = new SettingsKeyObserver(this);
        }

        getContentResolver().registerContentObserver(
                Settings.System.getUriFor(HConfig.driving_disable_video),
                true, mObserver);
    }

    /** 取消 Settings/System 键值监听 **/
    protected void unregisterSettingsObserver() {
        if (mObserver != null) {
            getContentResolver().unregisterContentObserver(mObserver);
        }
    }

    /**
     * 退出画中画状态
     * <pre>
     *     由于画中画的特殊性，前后装有些差异；
     *     我们暂时规定车速不影响画中画功能退出；
     * </pre>
     */
    @Override
    public void onDrivingWatchVideoStateChanged() {
        // 画中画模式，则立即退出视频
        if (isInPictureInPictureMode()
                && !Utility.canWatchVideoDriving(this)) {
            videoUiFinish(401);
        }
    }
}
