package com.hcn.media.adapter.base;

import android.graphics.Rect;
import android.view.View;

import androidx.recyclerview.widget.RecyclerView;

/**
 * Wallpaper List 选项装饰
 * <p> 计算 Item 之间的间隙；
 * @author 65821
 */
public class WallpaperItemDecoration extends RecyclerView.ItemDecoration {
    private int mSpace;

    public WallpaperItemDecoration(int space) {
        this.mSpace = space;
    }

    /**
     * 获取 Item 装饰边界
     *
     * @param outRect 接收输出的 Rect
     * @param view 要装饰的子视图
     * @param parent 当前正在装饰的 RecyclerView 视图
     * @param state RecyclerView 当前状态
     */
    @Override
    public void getItemOffsets(Rect outRect, View view,
                               RecyclerView parent, RecyclerView.State state) {
        outRect.top = 0;
        outRect.bottom = 0;
        outRect.left = 0;
        outRect.right = mSpace;
    }
}
