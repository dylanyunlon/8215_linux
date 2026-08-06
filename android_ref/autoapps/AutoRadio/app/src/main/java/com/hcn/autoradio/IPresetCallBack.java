package com.hcn.autoradio;

import android.view.View;

public interface IPresetCallBack {

    public View[] getCurrentPagePresets();

    public void setPresetViewEnabled(boolean bEnabled);

    public void setPresetViewAimed(int nIndex, boolean bAimed);
}
