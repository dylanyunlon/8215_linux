package com.hcn.media.video.common;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.LinearInterpolator;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.auto_compat.PlatformUtils;
import com.hcn.auto_compat.app.WindowConfiguration;
import com.hcn.media.R3;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_data.storage.StorageDeviceEx;
import com.hcn.media.vm.action.IMediaAction;

import java.util.Objects;

/**
 * 视频模块加载页
 * <p> 需要考虑小窗口（WINDOWING_MODE_FREEFORM）显示状态；
 *
 * @author 86158
 */
public class VideoLoadingFragment extends MediaFragment
        implements OnClickListener {

    private final static String FRAGMENT_NAME = "video-loading";
    private static final String TAG = VideoLoadingFragment.class.getSimpleName();

    private boolean mInitView = false;
    private View mVideoLoadingLayout = null;

    private TextView mPromptView = null;
    private ImageView mLoadingView = null;
    private Animation mRotateAnim = null;

    public VideoLoadingFragment() {
        super(FRAGMENT_NAME);
    }

    @Override
    public void initFragment() {
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_NO_MUSIC_FILE:
            case IMediaEvent.EVENT_MEDIA_LOADING_FILE:
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
                updateFragment();
                break;

            default:
                break;
        }
    }

    @Override
    public void onAttach(@NonNull Activity context) {
        super.onAttach(context);

        mContext = context;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fragment_videoloading;
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater,
                             @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);

        assert view != null;
        initView(view);
        return view;
    }

    private void initView(@NonNull View layout) {
        if (mInitView) {
            return;
        }

        mInitView = true;

        // 当前页面视图元素
        mVideoLoadingLayout = layout.findViewById(xId(R.id.llVideoLoading));
        mPromptView = layout.findViewById(xId(R.id.tv_main_tips));
        mLoadingView = layout.findViewById(xId(R.id.iv_loading));

        // 刷新点击按钮视图
        View refreshBtn = layout.findViewById(xId(R.id.lin_refresh));
        if (refreshBtn != null) {
            refreshBtn.setOnClickListener(this);
        }

        initAnimation();
        adjustLayoutByStatusBar();
    }

    /**
     * 依据状态栏调整局部参数
     * <pre>
     *    不同的主题需要调整的位置不一样；
     *    这里主要调整需要配置 android:paddingTop="@*android:dimen/status_bar_height" 的元素；
     *    原因，AndroidStudio 编译的 apk 运行时系统不认识这个常量；
     * </pre>
     */
    private void adjustLayoutByStatusBar() {
        // 使用了显示过扫描配置，不需要预留状态栏高度
        if (PlatformUtils.isDisplayOverscanning()) {
            return;
        }

        if (mVideoLoadingLayout != null) {
            int statusBarHeight = MiscUtils.statusBarHeight(mContext, R.dimen.status_bar_height);
            mVideoLoadingLayout.setPadding(
                    0, statusBarHeight, 0, 0);
        }
    }

    /**
     * 点击刷新事件
     * <p> R.id.lin_refresh
     */
    private void onRefreshEvent() {
        // 使用当前程序包的文本资源
        mPromptView.setText(xString(R3.string.tip_loading));
        mPromptView.setVisibility(View.VISIBLE);

        // 启动加载动画(Loading...)
        startLoadAnimation(true);

        // 重新扫描已存在的存储设备
        for (int i = 0; i < mAppData.mStorageDeviceList.size(); i++) {
            StorageDeviceEx deviceEx = mAppData.mStorageDeviceList.get(i);
            if (Objects.isNull(deviceEx)) {
                continue;
            }

            // 扫描指定路径的存储设备多媒体信息
            String filePath = mAppData.mStorageDeviceList.get(i).mFilePath;
            mVideoViewModel.playerRelay().accept(
                    t -> t.requestExecuteAction(
                            IMediaAction.scanStorageDeviceInfo, filePath, null));
        }
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (hidden) {
        } else {
            updateFragment();
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        updateFragment();
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (getId(v)) {
            case R.id.lin_refresh:
                onRefreshEvent();
                break;

            case -1:
            default:
                break;
        }
    }

    private void initAnimation() {
        // 兼容扩展主题包的加载动画方式
        mRotateAnim = xAnimation(R.anim.anim_rotate);
        if (mRotateAnim != null) {
            mRotateAnim.setInterpolator(new LinearInterpolator());
        }
    }

    /**
     * 开始停止加载动画（Image Animation）
     * @param start 开始停止状态
     */
    private void startLoadAnimation(boolean start) {
        // 小窗口模式不处理动画
        if (requestUiModel().isVideoWindowingMode(
                WindowConfiguration.WINDOWING_MODE_FREEFORM)) {
            mLoadingView.clearAnimation();
            mLoadingView.setVisibility(View.GONE);
            return;
        }

        if (start) {
            if (!Objects.isNull(mRotateAnim)) {
                mLoadingView.startAnimation(mRotateAnim);
            }

            mLoadingView.setVisibility(View.VISIBLE);
        } else {
            mLoadingView.clearAnimation();
            mLoadingView.setVisibility(View.INVISIBLE);
        }
    }

    /**
     * 更新 Fragment 页面
     * <pre>
     *    为了不出现提示信息显示不全的问题（布局限制导致）；
     *    WINDOWING_MODE_FREEFORM 模式不显示加载动画视图；
     * </pre>
     */
    private void updateFragment() {
        if (mPromptView == null) {
            return;
        }

        if (mAppData.mCurrentDevice.isLoading()) {
            mPromptView.setText(xString(R3.string.tip_loading));
            mPromptView.setVisibility(View.VISIBLE);
            startLoadAnimation(false);
        } else if (mAppData.mCurrentDevice.mVideoInfoList.isEmpty()) {
            mPromptView.setText(xString(R3.string.tip_refresh_video_info));
            mPromptView.setVisibility(View.VISIBLE);
            startLoadAnimation(false);
        } else {
            mPromptView.setText("");
            mPromptView.setVisibility(View.INVISIBLE);
            startLoadAnimation(false);
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mInitView = false;
    }
}
