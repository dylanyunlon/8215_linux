package com.hcn.media_dummy.render.view.listener;

import android.view.Surface;

/**
 * Surface 状态变化回调
 * @author 86158
 */
public interface IFunSurfaceListener {

    /**
     * Surface 可用
     *
     * @param surface 目标
     */
    void onSurfaceAvailable(Surface surface);

    /**
     * Surface 大小改变
     *
     * @param surface 目标
     * @param width 宽度
     * @param height 高度
     */
    void onSurfaceSizeChanged(Surface surface, int width, int height);

    /**
     * Surface 销毁
     *
     * @param surface 目标
     * @return 成功/失败
     */
    boolean onSurfaceDestroyed(Surface surface);

    /**
     * Surface 更新
     * @param surface 目标
     */
    void onSurfaceUpdated(Surface surface);
}