package com.hcn.media_view.recyclerview;

import android.util.Log;

import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.hcn.media_view.uitls.Utils;

import java.util.Objects;

/**
 * RecyclerView 工具类
 * @author 65821
 */
public class RecyclerViewUtils {
    private static final boolean DEBUG = Utils.isDebugVersion();
    private static final String TAG = RecyclerViewUtils.class.getSimpleName();

    /**
     * 移动 RecyclerView 到指定位置，
     *
     * @param manager 设置 RecyclerView 对应的 manager
     * @param recyclerView 当前的 RecyclerView
     * @param position 要跳转的位置
     */
    public static void moveToPosition(LinearLayoutManager manager,
                                      RecyclerView recyclerView,
                                      int position) {
        if (Objects.isNull(recyclerView)) {
            return;
        }

        // 先获取显示区域
        int firstItem = manager.findFirstVisibleItemPosition();
        int lastItem = manager.findLastVisibleItemPosition();

        if (DEBUG) {
            Log.v(TAG, "moveToPosition: "
                    + position + " | " + firstItem + "-" + lastItem);
        }

        // 直接设置显示位置
        if (position >= firstItem && position <= lastItem) {
            recyclerView.scrollToPosition(position);
        } else {
            int temp = (lastItem - firstItem)/2;
            temp = position - temp;
            position = Math.max(temp, 0);
            manager.scrollToPositionWithOffset(position, 0);
        }
    }

    /**
     * 平滑移动 RecyclerView 到指定位置，
     *
     * @param manager 设置 RecyclerView 对应的 manager
     * @param recyclerView 当前的 RecyclerView
     * @param position 要跳转的位置
     */
    public static void smoothScrollToPosition(LinearLayoutManager manager,
                                              RecyclerView recyclerView,
                                              int position) {
        if (Objects.isNull(recyclerView)) {
            return;
        }

        // 先获取显示区域
        int firstItem = manager.findFirstVisibleItemPosition();
        int lastItem = manager.findLastVisibleItemPosition();

        if (DEBUG) {
            Log.v(TAG, "smoothScrollToPosition: "
                    + position + "|" + firstItem + "-" + lastItem);
        }

        // 直接设置显示位置
        if (position >= firstItem && position <= lastItem) {
            recyclerView.scrollToPosition(position);
        } else {
            int temp;
            boolean needCorrectionStatus = false;
            if (position + 1 == firstItem) {
                temp = firstItem - 1;
                position = Math.max(temp, 0);
                manager.smoothScrollToPosition(recyclerView, null, position);
                needCorrectionStatus = true;
            } else if (position - 1 == lastItem) {
                temp = firstItem + 1;
                position = Math.min(temp, Objects.requireNonNull(
                        recyclerView.getAdapter()).getItemCount() - 1);
                manager.smoothScrollToPosition(recyclerView, null, position);
                needCorrectionStatus = true;
            } else {
                // 如果能置顶就置顶显示
                recyclerView.scrollToPosition(position);
            }

            //修正 smoothScrollToPosition 的状态（不然可能出现平滑过渡不走最优路径问题）
            if (needCorrectionStatus) {
                Objects.requireNonNull(recyclerView.getAdapter())
                        .notifyItemRangeChanged(position, recyclerView.getChildCount());
            }
        }
    }
}
