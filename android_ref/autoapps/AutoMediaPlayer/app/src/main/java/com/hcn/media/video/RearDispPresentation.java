package com.hcn.media.video;

import android.annotation.SuppressLint;
import android.app.Presentation;
import android.content.Context;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;

import androidx.annotation.NonNull;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.auto_compat.view.WindowManagerCompat;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.media.vm.base.BaseViewModel;

/**
 * The RearDispPresentation class is a Dialog, which can be shown in second display. In the dialog,
 * it contains a surfaceView to show video. And in the App, what we need is that the presentation be
 * shown when the front video playing background.
 * @author 86158
 */
public final class RearDispPresentation extends Presentation {
    private final String TAG = "RearDispPresentation";

    private AppGlobalData mAppData = null;
    private IPlayerEx mPlayerEx = null;

    public SurfaceView mRearVideoView = null;

    /**
     * Surface 状态监听
     * <p> 回调 SurfaceView 与之关联的 Surface 的创建、改变、销毁；
     */
    private final SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback() {
        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
            Log.e(TAG, "____surfaceChanged");
        }

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            Log.e(TAG, "____surfaceCreated");

            Surface surface = holder.getSurface();
            if (null != surface) {
                mAppData.mRearSurfaceHolder = holder;
                mPlayerEx.playerRelay().accept(
                        t -> t.requestExecuteAction(
                                IMediaAction.updateRearSurfaceHolder,
                                null,
                                null));

                if (mAppData.mAllowResumePlay) {
                    mPlayerEx.playerRelay().accept(
                            BaseViewModel.IPlayer::requestShouldPlayEvent);
                } else if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PAUSE)) {
                    if (mAppData.mPlayTimeInfo.mCurrentTime < mAppData.mPlayTimeInfo.mTotalTime) {
                        mPlayerEx.playerRelay().accept(
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
            Log.e(TAG, "____surfaceDestroyed");
            mAppData.mRearSurfaceHolder = null;
            mPlayerEx.playerRelay().accept(
                    t -> t.requestExecuteAction(
                            IMediaAction.updateRearSurfaceHolder,
                            null,
                            null));
        }
    };

    public RearDispPresentation(@NonNull Context context,
                                Display display,
                                @NonNull IPlayerEx player) {
        super(context, display);
        mPlayerEx = player;
    }

    @SuppressLint("WrongConstant")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "[onCreate], this = " + this);
        super.onCreate(savedInstanceState);

        Window win = getWindow();
        assert win != null;
        WindowManager.LayoutParams params = win.getAttributes();
        params.type = WindowManagerCompat.TYPE_DISPLAY_OVERLAY;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            params.type = WindowManagerCompat.TYPE_PRESENTATION;
        }

        mAppData = AppGlobalData.getInstance();
        getContext().getResources();
        setContentView(R.layout.fragment_surfaceview);
        mRearVideoView = findViewById(R.id.surfaceview_video);
        initView();
    }

    private void initView() {
        LogUtil.i(TAG, "____initView");
        mRearVideoView.getHolder().addCallback(mSHCallback);
        mRearVideoView.getHolder().setFormat(PixelFormat.RGBA_8888);
        mRearVideoView.getHolder().setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        mRearVideoView.requestFocus();
        mRearVideoView.setBackgroundColor(Color.TRANSPARENT);
    }

    @Override
    public void show() {
        super.show();

        mPlayerEx.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.updateRearSurfaceHolder,
                        null,
                        null));
    }

    @Override
    protected void onStart() {
        Log.i(TAG, "RearDispPresentation:[onStart], this=" + this);
        super.onStart();
    }

    @Override
    protected void onStop() {
        Log.i(TAG, "RearDispPresentation:[onStop], this=" + this);
        super.onStop();
    }
}
