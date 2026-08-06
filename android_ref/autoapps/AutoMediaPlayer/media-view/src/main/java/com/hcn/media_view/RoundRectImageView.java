package com.hcn.media_view;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Path;
import android.graphics.RectF;
import android.util.AttributeSet;

/**
 * @author 86158
 */
public class RoundRectImageView extends androidx.appcompat.widget.AppCompatImageView {
    /**
     * 默认的圆角大小，单位为 dp
     */
    private static final int DEFAULT_CORNER = 10;
    private float width, height;

    /**
     * 圆角半径
     */
    private final int mRadius;
    private final Path mPath;
    private final RectF mRect;

    public RoundRectImageView(Context context) {
        this(context, null);
    }

    public RoundRectImageView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public RoundRectImageView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        TypedArray typedArray = context.obtainStyledAttributes(attrs,
                R.styleable.RoundRectImageView);
        mRadius = (int) typedArray.getDimension(R.styleable.RoundRectImageView_radius,
                dp2px(context, DEFAULT_CORNER));
        typedArray.recycle();
        mPath = new Path();
        mRect = new RectF();
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        width = getWidth();
        height = getHeight();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        mPath.reset();
        mRect.set(0, 0, width, height);
        mPath.addRoundRect(mRect, mRadius, mRadius, Path.Direction.CW);
        canvas.clipPath(mPath);
        super.onDraw(canvas);
    }

    /**
     * dp转 px.
     *
     * @param value the value
     * @return the int
     */
    public static int dp2px(Context context, float value) {
        final float scale = context.getResources().getDisplayMetrics().densityDpi;
        return (int) (value * (scale / 160) + 0.5f);
    }
}
