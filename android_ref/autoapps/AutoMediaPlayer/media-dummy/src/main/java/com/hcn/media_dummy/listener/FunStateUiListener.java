package com.hcn.media_dummy.listener;

import com.hcn.media_dummy.view.base.FunMediaState;

/**
 * 状态变化监听
 * @author 65821
 */
public interface FunStateUiListener {
    /**
     * 状态改变
     * @param state {@link FunMediaState}
     */
    void onStateChanged(int state);
}
