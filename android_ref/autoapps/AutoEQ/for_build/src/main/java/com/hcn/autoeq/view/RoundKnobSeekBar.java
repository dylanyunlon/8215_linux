package com.hcn.autoeq.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;

/**
 * 圆形旋转的进度控件
 */
public class RoundKnobSeekBar extends View {

    public RoundKnobSeekBar(Context context) {
        this(context, null);
    }

    public RoundKnobSeekBar(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public RoundKnobSeekBar(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        setSaveEnabled(true);
        setLayerType(LAYER_TYPE_SOFTWARE, null);
    }

}
