package com.hcn.media_view.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.WindowInsets;
import android.widget.FrameLayout;

/**
 * 特殊帧布局(结合 Fragment 使用)
 * <pre>
 *    处理多个 Fragment 沉浸需求；
 *    多个 Fragment 布局文件设置 android:fitsSystemWindows=true 后，
 *    只有第一个设置的 Fragment 生效的问题，其他 Fragment 都是无效的问题；
 * </pre>
 *
 * @author 65821
 */
public class WindowInsetsFrameLayout extends FrameLayout {
    public WindowInsetsFrameLayout(Context context) {
        this(context, null);
    }

    public WindowInsetsFrameLayout(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public WindowInsetsFrameLayout(Context context, AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    public WindowInsetsFrameLayout(Context context,
                                   AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);

        setOnHierarchyChangeListener(new OnHierarchyChangeListener() {
            @Override
            public void onChildViewAdded(View parent, View child) {
                requestApplyInsets();
            }

            @Override
            public void onChildViewRemoved(View parent, View child) {
            }
        });
    }

    @Override
    public WindowInsets onApplyWindowInsets(WindowInsets insets) {
        int childCount = getChildCount();
        for (int index = 0; index < childCount; index++) {
            getChildAt(index).dispatchApplyWindowInsets(insets);
        }
        return insets;
    }
}
