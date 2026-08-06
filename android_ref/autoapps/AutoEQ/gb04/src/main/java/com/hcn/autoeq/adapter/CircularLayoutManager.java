package com.hcn.autoeq.adapter;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.hcn.autoeq.R;
import com.hcn_library.util.SkinUtils;


public class CircularLayoutManager extends LinearLayoutManager {
    private Context context;
    private boolean isTouchScroll = false;

    public CircularLayoutManager(Context context) {
        super(context);
        this.context = context;
    }

    public CircularLayoutManager(Context context, int orientation, boolean reverseLayout) {
        super(context, orientation, reverseLayout);
        this.context = context;
    }

    public CircularLayoutManager(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        this.context = context;
    }

    @Override
    public void onLayoutChildren(RecyclerView.Recycler recycler, RecyclerView.State state) {
        super.onLayoutChildren(recycler, state);
        if (getItemCount() == 0) {
            return;
        }
        // 确保只显示一个 item
        detachAndScrapAttachedViews(recycler);
        int itemCount = getItemCount();
        int position = 0;
        View view = recycler.getViewForPosition(position % itemCount);
        addView(view);
        measureChildWithMargins(view, 0, 0);
        int width = getDecoratedMeasuredWidth(view);
        int height = getDecoratedMeasuredHeight(view);
        layoutDecorated(view, 0, 0, width, height);
    }

    @Override
    public int scrollVerticallyBy(int dy, RecyclerView.Recycler recycler, RecyclerView.State state) {
        int scrolled = super.scrollVerticallyBy(dy, recycler, state);
        int itemCount = getItemCount();
        int toleranceDistance = (int) SkinUtils.getDimension(R.dimen.x2);
        Log.d("scrollVerticallyBy---------before", "findFirstVisibleItemPosition() = " + findFirstVisibleItemPosition() + "   findLastVisibleItemPosition() = " + findLastVisibleItemPosition() + " scrolled: " + scrolled + " dy: " + dy + " isTouchScroll: " + isTouchScroll);
        if (!isTouchScroll) {
            return scrolled;
        }
        if (dy < (0 - toleranceDistance) && findLastVisibleItemPosition() == 0) {
            // 向上滚动到顶部，将内容切换到最后一个 item
            detachAndScrapAttachedViews(recycler);
            int position = (itemCount - 1);
            View view = recycler.getViewForPosition(position);
            addView(view);
            measureChildWithMargins(view, 0, 0);
            int width = getDecoratedMeasuredWidth(view);
            int height = getDecoratedMeasuredHeight(view);
            layoutDecorated(view, 0, getHeight() - height, width, getHeight());
            scrolled = dy;
        } else if (dy > toleranceDistance && findFirstVisibleItemPosition() == (itemCount - 1)) {
            // 向下滚动到底部，将内容切换到第一个 item
            detachAndScrapAttachedViews(recycler);
            int position = 0;
            View view = recycler.getViewForPosition(position);
            addView(view);
            measureChildWithMargins(view, 0, 0);
            int width = getDecoratedMeasuredWidth(view);
            int height = getDecoratedMeasuredHeight(view);
            layoutDecorated(view, 0, 0, width, height);
            scrolled = dy;
        }
        return scrolled;
    }

    @Override
    public boolean canScrollVertically() {
        return true;
    }

    public void setTouchScroll(boolean isTouchScroll) {
        this.isTouchScroll = isTouchScroll;
    }

    public boolean getTouchScroll() {
        return isTouchScroll;
    }
}

