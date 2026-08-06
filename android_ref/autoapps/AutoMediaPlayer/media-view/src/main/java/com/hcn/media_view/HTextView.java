package com.hcn.media_view;

import android.content.Context;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.widget.TextView;

/**
 * TextView
 * <p> 处理焦点问题；
 *
 * @author 86158
 */
public class HTextView extends androidx.appcompat.widget.AppCompatTextView {

    private boolean mFocused = true;

    public HTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void setFocusEx(boolean bFocused) {
        mFocused = bFocused;
    }

    @Override
    protected void onFocusChanged(boolean focused, int direction, Rect previouslyFocusedRect) {
        if (focused) {
            super.onFocusChanged(true, direction, previouslyFocusedRect);
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        if (hasWindowFocus) {
            super.onWindowFocusChanged(true);
        }
    }

    @Override
    public boolean isFocused() {
        return mFocused;
    }
}
