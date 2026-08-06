package com.hcn.media_dummy.listener;

import android.view.View;

/**
 * 锁屏点击
 * @author 65821
 */
public interface LockClickListener {
    /**
     * 点击事件
     * @param view 锁屏按钮
     * @param lock 锁屏状态
     */
    void onClick(View view, boolean lock);
}
