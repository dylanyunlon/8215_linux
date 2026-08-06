package com.hcn.media_dummy.base.player;

import com.hcn.media_dummy.base.model.FunModel;

/**
 * 播放器管理基类
 * @author 65821
 */
public abstract class BasePlayerManager implements IPlayerManager {

    protected IPlayerInitListener mPlayerInitListener;

    public IPlayerInitListener getPlayerInitializeListener() {
        return mPlayerInitListener;
    }

    public void setPlayerInitializeListener(IPlayerInitListener listener) {
        this.mPlayerInitListener = listener;
    }

    protected void initSuccess(FunModel model) {
        if (mPlayerInitListener != null) {
            mPlayerInitListener.onPlayerInitSuccess(getMediaPlayer(), model);
        }
    }
}