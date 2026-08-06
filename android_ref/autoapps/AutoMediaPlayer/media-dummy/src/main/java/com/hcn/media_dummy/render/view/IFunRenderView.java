package com.hcn.media_dummy.render.view;

import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.view.View;

import com.hcn.media_dummy.listener.FunVideoShotListener;
import com.hcn.media_dummy.listener.FunVideoShotSaveListener;
import com.hcn.media_dummy.render.glrender.FunVideoGLViewBaseRender;
import com.hcn.media_dummy.render.view.listener.IFunSurfaceListener;
import com.hcn.media_dummy.utils.MeasureHelper;

import java.io.File;

/**
 * 渲染视图接口
 * @author 65821
 */
public interface IFunRenderView {

    /**
     * 获取 surface 监听接口
     * @return {@link IFunSurfaceListener}
     */
    IFunSurfaceListener getFunSurfaceListener();

    /**
     * Surface 变化监听，必须
     * @param surfaceListener surface 监听接口
     */
    void setFunSurfaceListener(IFunSurfaceListener surfaceListener);

    /**
     * 当前 view 高度，必须
     * @return 高
     */
    int getSizeH();

    /**
     * 当前 view 宽度，必须
     * @return 宽
     */
    int getSizeW();

    /**
     * 实现该接口的 view，必须
     * @return 实现视图
     */
    View getRenderView();

    /**
     * 渲染 view 通过 MeasureFormVideoParamsListener 获取视频的相关参数，必须
     * @param listener 视频参数监听
     */
    void setVideoParamsListener(MeasureHelper.MeasureFormVideoParamsListener listener);

    /**
     * 截图
     * @param funVideoShotListener 监听截图结果
     * @param shotHigh 需要高清
     */
    void taskShotPic(FunVideoShotListener funVideoShotListener, boolean shotHigh);

    /**
     * 保存当前帧
     * @param file 保存文件
     * @param high 需要高清
     * @param funVideoShotSaveListener 截图保存结果
     */
    void saveFrame(final File file, final boolean high, final FunVideoShotSaveListener funVideoShotSaveListener);

    /**
     * 获取当前画面的 bitmap，没有返回空
     * @return 位图对象
     */
    Bitmap initCover();

    /**
     * 获取当前画面的高质量 bitmap，没有返回空
     * @return 位图对象
     */
    Bitmap initCoverHigh();

    /** 恢复渲染 */
    void onRenderResume();

    /** 停止渲染 */
    void onRenderPause();

    /** 释放渲染器资源 */
    void releaseRenderAll();

    /**
     * 设置渲染模式
     * @param mode 模式
     */
    void setRenderMode(int mode);

    /**
     * 设置渲染器变换矩阵
     * @param transform 矩阵
     */
    void setRenderTransform(Matrix transform);

    /**
     * 设置渲染器
     * @param renderer 渲染器
     */
    void setGLRenderer(FunVideoGLViewBaseRender renderer);

    /**
     * 設置 MVP 变换矩阵
     * @param MVPMatrix 矩阵
     */
    void setGLMVPMatrix(float[] MVPMatrix);

    /**
     * 设置滤镜
     * @param effectFilter 滤镜
     */
    void setGLEffectFilter(FunVideoGLView.ShaderInterface effectFilter);
}
