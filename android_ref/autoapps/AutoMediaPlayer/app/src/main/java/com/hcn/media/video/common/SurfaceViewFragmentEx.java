package com.hcn.media.video.common;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.Nullable;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_model.MediaUtils;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_base.HMediaConfig;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.media_model.player.base.IMediaPlayer;
import com.hcn.media_theme.ThemeEx;

/**
 * 播放显示层
 * <p> 软解码使用；
 * @author 86158
 */
@SuppressLint("ValidFragment")
public class SurfaceViewFragmentEx extends MediaFragment {
    private static final String TAG = SurfaceViewFragmentEx.class.getSimpleName();
    private static final String FRAGMENT_NAME = "video-surface-ex";

    private boolean mInitView = false;
    private boolean mNeedResumeSurfaceView = false;
    private SurfaceView mSurfaceView = null;
    private boolean isSurfaceCreated = false;

    /**
     * Surface 状态回调
     * <p> 监听 SurfaceView 与之关联的 Surface 的创建、改变、销毁；
     */
    private final SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback() {
        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
            LogUtil.i(TAG, "____surfaceChanged: " + w + " x " + h);
        }

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            LogUtil.i(TAG, "____surfaceCreated.");

            Surface surface = holder.getSurface();
            if (null != surface) {
                mAppData.mFrontSurfaceHolderEx = holder;

                mVideoViewModel.playerRelay().accept(
                        t -> t.requestExecuteAction(
                                IMediaAction.updateVitamioSurfaceHolder, null, null));

                if (mAppData.mAllowResumePlay) {
                    mVideoViewModel.playerRelay().accept(
                            BaseViewModel.IPlayer::requestShouldPlayEvent);
                } else if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PAUSE)) {
                    if (mAppData.mPlayTimeInfo.mCurrentTime < mAppData.mPlayTimeInfo.mTotalTime) {
                        // [如果在暂停状态，好像没有必要 SeekToTime, 待确认]
                        mVideoViewModel.playerRelay().accept(
                                t -> t.requestExecuteAction(
                                        IMediaAction.seekToTime,
                                        mAppData.mPlayTimeInfo.mCurrentTime,
                                        null));
                    }
                }
            }

            isSurfaceCreated = true;
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            LogUtil.i(TAG, "____surfaceDestroyed.");

            if (isSurfaceCreated) {
                mAppData.mFrontSurfaceHolderEx = null;

                mVideoViewModel.playerRelay().accept(
                        t -> t.requestExecuteAction(
                                IMediaAction.updateVitamioSurfaceHolder, null, null));

                isSurfaceCreated = false;
            }
        }
    };

    public SurfaceViewFragmentEx(Context context, IMediaEventListener listener) {
        super(FRAGMENT_NAME);
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_SIZE: {
                setVideoSurfaceSize(1);
                break;
            }
            case IMediaEvent.EVENT_VIDEO_SHOW_BLACK_PAGE: {
                if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_STOP)) {
                    resetSurfaceHolder();
                }
                break;
            }
            default:
                break;
        }
    }

    public void setSurfaceViewLayout(int w, int h) {
        if (null == mSurfaceView) {
            return;
        }

        ViewGroup.LayoutParams layoutParams = mSurfaceView.getLayoutParams();
        if (layoutParams.width != w || layoutParams.height != h) {
            layoutParams.width = w;
            layoutParams.height = h;

            mSurfaceView.setLayoutParams(layoutParams);
        }
    }

    public void setSurfaceSize(int videoWidth, int videoHeight) {
        if (videoWidth <= 0 || videoHeight <= 0) {
            return;
        }

        boolean isPIPMode = getActivity().isInPictureInPictureMode();
        boolean isMultiWindMode = getActivity().isInMultiWindowMode();
        boolean isWndFullScreen = MediaUtils.isVideoWindowFullScreen();

        if (isPIPMode) {
            setSurfaceViewLayout(mAppData.mVideoUiWidth, mAppData.mVideoUiHeight);
            LogUtil.i(TAG, "setSurfaceSize: [return reason] isInPictureInPictureMode!");
            return;
        }

        LogUtil.i(TAG, "setSurfaceSize: " + videoWidth + " x " + videoHeight);

        // 计算视频尺寸
        Point point = MediaUtils.computeAndUpdateVideoSize(videoWidth, videoHeight);
        videoWidth = point.x;
        videoHeight = point.y;

        if (null != mSurfaceView) {
            ViewGroup.LayoutParams layoutParams = mSurfaceView.getLayoutParams();
            LogUtil.low_i(TAG, "setSurfaceSize [00]: "
                    + layoutParams.width + " x " + layoutParams.height
                    + " --> " + videoWidth + " x " + videoHeight);

            // [分屏模式切换到全屏 isInMultiWindowMode() 函数变化没这么快]
            if (isMultiWindMode
                    && !isWndFullScreen) {
                layoutParams.width = videoWidth;
                layoutParams.height = videoHeight;

                LogUtil.low_i(TAG, "setSurfaceSize [01]: "
                        + layoutParams.width + " x " + layoutParams.height);
                setSurfaceViewLayoutParams(layoutParams);
            } else {
                boolean alreadyLayout = false;

                // 需要再分
                // 参考：SurfaceViewFragment 注释
                boolean portraitDisplay = mAppData.mVideoUiWidth < mAppData.mVideoUiHeight;
                if (isWndFullScreen && portraitDisplay
                        || !isMultiWindMode && (portraitDisplay || !isWndFullScreen)) {
                    alreadyLayout = true;

                    layoutParams.width = videoWidth;
                    layoutParams.height = videoHeight;

                    LogUtil.low_i(TAG, "setSurfaceSize: [02]"
                            + layoutParams.width + " x " + layoutParams.height);
                    setSurfaceViewLayoutParams(layoutParams);
                }

                // [兼容横屏出货]
                if (!alreadyLayout) {
                    // 保持横屏兼容设备，出货状态。
                    if (ThemeEx.isHorizontalScreenDeviceCompat(requireContext())) {
                        videoWidth = mAppData.mVideoUiWidth;
                        videoHeight = mAppData.mVideoUiHeight;
                    }

                    layoutParams.width = videoWidth;
                    layoutParams.height = videoHeight;

                    LogUtil.low_i(TAG, "setSurfaceSize: [03]"
                            + layoutParams.width + " x " + layoutParams.height);
                    setSurfaceViewLayoutParams(layoutParams);
                }
            }

            // [不修改横屏出货状态，后续可以尝试去掉测试效果]
            if (ThemeEx.isHorizontalScreenDeviceCompat(requireContext())) {
                if (null != mAppData.mFrontSurfaceHolder) {
                    LogUtil.low_i(TAG, "setSurfaceSize: [04]"
                            + layoutParams.width + " x " + layoutParams.height);
                    mAppData.mFrontSurfaceHolder.setFixedSize(videoWidth, videoHeight);
                }
            }
        }
    }

    /** 如果窗口是透明的，这里会闪透明背景 **/
    private void setSurfaceViewLayoutParams(ViewGroup.LayoutParams params) {
        if (mSurfaceView != null) {
            mSurfaceView.setLayoutParams(params);
            // mSurfaceView.setBackgroundColor(Color.BLACK);
        }
    }

    /** [具体模式之间的切换还未实现] **/
    private void onVideoScaleTypeChanged() {
        int videoWidth = -1;
        int videoHeight = -1;

        IMediaPlayer player = mVideoViewModel.vitamioPlayer();
        if (player != null) {
            if (player.isPrepared()) {
                videoWidth = player.getVideoWidth();
                videoHeight = player.getVideoHeight();
            }
        }

        setSurfaceSize(videoWidth, videoHeight);
    }

    private void onConfigurationChanged() {
        int videoWidth = -1;
        int videoHeight = -1;

        IMediaPlayer player = mVideoViewModel.vitamioPlayer();
        if (player != null) {
            if (player.isPrepared()) {
                videoWidth = player.getVideoWidth();
                videoHeight = player.getVideoHeight();
            }
        }

        setSurfaceSize(videoWidth, videoHeight);
    }

    /**
     * 调整 SurfaceView 的显示大小
     * <p> 不可在 Fragment 未初始化情况下调用；
     *
     * @param reason 调整原因
     */
    public void setVideoSurfaceSize(int reason) {
        if (!mInitView) {
            return;
        }

        switch (reason) {
            // Button switch video display size
            case 0:
            // OnMusicClickListener.EVENT_CHANGE_SURFACE_VIEW_SIZE
            case 1:
                onVideoScaleTypeChanged();
                break;

            // OnMusicClickListener.EVENT_CONFIGURATION_CHANGED_SIZE
            case 2:
                onConfigurationChanged();
                break;

            default:
                break;
        }
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fragment_surfaceview;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        LogUtil.e(TAG, "____onCreateView");

        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);

        assert view != null;
        mSurfaceView = (SurfaceView) view.findViewById(xId(R.id.surfaceview_video));
        initView(false);
        return view;
    }

    @Override
    public void onResume() {
        super.onResume();
        LogUtil.e(TAG, "____onResume");

        initView(false);
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    @Override
    public void onPause() {
        super.onPause();
        LogUtil.e(TAG, "____onPause");

        mNeedResumeSurfaceView = false;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        LogUtil.e(TAG, "____onDestroy");

        mInitView = false;
        mSurfaceView = null;
        mNeedResumeSurfaceView = false;
    }

    @Override
    public void initFragment(boolean resume) {
        // TODO Auto-generated method stub
        super.initFragment(resume);
        LogUtil.e(TAG, "____initFragment");

        initView(true);
    }

    private void initView(boolean resumeSurfaceView) {
        LogUtil.e(TAG, "____initView, resumeSurfaceView: " + resumeSurfaceView);
        boolean isPIPMode = requireActivity().isInPictureInPictureMode();
        boolean isMultiWindMode = requireActivity().isInMultiWindowMode();

        if (mInitView) {
            if (isPIPMode) {
                // 在画中画模式
            } else {
                // 前台模式、多窗口、后台模式
                if (!resumeSurfaceView &&
                        View.VISIBLE != mSurfaceView.getVisibility()) {
                    if (!mNeedResumeSurfaceView) {
                        return;
                    }
                }

                if (!isResumed()) {
                    if (isMultiWindMode) {
                        // 分屏
                    } else {
                        // 后台、遮挡
                        if (resumeSurfaceView) {
                            mNeedResumeSurfaceView = true;
                            return;
                        }
                    }
                }
            }
        } else {
            // 第一次初始化
            // 有可能是后台、画中画、分屏、也可能是前台
            if (isPIPMode) {
                // 画中画
            } else {
                if (isResumed()) {
                    // 前台
                } else {
                    if (isMultiWindMode) {
                        // 分屏
                    } else {
                        // 后台、遮挡
                        return;
                    }
                }
            }
        }

        // [注意]不管什么模式，有概率触发 SurfaceView 回调
        if (null != mSurfaceView) {
            // 处理不好会黑屏
            if (mNeedResumeSurfaceView || resumeSurfaceView) {
                mNeedResumeSurfaceView = false;

                if (View.VISIBLE != mSurfaceView.getVisibility()) {
                    mSurfaceView.setVisibility(View.VISIBLE);
                }
            }

            mSurfaceView.getHolder().addCallback(mSHCallback);
            mSurfaceView.getHolder().setFormat(PixelFormat.RGBX_8888);
            mSurfaceView.setBackgroundColor(Color.TRANSPARENT);

            if (HMediaConfig.SURFACE_VIEW_FOCUSABLE) {
                mSurfaceView.setFocusable(true);
                mSurfaceView.setFocusableInTouchMode(true);
                mSurfaceView.requestFocus();
            }
        }

        mInitView = true;
    }

    @Override
    public void uninitFragment() {
        super.uninitFragment();
        LogUtil.e(TAG, "____uninitFragment");

        mNeedResumeSurfaceView = false;
        if (null != mSurfaceView) {
            mSurfaceView.setBackgroundColor(Color.BLACK);
            mSurfaceView.setVisibility(View.GONE);
        }
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        LogUtil.i(TAG, ">>> onHiddenChanged: " + hidden);
    }

    @SuppressLint("ResourceAsColor")
    private void resetSurfaceHolder() {
        if (null == mSurfaceView) {
            return;
        }

        SurfaceHolder holder = mSurfaceView.getHolder();
        if (null != holder) {
            Canvas canvas = holder.lockCanvas();

            if (canvas != null) {
                LogUtil.i(TAG, ">>> resetSurfaceHolder.");

                // [注意参数: @ColorInt]
                canvas.drawColor(Color.BLACK);
                holder.unlockCanvasAndPost(canvas);
            }
        }
    }
}
