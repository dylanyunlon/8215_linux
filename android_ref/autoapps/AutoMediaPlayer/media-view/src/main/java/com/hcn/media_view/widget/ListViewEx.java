package com.hcn.media_view.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.ListView;

/**
 * 列表视图增强
 * <p> 主要是为了方便查问题；
 *
 * @author 65821
 */
public class ListViewEx extends ListView {
    private static final String TAG = ListViewEx.class.getSimpleName();
    private static boolean DEBUG = false;

    /** 调试控制 **/
    public static void setDebug(boolean debug) {
        DEBUG = debug;
    }

    public ListViewEx(Context context) {
        super(context);
    }

    public ListViewEx(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public ListViewEx(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public ListViewEx(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        boolean intercepted = super.onInterceptTouchEvent(ev);
        if (DEBUG) {
            Log.v(TAG, "onInterceptTouchEvent: " +
                    "Action = " + ev.getAction() + ", intercepted = " + intercepted);
        }

        return intercepted;
    }
}
