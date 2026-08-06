package com.hcn.media_view.widget;

import android.content.Context;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.animation.TranslateAnimation;
import android.widget.LinearLayout;

import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.util.Objects;

/**
 * 自定义带边界尼阻效果的滑动布局
 *
 * @Author youwj/65821
 * @Create 2021/4/15 17:02
 */
public class NestedScrollLayout extends LinearLayout {
    /**
     * 动画时间
     */
    private static final int ANIM_TIME_DURATION = 400;

    /**
     * 阻尼系数
     */
    private static final float DAMPING_COEFFICIENT = 0.3f;

    /**
     * 孩子视图
     */
    private RecyclerView mRecyclerView;

    /**
     * 原始位置
     */
    private final Rect mOriginalRect = new Rect();

    /**
     * 移动标记
     */
    private boolean mIsMoved = false;

    /**
     * 开始位置
     */
    private float mStartPosX = 0.0f;
    private float mStartPosY = 0.0f;

    /**
     * 触发成功
     */
    private boolean mIsSuccess = false;

    /**
     * 滑动回调
     */
    private IScrollListener mScrollListener;

    public interface IScrollListener {
        /**
         * 滚动事件回调
         */
        void onScrollEvent();
    }

    public NestedScrollLayout(Context context) {
        this(context, null);
    }

    public NestedScrollLayout(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public NestedScrollLayout(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mRecyclerView = (RecyclerView) getChildAt(0);
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        super.onLayout(changed, l, t, r, b);
        mOriginalRect.set(mRecyclerView.getLeft(),
                mRecyclerView.getTop(), mRecyclerView.getRight(), mRecyclerView.getBottom());
    }

    /**
     * 设置滑动事件监听
     *
     * @param listener 监督者
     */
    public void setScrollListener(IScrollListener listener) {
        mScrollListener = listener;
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        float touchPosY = ev.getY();
        if (touchPosY >= mOriginalRect.bottom || touchPosY <= mOriginalRect.top) {
            if (mIsMoved) {
                recoverLayout();
            }
            return true;
        }

        switch (ev.getAction()) {
            case MotionEvent.ACTION_DOWN:
                mStartPosX = ev.getX();
                mStartPosY = ev.getY();
            case MotionEvent.ACTION_MOVE: {
                int scrollPosX = (int) (ev.getX() - mStartPosX);
                int scrollPosY = (int) (ev.getY() - mStartPosY);

                boolean pullDown = scrollPosY > 0 && canPullDown();
                boolean pullUp = scrollPosY < 0 && canPullUp();
                if (pullDown || pullUp) {
                    cancelChild(ev);

                    int offset = (int) (scrollPosY * DAMPING_COEFFICIENT);
                    mRecyclerView.layout(mOriginalRect.left,
                            mOriginalRect.top + offset,
                            mOriginalRect.right,
                            mOriginalRect.bottom + offset);

                    mIsMoved = true;
                    mIsSuccess = false;

                    if (mScrollListener != null) {
                        mScrollListener.onScrollEvent();
                    }

                    getParent().requestDisallowInterceptTouchEvent(
                            Math.abs(scrollPosX) <= Math.abs(scrollPosY));
                    return true;
                }

                mStartPosY = ev.getY();
                mIsMoved = false;
                mIsSuccess = true;
                return super.dispatchTouchEvent(ev);
            }
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL: {
                if (mIsMoved) {
                    recoverLayout();
                }

                return !mIsSuccess || super.dispatchTouchEvent(ev);
            }
            default:
                return true;
        }
    }

    /**
     * 取消子 view 已经处理的事件
     *
     * @param ev event
     */
    private void cancelChild(MotionEvent ev) {
        ev.setAction(MotionEvent.ACTION_CANCEL);
        super.dispatchTouchEvent(ev);
    }

    /**
     * 布局位置还原
     * <p> 松手后回弹；
     */
    private void recoverLayout() {
        TranslateAnimation anim = new TranslateAnimation(
                0, 0, mRecyclerView.getTop() - mOriginalRect.top, 0);
        anim.setDuration(ANIM_TIME_DURATION);
        mRecyclerView.startAnimation(anim);
        mRecyclerView.layout(mOriginalRect.left,
                mOriginalRect.top, mOriginalRect.right, mOriginalRect.bottom);
        mIsMoved = false;
    }

    /**
     * 判断是否可以下拉
     *
     * @return 可以/不可以
     */
    private boolean canPullDown() {
        if (mRecyclerView.getAdapter() == null
                || mRecyclerView.getLayoutManager() == null) {
            return false;
        }

        final int firstVisiblePosition
                = ((LinearLayoutManager) Objects.requireNonNull(
                mRecyclerView.getLayoutManager())).findFirstVisibleItemPosition();
        if (firstVisiblePosition != 0
                && Objects.requireNonNull(
                mRecyclerView.getAdapter()).getItemCount() != 0) {
            return false;
        }

        int mostTop = mRecyclerView.getChildCount() > 0 ? mRecyclerView.getChildAt(0).getTop() : 0;
        return mostTop >= 0;
    }

    /**
     * 判断是否可以上拉
     *
     * @return 可以/不可以
     */
    private boolean canPullUp() {
        if (mRecyclerView.getAdapter() == null
                || mRecyclerView.getLayoutManager() == null) {
            return false;
        }

        final int lastItemPosition = Objects.requireNonNull(mRecyclerView.getAdapter()).getItemCount() - 1;
        final int lastVisiblePosition = ((LinearLayoutManager) Objects.requireNonNull(
                mRecyclerView.getLayoutManager())).findLastVisibleItemPosition();
        if (lastVisiblePosition >= lastItemPosition) {
            final int childIndex = lastVisiblePosition
                    - ((LinearLayoutManager) mRecyclerView.getLayoutManager()).findFirstVisibleItemPosition();
            final int childCount = mRecyclerView.getChildCount();
            final int index = Math.min(childIndex, childCount - 1);
            final View lastVisibleChild = mRecyclerView.getChildAt(index);
            if (lastVisibleChild != null) {
                return lastVisibleChild.getBottom() <= mRecyclerView.getBottom() - mRecyclerView.getTop();
            }
        }

        return false;
    }
}