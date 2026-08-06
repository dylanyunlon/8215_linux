package com.hcn.media_dummy.render.view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.AttributeSet;
import android.view.PixelCopy;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;

import com.hcn.common.misc.LogUtils;
import com.hcn.media_dummy.Config;
import com.hcn.media_dummy.listener.FunVideoShotListener;
import com.hcn.media_dummy.listener.FunVideoShotSaveListener;
import com.hcn.media_dummy.render.FunRenderView;
import com.hcn.media_dummy.render.glrender.FunVideoGLViewBaseRender;
import com.hcn.media_dummy.render.view.listener.IFunSurfaceListener;
import com.hcn.media_dummy.utils.MeasureHelper;

import java.io.File;

/**
 * SurfaceView 视图
 * <p> 最简单的视频显示视图，不支持着色、平移、旋转等操作；
 * @author 65821
 */
public class FunSurfaceView extends SurfaceView
        implements SurfaceHolder.Callback2, IFunRenderView, MeasureHelper.MeasureFormVideoParamsListener {

    private IFunSurfaceListener mFunSurfaceListener;

    private MeasureHelper.MeasureFormVideoParamsListener mVideoParamsListener;

    private MeasureHelper measureHelper;

    public FunSurfaceView(Context context) {
        super(context);
        init();
    }

    public FunSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    private void init() {
        measureHelper = new MeasureHelper(this, this);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        measureHelper.prepareMeasure(widthMeasureSpec, heightMeasureSpec, (int) getRotation());
        setMeasuredDimension(measureHelper.getMeasuredWidth(), measureHelper.getMeasuredHeight());
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if (mFunSurfaceListener != null) {
            mFunSurfaceListener.onSurfaceAvailable(holder.getSurface());
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (mFunSurfaceListener != null) {
            mFunSurfaceListener.onSurfaceSizeChanged(holder.getSurface(), width, height);
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        // 清空释放
        if (mFunSurfaceListener != null) {
            mFunSurfaceListener.onSurfaceDestroyed(holder.getSurface());
        }
    }

    @Override
    public void surfaceRedrawNeeded(SurfaceHolder holder) {
    }

    @Override
    public IFunSurfaceListener getFunSurfaceListener() {
        return mFunSurfaceListener;
    }

    @Override
    public void setFunSurfaceListener(IFunSurfaceListener surfaceListener) {
        getHolder().addCallback(this);
        this.mFunSurfaceListener = surfaceListener;
    }

    @Override
    public int getSizeH() {
        return getHeight();
    }

    @Override
    public int getSizeW() {
        return getWidth();
    }

    @Override
    public Bitmap initCover() {
        if (getSizeW() <= 0 || getSizeH() <= 0) {
            return null;
        }
        return Bitmap.createBitmap(
                getSizeW(), getSizeH(), Bitmap.Config.RGB_565);
    }

    /**
     * 暂停时初始化位图
     */
    @Override
    public Bitmap initCoverHigh() {
        if (getSizeW() <= 0 || getSizeH() <= 0) {
            return null;
        }
        return Bitmap.createBitmap(
                getSizeW(), getSizeH(), Bitmap.Config.ARGB_8888);
    }

    /**
     * 获取截图
     *
     * @param shotHigh 是否需要高清的
     */
    @Override
    @SuppressLint("ObsoleteSdkInt")
    public void taskShotPic(FunVideoShotListener videoShotListener, boolean shotHigh) {
        Bitmap bitmap;
        if (shotHigh) {
            bitmap = initCoverHigh();
        } else {
            bitmap = initCover();
        }

        try {
            HandlerThread handlerThread = new HandlerThread("PixelCopier");
            handlerThread.start();
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                PixelCopy.request(this, bitmap, copyResult -> {
                    if (copyResult == PixelCopy.SUCCESS) {
                        videoShotListener.getBitmap(bitmap);
                    }

                    handlerThread.quitSafely();
                }, new Handler(handlerThread.getLooper()));
            } else {
                LogUtils.dTag(Config.TAG, getClass().getSimpleName() +
                        " Build.VERSION.SDK_INT < Build.VERSION_CODES.N not support taskShotPic now.");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 保存截图
     *
     * @param high 是否需要高清的
     */
    @Override
    public void saveFrame(final File file,
                          final boolean high,
                          final FunVideoShotSaveListener videoShotSaveListener) {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support saveFrame now, use taskShotPic.");
    }

    @Override
    public View getRenderView() {
        return this;
    }

    @Override
    public void onRenderResume() {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support onRenderResume now.");
    }

    @Override
    public void onRenderPause() {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support onRenderPause now.");
    }

    @Override
    public void releaseRenderAll() {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support releaseRenderAll now.");
    }

    @Override
    public void setRenderMode(int mode) {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support setRenderMode now.");
    }


    @Override
    public void setRenderTransform(Matrix transform) {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support setRenderTransform now.");
    }

    @Override
    public void setGLRenderer(FunVideoGLViewBaseRender renderer) {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support setGLRenderer now.");
    }

    @Override
    public void setGLMVPMatrix(float[] MVPMatrix) {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support setGLMVPMatrix now.");
    }

    /** 设置滤镜效果 */
    @Override
    public void setGLEffectFilter(FunVideoGLView.ShaderInterface effectFilter) {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support setGLEffectFilter now.");
    }

    @Override
    public void setVideoParamsListener(MeasureHelper.MeasureFormVideoParamsListener listener) {
        mVideoParamsListener = listener;
    }

    @Override
    public int getCurrentVideoWidth() {
        if (mVideoParamsListener != null) {
            return mVideoParamsListener.getCurrentVideoWidth();
        }
        return 0;
    }

    @Override
    public int getCurrentVideoHeight() {
        if (mVideoParamsListener != null) {
            return mVideoParamsListener.getCurrentVideoHeight();
        }
        return 0;
    }

    /**
     * 添加播放的 view
     */
    public static FunSurfaceView addSurfaceView(Context context,
                                                ViewGroup textureViewContainer,
                                                int rotate,
                                                final IFunSurfaceListener surfaceListener,
                                                final MeasureHelper.MeasureFormVideoParamsListener videoParamsListener) {
        if (textureViewContainer.getChildCount() > 0) {
            textureViewContainer.removeAllViews();
        }

        FunSurfaceView showSurfaceView = new FunSurfaceView(context);
        showSurfaceView.setFunSurfaceListener(surfaceListener);
        showSurfaceView.setVideoParamsListener(videoParamsListener);
        showSurfaceView.setRotation(rotate);
        FunRenderView.addToParent(textureViewContainer, showSurfaceView);
        return showSurfaceView;
    }
}