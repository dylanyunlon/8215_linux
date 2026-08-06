package com.autochips.bluetooth.view;


import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.widget.SeekBar;

public class HcnSeekBar extends SeekBar {

    public HcnSeekBar(Context context) {
        super(context);
    }

    public HcnSeekBar(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public HcnSeekBar(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }


    @Override
    public boolean onTouchEvent(MotionEvent event) {
        return false;
    }

}
