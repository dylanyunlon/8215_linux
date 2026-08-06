package com.hcn.media_dummy.view.video;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.hcn.common.misc.HNetworkUtils;
import com.hcn.common.misc.LogUtils;
import com.hcn.media_dummy.R;
import com.hcn.media_dummy.listener.FunVideoShotListener;
import com.hcn.media_dummy.listener.FunVideoShotSaveListener;
import com.hcn.media_dummy.render.FunRenderView;

import java.io.File;

import moe.codeest.enviews.ENDownloadView;
import moe.codeest.enviews.ENPlayView;

/**
 * 标准的视频播放器
 * <p> 继承之后实现一些 ui 显示效果，如显示／隐藏 ui，播放按键等;
 *
 * @author 65821
 */
public class StandardVideoPlayer extends FunVideoPlayer {
    /** 亮度 dialog */
    protected Dialog mBrightnessDialog;

    /** 音量 dialog */
    protected Dialog mVolumeDialog;

    /** 触摸进度 dialog */
    protected Dialog mProgressDialog;

    /** 触摸进度条的 progress */
    protected ProgressBar mDialogProgressBar;

    /** 音量进度条的 progress */
    protected ProgressBar mDialogVolumeProgressBar;

    /** 亮度文本 */
    protected TextView mBrightnessDialogTv;

    /** 触摸移动显示文本 */
    protected TextView mDialogSeekTime;

    /** 触摸移动显示全部时间 */
    protected TextView mDialogTotalTime;

    /** 触摸移动方向 icon */
    protected ImageView mDialogIcon;

    protected Drawable mBottomProgressDrawable;

    protected Drawable mBottomShowProgressDrawable;

    protected Drawable mBottomShowProgressThumbDrawable;

    protected Drawable mVolumeProgressDrawable;

    protected Drawable mDialogProgressBarDrawable;

    protected int mDialogProgressHighLightColor = -11;

    protected int mDialogProgressNormalColor = -11;

    public StandardVideoPlayer(Context context, Boolean fullFlag) {
        super(context, fullFlag);
    }

    public StandardVideoPlayer(Context context) {
        super(context);
    }

    public StandardVideoPlayer(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void init(Context context) {
        super.init(context);

        if (mBottomProgressDrawable != null) {
            mBottomProgressBar.setProgressDrawable(mBottomProgressDrawable);
        }

        if (mBottomShowProgressDrawable != null) {
            mProgressBar.setProgressDrawable(mBottomProgressDrawable);
        }

        if (mBottomShowProgressThumbDrawable != null) {
            mProgressBar.setThumb(mBottomShowProgressThumbDrawable);
        }
    }

    /**
     * 获取布局资源
     * <p> 继承后重写可替换为你需要的布局
     *
     * @return 布局资源
     */
    @Override
    public int getLayoutId() {
        return R.layout.video_layout_standard;
    }

    /**
     * 开始播放业务逻辑
     * <p> 调用该接口后，只需要等待回调状态即可；
     */
    @Override
    public void startPlayLogic() {
        if (mVideoAllCallBack != null) {
            LogUtils.v("onClickStartThumb");
            mVideoAllCallBack.onClickStartThumb(
                    mOriginUrl, mTitle, StandardVideoPlayer.this);
        }

        prepareVideo();
        startDismissControlViewTimer();
    }

    /**
     * 显示wifi确定框，如需要自定义继承重写即可
     */
    @Override
    protected void showWifiDialog() {
        if (!HNetworkUtils.isAvailable()) {
            startPlayLogic();
            return;
        }

        AlertDialog.Builder builder =
                new AlertDialog.Builder(getActivityContext());
        builder.setMessage(
                getResources().getString(R.string.tips_not_wifi));
        builder.setPositiveButton(
                getResources().getString(R.string.tips_not_wifi_confirm),
                (dialog, which) -> {
                    dialog.dismiss();
                    startPlayLogic();
                });
        builder.setNegativeButton(
                getResources().getString(R.string.tips_not_wifi_cancel),
                (dialog, which) -> dialog.dismiss());
        builder.create().show();
    }

    /**
     * 触摸显示滑动进度 dialog，如需要自定义继承重写即可，记得重写 dismissProgressDialog
     */
    @SuppressLint("SetTextI18n")
    @Override
    @SuppressWarnings("ResourceType")
    protected void showProgressDialog(float deltaX,
                                      String seekTime,
                                      long seekTimePosition,
                                      String totalTime,
                                      long totalTimeDuration) {
        if (mProgressDialog == null) {
            View localView = LayoutInflater
                    .from(getActivityContext())
                    .inflate(getProgressDialogLayoutId(), null);

            int progressBar_id = getProgressDialogProgressId();
            if (localView.findViewById(progressBar_id) instanceof ProgressBar) {
                mDialogProgressBar = ((ProgressBar) localView.findViewById(progressBar_id));
                if (mDialogProgressBarDrawable != null) {
                    mDialogProgressBar.setProgressDrawable(mDialogProgressBarDrawable);
                }
            }

            int seekTime_id = getProgressDialogCurrentDurationTextId();
            if (localView.findViewById(seekTime_id) instanceof TextView) {
                mDialogSeekTime = ((TextView) localView.findViewById(seekTime_id));
            }

            int totalTime_id = getProgressDialogAllDurationTextId();
            if (localView.findViewById(totalTime_id) instanceof TextView) {
                mDialogTotalTime = ((TextView) localView.findViewById(totalTime_id));
            }

            int image_id = getProgressDialogImageId();
            if (localView.findViewById(image_id) instanceof ImageView) {
                mDialogIcon = ((ImageView) localView.findViewById(image_id));
            }

            mProgressDialog = new Dialog(
                    getActivityContext(), R.style.video_style_dialog_progress);
            mProgressDialog.setContentView(localView);
            mProgressDialog.getWindow().addFlags(Window.FEATURE_ACTION_BAR);
            mProgressDialog.getWindow().addFlags(32);
            mProgressDialog.getWindow().addFlags(16);
            mProgressDialog.getWindow().setLayout(getWidth(), getHeight());

            if (mDialogProgressNormalColor != -11 && mDialogTotalTime != null) {
                mDialogTotalTime.setTextColor(mDialogProgressNormalColor);
            }

            if (mDialogProgressHighLightColor != -11 && mDialogSeekTime != null) {
                mDialogSeekTime.setTextColor(mDialogProgressHighLightColor);
            }

            WindowManager.LayoutParams localLayoutParams =
                    mProgressDialog.getWindow().getAttributes();
            localLayoutParams.gravity = Gravity.TOP;
            localLayoutParams.width = getWidth();
            localLayoutParams.height = getHeight();

            int[] location = new int[2];
            getLocationOnScreen(location);
            localLayoutParams.x = location[0];
            localLayoutParams.y = location[1];
            mProgressDialog.getWindow().setAttributes(localLayoutParams);
        }

        if (!mProgressDialog.isShowing()) {
            mProgressDialog.show();
        }

        if (mDialogSeekTime != null) {
            mDialogSeekTime.setText(seekTime);
        }

        if (mDialogTotalTime != null) {
            mDialogTotalTime.setText(" / " + totalTime);
        }

        if (totalTimeDuration > 0) {
            if (mDialogProgressBar != null) {
                mDialogProgressBar.setProgress((int)(seekTimePosition * 100 / totalTimeDuration));
            }
        }

        if (deltaX > 0) {
            if (mDialogIcon != null) {
                mDialogIcon.setBackgroundResource(R.drawable.video_forward_icon);
            }
        } else {
            if (mDialogIcon != null) {
                mDialogIcon.setBackgroundResource(R.drawable.video_backward_icon);
            }
        }
    }

    @Override
    protected void dismissProgressDialog() {
        if (mProgressDialog != null) {
            mProgressDialog.dismiss();
            mProgressDialog = null;
        }
    }

    /**
     * 触摸音量dialog，如需要自定义继承重写即可，记得重写dismissVolumeDialog
     */
    @Override
    protected void showVolumeDialog(float deltaY, int volumePercent) {
        if (mVolumeDialog == null) {
            View localView = LayoutInflater
                    .from(getActivityContext())
                    .inflate(getVolumeLayoutId(), null);

            int progress_id = getVolumeProgressId();
            if (localView.findViewById(progress_id) instanceof ProgressBar) {
                mDialogVolumeProgressBar = ((ProgressBar) localView.findViewById(progress_id));
                if (mVolumeProgressDrawable != null && mDialogVolumeProgressBar != null) {
                    mDialogVolumeProgressBar.setProgressDrawable(mVolumeProgressDrawable);
                }
            }

            mVolumeDialog = new Dialog(
                    getActivityContext(), R.style.video_style_dialog_progress);
            mVolumeDialog.setContentView(localView);
            mVolumeDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE);
            mVolumeDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL);
            mVolumeDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE);
            mVolumeDialog.getWindow().setLayout(
                    ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);

            WindowManager.LayoutParams localLayoutParams = mVolumeDialog.getWindow().getAttributes();
            localLayoutParams.gravity = Gravity.TOP | Gravity.START;
            localLayoutParams.width = getWidth();
            localLayoutParams.height = getHeight();

            int[] location = new int[2];
            getLocationOnScreen(location);
            localLayoutParams.x = location[0];
            localLayoutParams.y = location[1];
            mVolumeDialog.getWindow().setAttributes(localLayoutParams);
        }
        if (!mVolumeDialog.isShowing()) {
            mVolumeDialog.show();
        }
        if (mDialogVolumeProgressBar != null) {
            mDialogVolumeProgressBar.setProgress(volumePercent);
        }
    }

    @Override
    protected void dismissVolumeDialog() {
        if (mVolumeDialog != null) {
            mVolumeDialog.dismiss();
            mVolumeDialog = null;
        }
    }

    /**
     * 触摸亮度dialog，如需要自定义继承重写即可，记得重写dismissBrightnessDialog
     */
    @SuppressLint("SetTextI18n")
    @Override
    protected void showBrightnessDialog(float percent) {
        if (mBrightnessDialog == null) {
            View localView = LayoutInflater
                    .from(getActivityContext())
                    .inflate(getBrightnessLayoutId(), null);

            int brightness_id = getBrightnessTextId();
            if (localView.findViewById(brightness_id) instanceof TextView) {
                mBrightnessDialogTv = (TextView) localView.findViewById(brightness_id);
            }

            mBrightnessDialog = new Dialog(
                    getActivityContext(), R.style.video_style_dialog_progress);
            mBrightnessDialog.setContentView(localView);
            mBrightnessDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE);
            mBrightnessDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL);
            mBrightnessDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE);
            mBrightnessDialog.getWindow().getDecorView()
                    .setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
            mBrightnessDialog.getWindow().setLayout(
                    ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);

            WindowManager.LayoutParams localLayoutParams = mBrightnessDialog.getWindow().getAttributes();
            localLayoutParams.gravity = Gravity.TOP | Gravity.END;
            localLayoutParams.width = getWidth();
            localLayoutParams.height = getHeight();

            int[] location = new int[2];
            getLocationOnScreen(location);
            localLayoutParams.x = location[0];
            localLayoutParams.y = location[1];
            mBrightnessDialog.getWindow().setAttributes(localLayoutParams);
        }

        if (!mBrightnessDialog.isShowing()) {
            mBrightnessDialog.show();
        }

        if (mBrightnessDialogTv != null) {
            mBrightnessDialogTv.setText((int) (percent * 100) + "%");
        }
    }

    @Override
    protected void dismissBrightnessDialog() {
        if (mBrightnessDialog != null) {
            mBrightnessDialog.dismiss();
            mBrightnessDialog = null;
        }
    }

    @Override
    protected void cloneParams(FunBaseVideoPlayer from, FunBaseVideoPlayer to) {
        super.cloneParams(from, to);

        StandardVideoPlayer sf = (StandardVideoPlayer) from;
        StandardVideoPlayer st = (StandardVideoPlayer) to;

        if (st.mProgressBar != null && sf.mProgressBar != null) {
            st.mProgressBar.setProgress(sf.mProgressBar.getProgress());
            st.mProgressBar.setSecondaryProgress(sf.mProgressBar.getSecondaryProgress());
        }

        if (st.mTotalTimeTextView != null && sf.mTotalTimeTextView != null) {
            st.mTotalTimeTextView.setText(sf.mTotalTimeTextView.getText());
        }

        if (st.mCurrentTimeTextView != null && sf.mCurrentTimeTextView != null) {
            st.mCurrentTimeTextView.setText(sf.mCurrentTimeTextView.getText());
        }
    }

    /**
     * 将自定义的效果也设置到全屏
     *
     * @param context
     * @param actionBar 是否有actionBar，有的话需要隐藏
     * @param statusBar 是否有状态bar，有的话需要隐藏
     * @return 新的全屏播放视图
     */
    @Override
    public FunBaseVideoPlayer startWindowFullscreen(Context context,
                                                    boolean actionBar,
                                                    boolean statusBar) {
        FunBaseVideoPlayer funBaseVideoPlayer =
                super.startWindowFullscreen(context, actionBar, statusBar);
        if (funBaseVideoPlayer != null) {
            StandardVideoPlayer funVideoPlayer = (StandardVideoPlayer) funBaseVideoPlayer;
            funVideoPlayer.setLockClickListener(mLockClickListener);
            funVideoPlayer.setNeedLockFull(isNeedLockFull());
            initFullUI(funVideoPlayer);

            // 比如你自定义了返回按键，但是因为返回按键底层已经设置了返回事件，所以你需要在这里重新增加的逻辑
        }
        return funBaseVideoPlayer;
    }

    /********************************各类UI的状态显示*********************************************/

    /**
     * 点击触摸显示和隐藏逻辑
     */
    @Override
    protected void onClickUiToggle(MotionEvent e) {
        if (mIfCurrentIsFullscreen
                && mLockCurScreen
                && mNeedLockFull) {
            setViewShowState(mLockScreen, VISIBLE);
            return;
        }

        if (mIfCurrentIsFullscreen
                && !mSurfaceErrorPlay
                && mCurrentState == CURRENT_STATE_ERROR) {
            if (mBottomContainer != null) {
                if (mBottomContainer.getVisibility() == View.VISIBLE) {
                    changeUiToPlayingClear();
                } else {
                    changeUiToPlayingShow();
                }
            }
        } else if (mCurrentState == CURRENT_STATE_PREPAREING) {
            if (mBottomContainer != null) {
                if (mBottomContainer.getVisibility() == View.VISIBLE) {
                    changeUiToPrepareingClear();
                } else {
                    changeUiToPreparingShow();
                }
            }
        } else if (mCurrentState == CURRENT_STATE_PLAYING) {
            if (mBottomContainer != null) {
                if (mBottomContainer.getVisibility() == View.VISIBLE) {
                    changeUiToPlayingClear();
                } else {
                    changeUiToPlayingShow();
                }
            }
        } else if (mCurrentState == CURRENT_STATE_PAUSE) {
            if (mBottomContainer != null) {
                if (mBottomContainer.getVisibility() == View.VISIBLE) {
                    changeUiToPauseClear();
                } else {
                    changeUiToPauseShow();
                }
            }
        } else if (mCurrentState == CURRENT_STATE_AUTO_COMPLETE) {
            if (mBottomContainer != null) {
                if (mBottomContainer.getVisibility() == View.VISIBLE) {
                    changeUiToCompleteClear();
                } else {
                    changeUiToCompleteShow();
                }
            }
        } else if (mCurrentState == CURRENT_STATE_PLAYING_BUFFERING_START) {
            if (mBottomContainer != null) {
                if (mBottomContainer.getVisibility() == View.VISIBLE) {
                    changeUiToPlayingBufferingClear();
                } else {
                    changeUiToPlayingBufferingShow();
                }
            }
        }
    }

    @Override
    protected void hideAllWidget() {
        setViewShowState(mBottomContainer, INVISIBLE);
        setViewShowState(mTopContainer, INVISIBLE);
        setViewShowState(mBottomProgressBar, VISIBLE);
        setViewShowState(mStartButton, INVISIBLE);
    }

    @Override
    protected void changeUiToNormal() {
        LogUtils.v("changeUiToNormal");

        setViewShowState(mTopContainer, VISIBLE);
        setViewShowState(mBottomContainer, INVISIBLE);
        setViewShowState(mStartButton, VISIBLE);
        setViewShowState(mLoadingProgressBar, INVISIBLE);
        setViewShowState(mThumbImageViewLayout, VISIBLE);
        setViewShowState(mBottomProgressBar, INVISIBLE);
        setViewShowState(mLockScreen,
                (mIfCurrentIsFullscreen && mNeedLockFull) ? VISIBLE : GONE);

        updateStartImage();
        if (mLoadingProgressBar instanceof ENDownloadView) {
            ((ENDownloadView) mLoadingProgressBar).reset();
        }
    }

    @Override
    protected void changeUiToPreparingShow() {
        LogUtils.v("changeUiToPreparingShow");

        setViewShowState(mTopContainer, VISIBLE);
        setViewShowState(mBottomContainer, VISIBLE);
        setViewShowState(mStartButton, INVISIBLE);
        setViewShowState(mLoadingProgressBar, VISIBLE);
        setViewShowState(mThumbImageViewLayout, INVISIBLE);
        setViewShowState(mBottomProgressBar, INVISIBLE);
        setViewShowState(mLockScreen, GONE);

        if (mLoadingProgressBar instanceof ENDownloadView) {
            ENDownloadView enDownloadView = (ENDownloadView) mLoadingProgressBar;
            if (enDownloadView.getCurrentState() == ENDownloadView.STATE_PRE) {
                ((ENDownloadView) mLoadingProgressBar).start();
            }
        }
    }

    @Override
    protected void changeUiToPlayingShow() {
        LogUtils.v("changeUiToPlayingShow");

        if (mLockCurScreen && mNeedLockFull) {
            setViewShowState(mLockScreen, VISIBLE);
            return;
        }

        setViewShowState(mTopContainer, VISIBLE);
        setViewShowState(mBottomContainer, VISIBLE);
        setViewShowState(mStartButton, VISIBLE);
        setViewShowState(mLoadingProgressBar, INVISIBLE);
        setViewShowState(mThumbImageViewLayout, INVISIBLE);
        setViewShowState(mBottomProgressBar, INVISIBLE);
        setViewShowState(mLockScreen,
                (mIfCurrentIsFullscreen && mNeedLockFull) ? VISIBLE : GONE);

        if (mLoadingProgressBar instanceof ENDownloadView) {
            ((ENDownloadView) mLoadingProgressBar).reset();
        }

        updateStartImage();
    }

    @Override
    protected void changeUiToPauseShow() {
        LogUtils.v("changeUiToPauseShow");
        if (mLockCurScreen && mNeedLockFull) {
            setViewShowState(mLockScreen, VISIBLE);
            return;
        }

        setViewShowState(mTopContainer, VISIBLE);
        setViewShowState(mBottomContainer, VISIBLE);
        setViewShowState(mStartButton, VISIBLE);
        setViewShowState(mLoadingProgressBar, INVISIBLE);
        setViewShowState(mThumbImageViewLayout, INVISIBLE);
        setViewShowState(mBottomProgressBar, INVISIBLE);
        setViewShowState(mLockScreen,
                (mIfCurrentIsFullscreen && mNeedLockFull) ? VISIBLE : GONE);

        if (mLoadingProgressBar instanceof ENDownloadView) {
            ((ENDownloadView) mLoadingProgressBar).reset();
        }

        updateStartImage();
        updatePauseCover();
    }

    @Override
    protected void changeUiToPlayingBufferingShow() {
        LogUtils.v("changeUiToPlayingBufferingShow");

        setViewShowState(mTopContainer, VISIBLE);
        setViewShowState(mBottomContainer, VISIBLE);
        setViewShowState(mStartButton, INVISIBLE);
        setViewShowState(mLoadingProgressBar, VISIBLE);
        setViewShowState(mThumbImageViewLayout, INVISIBLE);
        setViewShowState(mBottomProgressBar, INVISIBLE);
        setViewShowState(mLockScreen, GONE);

        if (mLoadingProgressBar instanceof ENDownloadView) {
            ENDownloadView enDownloadView = (ENDownloadView) mLoadingProgressBar;
            if (enDownloadView.getCurrentState() == ENDownloadView.STATE_PRE) {
                ((ENDownloadView) mLoadingProgressBar).start();
            }
        }
    }

    @Override
    protected void changeUiToCompleteShow() {
        LogUtils.v("changeUiToCompleteShow");

        setViewShowState(mTopContainer, VISIBLE);
        setViewShowState(mBottomContainer, VISIBLE);
        setViewShowState(mStartButton, VISIBLE);
        setViewShowState(mLoadingProgressBar, INVISIBLE);
        setViewShowState(mThumbImageViewLayout, VISIBLE);
        setViewShowState(mBottomProgressBar, INVISIBLE);
        setViewShowState(mLockScreen, (mIfCurrentIsFullscreen && mNeedLockFull) ? VISIBLE : GONE);

        if (mLoadingProgressBar instanceof ENDownloadView) {
            ((ENDownloadView) mLoadingProgressBar).reset();
        }

        updateStartImage();
    }

    @Override
    protected void changeUiToError() {
        LogUtils.v("changeUiToError");

        setViewShowState(mTopContainer, INVISIBLE);
        setViewShowState(mBottomContainer, INVISIBLE);
        setViewShowState(mStartButton, VISIBLE);
        setViewShowState(mLoadingProgressBar, INVISIBLE);
        setViewShowState(mThumbImageViewLayout, INVISIBLE);
        setViewShowState(mBottomProgressBar, INVISIBLE);
        setViewShowState(mLockScreen, (mIfCurrentIsFullscreen && mNeedLockFull) ? VISIBLE : GONE);

        if (mLoadingProgressBar instanceof ENDownloadView) {
            ((ENDownloadView) mLoadingProgressBar).reset();
        }

        updateStartImage();
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();

        dismissVolumeDialog();
        dismissBrightnessDialog();
    }

    /**
     * 触摸进度 dialog 的 layoutId
     * <pre>
     *    继承后重写可返回自定义
     *    有自定义的实现逻辑可重载 showProgressDialog 方法
     * </pre>
     */
    protected int getProgressDialogLayoutId() {
        return R.layout.video_progress_dialog;
    }

    /**
     * 触摸进度 dialog 的进度条 id
     * <pre>
     *    继承后重写可返回自定义，如果没有可返回空
     *    有自定义的实现逻辑可重载 showProgressDialog 方法
     * </pre>
     */
    protected int getProgressDialogProgressId() {
        return R.id.duration_progressbar;
    }

    /**
     * 触摸进度dialog的当前时间文本
     * 继承后重写可返回自定义，如果没有可返回空
     * 有自定义的实现逻辑可重载showProgressDialog方法
     */
    protected int getProgressDialogCurrentDurationTextId() {
        return R.id.tv_current;
    }

    /**
     * 触摸进度 dialog 全部时间文本
     * <pre>
     *    继承后重写可返回自定义，如果没有可返回空
     *    有自定义的实现逻辑可重载 showProgressDialog 方法
     * </pre>
     */
    protected int getProgressDialogAllDurationTextId() {
        return R.id.tv_duration;
    }

    /**
     * 触摸进度 dialog 的图片 id
     * <pre>
     *    继承后重写可返回自定义，如果没有可返回空
     *    有自定义的实现逻辑可重载 showProgressDialog 方法
     * </pre>
     */
    protected int getProgressDialogImageId() {
        return R.id.duration_image_tip;
    }

    /**
     * 音量 dialog 的 layoutId
     * <pre>
     *    继承后重写可返回自定义
     *    有自定义的实现逻辑可重载 showVolumeDialog 方法
     * </pre>
     */
    protected int getVolumeLayoutId() {
        return R.layout.video_volume_dialog;
    }

    /**
     * 音量 dialog 的百分比进度条 id
     * <pre>
     *    继承后重写可返回自定义，如果没有可返回空
     *    有自定义的实现逻辑可重载 showVolumeDialog 方法
     * </pre>
     */
    protected int getVolumeProgressId() {
        return R.id.volume_progressbar;
    }


    /**
     * 亮度 dialog 的 layoutId
     * <pre>
     *    继承后重写可返回自定义
     *    有自定义的实现逻辑可重载 showBrightnessDialog 方法
     * </pre>
     */
    protected int getBrightnessLayoutId() {
        return R.layout.video_brightness;
    }

    /**
     * 亮度 dialog 的百分比 text id
     * <pre>
     *    继承后重写可返回自定义，如果没有可返回空
     *    有自定义的实现逻辑可重载 showBrightnessDialog 方法
     * </pre>
     */
    protected int getBrightnessTextId() {
        return R.id.app_video_brightness;
    }

    protected void changeUiToPrepareingClear() {
        LogUtils.v("changeUiToPrepareingClear");

        setViewShowState(mTopContainer, INVISIBLE);
        setViewShowState(mBottomContainer, INVISIBLE);
        setViewShowState(mStartButton, INVISIBLE);
        setViewShowState(mLoadingProgressBar, INVISIBLE);
        setViewShowState(mThumbImageViewLayout, INVISIBLE);
        setViewShowState(mBottomProgressBar, INVISIBLE);
        setViewShowState(mLockScreen, GONE);

        if (mLoadingProgressBar instanceof ENDownloadView) {
            ((ENDownloadView) mLoadingProgressBar).reset();
        }
    }

    protected void changeUiToPlayingClear() {
        LogUtils.v("changeUiToPlayingClear");

        changeUiToClear();
        setViewShowState(mBottomProgressBar, VISIBLE);
    }

    protected void changeUiToPauseClear() {
        LogUtils.v("changeUiToPauseClear");

        changeUiToClear();
        setViewShowState(mBottomProgressBar, VISIBLE);
        updatePauseCover();
    }

    protected void changeUiToPlayingBufferingClear() {
        LogUtils.v("changeUiToPlayingBufferingClear");

        setViewShowState(mTopContainer, INVISIBLE);
        setViewShowState(mBottomContainer, INVISIBLE);
        setViewShowState(mStartButton, INVISIBLE);
        setViewShowState(mLoadingProgressBar, VISIBLE);
        setViewShowState(mThumbImageViewLayout, INVISIBLE);
        setViewShowState(mBottomProgressBar, VISIBLE);
        setViewShowState(mLockScreen, GONE);

        if (mLoadingProgressBar instanceof ENDownloadView) {
            ENDownloadView enDownloadView = (ENDownloadView) mLoadingProgressBar;
            if (enDownloadView.getCurrentState() == ENDownloadView.STATE_PRE) {
                ((ENDownloadView) mLoadingProgressBar).start();
            }
        }

        updateStartImage();
    }

    protected void changeUiToClear() {
        LogUtils.v("changeUiToClear");

        setViewShowState(mTopContainer, INVISIBLE);
        setViewShowState(mBottomContainer, INVISIBLE);
        setViewShowState(mStartButton, INVISIBLE);
        setViewShowState(mLoadingProgressBar, INVISIBLE);
        setViewShowState(mThumbImageViewLayout, INVISIBLE);
        setViewShowState(mBottomProgressBar, INVISIBLE);
        setViewShowState(mLockScreen, GONE);

        if (mLoadingProgressBar instanceof ENDownloadView) {
            ((ENDownloadView) mLoadingProgressBar).reset();
        }
    }

    protected void changeUiToCompleteClear() {
        LogUtils.v("changeUiToCompleteClear");

        setViewShowState(mTopContainer, INVISIBLE);
        setViewShowState(mBottomContainer, INVISIBLE);
        setViewShowState(mStartButton, VISIBLE);
        setViewShowState(mLoadingProgressBar, INVISIBLE);
        setViewShowState(mThumbImageViewLayout, VISIBLE);
        setViewShowState(mBottomProgressBar, VISIBLE);
        setViewShowState(mLockScreen,
                (mIfCurrentIsFullscreen && mNeedLockFull) ? VISIBLE : GONE);

        if (mLoadingProgressBar instanceof ENDownloadView) {
            ((ENDownloadView) mLoadingProgressBar).reset();
        }

        updateStartImage();
    }

    /**
     * 定义开始按键显示
     */
    protected void updateStartImage() {
        if (mStartButton instanceof ENPlayView) {
            ENPlayView enPlayView = (ENPlayView) mStartButton;
            enPlayView.setDuration(500);
            if (mCurrentState == CURRENT_STATE_PLAYING) {
                enPlayView.play();
            } else if (mCurrentState == CURRENT_STATE_ERROR) {
                enPlayView.pause();
            } else {
                enPlayView.pause();
            }
        } else if (mStartButton instanceof ImageView) {
            ImageView imageView = (ImageView) mStartButton;
            if (mCurrentState == CURRENT_STATE_PLAYING) {
                imageView.setImageResource(R.drawable.video_click_pause_selector);
            } else if (mCurrentState == CURRENT_STATE_ERROR) {
                imageView.setImageResource(R.drawable.video_click_error_selector);
            } else {
                imageView.setImageResource(R.drawable.video_click_play_selector);
            }
        }
    }

    /**
     * 全屏的 UI 逻辑
     *
     * @param standardVideoPlayer
     */
    private void initFullUI(StandardVideoPlayer standardVideoPlayer) {
        if (mBottomProgressDrawable != null) {
            standardVideoPlayer.setBottomProgressBarDrawable(mBottomProgressDrawable);
        }

        if (mBottomShowProgressDrawable != null
                && mBottomShowProgressThumbDrawable != null) {
            standardVideoPlayer.setBottomShowProgressBarDrawable(
                    mBottomShowProgressDrawable, mBottomShowProgressThumbDrawable);
        }

        if (mVolumeProgressDrawable != null) {
            standardVideoPlayer.setDialogVolumeProgressBar(mVolumeProgressDrawable);
        }

        if (mDialogProgressBarDrawable != null) {
            standardVideoPlayer.setDialogProgressBar(mDialogProgressBarDrawable);
        }

        if (mDialogProgressHighLightColor != -11 && mDialogProgressNormalColor != -11) {
            standardVideoPlayer.setDialogProgressColor(
                    mDialogProgressHighLightColor, mDialogProgressNormalColor);
        }
    }

    /**
     * 底部进度条 - 弹出的
     *
     * @param drawable 进度条资源
     * @param thumb 触摸资源
     */
    public void setBottomShowProgressBarDrawable(Drawable drawable, Drawable thumb) {
        mBottomShowProgressDrawable = drawable;
        mBottomShowProgressThumbDrawable = thumb;

        if (mProgressBar != null) {
            mProgressBar.setProgressDrawable(drawable);
            mProgressBar.setThumb(thumb);
        }
    }

    /**
     * 底部进度条 -非弹出
     *
     * @param drawable 进度条资源
     */
    public void setBottomProgressBarDrawable(Drawable drawable) {
        mBottomProgressDrawable = drawable;
        if (mBottomProgressBar != null) {
            mBottomProgressBar.setProgressDrawable(drawable);
        }
    }

    /**
     * 声音进度条
     *
     * @param drawable 进度条资源
     */
    public void setDialogVolumeProgressBar(Drawable drawable) {
        mVolumeProgressDrawable = drawable;
    }


    /**
     * 中间进度条
     *
     * @param drawable 进度条资源
     */
    public void setDialogProgressBar(Drawable drawable) {
        mDialogProgressBarDrawable = drawable;
    }

    /**
     * 中间进度条字体颜色
     *
     * @param highLightColor 高亮颜色
     * @param normalColor 正常颜色
     */
    public void setDialogProgressColor(int highLightColor, int normalColor) {
        mDialogProgressHighLightColor = highLightColor;
        mDialogProgressNormalColor = normalColor;
    }


    /************************************* 关于截图的 ****************************************/

    /**
     * 获取截图
     *
     * @param funVideoShotListener 监听
     */
    public void taskShotPic(FunVideoShotListener funVideoShotListener) {
        this.taskShotPic(funVideoShotListener, false);
    }

    /**
     * 获取截图
     *
     * @param funVideoShotListener 监听
     * @param high 是否需要高清的
     */
    public void taskShotPic(FunVideoShotListener funVideoShotListener,
                            boolean high) {
        if (getCurrentPlayer().getRenderProxy() != null) {
            getCurrentPlayer().getRenderProxy().taskShotPic(funVideoShotListener, high);
        }
    }

    /**
     * 保存截图
     *
     * @param file 截图保存到的文件
     * @param funVideoShotSaveListener 状态回调
     */
    public void saveFrame(final File file,
                          FunVideoShotSaveListener funVideoShotSaveListener) {
        saveFrame(file, false, funVideoShotSaveListener);
    }

    /**
     * 保存截图
     *
     * @param file 截图保存到的文件
     * @param high 是否需要高清的
     * @param funVideoShotSaveListener 状态回调
     */
    public void saveFrame(final File file,
                          final boolean high,
                          final FunVideoShotSaveListener funVideoShotSaveListener) {
        FunRenderView renderProxy = getCurrentPlayer().getRenderProxy();
        if (renderProxy != null) {
            renderProxy.saveFrame(file, high, funVideoShotSaveListener);
        }
    }

    /**
     * 重新开启进度查询以及控制 view 消失的定时任务
     * 用于解决 FunVideoHelper 中通过 removeView 方式做全屏切换导致的定时任务停止的问题
     * FunVideoControlView onDetachedFromWindow（）
     */
    public void restartTimerTask() {
        startProgressTimer();
        startDismissControlViewTimer();
    }
}
