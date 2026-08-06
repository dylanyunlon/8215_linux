package com.hcn.media_view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;

/**
 * 可滑动的 SeekBar
 *
 * @author 86158
 */
public class ScrollableSeekBar extends androidx.appcompat.widget.AppCompatSeekBar {

    boolean touchingProgressBar = true;

    public ScrollableSeekBar(Context context) {
        super(context);
    }

    public ScrollableSeekBar(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public ScrollableSeekBar(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public void setTouchingProgressBar(boolean touchingProgressBar) {
        this.touchingProgressBar = touchingProgressBar;
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (touchingProgressBar) {
            super.onTouchEvent(event);

            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    System.out.println("MotionEvent.ACTION_DOWN");
                    // 请求触摸事件不被打断
                    getParent().requestDisallowInterceptTouchEvent(true);
                    touchingProgressBar = true;
                    break;
                case MotionEvent.ACTION_UP:
                    System.out.println("MotionEvent.ACTION_UP");
                    // 当结束滑动时请求触摸事件可以被打断
                    getParent().requestDisallowInterceptTouchEvent(false);
                    touchingProgressBar = true;
                    break;
                case MotionEvent.ACTION_CANCEL:
                    getParent().requestDisallowInterceptTouchEvent(false);
                    break;
                default:
                    break;
            }
        }
        return true;
    }

    /**
     * 分派触摸事件
     * <p> 此处非常重要， 返回 true，后续事件（ACTION_MOVE、ACTION_UP）会再传递;
     *
     * @param event
     * @return
     */
    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        System.out.println("dispatchTouchEvent");
        super.dispatchTouchEvent(event);
        return true;
    }
}