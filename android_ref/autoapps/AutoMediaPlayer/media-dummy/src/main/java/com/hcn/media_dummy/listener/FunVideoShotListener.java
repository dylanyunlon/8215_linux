package com.hcn.media_dummy.listener;

import android.graphics.Bitmap;

/**
 * 截屏 bitmap 返回
 * @author 65821
 */
public interface FunVideoShotListener {

    /**
     * 返回位图接口
     * @param bitmap 位图
     */
    void getBitmap(Bitmap bitmap);
}
