package com.hcn.media_view.recyclerview;

import static java.lang.Math.abs;

import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.recyclerview.widget.LinearSnapHelper;
import androidx.recyclerview.widget.OrientationHelper;
import androidx.recyclerview.widget.RecyclerView;

/**
 * 滑动顶端对齐辅助对象
 * @author 65821
 */
public class TopLinearSnapHelper extends LinearSnapHelper {
    private OrientationHelper mVerticalHelper = null;
    private OrientationHelper mHorizontalHelper = null;

    @Override
    public int[] calculateDistanceToFinalSnap(RecyclerView.LayoutManager layoutManager,
                                              View targetView) {
        int[] out = new int[2];
        if (layoutManager.canScrollHorizontally()) {
            out[0] = distanceToTop(targetView, getHorizontalHelper(layoutManager));
        }

        if (layoutManager.canScrollVertically()) {
            out[1] = distanceToTop(targetView, getVerticalHelper(layoutManager));
        }
        return out;
    }

    @Override
    public View findSnapView(RecyclerView.LayoutManager layoutManager) {
        if (layoutManager.canScrollVertically()) {
            return findTopView(layoutManager, getVerticalHelper(layoutManager));
        } else if (layoutManager.canScrollHorizontally()) {
            return findTopView(layoutManager, getHorizontalHelper(layoutManager));
        }
        return null;
    }

    /**
     * 到顶部的距离
     * @param targetView 目标视图
     * @param helper 方向辅助器对象
     * @return 距离（）
     */
    private int distanceToTop(@NonNull View targetView, OrientationHelper helper) {
        int childTop = helper.getDecoratedStart(targetView);
        int containerTop = helper.getStartAfterPadding();
        return childTop - containerTop;
    }

    /**
     * 获取最接近顶端的 Item 视图对象
     *
     * @param layoutManager 布局管理器
     * @param helper 方向辅助器对象
     * @return {@link View} ItemView
     */
    @Nullable
    private View findTopView(RecyclerView.LayoutManager layoutManager,
                             OrientationHelper helper) {
        int childCount = layoutManager.getChildCount();
        if (childCount == 0) {
            return null;
        }

        View closestChild = null;
        int left = helper.getStartAfterPadding();
        int absClosest = Integer.MAX_VALUE;
        for (int i = 0; i < childCount; i++) {
            View child = layoutManager.getChildAt(i);
            int childLeft = helper.getDecoratedStart(child);
            int absDistance = abs(childLeft - left);
            if (absDistance < absClosest) {
                absClosest = absDistance;
                closestChild = child;
            }
        }
        return closestChild;
    }

    @NonNull
    private OrientationHelper getVerticalHelper(@NonNull RecyclerView.LayoutManager layoutManager) {
        if (mVerticalHelper == null
                || mVerticalHelper.getLayoutManager() != layoutManager) {
            mVerticalHelper = OrientationHelper.createVerticalHelper(layoutManager);
        }
        return mVerticalHelper;
    }

    @NonNull
    private OrientationHelper getHorizontalHelper(@NonNull RecyclerView.LayoutManager layoutManager) {
        if (mHorizontalHelper == null
                || mHorizontalHelper.getLayoutManager() != layoutManager) {
            mHorizontalHelper = OrientationHelper.createHorizontalHelper(layoutManager);
        }
        return mHorizontalHelper;
    }
}
