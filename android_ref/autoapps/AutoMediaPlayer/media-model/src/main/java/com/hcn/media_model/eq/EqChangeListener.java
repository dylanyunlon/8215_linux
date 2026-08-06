package com.hcn.media_model.eq;

/** 音效改变监听回调
 * @author 65821
 */
public interface EqChangeListener {
    /**
     * 音效数据改变
     * @param s 数据
     */
    void onEqChange(SharePreferencesTools s);
}