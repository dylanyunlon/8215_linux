package com.hcn.media_dummy.render.view.listener;

import com.hcn.media_dummy.render.glrender.FunVideoGLViewBaseRender;

/**
 * GL渲染错误
 * @author 86158
 */
public interface FunVideoGLRenderErrorListener {

    /**
     * 错误回调接口
     *
     * @param render 渲染器
     * @param Error 错误文本
     * @param code 错误代码
     * @param byChangedRenderError 错误是因为切换 effect 导致的
     */
    void onError(FunVideoGLViewBaseRender render,
                 String Error,
                 int code,
                 boolean byChangedRenderError);
}
