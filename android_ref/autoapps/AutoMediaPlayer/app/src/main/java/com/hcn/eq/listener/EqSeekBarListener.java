package com.hcn.eq.listener;

public interface EqSeekBarListener {
    public abstract void onMotionBegin();


    public abstract void onMotionChanged(int uId, int nIndex, boolean bUpdate);

    public abstract void onMotionFinished();


    public abstract void onSetValue(int nSource, Object object, boolean bUpdate);
}