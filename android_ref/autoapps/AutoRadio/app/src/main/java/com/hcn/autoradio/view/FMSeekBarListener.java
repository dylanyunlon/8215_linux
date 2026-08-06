package com.hcn.autoradio.view;

public interface FMSeekBarListener {

    public abstract void onMotionBegin();


    public abstract void onMotionChanged(int uId, int nIndex, boolean bUpdate);


    public abstract void onScrollThumbCenterLocation(int cx, int cy);

    public abstract void onMotionFinished();


    public abstract void onSetValue(int nSource, Object object, boolean bUpdate);
}