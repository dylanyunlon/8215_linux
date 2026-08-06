package com.hcn.media.base.fragment;

import static androidx.lifecycle.ViewModelProvider.*;

import android.animation.Animator;
import android.animation.AnimatorInflater;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.Configuration;
import android.content.res.TypedArray;
import android.os.Bundle;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;

import androidx.annotation.AnimRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentTransaction;
import androidx.lifecycle.ViewModelProvider;

import com.hcn.auto_compat.file.MediaUtilsEx;
import com.hcn.common.DiversityConfig;
import com.hcn.common.utils.HHandler;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.media.vm.base.VmCommand;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.ListSceneManager;
import com.hcn.media_model.MediaModel;
import com.hcn.media_model.base.IUiModel;
import com.hcn.media.vm.MusicViewModel;
import com.hcn.media.vm.VideoViewModel;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.media_base.fragment.IMediaType;
import com.hcn.media_base.fragment.IPageEventListener;
import com.hcn.media_base.fragment.PageEvent;
import com.hcn.media_base.impl.MediaEventPostbox;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.skinx.SkinX;
import com.hcn.skinx.extend.SkinExCompatFragment;
import com.orhanobut.logger.Logger;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Locale;
import java.util.Objects;

import io.reactivex.rxjava3.disposables.CompositeDisposable;

/**
 * 媒体页面基类
 * <p> Fragment 公共方法；
 *
 * @author 86158
 */
public abstract class MediaFragment extends SkinExCompatFragment {
    private static final String TAG = MediaFragment.class.getSimpleName();

    /**
     * 页面参数 KEY
     * <p> 构建 Fragment 时候可以传递特定的数据；
     */
    protected static final String PAGE_PARAM_KEY = "page_param_key";

    /**
     * 页面名字前装规则
     * <pre>
     *    音乐相关页面的前缀必须以 MUSIC_FRAGMENT_NAME_PREFIX 开头；
     *    视频相关页面的前缀必须以 VIDEO_FRAGMENT_NAME_PREFIX 开头；
     * </pre>
     */
    protected static final String MUSIC_FRAGMENT_NAME_PREFIX = "music-";
    protected static final String VIDEO_FRAGMENT_NAME_PREFIX = "video-";

    /**
     * 受保护全局对象
     * <p> 开放给子类使用；
     */
    protected Context mContext = null;
    protected AppGlobalData mAppData;

    /**
     * 媒体事件回调接口
     * <p> 子类对外的事件传递接口，建议少用回调，不好用；
     * @deprecated 新需求业务，尽可能少用和不用它；
     */
    @Deprecated
    protected IMediaEventListener mListener = null;

    /**
     * 音视频视图模式对象
     * <p> 在基类中初始化好，避免子类重复初始化；
     */
    protected MusicViewModel mMusicViewModel = null;
    protected VideoViewModel mVideoViewModel = null;

    /**
     * UI 是否在 onResume 状态
     * <p> [配合 onResume/onPause 一起使用]
     */
    private boolean mIsResumed = false;

    /**
     * UI 是否在 onStop 状态
     * <p> [配合 onStart/onStop 一起使用]
     */
    private boolean mIsStopped = false;

    /**
     * 订阅资源管理器
     * <p> 管理当前所有订阅资源，只能回收释放；
     */
    protected CompositeDisposable mCompositeDisposable;

    /**
     * 消息处理器对象
     * <p> 可以用来处理延时任务；
     */
    protected final HHandler H0 = new TaskHandler(Looper.getMainLooper(), this);

    /**
     * 当前 Fragment 的别名
     * <p> 标记名称使用，用来区分不同特定的 Fragment;
     */
    private @NonNull String mFragmentName = "default";
    private @IMediaType int mFragmentMediaType = IMediaType.MEDIA_FRAGMENT;

    /**
     * 当前 Fragment 的显示方向
     * <pre>
     *    这个参数是可选的，需要使用的就去初始化它，不需要的就不要处理；
     *    主要用来协助处理元素更新（视频模块分配到全屏不会重新创建 Activity，部分元素需要手动刷新）
     * </pre>
     */
    private int mFragmentOrientation = Configuration.ORIENTATION_UNDEFINED;

    /**
     * 当前页面的事件接收站点
     * <pre>
     *     提供给当前页面的内容视图投递消息使用；
     *     接收内部事件（e.g. 页面下的 Layout 上报事件）
     * </pre>
     */
    protected final MediaEventPostbox mPostbox = new MediaEventPostbox() {
        @Override
        public void onMediaEvent(int eventId, Object wParam, Object lParam) {
            onPostboxMediaEvent(eventId, wParam, lParam);
        }

        @Override
        public void onMediaAction(@NonNull final String action, Object wParam, Object lParam) {
            requestMediaAction(action, wParam, lParam);
        }
    };

    /**
     * 页面事件回调监听者
     * <pre>
     *    通常结合 ViewModel 用于 Fragment 之间传递事件；
     *    子类实现后，可以用于其它 Fragment 传递事件给子类；
     * </pre>
     */
    protected final IPageEventListener mPageEventListener = this::onHandlePageEvent;

    /** UI 显示差异配置 */
    protected boolean mIsKeepPlayingAnim = false;
    protected boolean mIsKeepRepeatMode = false;
    protected boolean mIsSupportListMemory = false;

    /** MediaFragment 构造函数 */
    public MediaFragment(@NonNull String name) {
        // 检查当前 Fragment 页面的名字
        if (!TextUtils.isEmpty(name)) {
            mFragmentName = name;

            // 从名字映射出 Fragment 媒体类型
            if (name.startsWith(MUSIC_FRAGMENT_NAME_PREFIX)) {
                mFragmentMediaType = IMediaType.MUSIC_FRAGMENT;
            } else {
                if (name.startsWith(VIDEO_FRAGMENT_NAME_PREFIX)) {
                    mFragmentMediaType = IMediaType.VIDEO_FRAGMENT;
                }
            }
        }

        // 全局数据对象（历史遗留，已无法替换）
        mAppData = AppGlobalData.getInstance();

        // 设置列表记忆模式是否启用
        ListSceneManager.getInstance().setSupportListMemory(mIsSupportListMemory
                || DiversityConfig.MODE_FOLDER.equals(DiversityConfig.getListMemoryMode()));
    }

    /**
     * 获取当前 Fragment 的别买
     * <p> 一般用于辅助调试使用，或者用于特殊处理判断条件；
     *
     * @return 页面名称
     */
    public String fragmentName() {
        return mFragmentName;
    }

    /**
     * 是否是指定的 Fragment 媒体类型
     * <pre>
     *    这个函数是为了避免每次都通过 {@link #mFragmentName} 来区分页面类型;
     *    毕竟通过字符串比较判断类型效率比较低；
     * </pre>
     *
     * @param type {@link IMediaType}
     *
     * @return 是/否
     */
    public boolean isFragmentMediaType(@IMediaType int type) {
        return mFragmentMediaType == type;
    }

    /**
     * 是 onResume 状态
     * <pre>
     *    注意：函数名称 isResumed() 已经在 Activity 中存在；
     *    当前 Fragment 是否已经执行 super.onResume() 函数；
     * </pre>
     * @return 是/否
     */
    protected boolean isResumedEx() {
        return mIsResumed;
    }

    /**
     * 是 onStop 状态
     * <p> 当前 Fragment 是否已经执行 super.onStop() 函数；
     * @return 是/否
     */
    protected boolean isStopped() {
        return mIsStopped;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mCompositeDisposable = new CompositeDisposable();
        mFragmentOrientation = getResources().getConfiguration().orientation;

        // UI 差异化配置信息
        mIsKeepPlayingAnim = SkinX.getBoolean("is_keep_playing_anim", false);
        mIsKeepRepeatMode = SkinX.getBoolean("is_keep_repeat_mode", false);
        mIsSupportListMemory = SkinX.getBoolean("is_support_list_memory", false);
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);

        // 当前页面上下文
        mContext = context;

        // [MusicViewModel]
        if (isFragmentMediaType(IMediaType.MUSIC_FRAGMENT)) {
            mMusicViewModel = new ViewModelProvider(requireActivity(),
                    (Factory) new AndroidViewModelFactory(
                            requireActivity().getApplication()))
                    .get(MusicViewModel.class);
        } else {
            // [VideoViewModel]
            if (isFragmentMediaType(IMediaType.VIDEO_FRAGMENT)) {
                mVideoViewModel = new ViewModelProvider(requireActivity(),
                        (Factory) new AndroidViewModelFactory(
                                requireActivity().getApplication()))
                        .get(VideoViewModel.class);
            }
        }
    }

    /** 获取 UiModel 对象 **/
    protected IUiModel requestUiModel() {
        return MediaModel.call().uiModel();
    }

    /**
     * 判断当前 Fragment 显示方向配置
     * <p> {@link #onConfigurationChanged(Configuration)}
     *
     * @param orientation 期望的方向配置
     * @return 是/否
     */
    public boolean isOrientation(int orientation) {
        return mFragmentOrientation == orientation;
    }

    /**
     * 当前是横屏设备竖屏显示状态
     * <p> 横屏设备进入分屏状态，且分屏显示状态是竖屏状态；
     *
     * @return 是/否
     */
    public boolean isHorizontalDevicePortraitShow() {
        boolean isMultiWindowMode = requireActivity().isInMultiWindowMode();
        boolean horizontalScreen = MiscUtils.isHorizontalScreenDevice(requireContext());
        return  horizontalScreen
                && isMultiWindowMode
                && isOrientation(Configuration.ORIENTATION_PORTRAIT);
    }

    /**
     * 当前是竖屏设备横屏显示状态
     * <p> 竖屏设备进入分屏状态，且分屏显示状态是横屏状态；
     *
     * @return 是/否
     */
    public boolean isPortraitDeviceHorizontalShow() {
        boolean isMultiWindowMode = requireActivity().isInMultiWindowMode();
        boolean portraitScreen = !MiscUtils.isHorizontalScreenDevice(requireContext());
        return  portraitScreen
                && isMultiWindowMode
                && isOrientation(Configuration.ORIENTATION_LANDSCAPE);
    }

    /**
     * 历史遗留的初始化接口
     * <p> 非安全接口，权限过大，使用不受控，被滥用了；
     *
     * @deprecated 不再维护，少用；
     */
    @Deprecated
    public void initFragment() {
        initFragment(false);
    }

    /**
     * 初始化 Fragment 组件时调用
     * <p> 历史遗留接口，后续尽可能少用吧；
     *
     * @param resume 是否由 onResume() 触发调用
     * @deprecated 不再维护，少用；
     */
    @Deprecated
    public void initFragment(boolean resume) {
        // TODO Auto-generated method stub
    }

    /**
     * 历史遗留的反初始化接口
     * <p> 非安全接口，权限过大，使用不受控，被滥用了；
     *
     * @deprecated 不再维护，少用；
     */
    @Deprecated
    public void uninitFragment() {
        uninitFragment(false);
    }

    /**
     * 反初始化 Fragment 组件时调用
     * <p> 历史遗留接口，后续尽可能少用吧；
     *
     * @param pause 是否由 onPause() 触发调用
     * @deprecated 不再维护，少用；
     */
    @Deprecated
    public void uninitFragment(boolean pause) {
        // TODO Auto-generated method stub
    }

    /** 是否是 USB 挂载状态 **/
    protected boolean isUsbMounted() {
        if (isFragmentMediaType(IMediaType.MUSIC_FRAGMENT)) {
            final boolean[] isUsbMounted = {false};
            mMusicViewModel.playerRelay().accept(
                    t -> isUsbMounted[0] = t.requestQueryState(
                            IMediaAction.isUsbMounted, null));
            return isUsbMounted[0];
        } else {
            if (isFragmentMediaType(IMediaType.VIDEO_FRAGMENT)) {
                final boolean[] isUsbMounted = {false};
                mVideoViewModel.playerRelay().accept(
                        t -> isUsbMounted[0] = t.requestQueryState(
                                IMediaAction.isUsbMounted, null));
                return isUsbMounted[0];
            }
        }

        return false;
    }

    /** 是否是 SDCard 挂载状态 **/
    protected boolean isSdcardMounted() {
        if (isFragmentMediaType(IMediaType.MUSIC_FRAGMENT)) {
            final boolean[] isSdcardMounted = {false};
            mMusicViewModel.playerRelay().accept(
                    new VmCommand.Action<BaseViewModel.IPlayer>() {
                        @Override
                        public void exec(BaseViewModel.IPlayer t) {
                            isSdcardMounted[0] = t.requestQueryState(
                                    IMediaAction.isSdcardMounted, null);
                        }
                    });
            return isSdcardMounted[0];
        } else {
            if (isFragmentMediaType(IMediaType.VIDEO_FRAGMENT)) {
                final boolean[] isSdcardMounted = {false};
                mVideoViewModel.playerRelay().accept(
                        t -> isSdcardMounted[0] = t.requestQueryState(
                                IMediaAction.isSdcardMounted, null));
                return isSdcardMounted[0];
            }
        }

        return false;
    }

    /** 是否是 ACC 点火状态 **/
    protected boolean isAccOnState() {
        if (isFragmentMediaType(IMediaType.MUSIC_FRAGMENT)) {
            final boolean[] isAccOnState = {false};
            mMusicViewModel.playerRelay().accept(
                    t -> isAccOnState[0] = t.requestQueryState(
                            IMediaAction.inAccOnState, null));
            return isAccOnState[0];
        } else {
            if (isFragmentMediaType(IMediaType.VIDEO_FRAGMENT)) {
                final boolean[] isAccOnState = {false};
                mVideoViewModel.playerRelay().accept(
                        t -> isAccOnState[0] = t.requestQueryState(
                                IMediaAction.inAccOnState, null));
                return isAccOnState[0];
            }
        }

        return false;
    }

    /**
     * 计算拖动进度条的目标时间与时间差
     * <p> 这个函数名字和参数名取的就很奇怪；
     * @param newTime 为需要 Seek 的目标时间;
     * @param delayTime 为需要 Seek 的时间差;
     */
    protected String[] computeSeekBarDelayTime(int newTime, int delayTime) {
        int time = Math.abs(delayTime);
        String strNewTime;
        String strDelayTime;
        if (newTime  >= 6000) {
            strNewTime = String.format(Locale.getDefault(),
                    "%03d:%02d",  newTime / 60, newTime % 60);
        } else {
            strNewTime = String.format(Locale.getDefault(),
                    "%02d:%02d", newTime / 60 % 100, newTime % 60);
        }

        if (delayTime >= 0) {
            if (delayTime >= 6000) {
                strDelayTime = String.format(Locale.getDefault(),
                        "[+%03d:%02d]", time / 60, time % 60);
            }else {
                strDelayTime = String.format(Locale.getDefault(),
                        "[+%02d:%02d]", time / 60 % 100, time % 60);
            }

        } else {

            if (time >= 6000) {
                strDelayTime = String.format(Locale.getDefault(),
                        "[-%03d:%02d]", time / 60, time % 60);
            }else {
                strDelayTime = String.format(Locale.getDefault(),
                        "[-%02d:%02d]", time / 60 % 100, time % 60);
            }

        }

        return new String[]{strNewTime, strDelayTime};
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater,
                             @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        return super.onCreateView(inflater, container, savedInstanceState);
    }

    /**
     * 可处理视图元素的初始化任务
     * <pre>
     *    请使用这个接口替代 {@link #initFragment()} 接口任务；
     *    子类可以继承这个函数，在此做 findViewById/findViewByName 动作；
     *    它在 {@link #onViewCreated(View, Bundle)} 中被调用；
     * </pre>
     *
     * @param savedInstanceState
     */
    protected void onInitializeElements(@Nullable Bundle savedInstanceState) {
        // TODO: 处理视图元素的初始化工作
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        // 初始化视图元素
        onInitializeElements(savedInstanceState);
        super.onViewCreated(view, savedInstanceState);

        // 连接页面中继器
        connectPageEventRelay();
    }

    /**
     * 处理外部下发的事件
     * <pre>
     *     Activity 给 Fragment 下发事件；
     *     过时的接口，设计太狗屎，后续不再维护；
     * </pre>
     *
     * @param eventId {@link IMediaEvent}
     * @deprecated 建议使用 {@link #onHandlePageEvent(int, Object, Object)} 替换；
     */
    @Deprecated
    public void doCallbackEvent(int eventId) {
        doCallbackEvent(eventId, -1, -1);
    }

    /**
     * 处理外部下发的事件
     * <p> Activity 给 Fragment 下发事件；
     *
     * @param eventId {@link IMediaEvent}
     * @param arg1 附加参数对象 1
     * @param arg2 附加参数对象 2
     * @deprecated 建议使用 {@link #onHandlePageEvent(int, Object, Object)} 替换；
     */
    @Deprecated
    public void doCallbackEvent(int eventId, int arg1, int arg2) {
        // TODO Auto-generated method stub
    }

    public void onKeyEventMessage(int keyCode, int keyStatus, int keySrc) {
        // TODO Auto-generated method stub
    }

    public void onExternalEvent(String event, int arg1, int arg2) {
        // TODO Auto-generated method stub
    }

    /**
     * 媒体事件监听
     * <pre>
     *    受保护的成员，禁止外部直接调用它；
     *    用来接收组件内的子元素投递的媒体事件；
     *    参见 {@link IMediaEventListener} 接口；
     * </pre>
     *
     * @param eventId 事件 ID
     * @param wParam  附加参数 1
     * @param lParam  附加阐述 2
     * @see MediaEventPostbox
     */
    protected abstract void onPostboxMediaEvent(int eventId, Object wParam, Object lParam);

    /** @see #requestMediaAction(String, Object) **/
    protected void requestMediaAction(@NonNull final String action) {
        requestMediaAction(action, null);
    }

    /** @see #requestMediaAction(String, Object, Object) **/
    protected void requestMediaAction(@NonNull final String action, Object wParam) {
        requestMediaAction(action, wParam, null);
    }

    /**
     * 请求执行媒体活动
     * <pre>
     *    一般用来执行媒体播放相关的接口动作，做代码层次隔离；
     *    主要是为了绕开直接使用 {@link android.app.Application} 对象访问接口的问题；
     * </pre>
     *
     * @param action 动作名称
     * @param wParam 附加参数 1
     * @param lParam 附加参数 2
     */
    @SuppressWarnings("unchecked")
    protected void requestMediaAction(@NonNull final String action, Object wParam, Object lParam) {
        if (Objects.isNull(mContext)
                || Objects.isNull(mMusicViewModel)) {
            return;
        }

        switch (action) {
            case IMediaAction.playControl:
                mMusicViewModel.playerRelay().accept(
                        t -> t.requestPlayControl((Integer) wParam));
                break;
            case IMediaAction.requestPlayMusicInfo:
                mMusicViewModel.playerRelay().accept(
                        t -> t.requestPlayTarget(IPlaylistType.DEVICE_LIST,
                                (List<MusicInfo>) wParam, (Integer) lParam));
                break;
            case IMediaAction.seekToTime:
            case IMediaAction.setSeekTimeZero:
            case IMediaAction.scanStorageDeviceInfo:
            case IMediaAction.switchPlayRepeatMode:
                mMusicViewModel.playerRelay().accept(
                        t -> t.requestExecuteAction(action, wParam, lParam));
                break;
            default:
                break;
        }
    }

    /**
     * 连接页面事件中继器
     * <pre>
     *    请结合 {@link MediaFragment#onHandlePageEvent(int, Object, Object)} 一起使用；
     *    使用方法：
     *      1、子类也可重载本方法，结合 ViewModel 实现页面事件监听；
     *      2、子类重载 onPageEvent 方法处理筛选需要的页面事件；
     *      3、本方法在 {@link androidx.fragment.app.Fragment#onViewCreated(View, Bundle)} 触发调用；
     * </pre>
     */
    protected void connectPageEventRelay() {
        // 订阅音乐相关的 PageEvent 事件
        if (isFragmentMediaType(IMediaType.MUSIC_FRAGMENT)) {
            mCompositeDisposable.add(
                    mMusicViewModel.pageEventRelay().subscribe(
                            listenerAction -> listenerAction.exec(mPageEventListener)));
        } else {
            // 订阅视频相关的 PageEvent 事件
            if (isFragmentMediaType(IMediaType.VIDEO_FRAGMENT)) {
                mCompositeDisposable.add(
                        mVideoViewModel.pageEventRelay().subscribe(
                                listenerAction -> listenerAction.exec(mPageEventListener)));
            }
        }
    }

    /**
     * 需要重载的页面事件接受函数
     * <pre>
     *    请结合 {@link MediaFragment#connectPageEventRelay()} 一起使用；
     *    使用方法：参考 connectPageEventRelay 接口说明；
     *    方法提示：当前页面事件生命周期约束，[onViewCreated, onDestroyView]；
     * </pre>
     *
     * @param event 事件 ID
     * @param obj1 附加数据对象 1
     * @param obj2 附加数据对象 2
     */
    protected void onHandlePageEvent(int event, Object obj1, Object obj2) {
        // TODO: 子类要使用就重载该函数
    }

    /**
     * 页面事件发送函数
     * <pre>
     *    请结合 {@link MediaFragment#onHandlePageEvent(int, Object, Object)} 一起使用；
     *    使用方法：参考 onPageEvent 接口说明；
     *    方法提示：当前页面事件生命周期约束，[onViewCreated, onDestroyView]；
     * </pre>
     *
     * @param event 事件 ID {@link PageEvent}
     * @param obj1 附加数据对象 1
     * @param obj2 附加数据对象 2
     */
    protected void sendPageEvent(int event, Object obj1, Object obj2) {
        // 发送音乐相关的 PageEvent 事件
        if (isFragmentMediaType(IMediaType.MUSIC_FRAGMENT)) {
            mMusicViewModel.pageEventRelay().accept(
                    t -> t.onPageEvent(event, obj1, obj2));
        } else {
            // 发送视频相关的 PageEvent 事件
            if (isFragmentMediaType(IMediaType.VIDEO_FRAGMENT)) {
                mVideoViewModel.pageEventRelay().accept(
                        t -> t.onPageEvent(event, obj1, obj2));
            }
        }
    }

    /**
     * Fragment 显示方向改变事件
     * <pre>
     *    关心方向改变事件的可以重载该函数；
     *    Caller：{@link  #onConfigurationChanged(Configuration)}
     * </pre>
     *
     * @param newConfig 当前配置
     */
    protected void onOrientationChangedEvent(@NonNull Configuration newConfig) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        // 显示方向改变
        if (mFragmentOrientation != newConfig.orientation) {
            mFragmentOrientation = newConfig.orientation;
            onOrientationChangedEvent(newConfig);
        }
    }

    /** 当前页面消息定义 **/
    protected interface H {
        int MSG_NONE = -1;
        int MSG_VIEW_CLICKED_FILTER = 0;

        // 定义阈值
        int MSG_BASE_THRESHOLD = 10;
    }

    /**
     * 实现该接口可以处理自定义消息
     *
     * @param msg 消息对象
     */
    protected void onHandleMessage(@NonNull Message msg) {
        // TODO Auto-generated method stub
    }

    /**
     * 是否允许视图执行点击事件
     * <p> 只有允许执行点击事件，禁用点击的时间才会生效；
     *
     * @param delayMillis 禁用点击事件时间
     * @return 允许/不允许
     */
    protected boolean allowExecuteClickEvent(int delayMillis) {
        if (H0.hasMessages(H.MSG_VIEW_CLICKED_FILTER)) {
            LogUtil.v(TAG, "Not allow execute click event.");
            return false;
        }

        H0.sendEmptyUniqueMessageDelayed(
                H.MSG_VIEW_CLICKED_FILTER, delayMillis);
        return true;
    }

    @AnimRes
    private static int toActivityTransitResId(@NonNull Context context, int attrInt) {
        int resId;
        TypedArray typedArray = context.obtainStyledAttributes(
                android.R.style.Animation_Activity, new int[]{attrInt});
        resId = typedArray.getResourceId(0, View.NO_ID);
        typedArray.recycle();
        return resId;
    }

    @SuppressLint({"PrivateResource", "ResourceType"})
    @Nullable
    @Override
    public Animator onCreateAnimator(int transit, boolean enter, int nextAnim) {
        Logger.t(TAG).d("onCreateAnimation["  + mFragmentName + "]: "
                + "transit = " + transit + ", enter = " + enter + ", nextAnim = " + nextAnim);

        switch (transit) {
            case FragmentTransaction.TRANSIT_FRAGMENT_OPEN:
                if (nextAnim == 0) {
                    nextAnim = enter ?
                            androidx.fragment.R.animator.fragment_open_enter :
                            androidx.fragment.R.animator.fragment_open_exit;
                }
                break;
            case FragmentTransaction.TRANSIT_FRAGMENT_CLOSE:
                if (nextAnim == 0) {
                    nextAnim = enter ?
                            androidx.fragment.R.animator.fragment_close_enter :
                            androidx.fragment.R.animator.fragment_close_exit;
                }
                break;
            case FragmentTransaction.TRANSIT_FRAGMENT_FADE:
                if (nextAnim == 0) {
                    nextAnim = enter ?
                            androidx.fragment.R.animator.fragment_fade_enter :
                            androidx.fragment.R.animator.fragment_fade_exit;
                }
                break;
            case FragmentTransaction.TRANSIT_FRAGMENT_MATCH_ACTIVITY_OPEN:
                if (nextAnim == 0) {
                    nextAnim = enter
                            ? toActivityTransitResId(mContext, android.R.attr.activityOpenEnterAnimation)
                            : toActivityTransitResId(mContext, android.R.attr.activityOpenExitAnimation);
                }
                break;
            case FragmentTransaction.TRANSIT_FRAGMENT_MATCH_ACTIVITY_CLOSE:
                if (nextAnim == 0) {
                    nextAnim = enter
                            ? toActivityTransitResId(mContext, android.R.attr.activityCloseEnterAnimation)
                            : toActivityTransitResId(mContext, android.R.attr.activityCloseExitAnimation);
                }
                break;
            default:
                break;
        }

        if (nextAnim != 0) {
            Animator animator;
            String dir = getResources().getResourceTypeName(nextAnim);
            boolean isAnimator = "animator".equals(dir);
            if (isAnimator) {
                try {
                    animator = AnimatorInflater.loadAnimator(mContext, nextAnim);
                    if (animator != null) {
                        return animator;
                    }
                } catch (RuntimeException e) {
                    Logger.t(TAG).e("onCreateAnimator/loadAnimator Exception: " + e);
                }
            }
        }

        return super.onCreateAnimator(transit, enter, nextAnim);
    }

    @Nullable
    @Override
    public Animation onCreateAnimation(int transit, boolean enter, int nextAnim) {
        if (nextAnim != 0) {
            Animation animation;
            String dir = mContext.getResources().getResourceTypeName(nextAnim);
            boolean isAnim = "anim".equals(dir);
            if (isAnim) {
                try {
                    animation = AnimationUtils.loadAnimation(mContext, nextAnim);
                    if (animation != null) {
                        return animation;
                    }
                } catch (RuntimeException e) {
                    Logger.t(TAG).e("onCreateAnimation/loadAnimation Exception: " + e);
                }
            }
        }

        return super.onCreateAnimation(transit, enter, nextAnim);
    }

    @Override
    public void onStart() {
        super.onStart();
        mIsStopped = false;
    }

    @Override
    public void onResume() {
        super.onResume();
        mIsResumed = true;

        // 更新多语言相关的文本显示
        onUpdateLanguageSkinText();
    }

    @Override
    public void onPause() {
        super.onPause();
        mIsResumed = false;
    }

    @Override
    public void onStop() {
        super.onStop();
        mIsStopped = true;
    }

    /**
     * 请求执行目标类型方法
     * <pre>
     *    由外部请求触发执行，具体由有需求的子类去实现；
     *    返回 {@link MediaUtilsEx#UNSUPPORTED} 表示请求执行的方法类型不支持
     * </pre>
     *
     * @param method 方法类型
     * @param objects 参数集
     * @return 执行结果 {根据实际情况约定}
     */
    @Override
    protected Object requestExecuteMethod_Impl(String method, Object... objects) {
        return MediaUtilsEx.UNSUPPORTED;
    }

    /**
     * 统一更新语言变化相关的文本控件
     * <pre>
     *    子类可以继承它处理文本空间信息刷新；
     *    由于扩展皮肤包中的文本控件相关的多语言资源不可能齐全，所以需要强制刷新；
     * </pre>
     * @see #onResume()
     */
    protected void onUpdateLanguageSkinText() {
        // TODO: 统一更新和语言相关的文字信息；
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();

        // 取消订阅事件
        mCompositeDisposable.clear();

        // 移除所有消息
        H0.removeCallbacksAndMessages(null);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        // 释放容器资源
        mCompositeDisposable.dispose();
    }

    /**
     * 主消息处理器
     * <p> 消息处理器，对所有继承 MediaFragment 扩展使用；
     */
    protected static final class TaskHandler extends HHandler {
        /** 当前所有者引用 **/
        private final Reference<MediaFragment> mOwnerRef;

        public TaskHandler(@NonNull Looper looper, MediaFragment owner) {
            super(looper);
            mOwnerRef = new WeakReference<>(owner);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);

            MediaFragment owner = mOwnerRef.get();
            if (owner != null) {
                owner.onHandleMessage(msg);
            }
        }
    }
}
