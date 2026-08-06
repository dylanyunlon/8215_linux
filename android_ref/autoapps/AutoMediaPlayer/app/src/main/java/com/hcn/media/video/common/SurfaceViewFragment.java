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
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_base.HMediaConfig;
import com.hcn.media_model.MediaUtils;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.media_model.player.base.IMediaPlayer;
import com.hcn.media_theme.ThemeEx;

import java.util.Objects;

/**
 * 播放显示层
 * <p> 硬解码使用；
 *
 * @author 86158
 */
@SuppressLint("ValidFragment")
public class SurfaceViewFragment extends MediaFragment {
    private static final String FRAGMENT_NAME = "video-surface";
    private static final String TAG = SurfaceViewFragment.class.getSimpleName();

    private boolean mInitView = false;
    private boolean mNeedResumeSurfaceView = false;
    private SurfaceView mSurfaceView = null;

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
                // [很奇怪这里不可调用 resetSurfaceHolder(), 否则会导致 setDisplay(.)
                //  报错: <E MediaPlayerService: setVideoSurfaceTexture failed: -22> ]
                mAppData.mFrontSurfaceHolder = holder;

                // 更新 SurfaceHolder -> MediaPlayer
                mVideoViewModel.playerRelay().accept(
                        t -> t.requestExecuteAction(
                                IMediaAction.updateCoreSurfaceHolder, null, null));

                // 检查并恢复当前视频模式的播放状态
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
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            LogUtil.i(TAG, "____surfaceDestroyed.");
            mAppData.mFrontSurfaceHolder = null;
            mVideoViewModel.playerRelay().accept(
                    t -> t.requestExecuteAction(
                            IMediaAction.updateCoreSurfaceHolder, null, null));
        }
    };

    @SuppressLint("ValidFragment")
    public SurfaceViewFragment(Context context,
                               IMediaEventListener listener) {
        super(FRAGMENT_NAME);
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        if (!isVisible()) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_SIZE: {
                setVideoSurfaceSize(1);
                break;
            }

            case IMediaEvent.EVENT_VIDEO_SHOW_BLACK_PAGE: {
                // [注意: setDisplay(holder) 后不可以调用这个函数 resetSurfaceHolder()]
                if (null != mSurfaceView) {
                    mSurfaceView.setBackgroundColor(Color.BLACK);
                    LogUtil.low_i(TAG, "[EVENT_VIDEO_SHOW_BLACK_PAGE]SurfaceView: Color.BLACK.");
                }
                break;
            }

            case IMediaEvent.EVENT_VIDEO_HIDE_BLACK_PAGE: {
                if (null != mSurfaceView) {
                    mSurfaceView.setBackgroundColor(Color.TRANSPARENT);
                    LogUtil.low_i(TAG, "[EVENT_VIDEO_HIDE_BLACK_PAGE]SurfaceView: Color.TRANSPARENT.");
                }
                break;
            }

            default: {
                break;
            }
        }
    }

    /**
     * 调整布局的大小, 主要是画中画的时候调用
     *
     * @param w 宽度
     * @param h 高度
     */
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

    private void setSurfaceSize(int videoWidth, int videoHeight) {
        if (videoWidth <= 0 || videoHeight <= 0) {
            // Illegal parameter
            return;
        }

        boolean isPIPMode = requireActivity().isInPictureInPictureMode();
        boolean isMultiWindMode = requireActivity().isInMultiWindowMode();
        boolean isWndFullScreen = MediaUtils.isVideoWindowFullScreen();

        if (isPIPMode) {
            setSurfaceViewLayout(mAppData.mVideoUiWidth, mAppData.mVideoUiHeight);
            LogUtil.i(TAG, "setSurfaceSize: [return reason] isInPictureInPictureMode!");
            return;
        }

        // [计算视频尺寸]
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
                // isWndFullScreen && portraitDisplay 全屏 且 是竖屏显示比例
                // !isMultiWindMode && portraitDisplay 非多窗口模式 且 是竖屏显示比例
                // !isMultiWindMode && !isWndFullScreen 非多窗口模式 且 非全屏显示
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

    /**
     * 设置 SurfaceView 布局参数
     * <p> 如果窗口是透明的，这里会闪透明背景
     * @param params 布局参数
     */
    private void setSurfaceViewLayoutParams(ViewGroup.LayoutParams params) {
        if (mSurfaceView != null) {
            mSurfaceView.setLayoutParams(params);
            // mSurfaceView.setBackgroundColor(Color.BLACK);
        }
    }

    /**
     * 处理视频显示大小改变
     * <p> [具体模式之间的切换还未实现]
     */
    public void onVideoScaleTypeChanged() {
        IMediaPlayer player = mVideoViewModel.corePlayer();
        if (Objects.isNull(player)) {
            return;
        }

        int videoWidth = player.getVideoWidth();
        int videoHeight = player.getVideoHeight();

        LogUtil.low_i(TAG, "onVideoScaleTypeChanged V: "
                + videoWidth + " x " + videoHeight);
        LogUtil.low_i(TAG, "onVideoScaleTypeChanged U: "
                + mAppData.mVideoUiWidth + " x " + mAppData.mVideoUiHeight);

        setSurfaceSize(videoWidth, videoHeight);
    }

    private void onConfigurationChanged() {
        IMediaPlayer player = mVideoViewModel.corePlayer();
        if (Objects.isNull(player)) {
            return;
        }

        int videoWidth = player.getVideoWidth();
        int videoHeight = player.getVideoHeight();

        LogUtil.low_i(TAG, "onConfigurationChanged V: "
                + videoWidth + " x " + videoHeight);
        LogUtil.low_i(TAG, "onConfigurationChanged U: "
                + mAppData.mVideoUiWidth + " x " + mAppData.mVideoUiHeight);

        setSurfaceSize(videoWidth, videoHeight);
    }

    public void setVideoSurfaceSize(int reason) {
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

    /**
     * [后台播放再进入前台，不管当前 Fragment 是否是 Hidden 状态，它都会触发 onResume()] [所以: 不要随意给 SurfaceView 设置 View.VISIBLE 状态,
     * 需要根据当前场景来判断恢复]
     */
    @Override
    public void onResume() {
        super.onResume();
        LogUtil.i(TAG, "____onResume");

        initView(false);
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    @Override
    public void onPause() {
        super.onPause();
        mNeedResumeSurfaceView = false;

        LogUtil.i(TAG, "____onPause");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        LogUtil.i(TAG, "____onDestroy");

        mInitView = false;
        mSurfaceView = null;
        mNeedResumeSurfaceView = false;
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        LogUtil.i(TAG, ">>> onHiddenChanged: " + hidden);
    }

    private void initView(boolean resumeSurfaceView) {
        boolean isPIPMode = requireActivity().isInPictureInPictureMode();
        boolean isMultiWindMode = requireActivity().isInMultiWindowMode();

        LogUtil.i(TAG,
                "____initView,"
                        + "\n     resumeSurfaceView: " + resumeSurfaceView
                        + "\n     isResumed: " + isResumed()
                        + "\n     PIP: " + isPIPMode
                        + "\n     MultiWind: " + isMultiWindMode);

        if (mInitView) {
            if (isPIPMode) {
                // 在画中画模式
            } else {
                // 前台模式、多窗口、后台模式
                if (!resumeSurfaceView &&
                        View.VISIBLE != mSurfaceView.getVisibility()) {
                    if (!mNeedResumeSurfaceView) {
                        LogUtil.i(TAG, "____initView, return: 101.");
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
                            LogUtil.i(TAG, "____initView, return: 102.");
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
                        LogUtil.i(TAG, "____initView, return: 103.");
                        return;
                    }
                }
            }
        }

        // [注意]不管什么模式，有概率触发 SurfaceView 回调
        if (null != mSurfaceView) {
            // 处理不好会黑屏
            boolean isVisibleChange = false;

            if (mNeedResumeSurfaceView || resumeSurfaceView) {
                mNeedResumeSurfaceView = false;

                // 如果已经增加 Callback，显示状态改变会触发阻塞回调
                if (View.VISIBLE != mSurfaceView.getVisibility()) {
                    // 标记可视状态发生改变
                    isVisibleChange = true;
                    mSurfaceView.setVisibility(View.VISIBLE);
                }
            }

            // 如果控件已是 VISIBLE, 第一次 addCallback 会触发阻塞回调
            mSurfaceView.getHolder().addCallback(mSHCallback);
            mSurfaceView.getHolder().setFormat(PixelFormat.RGBA_8888);

            if (HMediaConfig.SURFACE_VIEW_FOCUSABLE) {
                mSurfaceView.setFocusable(true);
                mSurfaceView.setFocusableInTouchMode(true);
                mSurfaceView.requestFocus();
            }

            if (isPIPMode && isVisibleChange) {
                // 原因: 画中画由软解码 SurfaceView 切换到硬解码 SurfaceView 会出现切换瞬间闪烁透视底层窗口显示问题;
                // 需要谨慎设置 BLACK 颜色, 如果设置后无法收到 EVENT_VIDEO_HIDE_BLACK_PAGE 消息, 将一直黑屏到下一曲才会恢复
                mSurfaceView.setBackgroundColor(Color.BLACK);
                LogUtil.low_i(TAG, "[initView]Surface: Color.BLACK.");
            } else {
                mSurfaceView.setBackgroundColor(Color.TRANSPARENT);
                LogUtil.low_i(TAG, "[initView]Surface: Color.TRANSPARENT.");
            }
        }

        mInitView = true;
    }

    @Override
    public void initFragment(boolean resume) {
        super.initFragment(resume);
        LogUtil.i(TAG, "____initFragment");

        initView(true);
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

    @SuppressLint("ResourceAsColor")
    private void resetSurfaceHolder() {
        if (null == mSurfaceView) {
            return;
        }

        /*
         * [ 硬解码的 SurfaceHolder.lockCanvas() 会报异常;
         *   at android.view.Surface.nativeLockCanvas:
         *            java.lang.IllegalArgumentException
         *   原因未知，初步怀疑是杰发平台本身的原因，待验证. ]
         */

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
