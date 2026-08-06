package com.hcn.media_view.recyclerview;

import android.content.Context;
import android.graphics.PointF;
import android.util.DisplayMetrics;

import androidx.annotation.Nullable;
import androidx.recyclerview.widget.LinearSmoothScroller;

/**
 * 线性平滑滚动条扩展
 * @author 65821
 */
public class TopLinearSmoothScroller extends LinearSmoothScroller {
    public TopLinearSmoothScroller(Context context) {
        super(context);
    }

    @Override
    protected int getHorizontalSnapPreference() {
        return SNAP_TO_START;
    }

    /**
     * 垂直捕捉首选项
     * <p> 将子 view 与父 view 顶部对齐
     *
     * @return 子视图顶部与父视图顶部对齐
     */
    @Override
    protected int getVerticalSnapPreference() {
        return SNAP_TO_START;
    }

    @Nullable
    @Override
    public PointF computeScrollVectorForPosition(int targetPosition) {
        return super.computeScrollVectorForPosition(targetPosition);
    }

    /**
     * 获取滑动一个 pixel 需要多少毫秒
     *
     * @param displayMetrics 显示度量指标对象
     * @return 毫秒
     */
    @Override
    protected float calculateSpeedPerPixel(DisplayMetrics displayMetrics) {
        return 2.5F / displayMetrics.density;
    }
}
