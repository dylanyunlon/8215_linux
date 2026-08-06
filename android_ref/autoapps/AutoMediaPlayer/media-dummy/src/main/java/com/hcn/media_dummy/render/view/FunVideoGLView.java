package com.hcn.media_dummy.render.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;

import com.hcn.common.misc.LogUtils;
import com.hcn.media_dummy.Config;
import com.hcn.media_dummy.listener.FunVideoShotListener;
import com.hcn.media_dummy.listener.FunVideoShotSaveListener;
import com.hcn.media_dummy.render.FunRenderView;
import com.hcn.media_dummy.render.effect.NoEffect;
import com.hcn.media_dummy.render.glrender.FunVideoGLViewBaseRender;
import com.hcn.media_dummy.render.glrender.FunVideoGLViewSimpleRender;
import com.hcn.media_dummy.render.view.listener.FunVideoGLRenderErrorListener;
import com.hcn.media_dummy.render.view.listener.GLSurfaceListener;
import com.hcn.media_dummy.render.view.listener.IFunSurfaceListener;
import com.hcn.media_dummy.utils.FileUtils;
import com.hcn.media_dummy.utils.MeasureHelper;

import java.io.File;

/**
 * GLSurfaceView
 * <pre>
 *    在 VidEffects 的基础扩展的，支持扩展滤镜;
 *    https://github.com/krazykira/VidEffects
 * </pre>
 *
 * @author 65821
 */
public class FunVideoGLView extends GLSurfaceView
        implements GLSurfaceListener, IFunRenderView, MeasureHelper.MeasureFormVideoParamsListener {

    private static final String TAG = FunVideoGLView.class.getName();

    /**
     * 利用布局计算大小
     */
    public static final int MODE_LAYOUT_SIZE = 0;

    /**
     * 利用 Render 计算大小
     */
    public static final int MODE_RENDER_SIZE = 1;

    private FunVideoGLViewBaseRender mRenderer;

    private Context mContext;

    private ShaderInterface mEffect = new NoEffect();

    private MeasureHelper.MeasureFormVideoParamsListener mVideoParamsListener;

    private MeasureHelper measureHelper;

    private GLSurfaceListener mGLSurfaceListener;

    private IFunSurfaceListener mFunSurfaceListener;

    private float[] mMVPMatrix;

    private int mMode = MODE_LAYOUT_SIZE;

    public interface ShaderInterface {
        String getShader(GLSurfaceView mGlSurfaceView);
    }

    public FunVideoGLView(Context context) {
        super(context);
        init(context);
    }

    public FunVideoGLView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    private void init(Context context) {
        mContext = context;
        setEGLContextClientVersion(2);
        mRenderer = new FunVideoGLViewSimpleRender();
        measureHelper = new MeasureHelper(this, this);
        mRenderer.setSurfaceView(FunVideoGLView.this);
    }

    @Override
    public void onResume() {
        super.onResume();

        if (mRenderer != null) {
            mRenderer.initRenderSize();
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        if (mMode == MODE_RENDER_SIZE) {
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
            measureHelper.prepareMeasure(widthMeasureSpec, heightMeasureSpec, (int) getRotation());
            initRenderMeasure();
        } else {
            measureHelper.prepareMeasure(widthMeasureSpec, heightMeasureSpec, (int) getRotation());
            setMeasuredDimension(measureHelper.getMeasuredWidth(), measureHelper.getMeasuredHeight());
        }
    }

    @Override
    public IFunSurfaceListener getFunSurfaceListener() {
        return mFunSurfaceListener;
    }

    @Override
    public void setFunSurfaceListener(IFunSurfaceListener surfaceListener) {
        setGLSurfaceListener(this);
        mFunSurfaceListener = surfaceListener;
    }

    @Override
    public void onSurfaceAvailable(Surface surface) {
        if (mFunSurfaceListener != null) {
            mFunSurfaceListener.onSurfaceAvailable(surface);
        }
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
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support initCover now");
        return null;
    }

    @Override
    public Bitmap initCoverHigh() {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support initCoverHigh now");
        return null;
    }

    /**
     * 获取截图
     * @param shotHigh 是否需要高清的
     */
    @Override
    public void taskShotPic(FunVideoShotListener videoShotListener, boolean shotHigh) {
        if (videoShotListener != null) {
            setVideoShotListener(videoShotListener, shotHigh);
            takeShotPic();
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

        setVideoShotListener(videoShotListener, high);
        takeShotPic();
    }

    @Override
    public View getRenderView() {
        return this;
    }

    @Override
    public void onRenderResume() {
        requestLayout();
        onResume();
    }

    @Override
    public void onRenderPause() {
        requestLayout();
        onPause();
    }

    @Override
    public void releaseRenderAll() {
        requestLayout();
        releaseAll();
    }

    @Override
    public void setRenderMode(int mode) {
        setMode(mode);
    }

    @Override
    public void setRenderTransform(Matrix transform) {
        LogUtils.vTag(Config.TAG,
                getClass().getSimpleName()
                        + " not support setRenderTransform now");
    }

    @Override
    public void setGLRenderer(FunVideoGLViewBaseRender renderer) {
        setCustomRenderer(renderer);
    }

    @Override
    public void setGLMVPMatrix(float[] MVPMatrix) {
        setMVPMatrix(MVPMatrix);
    }

    /**
     * 设置滤镜效果
     */
    @Override
    public void setGLEffectFilter(FunVideoGLView.ShaderInterface effectFilter) {
        setEffect(effectFilter);
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

    protected void initRenderMeasure() {
        if (mVideoParamsListener != null && mMode == MODE_RENDER_SIZE) {
            try {
                int videoWidth = mVideoParamsListener.getCurrentVideoWidth();
                int videoHeight = mVideoParamsListener.getCurrentVideoHeight();
                if (mRenderer != null) {
                    mRenderer.setCurrentViewWidth(measureHelper.getMeasuredWidth());
                    mRenderer.setCurrentViewHeight(measureHelper.getMeasuredHeight());
                    mRenderer.setCurrentVideoWidth(videoWidth);
                    mRenderer.setCurrentVideoHeight(videoHeight);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void initRender() {
        setRenderer(mRenderer);
    }

    public void setVideoGLRenderErrorListener(FunVideoGLRenderErrorListener videoGLRenderErrorListener) {
        mRenderer.setVideoGLRenderErrorListener(videoGLRenderErrorListener);
    }

    /**
     * 设置自定义的render，其他自定义设置会被取消，需要重新设置
     * 在initRender() 前设置才会生效
     *
     * @param CustomRender
     */
    public void setCustomRenderer(FunVideoGLViewBaseRender CustomRender) {
        mRenderer = CustomRender;
        mRenderer.setSurfaceView(FunVideoGLView.this);
        initRenderMeasure();
    }

    public void setGLSurfaceListener(GLSurfaceListener glSurfaceListener) {
        mGLSurfaceListener = glSurfaceListener;
        mRenderer.setGLSurfaceListener(mGLSurfaceListener);
    }

    public void setEffect(ShaderInterface shaderEffect) {
        if (shaderEffect != null) {
            mEffect = shaderEffect;
            mRenderer.setEffect(mEffect);
        }
    }

    public void setMVPMatrix(float[] MVPMatrix) {
        if (MVPMatrix != null) {
            mMVPMatrix = MVPMatrix;
            mRenderer.setMVPMatrix(MVPMatrix);
        }
    }

    public void takeShotPic() {
        mRenderer.takeShotPic();
    }

    public void setVideoShotListener(FunVideoShotListener listener, boolean high) {
        mRenderer.setVideoShotListener(listener, high);
    }

    public int getMode() {
        return mMode;
    }

    /**
     * @param mode MODE_LAYOUT_SIZE = 0,  MODE_RENDER_SIZE = 1
     */
    public void setMode(int mode) {
        mMode = mode;
    }

    public void releaseAll() {
        if (mRenderer != null) {
            mRenderer.releaseAll();
        }
    }

    public FunVideoGLViewBaseRender getRenderer() {
        return mRenderer;
    }

    public ShaderInterface getEffect() {
        return mEffect;
    }

    public float[] getMVPMatrix() {
        return mMVPMatrix;
    }

    /**
     * 添加播放的 view
     */
    public static FunVideoGLView addGLView(final Context context,
                                           final ViewGroup textureViewContainer,
                                           final int rotate,
                                           final IFunSurfaceListener surfaceListener,
                                           final MeasureHelper.MeasureFormVideoParamsListener videoParamsListener,
                                           final FunVideoGLView.ShaderInterface effect,
                                           final float[] transform,
                                           final FunVideoGLViewBaseRender customRender,
                                           final int renderMode) {
        if (textureViewContainer.getChildCount() > 0) {
            textureViewContainer.removeAllViews();
        }

        final FunVideoGLView funVideoGLView = new FunVideoGLView(context);
        if (customRender != null) {
            funVideoGLView.setCustomRenderer(customRender);
        }

        funVideoGLView.setEffect(effect);
        funVideoGLView.setVideoParamsListener(videoParamsListener);
        funVideoGLView.setRenderMode(renderMode);
        funVideoGLView.setFunSurfaceListener(surfaceListener);
        funVideoGLView.setRotation(rotate);
        funVideoGLView.initRender();
        funVideoGLView.setVideoGLRenderErrorListener(
                (render, Error, code, byChangedRenderError) -> {
                    if (byChangedRenderError) {
                        addGLView(context,
                                textureViewContainer,
                                rotate,
                                surfaceListener,
                                videoParamsListener,
                                render.getEffect(),
                                render.getMVPMatrix(),
                                render, renderMode);
                    }
                });

        if (transform != null && transform.length == 16) {
            funVideoGLView.setMVPMatrix(transform);
        }

        FunRenderView.addToParent(textureViewContainer, funVideoGLView);
        return funVideoGLView;
    }
}
