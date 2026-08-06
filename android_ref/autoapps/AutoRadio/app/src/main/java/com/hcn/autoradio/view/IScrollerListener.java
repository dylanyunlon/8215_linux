package com.hcn.autoradio.view;

import android.widget.Scroller;

public interface IScrollerListener {

    public void scrollStart();

    public void scrollProcess(Scroller scroller);

    public void scrollEnd();
}
