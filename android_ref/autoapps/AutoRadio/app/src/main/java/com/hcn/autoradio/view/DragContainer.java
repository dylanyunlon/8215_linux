package com.hcn.autoradio.view;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

public class DragContainer extends ProxyComponent {

    private View view = null;
    private boolean mTouchEnabled = true;

    public DragContainer(Context context) {
        super(context);
    }

    public DragContainer(Context context, AttributeSet attr) {
        super(context, attr);
    }

    public DragContainer(Context context, AttributeSet attr, int defStyle) {
        super(context, attr, defStyle);
    }

    public void deleteView() {
        removeView(this.view);
    }

    public boolean canTouchEnabled() {
        return mTouchEnabled;
    }

    public void setTouchEnabled(boolean bTouchEnabled) {
        mTouchEnabled = bTouchEnabled;
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {

        return super.onInterceptTouchEvent(ev);
    }


    public void setView(View view) {

        if (this.view != null) {
            deleteView();
        }

        this.view = view;
        addView(view, new FMFreeLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
    }

    public void setView(View view, FMFreeLayout.LayoutParams layoutParams) {
        this.view = view;

        addView(view, layoutParams);
    }
}