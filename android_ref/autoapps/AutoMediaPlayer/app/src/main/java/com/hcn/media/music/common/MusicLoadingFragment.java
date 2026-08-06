package com.hcn.media.music.common;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.LinearInterpolator;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;

import com.google.common.eventbus.Subscribe;
import com.hcn.AutoMediaPlayer.R;
import com.hcn.config.Feature;
import com.hcn.media.R3;
import com.hcn.media_base.IAutoEvent;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.HEventBus;
import com.hcn.media_common.HMessage;
import com.hcn.media_data.storage.StorageDeviceEx;
import com.hcn.media.vm.action.IMediaAction;

import java.util.Objects;

/**
 * 音乐加载页
 * @author 65821
 */
public class MusicLoadingFragment extends MediaFragment implements OnClickListener {
    private static final String FRAGMENT_NAME = "music-loading";
    private static final String TAG = MusicLoadingFragment.class.getSimpleName();

    private boolean mInitView = false;

    private TextView mTipView = null;
    private ImageView mLoadingView = null;
    private Animation mRotateAnim = null;

    private UiShowType mCurrUIShowType = UiShowType.UIT_NONE;

    /**
     * 显示类型
     * <p> 简单标记当前加载页面的显示元素；
     */
    enum UiShowType {
        // 默认
        UIT_NONE,

        // 加载中...
        UIT_LOADING,

        // 没有文件提示
        UIT_NO_FILE,
    }


    /**
     * 当前 Fragment 实例化接口
     * <p> [谷歌建议 Fragment 不要创建带参构造函数]
     *
     * @param parentName 父级名称
     * @return {@link MusicLoadingFragment} 对象
     */
    public static MusicLoadingFragment newInstance(String parentName) {
        MusicLoadingFragment f = new MusicLoadingFragment();

        // [启动参数传递]
        Bundle bdl = new Bundle(1);
        bdl.putString("Parent", parentName);
        f.setArguments(bdl);
        return f;
    }

    public MusicLoadingFragment() {
        super(FRAGMENT_NAME);
    }

    /**
     * [Handler 任务处理器, 处理延时和检查任务]
     */
    private final TaskHandler mTaskHandler = new TaskHandler(Looper.getMainLooper());

    @SuppressLint("HandlerLeak")
    private class TaskHandler extends Handler {
        public static final int MSG_NONE = -1;
        public static final int MSG_NO_FILE_TIMEOUT = 1;

        public TaskHandler(@NonNull Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            switch (msg.what) {
                case MSG_NO_FILE_TIMEOUT: {
                    if (getActivity() != null) {
                        // [ACC-OFF 状态下不处理超时退出]
                        if (!isAccOnState()) {
                            return;
                        }

                        // [多窗口模式可以不用退出当前 Activity]
                        if (getActivity().isInMultiWindowMode()) {
                            return;
                        }

                        LogUtil.i(TAG, " -- MSG_NO_FILE_TIMEOUT: finish!");
                        getActivity().finish();
                    }
                    break;
                }

                case MSG_NONE:
                default:
                    break;
            }
        }

        public void enterNoFileTimeout(boolean accOnTriggered) {
            // [ACC OFF 状态不要发送超时退出消息]
            if (!isAccOnState()) {
                removeMessages(MSG_NO_FILE_TIMEOUT);
                return;
            }

            if (!hasMessages(MSG_NO_FILE_TIMEOUT) && Feature.instance().hasFeature(Feature.BIT.SUPPORT_NO_FILE_TIMEOUT_EXIT)) {
                int delayMillis = accOnTriggered? 60 * 1000: 30 * 1000;
                sendEmptyMessageDelayed(MSG_NO_FILE_TIMEOUT, delayMillis);
            }
        }

        public void exitNoFileTimeout() {
            removeMessages(MSG_NO_FILE_TIMEOUT);
        }
    }

    // [设置当前 UI 显示类型]
    private void updateUIShowType(UiShowType type) {
        updateUIShowType(type, false);
    }

    private void updateUIShowType(UiShowType type, boolean accOnTriggered) {
        mCurrUIShowType = type;

        switch (type) {
            case UIT_NO_FILE:
                if (isVisible() && isResumed()) {
                    mTaskHandler.enterNoFileTimeout(accOnTriggered);
                }
                break;

            case UIT_LOADING:
            case UIT_NONE:
                mTaskHandler.exitNoFileTimeout();
                break;

            default:
                break;
        }
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);

        // [读取启动参数]
        Bundle bundle = getArguments();
        if (bundle != null) {
            String name = bundle.getString("Parent");
            LogUtil.i(TAG, ">>> onAttach: name = " + name);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        HEventBus.register(this);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fragment_loading;
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        LogUtil.i(TAG, ">>> onCreateView.");

        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);
        assert view != null;
        initView(view);
        return view;
    }

    private void initView(View layout) {
        if (mInitView) {
            return;
        }

        mInitView = true;
        initRotateAnimation();

        mTipView = layout.findViewById(xId(R.id.tv_main_tips));
        mLoadingView = layout.findViewById(xId(R.id.iv_loading));

        View refreshView = layout.findViewById(xId(R.id.lin_refresh));
        if (refreshView != null) {
            refreshView.setOnClickListener(this);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        LogUtil.i(TAG, ">>> onResume");

        initFragment();
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    @Override
    public void onPause() {
        super.onPause();
        LogUtil.i(TAG, ">>> onPause");

        uninitFragment();
    }

    @Override
    public void onStop() {
        super.onStop();

        LogUtil.i(TAG, ">>> onStop");
    }

    @Override
    public void onSaveInstanceState(@NonNull Bundle outState) {
        LogUtil.i(TAG, ">>> onSaveInstanceState().");

        // [如果 Activity 屏蔽了 onSaveInstanceState(Bundle outState), 就不会调用到这里]
        super.onSaveInstanceState(outState);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LogUtil.i(TAG, ">>> onDestroyView.");

        mInitView = false;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        LogUtil.i(TAG, ">>> onDestroy.");

        mTaskHandler.removeCallbacksAndMessages(null);
        HEventBus.unregister(this);
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        LogUtil.i(TAG, ">>> onHiddenChanged: " + hidden);

        if (hidden) {
            mTaskHandler.exitNoFileTimeout();
        } else {
            updateUIShowType(mCurrUIShowType);
        }
    }

    @Override
    public void initFragment() {
        updateFragmentUiInfo();
    }

    @Override
    public void uninitFragment() {
        mTaskHandler.exitNoFileTimeout();
    }

    private void updateFragmentUiInfo() {
        if (!mInitView) {
            return;
        }

        if (mAppData.mCurrentDevice.isLoading()) {
            mTipView.setText(xString(R3.string.tip_loading));
            mTipView.setVisibility(View.VISIBLE);
            updateUIShowType(UiShowType.UIT_LOADING);

            onStartAnimation(false);
        } else if (mAppData.mCurrentDevice.mMusicInfoList.isEmpty()) {
            mTipView.setText(
                   xString(R3.string.tip_refresh_music_info));
            mTipView.setVisibility(View.VISIBLE);
            updateUIShowType(UiShowType.UIT_NO_FILE);

            onStartAnimation(false);
        } else {
            mTipView.setText("");
            mTipView.setVisibility(View.INVISIBLE);
            updateUIShowType(UiShowType.UIT_NONE);

            onStartAnimation(false);
        }
    }

    private void onRefreshEvent() {
        mTipView.setText(xString(R3.string.tip_loading));
        mTipView.setVisibility(View.VISIBLE);
        updateUIShowType(UiShowType.UIT_LOADING);

        onStartAnimation(true);

        // 检索所有存储设备并刷新存储信息
        for (int i = 0; i < mAppData.mStorageDeviceList.size(); i++) {
            StorageDeviceEx deviceEx = mAppData.mStorageDeviceList.get(i);
            if (Objects.isNull(deviceEx)) {
                continue;
            }

            // 刷新存储设备路径
            String filePath = deviceEx.mFilePath;
            mMusicViewModel.playerRelay().accept(
                    t -> t.requestExecuteAction(
                            IMediaAction.scanStorageDeviceInfo, filePath, null));
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (getId(v)) {
            case R.id.lin_refresh:
                onRefreshEvent();
                break;

            default:
                break;
        }
    }

    private void initRotateAnimation() {
        // 使用本地公共的动画资源
        mRotateAnim = AnimationUtils.loadAnimation(mContext, R.anim.anim_rotate);
        LinearInterpolator lin = new LinearInterpolator();
        mRotateAnim.setInterpolator(lin);
    }

    private void onStartAnimation(boolean bStart) {
        if (mLoadingView == null) {
            return;
        }

        if (bStart) {
            mLoadingView.startAnimation(mRotateAnim);
            mLoadingView.setVisibility(View.VISIBLE);
        } else {
            mLoadingView.clearAnimation();
            mLoadingView.setVisibility(View.INVISIBLE);
        }
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        if (IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME != eventId) {
            LogUtil.low_i(TAG, "eventId: " + eventId);
        }

        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_NO_MUSIC_FILE:
            case IMediaEvent.EVENT_MEDIA_LOADING_FILE:
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
                updateFragmentUiInfo();
                break;

            default:
                break;
        }
    }

    @Subscribe
    public void onEventMainThread(HMessage message) {
        LogUtil.low_i(TAG, "onEventMainThread: " + message.what);

        switch (message.what) {
            case IAutoEvent.EVENT_ACC_STATUS_OFF:
                // 进入 ACC-OFF 状态中断超时退出定时器
                mTaskHandler.exitNoFileTimeout();
                break;

            case IAutoEvent.EVENT_ACC_STATUS_ON:
                // 进入 ACC-ON 状态恢复上一次 UI 状态
                updateUIShowType(mCurrUIShowType, true);
                break;

            default:
                break;
        }
    }
}
