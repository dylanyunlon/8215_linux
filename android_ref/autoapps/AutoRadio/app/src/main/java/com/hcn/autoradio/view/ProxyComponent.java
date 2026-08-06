package com.hcn.autoradio.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.MeasureSpec;

public class ProxyComponent extends FMFreeLayout {

    public ProxyComponent(Context context) {
        super(context);
    }

    public ProxyComponent(Context context, AttributeSet attr) {
        super(context, attr);
    }

    public ProxyComponent(Context context, AttributeSet attr, int defStyle) {
        super(context, attr, defStyle);
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {

        int nCount = getChildCount();
        int nWidth = r - l;
        int nHeight = b - t;

        int nPaddingL = getPaddingLeft();
        int nPaddingT = getPaddingTop();
        int nPaddingR = getPaddingRight();
        int nPaddingB = getPaddingBottom();

        for (int i = 0; i < nCount; i++) {

            View childView = getChildAt(i);

            if (childView.getVisibility() == View.GONE) {
                continue;
            }

            int nW = nWidth - nPaddingR;
            int nH = nHeight - nPaddingB;

            if ((childView.getWidth() != nW) || (childView.getHeight() != nH)) {

                int widthMeasureSpec = View.MeasureSpec.makeMeasureSpec(nW, MeasureSpec.AT_MOST);
                int heightMeasureSpec = View.MeasureSpec.makeMeasureSpec(nH, MeasureSpec.AT_MOST);

                childView.forceLayout();
                childView.measure(widthMeasureSpec, heightMeasureSpec);
            }

            childView.layout(nPaddingL, nPaddingT, nPaddingL + nW, nPaddingT + nH);
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {

        int nCount = getChildCount();
        int nMaxRight = 0;
        int nMaxBottom = 0;

        for (int i = 0; i < nCount; i++) {

            View childView = getChildAt(i);

            if (childView.getVisibility() == View.GONE) {
                continue;
            }

            measureChild(childView, widthMeasureSpec, heightMeasureSpec);
            FMFreeLayout.LayoutParams layoutParams = (FMFreeLayout.LayoutParams) childView
                    .getLayoutParams();

            layoutParams.width = childView.getMeasuredWidth();
            layoutParams.height = childView.getMeasuredHeight();

            int nRight = layoutParams.x + layoutParams.width;
            int nBottom = layoutParams.y + layoutParams.height;

            nMaxRight = Math.max(nMaxRight, nRight);
            nMaxBottom = Math.max(nMaxBottom, nBottom);
        }

        int nWidth = Math.max(nMaxRight
                        + (getPaddingLeft() + getPaddingRight()),
                getSuggestedMinimumWidth());
        int nHeight = Math.max(nMaxBottom
                        + (getPaddingTop() + getPaddingBottom()),
                getSuggestedMinimumHeight());

        setMeasuredDimension(
                resolveSize(nWidth, widthMeasureSpec),
                resolveSize(nHeight, heightMeasureSpec));
    }

}