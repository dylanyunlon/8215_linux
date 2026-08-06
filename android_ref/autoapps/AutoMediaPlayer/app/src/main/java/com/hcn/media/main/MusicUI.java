package com.hcn.media.main;

import android.annotation.SuppressLint;
import android.carstatus.CarStatus;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.UserHandle;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;

import androidx.annotation.NonNull;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HFileUtils;
import com.hcn.common.utils.HImageUtils;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media_common.HBroadcastEx.SpecialChain;
import com.hcn.media_common.file.MediaUriUtils;
import com.hcn.media_data.ui.MediaPageState;
import com.hcn.media_data.ui.base.PageDataKV;
import com.hcn.media_theme.Argument;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.media_theme.ThemeUtilsEx;
import com.hcn.media_theme.ThemeX;
import com.hcn.media_base.IMediaBroadcast;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.fragment.IMusicPage;
import com.hcn.MediaActivity.MusicPlayerUiActivity;
import com.hcn.common.misc.HBroadcastUtils;
import com.hcn.media.main.base.BaseMusicActivity;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.common.Utility;
import com.hcn.media.music.common.MusicLoadingFragment;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media.music.MusicViewPaperFragment;
import com.hcn.media.music.common.MusicSearchFragment;
import com.hcn.media.music.gallery.MusicInfoFragmentEx;
import com.hcn.media.music.gallery.MusicListFragmentEx;
import com.hcn.media.music.gallery.MusicSearchFragmentEx;
import com.hcn.media.music.mcc201.Mcc201MusicInfoFragment;
import com.hcn.media.music.mcc201.Mcc201MusicListFragment;
import com.hcn.media.music.common.MusicInfoFragment;
import com.hcn.media.music.common.MusicListFragment;
import com.hcn.skinx.SkinX;
import com.orhanobut.logger.Logger;

import java.util.Objects;

/**
 * 音乐播放器主页面入口
 * <pre>
 *    由于历史传承，这个包名类名已经不能再修改，牵扯太多项目工程：
 *    e.g. framework/carservices、HMediaService、HVoice、SystemUI、Settings等；
 * </pre>
 *
 * @author 86158
 */
public class MusicUI extends BaseMusicActivity implements IMediaEventListener {

    /**
     * 音乐 Activity 类对象别名
     * <p> 为兼容各平台的历史交互逻辑使用；
     */
    public static Class<?> sAliasClass = MusicPlayerUiActivity.class;

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
     * 音乐 Activity 使用到的 Fragment 页面
     * <pre>
     *    这是由于需要兼容多套皮肤而定义的页面对象；
     *    虽然很多客户需求不一样，但是请可能复用现有的页面类，不要随意添加新的类对象；
     * </pre>
     */
    private MusicViewPaperFragment mMainViewPaperFragment = null;

    private MediaFragment mLoadingFragment = null;
    private MediaFragment mMusicSearchFragment = null;
    private MediaFragment mMusicInfoFragment = null;
    private MediaFragment mMusicListFragment = null;

    private MediaFragment mMusicInfoFragmentEx = null;
    private MediaFragment mMusicListFragmentEx = null;
    private MediaFragment mMusicSearchFragmentEx = null;

    private MediaFragment mMcc201MusicListFragment = null;
    private MediaFragment mMcc201MusicInfoFragment = null;

    /**
     * 广播接受者
     * <pre>
     *     内部事件处理;
     *     非跨进程广播，无 Binder 通讯消耗；
     * </pre>
     */
    private final BroadcastReceiver mEventReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            // 是内部 Local 广播事件
            if (SpecialChain.ACTION_LOCAL_CALLBACK.equals(action)) {
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

            // [onDestroy 才销毁这个广播接受者, 进入视频它不一定会销毁]
            if (!mAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
                // 处理 Video 相关事件
                if (mAppData.isMediaType(IMusicState.MEDIA_TYPE_VIDEO)) {
                    if (eventId == IMediaEvent.EVENT_GOTO_RESUME_VIDEO_PLAYER_UI) {
                        if (isStopped() && !isFinishing()) {
                            musicUiFinish(104);
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
                case IMediaEvent.EVENT_SERVICE_INITIALIZED: {
                    // 本地服务是否连接
                    if (mViewModel.isServiceReadyState()) {
                        initFirstFragment();
                    }
                    break;
                }
                case IMediaEvent.EVENT_NONE:
                default: {
                    onLocalMediaEvent(eventId, data);
                    break;
                }
            }
        }
    };

    /**
     * 一定要是默认无参数构造函数
     * <p> 可以在构造函数中初始化部分变量；
     */
    public MusicUI() {
        super();
        Logger.t(TAG).d("Constructor.");
    }

    @SuppressLint("ObsoleteSdkInt")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
       Logger.t(TAG).d("onCreate...");

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_music);

        Intent intent = getIntent();
        mAppData.mSingleMusicPlay = false;
        mAppData.mSingleMusicFilePath = getPathFromIntent(intent);
        getMusicRegInfo(intent);

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(SpecialChain.ACTION_LOCAL_CALLBACK);
        HBroadcastUtils.getInstance(this)
                .registerReceiver(mEventReceiver, intentFilter);

        registerAutoReceiver();

        // 检查并打印 Overlay 包版本
        if (HUtilsEx.isAppDebug()) {
            LogUtils.iTag(TAG, "overlay: "
                    + SkinX.getString("overlay_version"));
        }
    }

    /**
     * 检查并同步背景
     * <pre>
     *    优先检测用户设置的背景；
     *    再次使用外部资源包的背景；
     * </pre>
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

        // 检查是否有指定资源背景
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
                LogUtil.d(TAG, "background is ColorDrawable: " + colorValue);
            }
        }

        // 背景是否有效
        if (validBackground) {
            findViewById(xId(R.id.main_bg)).setBackground(wallPaper);
        }
    }

    /**
     * 测试外部资源背景
     * <p> 使用外部资源包的才需要测试；
     *
     * @test 测试用
     */
    private void testSkinXBackground() {
        if (!SkinX.supportTestSkinPackage()) {
            return;
        }

        Drawable wallPaper = SkinX.getDrawable(R.drawable.background);
        findViewById(xId(R.id.main_bg)).setBackground(wallPaper);
    }

    @Override
    public void onMultiWindowModeChanged(boolean isInMultiWindowMode, Configuration newConfig) {
        super.onMultiWindowModeChanged(isInMultiWindowMode, newConfig);
        LogUtil.e(TAG, ">>> onMultiWindowModeChanged: " + isInMultiWindowMode);
    }

    @SuppressLint("MissingSuperCall")
    @Override
    protected void onSaveInstanceState(@NonNull Bundle outState) {
        LogUtils.vTag(TAG, ">>> onSaveInstanceState().");

        // [recreate 时候会保存状态, 由于我们 Fragment 的用法没有处理  onCreateView(...)
        //  的 savedInstanceState 状态，这里屏蔽掉保存状态动作，避免 Fragment 状态混乱.]
        // super.onSaveInstanceState(outState);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        Logger.t(TAG).d("onNewIntent.");
        super.onNewIntent(intent);

        String action = intent.getAction();
        if (action != null && action.equals(Intent.ACTION_VIEW)) {
            mAppData.mSingleMusicPlay = false;
            mAppData.mSingleMusicFilePath = getPathFromIntent(intent);
            getMusicRegInfo(intent);
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    /**
     * 恢复前处理
     * <p> 在 {@link MusicUI#onResume()} 前调用；
     */
    @Override
    protected void onPreResume() {
        super.onPreResume();

        // 通知 Video Player UI 即将进入显示模式
        enterAndResumeMusicPlayerUI(PRE_RESUME_STATE);
    }

    @Override
    protected void onResume() {
        Logger.t(TAG).d("[Enter] onResume.");
        super.onResume();

        // 注册语音广播
        registerVoiceReceiver();

        // 更新显示页面
        if (localServiceConnected()) {
            initFirstFragment();
        }

        Logger.t(TAG).d("[Leave] onResume.");
    }

    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();
        LogUtils.vTag(TAG, "onAttachedToWindow");
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    public void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        LogUtils.vTag(TAG, "onDetachedFromWindow");
    }

    /**
     * 特定场景下监听的语音控制类广播
     * <p> e.g. 处理语音 “打开/关闭 + 音乐/视频” 指令；
     */
    private VoiceBroadcastReceiver mVoiceBroadcastReceiver = new VoiceBroadcastReceiver();

    private final class VoiceBroadcastReceiver extends BroadcastReceiver {
        private final String VOICE_CLOSE_MEDIA_INFO_KEY = "CLOSE_MEDIA";

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (IConstant.ACTION_VOICE_2_HMEDIA.equals(action)) {
                String szEvent = intent.getStringExtra(IConstant.EXTRA_MEDIA_EVENT);

                if ("open_video".equals(szEvent)) {
                    if (!isFinishing()) {
                        MiscUtils.goToHome(context);
                        musicUiFinish(11);
                    }
                }
            } else if (VOICE_CLOSE_MEDIA_ACTION.equals(action)) {
                String stringExtra = intent.getStringExtra(VOICE_CLOSE_MEDIA_INFO_KEY);
                if ("close_music".equals(stringExtra)){
                    MiscUtils.goToHome(context);
                    musicUiFinish(11);
                }
            }
        }
    }

    /**
     * 注册语音关联广播
     * <p> HVoice 进程关联广播, e.g 同行者...
     */
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

    /**
     * 车载系统相关广播处理器；
     * <pre>
     *    建议只处理和车载系统相关的特定广播;
     *    例如：倒车、ACC、安全相关等系统广播；
     * </pre>
     */
    private AutoBroadcastReceiver mAutoBroadcastReceiver = new AutoBroadcastReceiver();

    private final class AutoBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            // 倒车事件处理
            if (CarStatus.ACTION_REVSTATUS.equals(action)) {
                if (mCurrentFragment != null) {
                    mCurrentFragment.onExternalEvent("reverse", 0, 0);
                }
            }
        }
    }

    /**
     * 注册车载系统相关广播
     * <pre>
     *    倒车状态广播
     *    ACC ON/OFF 状态广播
     *    ...
     * </pre>
     */
    private void registerAutoReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(CarStatus.ACTION_REVSTATUS);

        if (null == mAutoBroadcastReceiver) {
            mAutoBroadcastReceiver = new AutoBroadcastReceiver();
        }

        registerReceiver(mAutoBroadcastReceiver, intentFilter);
    }

    private void unregisterAutoReceiver() {
        if (mAutoBroadcastReceiver != null) {
            unregisterReceiver(mAutoBroadcastReceiver);
            mAutoBroadcastReceiver = null;
        }
    }

    @Override
    protected void onFragment2MainEvent(int event, Object obj1, Object obj2) {
        if (!isResumedEx()) {
            Logger.t(TAG).w("onFragment2MainEvent," +
                    " The event is not in a valid life cycle!");
            return;
        }

        // 显示音频 Fragment 页面事件
        if (event == IMediaEvent.EVENT_SHOW_MUSIC_FRAGMENT) {
            assert obj1 instanceof Integer;
            int pageEvent = (int) obj1;
            switch (pageEvent) {
                case IMusicPage.E_GROUP_SHOW_MUSIC_SEARCH:
                case IMusicPage.MCC204_E_GROUP_SHOW_MUSIC_INFO:
                case IMusicPage.E_GROUP_SHOW_MUSIC_INFO_EX:
                case IMusicPage.E_GROUP_SHOW_MUSIC_SEARCH_EX:
                case IMusicPage.E_GROUP_SHOW_MUSIC_LIST_EX:
                case IMusicPage.MCC204_E_GROUP_SHOW_MUSIC_LIST:
                    onShowFragmentEvent(pageEvent);
                default:
                    break;
            }

            return;
        }

        // 也可以传递媒体事件
        onMediaEvent(event, obj1, obj2);
    }

    @Override
    protected void onPause() {
        Logger.t(TAG).d("onPause.");
        super.onPause();

        unregisterVoiceReceiver();
    }

    @Override
    protected void onStop() {
        super.onStop();
        Logger.t(TAG).d("onStop.");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Logger.t(TAG).d("onDestroy.");

        // [是触发 finish() 结束窗口]
        if (isFinishing()) {
            LogUtil.i(TAG, "[onDestroy]reason: call finish function!");
        } else {
            if (isRecreating()) {
                LogUtil.i(TAG, "[onDestroy]reason: is trigger recreate!");
            }
        }

        unregisterAutoReceiver();
        HBroadcastUtils.getInstance(this).unregisterReceiver(mEventReceiver);

        boolean isMultiWndMode = isInMultiWindowMode();
        LogUtil.i(TAG, "[onDestroy]isMultiWndMode: " + isMultiWndMode);

        // 分屏时，activity 也会先销毁再重建，所以不需要退出进程
        if (Utility.isExitOnBackKey()
                && isFinishing()
                && !isMultiWndMode
                && !isRecreating()
                && !onlyFinishMusicUI()) {
            mViewModel.exitApplication(1);
        }
    }

    /**
     * 从意图获取文件路径
     *
     * @param intent 意图对象
     * @return 文件路径
     */
    private String getPathFromIntent(Intent intent) {
        if (intent != null) {
            String action = intent.getAction();
            if (action != null && action.equals(Intent.ACTION_VIEW)) {
                Uri uri = intent.getData();
                if (uri != null) {
                    return MediaUriUtils.parseAudioUriFilePath(getApplicationContext(), uri);
                }
            }
        }
        return null;
    }

    /**
     * 历史接口，处理特定意图。
     * <p> 获取是否有外部指定的播放信息；
     *
     * @param intent 意图
     */
    private void getMusicRegInfo(Intent intent) {
        if (Objects.isNull(intent)) {
            return;
        }

        Bundle bundle = intent.getExtras();
        if (bundle != null && bundle.getString("isHasSong") != null) {
            mAppData.mMusicRegInfo.mEnable = true;
            mAppData.mMusicRegInfo.mTitle = bundle.getString("songs");
            mAppData.mMusicRegInfo.mArtist = bundle.getString("singer");
            mAppData.mMusicRegInfo.mAlbum = bundle.getString("album");

            LogUtil.d(TAG, "External trigger: "
                    + mAppData.mMusicRegInfo.mArtist + "/"
                    + mAppData.mMusicRegInfo.mAlbum + "/"
                    + mAppData.mMusicRegInfo.mTitle);
        }
    }

    /** 初始化第一个显示页面 **/
    private void initFirstFragment() {
        mViewModel.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.onRequestAudioFocus, null, null));
        mViewModel.playerRelay().accept(
                BaseViewModel.IPlayer::requestPlayTask);

        if (!TextUtils.isEmpty(mAppData.mSingleMusicFilePath)) {
            // 独立播放任务
        } else if (mAppData.mAllowResumePlay &&
                mAppData.mMediaType == IMusicState.MEDIA_TYPE_MUSIC) {
            mViewModel.playerRelay().accept(
                    BaseViewModel.IPlayer::requestShouldPlayEvent);
        }

        onScanStorageEvent();
    }

    private void onScanStorageEvent() {
        if (mLoadingFragment != null) {
            mLoadingFragment.initFragment();
        }

        onUpdateFragmentEvent();
    }

    @Override
    public void onBackPressed() {
        LogUtil.i(TAG, "onBackPressed.");

        if (Utility.isExitOnBackKey()) {
            mViewModel.exitApplication(2);
        } else {
            musicUiFinish(0);
        }

        super.onBackPressed();
    }

    /**
     * 仅仅销毁音乐 UI 页面
     * <p> 用来判定是否只是销毁 UI 而不是退出进程；
     *
     * @return {@code true}: yes<br>{@code false}: no
     */
    private boolean onlyFinishMusicUI() {
        return finishReason() == 104;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        LogUtil.d(TAG, "keycode: " + keyCode);

        // 返回按键[如果在播放信息界面会触发进程退出]
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (onBackKeyDownEvent()) {
                return true;
            }
        } else if (keyCode == KeyEvent.KEYCODE_ENTER) {
            onEnterKeyDownEvent();
        }

        return super.onKeyDown(keyCode, event);
    }

    /** Enter 键拦截处理 **/
    public void onEnterKeyDownEvent() {
        Intent intent = new Intent(IMediaBroadcast.ACTION_EVENT_K_ENTER);
        sendBroadcastAsUser(intent, UserHandle.getUserHandleForUid(BaseMediaData.UID));
    }

    /** Back 键拦截处理 **/
    public boolean onBackKeyDownEvent() {
        LogUtil.e(TAG, "onBackKeyDownEvent, mShowGroupType = " + mShowGroupType);

        if (mShowGroupType == E_GROUP_SHOW_MUSIC_INFO) {
            if (null != mMainViewPaperFragment) {
                if (mMainViewPaperFragment.showMusicInfo()) {
                    return true;
                }
            }
        } else if (mShowGroupType == E_GROUP_SHOW_MUSIC_SEARCH) {
            if (E_THEME_TYPE == ThemeX.ET_GOD_204) {
                mShowGroupType = MCC204_E_GROUP_SHOW_MUSIC_LIST;
                getSupportFragmentManager()
                        .beginTransaction()
                        .hide(mMusicSearchFragment)
                        .commitAllowingStateLoss();
                getSupportFragmentManager().popBackStack();
                mCurrentFragment = mMusicListFragment;
            } else {
                mShowGroupType = E_GROUP_SHOW_MUSIC_INFO;
                getSupportFragmentManager()
                        .beginTransaction()
                        .hide(mMusicSearchFragment)
                        .commitAllowingStateLoss();

                getSupportFragmentManager().popBackStack();
                mCurrentFragment = mMainViewPaperFragment;
            }
            return true;
        } else if (mShowGroupType == MCC201_E_GROUP_SHOW_MUSIC_LIST) {
            mShowGroupType = MCC201_E_GROUP_SHOW_MUSIC_INFO;
            getSupportFragmentManager()
                    .beginTransaction()
                    .hide(mMcc201MusicListFragment)
                    .commitAllowingStateLoss();
            getSupportFragmentManager().popBackStack();
            mCurrentFragment = mMcc201MusicInfoFragment;
            return true;
        } else if (mShowGroupType == E_GROUP_SHOW_MUSIC_LIST_EX) {
            mShowGroupType = E_GROUP_SHOW_MUSIC_INFO_EX;
            getSupportFragmentManager()
                    .beginTransaction()
                    .hide(mMusicListFragmentEx)
                    .commitAllowingStateLoss();
            getSupportFragmentManager().popBackStack();
            mCurrentFragment = mMusicInfoFragmentEx;
            return true;
        } else if (mShowGroupType == E_GROUP_SHOW_MUSIC_SEARCH_EX) {
            mShowGroupType = E_GROUP_SHOW_MUSIC_LIST_EX;
            getSupportFragmentManager()
                    .beginTransaction()
                    .hide(mMusicSearchFragmentEx)
                    .commitAllowingStateLoss();
            getSupportFragmentManager().popBackStack();
            mCurrentFragment = mMusicListFragmentEx;
            return true;
        } else if (mShowGroupType == MCC204_E_GROUP_SHOW_MUSIC_LIST) {
            mShowGroupType = MCC204_E_GROUP_SHOW_MUSIC_INFO;
            getSupportFragmentManager()
                    .beginTransaction()
                    .hide(mMusicListFragment)
                    .commitAllowingStateLoss();
            getSupportFragmentManager().popBackStack();
            mCurrentFragment = mMusicInfoFragment;
            return true;
        }

        musicUiFinish(1);
        return false;
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
        // 不在音乐模式状态不处理
        if (!mAppData.isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
            return;
        }

        LogUtil.e(TAG, "onMediaEvent eventId:" + eventId);
        switch (eventId) {
            case IMediaEvent.EVENT_GOTO_MUSIC_INFO_PAGE:
                onShowInfoFragmentEvent();
                return;
            case IMediaEvent.EVENT_GOTO_FILE_LIST_ITEM_PAGE:
                onShowFragmentEvent(E_GROUP_SHOW_MUSIC_FILE_ITEM);
                return;
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
                onChangeMusicItemEvent();
                break;
            case IMediaEvent.MCC201_EVENT_GOTO_MUSIC_LIST:
                onShowFragmentEvent(MCC201_E_GROUP_SHOW_MUSIC_LIST);
                break;
            default:
                break;
        }

        // 分发 Local 媒体事件
        onLocalMediaEvent(eventId, null);
    }

    /**
     * 指定显示 Fragment 页面
     * <pre>
     *    非显示状态下不处理显示逻辑；
     *    FragmentTransaction 是异步的；
     * </pre>
     *
     * @param type fragment 类型
     */
    public void onShowFragmentEvent(int type) {
        LogUtil.d(TAG, "onShowFragmentEvent[" + isResumedEx() + "], "
                + "mShowGroupType = " + mShowGroupType + " type = " + type);

        // [非显示状态]
        if (!isResumedEx()) {
            return;
        }

        // [过滤重复操作]
        if (mShowGroupType == type) {
            return;
        }

        mShowGroupType = type;
        MediaFragment fragment;
        FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();

        switch (type) {
            case E_GROUP_SHOW_LOADING: {
                if (null == mLoadingFragment) {
                    mLoadingFragment = MusicLoadingFragment.newInstance("MusicUI");
                }
                fragment = mLoadingFragment;
                break;
            }
            case E_GROUP_SHOW_MUSIC_INFO: {
                if (null == mMainViewPaperFragment) {
                    mMainViewPaperFragment = new MusicViewPaperFragment();
                }
                fragment = mMainViewPaperFragment;
                break;
            }
            case MCC201_E_GROUP_SHOW_MUSIC_INFO: {
                if (null == mMcc201MusicInfoFragment) {
                    mMcc201MusicInfoFragment = new Mcc201MusicInfoFragment(this);
                }
                fragment = mMcc201MusicInfoFragment;
                break;
            }
            case MCC201_E_GROUP_SHOW_MUSIC_LIST: {
                if (null == mMcc201MusicListFragment) {
                    mMcc201MusicListFragment = new Mcc201MusicListFragment();
                }
                fragment = mMcc201MusicListFragment;
                break;
            }
            case E_GROUP_SHOW_MUSIC_INFO_EX: {
                if (null == mMusicInfoFragmentEx) {
                    mMusicInfoFragmentEx = new MusicInfoFragmentEx(this);
                }
                fragment = mMusicInfoFragmentEx;
                break;
            }
            case E_GROUP_SHOW_MUSIC_LIST_EX: {
                if (null == mMusicListFragmentEx) {
                    mMusicListFragmentEx = new MusicListFragmentEx();
                }
                fragment = mMusicListFragmentEx;
                break;
            }
            case E_GROUP_SHOW_MUSIC_SEARCH_EX: {
                if (null == mMusicSearchFragmentEx) {
                    mMusicSearchFragmentEx = new MusicSearchFragmentEx();
                }
                fragment = mMusicSearchFragmentEx;
                break;
            }
            case MCC204_E_GROUP_SHOW_MUSIC_INFO: {
                if (null == mMusicInfoFragment) {
                    mMusicInfoFragment = new MusicInfoFragment();
                }
                fragment = mMusicInfoFragment;
                break;
            }
            case MCC204_E_GROUP_SHOW_MUSIC_LIST: {
                if (null == mMusicListFragment) {
                    mMusicListFragment = new MusicListFragment();
                }
                fragment = mMusicListFragment;
                break;
            }
            case E_GROUP_SHOW_MUSIC_SEARCH: {
                if (null == mMusicSearchFragment) {
                    mMusicSearchFragment = new MusicSearchFragment();
                }
                fragment = mMusicSearchFragment;
                break;
            }
            default:
                return;
        }

        transaction.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN);

        //  [只有 Fragment 发生改变才需要隐藏当前页面]
        if (mCurrentFragment != fragment) {
            if (mCurrentFragment != null) {
                mCurrentFragment.uninitFragment();
                transaction.hide(mCurrentFragment);
            }
        }

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
            case E_GROUP_SHOW_LOADING:
                tryAddKey = "music-loading";
                break;
            case MCC201_E_GROUP_SHOW_MUSIC_LIST:
                tryAddKey = "mcc201-music-list";
                break;
            case E_GROUP_SHOW_MUSIC_INFO_EX:
                popBackStackFlag = 0;
                tryAddKey = "mcc154-music-info";
                tryPopKey = "mcc154-music-info";
                break;
            case E_GROUP_SHOW_MUSIC_LIST_EX:
                tryAddKey = "mcc154-music-list";
                break;
            case E_GROUP_SHOW_MUSIC_SEARCH_EX:
                tryAddKey = "mcc154-music-search";
                break;
            case MCC204_E_GROUP_SHOW_MUSIC_INFO:
                popBackStackFlag = 0;
                tryAddKey = "mcc204-music-info";
                tryPopKey = "mcc204-music-info";
                break;
            case MCC204_E_GROUP_SHOW_MUSIC_LIST:
                tryAddKey = "mcc204-music-list";
                tryPopKey = "music-search";
                break;
            case E_GROUP_SHOW_MUSIC_SEARCH:
                tryAddKey = "music-search";
                break;
            default:
                break;
        }

        // 需要添加到回退栈中
        if (!TextUtils.isEmpty(tryAddKey)) {
            // 只添加一次，避免循环回退
            if (!inFragmentBackStack(tryAddKey)) {
                transaction.addToBackStack(tryAddKey);
                LogUtil.v(TAG, "addToBackStack: " + tryAddKey);
            }
        }

        transaction.commitAllowingStateLoss();
        mCurrentFragment = fragment;

        // 尝试从回退栈出栈
        if (!TextUtils.isEmpty(tryPopKey)) {
            // StateSaved 状态不可以回退
            boolean isStateSaved = getSupportFragmentManager().isStateSaved();
            if (inFragmentBackStack(tryPopKey) && !isStateSaved) {
                LogUtil.v(TAG, "popBackStack: " + tryPopKey + ", " + popBackStackFlag);
                getSupportFragmentManager().popBackStack(tryPopKey, popBackStackFlag);
            }
        }
    }

    // [打印频率控制]
    private static int sPlaytimeEventCount = 0;

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
        // [打印控制, 播放进度时间 1S 一次, 太过频繁]
        if (IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME == eventId) {
            if (0 == sPlaytimeEventCount % 15) {
                sPlaytimeEventCount = 0;
                LogUtil.d(TAG, "onLocalMediaEvent: eventId = " + eventId);
            }

            sPlaytimeEventCount++;
        } else {
            LogUtil.d(TAG, "onLocalMediaEvent:" +
                    " event = " + eventId + ", showType = " + mShowGroupType);
        }

        // 拦截处理特定的 Local 媒体事件
        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_NO_MUSIC_FILE:
            case IMediaEvent.EVENT_MEDIA_LOADING_FILE:
                onUpdateFragmentEvent();
                break;
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
                if (mShowGroupType != MCC204_E_GROUP_SHOW_MUSIC_LIST) {
                    if (mShowGroupType == E_GROUP_SHOW_MUSIC_SEARCH) {
                        onGoBackShowListPageEvent();
                    } else {
                        onShowInfoFragmentEvent();
                    }
                } else {
                    if (!TextUtils.isEmpty(data)) {
                        if ("clickListItem".equals(data)) {
                            onShowInfoFragmentEvent();
                        }
                    }

                    LogUtil.d(TAG, "EVENT_CHANGE_MUSIC_LIST:" +
                            " <MCC204_E_GROUP_SHOW_MUSIC_LIST == mShowGroupType>, data = " + data);
                }
                break;
            default:
                break;
        }

        dispatchLocalMediaEvent(eventId, data);
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

        if (mMainViewPaperFragment != null) {
            mMainViewPaperFragment.doCallbackEvent(eventId);
        }

        if (mMcc201MusicInfoFragment != null) {
            mMcc201MusicInfoFragment.doCallbackEvent(eventId);
        }

        if (mMcc201MusicListFragment != null) {
            mMcc201MusicListFragment.doCallbackEvent(eventId);
        }

        if (mMusicInfoFragmentEx != null) {
            mMusicInfoFragmentEx.doCallbackEvent(eventId);
        }

        if (mMusicListFragmentEx != null) {
            mMusicListFragmentEx.doCallbackEvent(eventId);
        }

        if (mMusicSearchFragmentEx != null) {
            mMusicSearchFragmentEx.doCallbackEvent(eventId);
        }

        if (mMusicInfoFragment != null) {
            mMusicInfoFragment.doCallbackEvent(eventId);
        }

        if (mMusicListFragment != null) {
            mMusicListFragment.doCallbackEvent(eventId);
        }

        if (mMusicSearchFragment != null) {
            mMusicSearchFragment.doCallbackEvent(eventId);
        }
    }

    /** [显示 Music Info 页面] **/
    private void showMusicInfoFragment() {
        switch (E_THEME_TYPE) {
            case ThemeX.ET_GOD_201:
                onShowFragmentEvent(MCC201_E_GROUP_SHOW_MUSIC_INFO);
                break;
            case ThemeX.ET_GOD_154:
                onShowFragmentEvent(E_GROUP_SHOW_MUSIC_INFO_EX);
                break;
            case ThemeX.ET_GOD_204:
                onShowFragmentEvent(MCC204_E_GROUP_SHOW_MUSIC_INFO);
                break;
            default:
                onShowFragmentEvent(E_GROUP_SHOW_MUSIC_INFO);
                break;
        }
    }

    private void onUpdateFragmentEvent() {
        if (!TextUtils.isEmpty(mAppData.mSingleMusicFilePath)) {
            showMusicInfoFragment();
            return;
        }

        if (mAppData.mCurrentDevice.isLoading()) {
            onShowFragmentEvent(E_GROUP_SHOW_LOADING);
        } else if (mAppData.mCurrentDevice.mMusicInfoList.isEmpty()) {
            onShowFragmentEvent(E_GROUP_SHOW_LOADING);
        } else if (mShowGroupType == E_GROUP_SHOW_LOADING || mShowGroupType == -1) {
            showMusicInfoFragment();
        }
    }

    public void onShowInfoFragmentEvent() {
        if (!TextUtils.isEmpty(mAppData.mSingleMusicFilePath)) {
            showMusicInfoFragment();
            return;
        }

        if (mAppData.mCurrentDevice.isLoading()) {
            onShowFragmentEvent(E_GROUP_SHOW_LOADING);
        } else if (mAppData.mCurrentDevice.mMusicInfoList.isEmpty()) {
            onShowFragmentEvent(E_GROUP_SHOW_LOADING);
        } else {
            showMusicInfoFragment();
        }
    }

    /**
     * 回退显示播放列表事件
     * <p> 只有 mShowGroupType == E_GROUP_SHOW_MUSIC_SEARCH 时候才可以调用该接口
     */
    private void onGoBackShowListPageEvent() {
        switch (E_THEME_TYPE) {
            case ThemeX.ET_GOD_152:
            case ThemeX.ET_GOD_202:
            case ThemeX.ET_GOD_204: {
                // 如果当前设备在加载中
                if (mAppData.mCurrentDevice.isLoading()) {
                    onShowFragmentEvent(E_GROUP_SHOW_LOADING);
                    break;
                }

                // 如果当前设备没有音乐文件
                if (mAppData.mCurrentDevice.mMusicInfoList.isEmpty()) {
                    onShowFragmentEvent(E_GROUP_SHOW_LOADING);
                    break;
                }

                // 是否回播放界面，还是列表界面；
                switch (Argument.E_THEME_GOD) {
                    case ThemeX.ET_GOD_152:
                    case ThemeX.ET_GOD_202:
                    case ThemeX.ET_GOD_400:
                    case ThemeX.ET_GOD_501:
                    case ThemeX.ET_GOD_600:
                        // 直接跳转回播放界面
                        showMusicInfoFragment();
                        break;
                    default:
                        // 列表界面:操作同步+搜索按键
                        onShowFragmentEvent(MCC204_E_GROUP_SHOW_MUSIC_LIST);
                        break;
                }
                break;
            }
            case ThemeX.ET_GOD_110:
            case ThemeX.ET_GOD_300:
            case ThemeX.ET_GOD_402: {
                // mcc402(竖屏) 处理分支
                // mcc402 的页面是 ViewPager 管理的，但是又有几个 Fragment 结合。
                onShowFragmentEvent(E_GROUP_SHOW_MUSIC_INFO);
                break;
            }
            default:
                break;
        }
    }

    /**
     * 这个接口淘汰了
     * <pre>
     *    处理 Fragment 上报来的事件；
     *    不再有 onMediaEvent(IMediaEvent.EVENT_CHANGE_MUSIC_ITEM...) 事件回调；
     * </pre>
     *
     * @deprecated
     */
    @Deprecated
    private void onChangeMusicItemEvent() {
        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            MusicInfo info = mAppData.musicPlayPositionInfo();
            mViewModel.playerRelay().accept(t -> t.requestPlayTarget(info));
        }
    }
}
