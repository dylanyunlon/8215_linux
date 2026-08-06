package com.hcn.media_dummy.view.video;

import android.content.Context;
import android.util.AttributeSet;

import com.hcn.media_dummy.FunMediaManager;
import com.hcn.media_dummy.base.FunMediaViewBridge;

/**
 * 兼容的空 View，目前用于 FunVideoManager 的设置
 * @author 65821
 */
public abstract class FunVideoPlayer extends FunBaseVideoPlayer {

    public FunVideoPlayer(Context context, Boolean fullFlag) {
        super(context, fullFlag);
    }

    public FunVideoPlayer(Context context) {
        super(context);
    }

    public FunVideoPlayer(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public FunVideoPlayer(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    /******************************* 下面方法为管理器和播放控件交互的方法 ****************************************/

    @Override
    public FunMediaViewBridge getFunMediaManager() {
        FunMediaManager.instance().initContext(getContext().getApplicationContext());
        return FunMediaManager.instance();
    }

    @Override
    protected boolean backFromFull(Context context) {
        return FunMediaManager.backFromWindowFull(context);
    }

    @Override
    protected void releaseVideos() {
        FunMediaManager.releaseAllMedia();
    }

    @Override
    protected int getFullId() {
        return FunMediaManager.FULLSCREEN_ID;
    }

    @Override
    protected int getSmallId() {
        return FunMediaManager.SMALL_ID;
    }
}
