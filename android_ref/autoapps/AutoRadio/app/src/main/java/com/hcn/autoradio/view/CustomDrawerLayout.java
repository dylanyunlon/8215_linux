package com.hcn.autoradio.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.view.GravityCompat;
import androidx.core.view.ViewCompat;
import androidx.customview.widget.ViewDragHelper;
import androidx.drawerlayout.widget.DrawerLayout;

import java.lang.reflect.Field;

public class CustomDrawerLayout extends DrawerLayout {
    public CustomDrawerLayout(@NonNull Context context) {
        super(context);
    }

    public CustomDrawerLayout(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public CustomDrawerLayout(@NonNull Context context, @Nullable AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        if (ev.getAction() == MotionEvent.ACTION_DOWN) {
            final float x = ev.getX();
            final float y = ev.getY();
            final View touchedView = findTopChildUnder((int) x, (int) y);
            if (touchedView != null && isContentView(touchedView)
                    && this.isDrawerOpen(GravityCompat.END)) {
                return false;
            }
            //抽屉菜单界面横向滑动时拦截不让抽屉收起
            if (touchedView != null && isDrawerView(touchedView)
                    && this.isDrawerOpen(GravityCompat.END)) {
                return false;
            }
        }
        return super.onInterceptTouchEvent(ev);
    }

    /**
     * 判断点击位置是否位于相应的View内
     *
     * @param x x坐标
     * @param y y坐标
     * @return 返回当前view
     */
    public View findTopChildUnder(int x, int y) {
        final int childCount = getChildCount();
        for (int i = childCount - 1; i >= 0; i--) {
            final View child = getChildAt(i);
            if (x >= child.getLeft() && x < child.getRight() &&
                    y >= child.getTop() && y < child.getBottom()) {
                return child;
            }
        }
        return null;
    }

    /**
     * 判断点击触摸点的View是否是ContentView(即是主页面的View)
     *
     * @param child 子view
     * @return boolean
     */
    boolean isContentView(View child) {
        return ((LayoutParams) child.getLayoutParams()).gravity == Gravity.NO_GRAVITY;
    }

    /**
     * 判断点击触摸点的View是否是DrawerView(即是侧边栏的View)
     *
     * @param child 子view
     * @return boolean
     */
    boolean isDrawerView(View child) {
        final int gravity = ((LayoutParams) child.getLayoutParams()).gravity;
        final int absGravity = GravityCompat.getAbsoluteGravity(gravity,
                ViewCompat.getLayoutDirection(child));
        if ((absGravity & Gravity.START) != 0) {
            // This child is a left-edge drawer
            return true;
        }
        if ((absGravity & Gravity.END) != 0) {
            // This child is a right-edge drawer
            return true;
        }
        return false;
    }

    /**
     * 设置左侧拖动器边缘感应大小
     * <pre>
     *     注意 DrawerLayout 的 onLayout 会调整边缘大小到默认值；
     *     所以，最好视图在初始化的时候就调整好触摸边界的感应默认值大小；
     * </pre>
     *
     * @param newEdgeSize 边缘感应大小
     */
    public void setLeftDragEdgeSize(int newEdgeSize) {
        try {
            // 左侧拖拽器
            Field draggerField = DrawerLayout.class.getDeclaredField("mLeftDragger");
            draggerField.setAccessible(true);
            ViewDragHelper vdh = (ViewDragHelper) draggerField.get(this);

            // 设置边缘值
            Field edgeSizeField = ViewDragHelper.class.getDeclaredField("mEdgeSize");
            edgeSizeField.setAccessible(true);
            edgeSizeField.setInt(vdh, newEdgeSize);

        } catch (Exception ignored) {
            ignored.printStackTrace();
        }
    }

    /**
     * 设置右侧拖动器边缘感应大小
     * <pre>
     *     注意 DrawerLayout 的 onLayout 会调整边缘大小到默认值；
     *     所以，最好视图在初始化的时候就调整好触摸边界的感应默认值大小；
     * </pre>
     *
     * @param newEdgeSize 边缘感应大小
     */
    public void setRightDragEdgeSize(int newEdgeSize) {
        try {
            // 左侧拖拽器
            Field draggerField = DrawerLayout.class.getDeclaredField("mRightDragger");
            draggerField.setAccessible(true);
            ViewDragHelper vdh = (ViewDragHelper) draggerField.get(this);

            // 设置边缘值
            Field edgeSizeField = ViewDragHelper.class.getDeclaredField("mEdgeSize");
            edgeSizeField.setAccessible(true);
            edgeSizeField.setInt(vdh, newEdgeSize);

        } catch (Exception ignored) {
            ignored.printStackTrace();
        }
    }
}
