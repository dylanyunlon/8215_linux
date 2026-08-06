package com.hcn.autoradio.view;

import java.math.BigDecimal;

import com.hcn.autoradio.ScreenSpec;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RemoteViews.RemoteView;
import android.widget.Scroller;

@RemoteView
public class FMFreeLayout extends ViewGroup {

    private boolean mIsMoving = false;
    private Scroller mAutoScroller = null;
    private int mMoveDuration = 0;

    private int mCurrCenterX = 0;
    private int mCurrCenterY = 0;
    private IScrollerListener mScrollerListener = null;

    private int mCurrTouchX = 0;
    private int mCurrTouchY = 0;

    public FMFreeLayout(Context context) {
        super(context);

        initRadiolayout(context);
    }

    public FMFreeLayout(Context context, AttributeSet attrs) {
        super(context, attrs);

        initRadiolayout(context);
    }

    public FMFreeLayout(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

        initRadiolayout(context);
    }

    private void initRadiolayout(Context context) {

        if (null == mAutoScroller) {
            mAutoScroller = new Scroller(context);
        }
    }

    public void setMoveDuration(int nMillisecond) {
        mMoveDuration = nMillisecond;
    }

    public boolean isMoving() {
        return mIsMoving;
    }

    public void setMoving(boolean mIsMoving) {
        this.mIsMoving = mIsMoving;
    }

    public void setOnScrollListener(IScrollerListener il) {

        if (null != il) {
            mScrollerListener = il;
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int count = getChildCount();

        int maxHeight = 0;
        int maxWidth = 0;

        measureChildren(widthMeasureSpec, heightMeasureSpec);

        for (int i = 0; i < count; i++) {
            View child = getChildAt(i);
            if (child.getVisibility() != GONE) {
                int childRight;
                int childBottom;

                FMFreeLayout.LayoutParams lp = (FMFreeLayout.LayoutParams) child
                        .getLayoutParams();

                childRight = lp.x + child.getMeasuredWidth();
                childBottom = lp.y + child.getMeasuredHeight();

                maxWidth = Math.max(maxWidth, childRight);
                maxHeight = Math.max(maxHeight, childBottom);
            }
        }

        maxWidth += getPaddingLeft() + getPaddingRight();
        maxHeight += getPaddingTop() + getPaddingBottom();

        maxHeight = Math.max(maxHeight, getSuggestedMinimumHeight());
        maxWidth = Math.max(maxWidth, getSuggestedMinimumWidth());

        setMeasuredDimension(
                resolveSizeAndState(maxWidth, widthMeasureSpec, 0),
                resolveSizeAndState(maxHeight, heightMeasureSpec, 0));
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {

        if (MotionEvent.ACTION_MOVE == event.getAction()) {
            mCurrTouchX = (int) event.getRawX();
            mCurrTouchY = (int) event.getRawY();
        }

        return super.onTouchEvent(event);
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {

        return super.onInterceptTouchEvent(ev);
    }

    @Override
    protected ViewGroup.LayoutParams generateDefaultLayoutParams() {
        return new LayoutParams(LayoutParams.WRAP_CONTENT,
                LayoutParams.WRAP_CONTENT, 0, 0);
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        int count = getChildCount();

        for (int i = 0; i < count; i++) {
            View child = getChildAt(i);
            if (child.getVisibility() != GONE) {

                FMFreeLayout.LayoutParams lp = (FMFreeLayout.LayoutParams) child
                        .getLayoutParams();

                int childLeft = getPaddingLeft() + lp.x;
                int childTop = getPaddingTop() + lp.y;
                child.layout(childLeft, childTop,
                        childLeft + child.getMeasuredWidth(),
                        childTop + child.getMeasuredHeight());

            }
        }
    }

    @Override
    public ViewGroup.LayoutParams generateLayoutParams(AttributeSet attrs) {
        return new FMFreeLayout.LayoutParams(getContext(), attrs);
    }

    // Override to allow type-checking of LayoutParams.
    @Override
    protected boolean checkLayoutParams(ViewGroup.LayoutParams p) {
        return p instanceof FMFreeLayout.LayoutParams;
    }

    @Override
    protected ViewGroup.LayoutParams generateLayoutParams(
            ViewGroup.LayoutParams p) {
        return new LayoutParams(p);
    }

    @Override
    public boolean shouldDelayChildPressedState() {
        return false;
    }

    public void addComponentView(FMFreeLayout component) {

        addView(component);
    }

    @Override
    public void computeScroll() {
        super.computeScroll();

        if (mAutoScroller.computeScrollOffset()) {
            mCurrCenterX = mAutoScroller.getCurrX();
            mCurrCenterY = mAutoScroller.getCurrY();

            setCenterPosition(mCurrCenterX, mCurrCenterY);

            if (null != mScrollerListener) {
                mScrollerListener.scrollProcess(mAutoScroller);
            }

            invalidate();
        }

        if (mAutoScroller.isFinished()) {
            if (isMoving()) {
                setMoving(false);
                if (null != mScrollerListener) {
                    mScrollerListener.scrollEnd();
                }
            }
        }
    }

    public boolean setSizeByCenter(int width, int height) {

        LayoutParams layoutParams = (LayoutParams) getLayoutParams();

        if (null != layoutParams) {

            if ((layoutParams.width != width)
                    || (layoutParams.height != height)) {

                layoutParams.x += ((layoutParams.width - width) >> 1);
                layoutParams.y += ((layoutParams.height - height) >> 1);
                layoutParams.width = width;
                layoutParams.height = height;
                layout(layoutParams);

                return true;
            }
        }

        return false;
    }

    public void setTargetCenterPosAnimated(int nCenterX, int nCenterY) {

        mAutoScroller.forceFinished(true);
        LayoutParams layoutParams = (LayoutParams) getLayoutParams();

        if (null != layoutParams) {

            mCurrCenterX = (layoutParams.x + (layoutParams.width >> 1));
            mCurrCenterY = (layoutParams.y + (layoutParams.height >> 1));

            int nMoveOffsetX = nCenterX - mCurrCenterX;
            int nMoveOffsetY = nCenterY - mCurrCenterY;

            setMoving(true);
            this.mAutoScroller.startScroll(mCurrCenterX, mCurrCenterY,
                    nMoveOffsetX, nMoveOffsetY, mMoveDuration);

            if (null != mScrollerListener) {
                mScrollerListener.scrollStart();
            }

            invalidate();
        }
    }

    public boolean setCenterPosition(int nPointX, int nPointY) {

        LayoutParams layoutParams = (LayoutParams) getLayoutParams();

        if (null != layoutParams) {

            int nPosX = nPointX - (layoutParams.width >> 1);
            int nPosY = nPointY - (layoutParams.height >> 1);

            nPosX = Math.max(nPosX, 0);
            nPosY = Math.max(nPosY, 0);
            nPosX = (int) Math.min(nPosX, ScreenSpec.mScreenWidth*ScreenSpec.mScreenDensity - layoutParams.width);
            nPosY = (int) Math.min(nPosY, ScreenSpec.mScreenHeight*ScreenSpec.mScreenDensity - layoutParams.height);

            if ((layoutParams.x != nPosX) || (layoutParams.y != nPosY)) {

                layoutParams.x = nPosX;
                layoutParams.y = nPosY;
                layout(layoutParams);

                return true;
            }
        }

        return false;
    }

    protected final void layout(LayoutParams layoutParams) {

        if (layoutParams instanceof FMFreeLayout.LayoutParams) {

            layout(layoutParams.x, layoutParams.y, layoutParams.x
                    + layoutParams.width, layoutParams.y + layoutParams.height);
        }
    }

    public static class LayoutParams extends ViewGroup.LayoutParams {

        public int x = 0;
        public int y = 0;

        public LayoutParams() {
            super(WRAP_CONTENT, WRAP_CONTENT);

            this.x = 0;
            this.y = 0;
        }

        public LayoutParams(int width, int height) {
            super(width, height);

            this.x = 0;
            this.y = 0;
        }

        public LayoutParams(int width, int height, int x, int y) {
            super(width, height);

            this.x = x;
            this.y = y;
        }

        public LayoutParams(Context c, AttributeSet attrs) {
            super(c, attrs);

        }

        public LayoutParams(ViewGroup.LayoutParams source) {
            super(source);
        }

        protected static String sizeToString(int property) {

            if (property == ViewGroup.LayoutParams.WRAP_CONTENT) {
                return "wrap-content";
            }
            if (property == ViewGroup.LayoutParams.MATCH_PARENT) {
                return "match-parent";
            }

            return String.valueOf(property);
        }

        public String debug(String output) {
            return output + "Absolute.LayoutParams={width="
                    + sizeToString(width) + ", height=" + sizeToString(height)
                    + " x=" + x + " y=" + y + "}";
        }
    }
}
