package com.hcn.media_dummy.view.video;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.TextureView;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.annotation.AttrRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.media_dummy.render.FunRenderView;
import com.hcn.media_dummy.render.effect.NoEffect;
import com.hcn.media_dummy.render.glrender.FunVideoGLViewBaseRender;
import com.hcn.media_dummy.render.view.FunVideoGLView;
import com.hcn.media_dummy.render.view.listener.IFunSurfaceListener;
import com.hcn.media_dummy.utils.FunVideoType;
import com.hcn.media_dummy.utils.MeasureHelper;

/**
 * 绘制视图布局（）
 * @author 65821
 */
public abstract class FunTextureRenderView extends FrameLayout
        implements IFunSurfaceListener, MeasureHelper.MeasureFormVideoParamsListener {

    /** native 绘制 */
    protected Surface mSurface;

    /** 渲染控件 */
    protected FunRenderView mTextureView;

    /** 渲染控件父类 */
    protected ViewGroup mTextureViewContainer;

    /** 满屏填充暂停位图 */
    protected Bitmap mFullPauseBitmap;

    /** GL 的滤镜 */
    protected FunVideoGLView.ShaderInterface mEffectFilter = new NoEffect();

    /** GL 的自定义渲染 */
    protected FunVideoGLViewBaseRender mRenderer;

    /** GL 的角度 */
    protected float[] mMatrixGL = null;

    /** 画面选择角度 */
    protected int mRotate;

    /** GL 的布局模式 */
    protected int mMode = FunVideoGLView.MODE_LAYOUT_SIZE;

    public FunTextureRenderView(@NonNull Context context) {
        super(context);
    }

    public FunTextureRenderView(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public FunTextureRenderView(@NonNull Context context, @Nullable AttributeSet attrs, @AttrRes int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    public void onSurfaceAvailable(Surface surface) {
        pauseLogic(surface,
                (mTextureView != null && mTextureView.getShowView() instanceof TextureView));
    }

    @Override
    public void onSurfaceSizeChanged(Surface surface, int width, int height) {
    }

    @Override
    public boolean onSurfaceDestroyed(Surface surface) {
        // 清空释放
        setDisplay(null);

        // 同一消息队列中去 release
        releaseSurface(surface);

        return true;
    }

    @Override
    public void onSurfaceUpdated(Surface surface) {
        // 如果播放的是暂停全屏了
        releasePauseCover();
    }

    /**
     * 暂停逻辑
     */
    protected void pauseLogic(Surface surface, boolean pauseLogic) {
        mSurface = surface;
        if (pauseLogic) {
            // 显示暂停切换显示的图片
            showPauseCover();
        }

        setDisplay(mSurface);
    }

    /**
     * 添加播放的 view
     * <pre>
     *     继承后重载 addTextureView 函数；
     *     继承 FunRenderView 后实现自己的 IFunRenderView 类，既可以使用自己自定义的显示层；
     * </pre>
     */
    protected void addTextureView() {
        mTextureView = new FunRenderView();
        mTextureView.addView(getContext(),
                mTextureViewContainer, mRotate, this,
                this, mEffectFilter, mMatrixGL, mRenderer, mMode);
    }

    /**
     * 获取布局参数
     * <p> 默认是匹配父窗口；
     *
     * @return 布局参数
     */
    protected int getTextureParams() {
        boolean typeChanged = (FunVideoType.getShowType() != FunVideoType.SCREEN_TYPE_DEFAULT);
        return (typeChanged) ? ViewGroup.LayoutParams.WRAP_CONTENT : ViewGroup.LayoutParams.MATCH_PARENT;
    }

    /**
     * 调整 TextureView 去适应比例变化
     * <p> 16:09/FULLSCREEN/04:03/...
     */
    protected void changeTextureViewShowType() {
        if (mTextureView != null) {
            int params = getTextureParams();
            ViewGroup.LayoutParams layoutParams = mTextureView.getLayoutParams();
            layoutParams.width = params;
            layoutParams.height = params;
            mTextureView.setLayoutParams(layoutParams);
        }
    }

    /**
     * 暂停时初始化位图
     */
    protected void initCover() {
        if (mTextureView != null) {
            mFullPauseBitmap = mTextureView.initCover();
        }
    }

    /**
     * 小窗口渲染
     */
    protected void setSmallVideoTextureView(OnTouchListener onTouchListener) {
        mTextureViewContainer.setOnTouchListener(onTouchListener);
        mTextureViewContainer.setOnClickListener(null);
        setSmallVideoTextureView();
    }

    public FunVideoGLView.ShaderInterface getEffectFilter() {
        return mEffectFilter;
    }

    /**
     * 获取渲染的代理层
     */
    public FunRenderView getRenderProxy() {
        return mTextureView;
    }

    /**
     * 设置滤镜效果
     * <p> 黑白滤镜、磨砂滤镜、重叠滤镜...
     */
    public void setEffectFilter(FunVideoGLView.ShaderInterface effectFilter) {
        mEffectFilter = effectFilter;
        if (mTextureView != null) {
            mTextureView.setEffectFilter(effectFilter);
        }
    }

    /**
     * GL 模式下的画面 matrix 效果
     *
     * @param matrixGL 16 位长度
     */
    public void setMatrixGL(float[] matrixGL) {
        this.mMatrixGL = matrixGL;
        if (mTextureView != null) {
            mTextureView.setMatrixGL(mMatrixGL);
        }
    }

    /**
     * 自定义 GL 的渲染 render
     * @param renderer 渲染器
     */
    public void setCustomGLRenderer(FunVideoGLViewBaseRender renderer) {
        this.mRenderer = renderer;
        if (mTextureView != null) {
            mTextureView.setGLRenderer(renderer);
        }
    }

    /**
     * GL 布局的绘制模式，利用布局计算大小还是使用 render 计算大小
     *
     * @param mode MODE_LAYOUT_SIZE = 0,  MODE_RENDER_SIZE = 1
     */
    public void setGLRenderMode(int mode) {
        mMode = mode;
        if (mTextureView != null) {
            mTextureView.setGLRenderMode(mode);
        }
    }

    /** 暂停时使用绘制画面显示暂停、避免黑屏 */
    protected abstract void showPauseCover();

    /** 清除暂停画面 */
    protected abstract void releasePauseCover();

    /** 小屏幕绘制层 */
    protected abstract void setSmallVideoTextureView();

    /**
     * 设置播放 Surface
     * @param surface 显示缓冲区
     */
    protected abstract void setDisplay(Surface surface);

    /**
     * 释放 Surface
     * @param surface 显示缓冲区
     */
    protected abstract void releaseSurface(Surface surface);

}
