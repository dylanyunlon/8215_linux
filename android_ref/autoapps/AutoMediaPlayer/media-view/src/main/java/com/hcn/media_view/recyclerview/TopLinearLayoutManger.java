package com.hcn.media_view.recyclerview;

import android.content.Context;

import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

/**
 * 线性布局管理扩展
 * @author 65821
 */
public class TopLinearLayoutManger extends LinearLayoutManager {
    private Context mContext;

    public TopLinearLayoutManger(Context context, int orientation, boolean reverseLayout) {
        super(context, orientation, reverseLayout);
        mContext = context;
    }

    @Override
    public void smoothScrollToPosition(RecyclerView recyclerView, RecyclerView.State state, int position) {
        TopLinearSmoothScroller topSmoothScroller = new TopLinearSmoothScroller(mContext);
        topSmoothScroller.setTargetPosition(position);
        startSmoothScroll(topSmoothScroller);
    }
}
