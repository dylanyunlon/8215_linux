package com.hcn.media_dummy.render.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.graphics.SurfaceTexture;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;

import com.hcn.common.misc.LogUtils;
import com.hcn.media_dummy.Config;
import com.hcn.media_dummy.listener.FunVideoShotListener;
import com.hcn.media_dummy.listener.FunVideoShotSaveListener;
import com.hcn.media_dummy.render.FunRenderView;
import com.hcn.media_dummy.render.glrender.FunVideoGLViewBaseRender;
import com.hcn.media_dummy.render.view.listener.IFunSurfaceListener;
import com.hcn.media_dummy.utils.FileUtils;
import com.hcn.media_dummy.utils.FunVideoType;
import com.hcn.media_dummy.utils.MeasureHelper;

import java.io.File;

/**
 * 纹理视图
 * <p> 增强视频显示视图，做了横屏与竖屏的匹配，还有需要 rotation 需求的播放器，不支持着色渲染；
 * @author 65821
 */
public class FunTextureView extends TextureView
        implements IFunRenderView,
        TextureView.SurfaceTextureListener,
        MeasureHelper.MeasureFormVideoParamsListener {

    private IFunSurfaceListener mFunSurfaceListener;
    private MeasureHelper.MeasureFormVideoParamsListener mVideoParamsListener;
    private MeasureHelper measureHelper;

    private SurfaceTexture mSaveTexture;
    private Surface mSurface;

    public FunTextureView(Context context) {
        super(context);
        init();
    }

    public FunTextureView(Context context, AttributeSet attrs) {
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
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        if (FunVideoType.isMediaCodecTexture()) {
            if (mSaveTexture == null) {
                mSaveTexture = surface;
                mSurface = new Surface(surface);
            } else {
                setSurfaceTexture(mSaveTexture);
            }
        } else {
            mSurface = new Surface(surface);
        }

        if (mFunSurfaceListener != null) {
            mFunSurfaceListener.onSurfaceAvailable(mSurface);
        }
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        if (mFunSurfaceListener != null) {
            mFunSurfaceListener.onSurfaceSizeChanged(mSurface, width, height);
        }
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        // 清空释放
        if (mFunSurfaceListener != null) {
            mFunSurfaceListener.onSurfaceDestroyed(mSurface);
        }

        if (FunVideoType.isMediaCodecTexture()) {
            return (mSaveTexture == null);
        } else {
            return true;
        }
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        // 如果播放的是暂停全屏了
        if (mFunSurfaceListener != null) {
            mFunSurfaceListener.onSurfaceUpdated(mSurface);
        }
    }

    @Override
    public IFunSurfaceListener getFunSurfaceListener() {
        return mFunSurfaceListener;
    }

    @Override
    public void setFunSurfaceListener(IFunSurfaceListener surfaceListener) {
        setSurfaceTextureListener(this);
        mFunSurfaceListener = surfaceListener;
    }

    @Override
    public int getSizeH() {
        return getHeight();
    }

    @Override
    public int getSizeW() {
        return getWidth();
    }

    /**
     * 暂停时初始化位图
     */
    @Override
    public Bitmap initCover() {
        if (getSizeW() <= 0 || getSizeH() <= 0) {
            return null;
        }

        Bitmap bitmap = Bitmap.createBitmap(
                getSizeW(), getSizeH(), Bitmap.Config.RGB_565);
        return getBitmap(bitmap);
    }

    /**
     * 暂停时初始化位图
     */
    @Override
    public Bitmap initCoverHigh() {
        if (getSizeW() <= 0 || getSizeH() <= 0) {
            return null;
        }

        Bitmap bitmap = Bitmap.createBitmap(
                getSizeW(), getSizeH(), Bitmap.Config.ARGB_8888);
        return getBitmap(bitmap);
    }


    /**
     * 获取截图
     * @param shotHigh 是否需要高清的
     */
    @Override
    public void taskShotPic(FunVideoShotListener videoShotListener, boolean shotHigh) {
        if (shotHigh) {
            videoShotListener.getBitmap(initCoverHigh());
        } else {
            videoShotListener.getBitmap(initCover());
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
        FunVideoShotListener videoShotListener = new FunVideoShotListener() {
            @Override
            public void getBitmap(Bitmap bitmap) {
                if (bitmap == null) {
                    videoShotSaveListener.result(false, file);
                } else {
                    FileUtils.saveBitmap(bitmap, file);
                    videoShotSaveListener.result(true, file);
                }
            }
        };
        if (high) {
            videoShotListener.getBitmap(initCoverHigh());
        } else {
            videoShotListener.getBitmap(initCover());
        }
    }


    @Override
    public View getRenderView() {
        return this;
    }

    @Override
    public void onRenderResume() {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support onRenderResume now");
    }

    @Override
    public void onRenderPause() {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support onRenderPause now");
    }

    @Override
    public void releaseRenderAll() {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support releaseRenderAll now");
    }

    @Override
    public void setRenderMode(int mode) {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support setRenderMode now");
    }

    @Override
    public void setRenderTransform(Matrix transform) {
        setTransform(transform);
    }

    @Override
    public void setGLRenderer(FunVideoGLViewBaseRender renderer) {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support setGLRenderer now");
    }

    @Override
    public void setGLMVPMatrix(float[] MVPMatrix) {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support setGLMVPMatrix now");
    }

    /**
     * 设置滤镜效果
     */
    @Override
    public void setGLEffectFilter(FunVideoGLView.ShaderInterface effectFilter) {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support setGLEffectFilter now");
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
     * 添加播放的view
     */
    public static FunTextureView addTextureView(Context context,
                                                ViewGroup textureViewContainer,
                                                int rotate,
                                                final IFunSurfaceListener surfaceListener,
                                                final MeasureHelper.MeasureFormVideoParamsListener videoParamsListener) {
        if (textureViewContainer.getChildCount() > 0) {
            textureViewContainer.removeAllViews();
        }

        FunTextureView funTextureView = new FunTextureView(context);
        funTextureView.setFunSurfaceListener(surfaceListener);
        funTextureView.setVideoParamsListener(videoParamsListener);
        funTextureView.setRotation(rotate);
        FunRenderView.addToParent(textureViewContainer, funTextureView);

        return funTextureView;
    }
}