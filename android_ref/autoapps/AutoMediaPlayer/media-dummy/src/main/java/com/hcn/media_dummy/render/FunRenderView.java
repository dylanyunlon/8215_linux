package com.hcn.media_dummy.render;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.RelativeLayout;

import com.hcn.media_dummy.listener.FunVideoShotListener;
import com.hcn.media_dummy.listener.FunVideoShotSaveListener;
import com.hcn.media_dummy.render.glrender.FunVideoGLViewBaseRender;
import com.hcn.media_dummy.render.view.FunSurfaceView;
import com.hcn.media_dummy.render.view.FunTextureView;
import com.hcn.media_dummy.render.view.FunVideoGLView;
import com.hcn.media_dummy.render.view.IFunRenderView;
import com.hcn.media_dummy.render.view.listener.IFunSurfaceListener;
import com.hcn.media_dummy.utils.FunVideoType;
import com.hcn.media_dummy.utils.MeasureHelper;

import java.io.File;

/**
 * 好玩的渲染器视图（维护渲染视图接口）
 * <p> 提供给要显示视频的父容器视图使用，用来管理视频最终的显示 Surface 视图；
 *
 * @author 65821
 */
public class FunRenderView {
    protected IFunRenderView mShowView;

    /************************* RenderView function start *************************/

    public void requestLayout() {
        if (mShowView != null) {
            mShowView.getRenderView().requestLayout();
        }
    }

    public float getRotation() {
        return mShowView.getRenderView().getRotation();
    }

    public void setRotation(float rotation) {
        if (mShowView != null) {
            mShowView.getRenderView().setRotation(rotation);
        }
    }

    public void invalidate() {
        if (mShowView != null) {
            mShowView.getRenderView().invalidate();
        }
    }

    public int getWidth() {
        return (mShowView != null) ? mShowView.getRenderView().getWidth() : 0;
    }

    public int getHeight() {
        return (mShowView != null) ? mShowView.getRenderView().getHeight() : 0;
    }

    public View getShowView() {
        if (mShowView != null) {
            return mShowView.getRenderView();
        }

        return null;
    }

    public ViewGroup.LayoutParams getLayoutParams() {
        return mShowView.getRenderView().getLayoutParams();
    }

    public void setLayoutParams(ViewGroup.LayoutParams layoutParams) {
        if (mShowView != null) {
            mShowView.getRenderView().setLayoutParams(layoutParams);
        }
    }

    /**
     * 添加视频播放视图
     *
     * @param context 上下文
     * @param textureViewContainer 添加到的视图容器
     * @param rotate 视图旋转角度(顺时针)
     * @param funSurfaceListener surface 状态监听器
     * @param videoParamsListener 视频参数监听
     * @param effect 滤镜
     * @param transform 渲染变换
     * @param customRender 渲染器
     * @param mode 渲染模式
     */
    public void addView(final Context context,
                        final ViewGroup textureViewContainer,
                        final int rotate,
                        final IFunSurfaceListener funSurfaceListener,
                        final MeasureHelper.MeasureFormVideoParamsListener videoParamsListener,
                        final FunVideoGLView.ShaderInterface effect,
                        final float[] transform,
                        final FunVideoGLViewBaseRender customRender,
                        int mode) {
        if (FunVideoType.getRenderType() == FunVideoType.SURFACE) {
            mShowView = FunSurfaceView.addSurfaceView(context,
                    textureViewContainer, rotate, funSurfaceListener, videoParamsListener);
        } else if (FunVideoType.getRenderType() == FunVideoType.GLSURFACE) {
            mShowView = FunVideoGLView.addGLView(context,
                    textureViewContainer, rotate, funSurfaceListener, videoParamsListener,
                    effect, transform, customRender, mode);
        } else {
            mShowView = FunTextureView.addTextureView(context,
                    textureViewContainer, rotate, funSurfaceListener, videoParamsListener);
        }
    }

    /************************* RenderView function end *************************/

    /************************* ShowView function start *************************/

    /**
     * 主要针对 TextureView，设置旋转
     * @param transform 矩阵
     */
    public void setTransform(Matrix transform) {
        if (mShowView != null) {
            mShowView.setRenderTransform(transform);
        }
    }

    /**
     * 暂停时初始化位图
     * @return {@link Bitmap}
     */
    public Bitmap initCover() {
        if (mShowView != null) {
            return mShowView.initCover();
        }
        return null;
    }

    /**
     * 暂停时初始化位图
     * @return {@link Bitmap}
     */
    public Bitmap initCoverHigh() {
        if (mShowView != null) {
            return mShowView.initCoverHigh();
        }
        return null;
    }

    /**
     * 获取截图
     * @param videoShotListener 截图回调
     */
    public void taskShotPic(FunVideoShotListener videoShotListener) {
        this.taskShotPic(videoShotListener, false);
    }

    /**
     * 获取截图
     * @param videoShotListener 截图回调
     * @param shotHigh 是否需要高清的
     */
    public void taskShotPic(FunVideoShotListener videoShotListener, boolean shotHigh) {
        if (mShowView != null) {
            mShowView.taskShotPic(videoShotListener, shotHigh);
        }
    }

    /**
     * 保存截图
     * @param file 存储文件
     * @param videoShotSaveListener 保存监听
     */
    public void saveFrame(final File file, FunVideoShotSaveListener videoShotSaveListener) {
        saveFrame(file, false, videoShotSaveListener);
    }

    /**
     * 保存截图
     * @param file 存储文件
     * @param high 是否需要高清的
     * @param videoShotSaveListener 保存监听
     */
    public void saveFrame(final File file,
                          final boolean high,
                          final FunVideoShotSaveListener videoShotSaveListener) {
        if (mShowView != null) {
            mShowView.saveFrame(file, high, videoShotSaveListener);
        }
    }

    /**
     * 主要针对 GL
     */
    public void onResume() {
        if (mShowView != null) {
            mShowView.onRenderResume();
        }
    }

    /**
     * 主要针对 GL
     */
    public void onPause() {
        if (mShowView != null) {
            mShowView.onRenderPause();
        }
    }

    /**
     * 主要针对 GL
     */
    public void releaseAll() {
        if (mShowView != null) {
            mShowView.releaseRenderAll();
        }
    }

    /**
     * 主要针对 GL
     * @param mode 渲染模式
     */
    public void setGLRenderMode(int mode) {
        if (mShowView != null) {
            mShowView.setRenderMode(mode);
        }
    }

    /**
     * 自定义 GL 的渲染 render
     * @param renderer 渲染器
     */
    public void setGLRenderer(FunVideoGLViewBaseRender renderer) {
        if (mShowView != null) {
            mShowView.setGLRenderer(renderer);
        }
    }

    /**
     * GL模式下的画面 matrix 效果
     * @param matrixGL 16 位长度
     */
    public void setMatrixGL(float[] matrixGL) {
        if (mShowView != null) {
            mShowView.setGLMVPMatrix(matrixGL);
        }
    }

    /**
     * 设置滤镜效果
     * @param effectFilter 滤镜效果
     */
    public void setEffectFilter(FunVideoGLView.ShaderInterface effectFilter) {
        if (mShowView != null) {
            mShowView.setGLEffectFilter(effectFilter);
        }
    }

    /************************* ShowView function end *************************/

    /**************************** common function ****************************/

    /**
     * 增加渲染视图到父容器
     *
     * @param textureViewContainer 父容器
     * @param render 渲染视图
     */
    public static void addToParent(ViewGroup textureViewContainer, View render) {
        int params = getTextureParams();
        if (textureViewContainer instanceof RelativeLayout) {
            RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(params, params);
            layoutParams.addRule(RelativeLayout.CENTER_IN_PARENT);
            textureViewContainer.addView(render, layoutParams);
        } else if (textureViewContainer instanceof FrameLayout) {
            FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(params, params);
            layoutParams.gravity = Gravity.CENTER;
            textureViewContainer.addView(render, layoutParams);
        }
    }

    /**
     * 获取布局参数
     * @return 布局参数 {@link ViewGroup.LayoutParams#WRAP_CONTENT ...}
     */
    public static int getTextureParams() {
        boolean typeChanged = (FunVideoType.getShowType() != FunVideoType.SCREEN_TYPE_DEFAULT);
        return (typeChanged) ? ViewGroup.LayoutParams.WRAP_CONTENT : ViewGroup.LayoutParams.MATCH_PARENT;
    }
}
