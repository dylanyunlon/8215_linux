package com.hcn.media_dummy.render.view.listener;

import android.view.Surface;

/**
 * GLSurfaceView surface 状态变化回调
 * @author 86158
 */
public interface GLSurfaceListener {

    /**
     * Surface 可用
     * @param surface
     */
    void onSurfaceAvailable(Surface surface);
}
