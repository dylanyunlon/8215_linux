package com.hcn.autoeq.view;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.HorizontalScrollView;

import com.hcn.autoeq.R;
import com.hcn_library.util.SkinUtils;

public class NineCustomHorizontalScrollView extends HorizontalScrollView {
    private static final String TAG = "CustomHorizontalScrollView";
    private ParentInterceptListener parentInterceptListener;
    private float startY;

    public interface ParentInterceptListener {
        void onVerticalScrollDetected();
    }

    public NineCustomHorizontalScrollView(Context context) {
        super(context);
    }

    public NineCustomHorizontalScrollView(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
    }

    public NineCustomHorizontalScrollView(Context context, AttributeSet attributeSet, int i) {
        super(context, attributeSet, i);
    }

    @Override 
    public boolean dispatchTouchEvent(MotionEvent motionEvent) {
        Log.d(TAG, "onInterceptTouchEvent event:  " + motionEvent);
        int action = motionEvent.getAction();
        if (action == MotionEvent.ACTION_DOWN) {
            startY = motionEvent.getY();
        } else if (action == MotionEvent.ACTION_MOVE) {
            float y = motionEvent.getY();
            float abs = Math.abs(y - startY);
            Log.d(TAG, "onInterceptTouchEvent currentY  " + y + " startY: " + startY);
            if (abs >  SkinUtils.getDimension(R.dimen.y260)) {
                if (y >= startY || parentInterceptListener == null) {
                    return false;
                }
                parentInterceptListener.onVerticalScrollDetected();
                return false;
            }
        }
        return super.dispatchTouchEvent(motionEvent);
    }

    public void setParentInterceptListener(ParentInterceptListener parentInterceptListener) {
        this.parentInterceptListener = parentInterceptListener;
    }
}