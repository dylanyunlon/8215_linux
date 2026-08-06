package com.hcn.media_view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import androidx.viewpager.widget.ViewPager;

/**
 * @author 86158
 */
public class CustomClickScrollViewPager extends ViewPager {

    private boolean noScroll = false;

    public CustomClickScrollViewPager(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public CustomClickScrollViewPager(Context context) {
        super(context);
    }

    // 控制是否可滑动
    public void setNoScroll(boolean noScroll) {
        this.noScroll = noScroll;
    }

    @Override
    public void scrollTo(int x, int y) {
        super.scrollTo(x, y);
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouchEvent(MotionEvent arg0) {
        if (noScroll) {
            return false;
        } else {
            return super.onTouchEvent(arg0);
        }
    }

    // 请求允许打断滑动或不允许打断该组件的滑动事件
    @Override
    public boolean onInterceptTouchEvent(MotionEvent arg0) {

        if (arg0.getAction() == MotionEvent.ACTION_MOVE){
            return true;
        }

        if (noScroll){
            getParent().requestDisallowInterceptTouchEvent(false);
            return false;
        } else{
            return super.onInterceptTouchEvent(arg0);
        }
    }

    @Override
    public void setCurrentItem(int item, boolean smoothScroll) {
        super.setCurrentItem(item, smoothScroll);
    }

    @Override
    public void setCurrentItem(int item) {
        super.setCurrentItem(item);
    }
}