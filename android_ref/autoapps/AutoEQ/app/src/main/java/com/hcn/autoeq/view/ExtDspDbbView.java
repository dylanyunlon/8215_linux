package com.hcn.autoeq.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import androidx.annotation.Nullable;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.SkinUtils;

public class ExtDspDbbView extends View {
    private static final String TAG = ExtDspDbbView.class.getSimpleName();
    private static final boolean DEBUG = /*BuildConfig.DEBUG ||*/ Log.isLoggable(ExtDspDbbView.class.getSimpleName(), Log.DEBUG);

    private static final int STROKE_WIDTH = 4; // 线条宽度（最好是双数）
    private static final float PER = 6f; // 参考点的 x y 坐标相对于其他点的比例

    private Paint mPaint, mPaintPoint;
    private Path mPath;
    private int mWidth, mHeight;
    private float mTouchX, mTouchY;
    private float initTouchX, initTouchY;

    private IExtDspDbbCallback iExtDspDbbCallback;

    public ExtDspDbbView(Context context) {
        super(context);
        init();
    }

    public ExtDspDbbView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public ExtDspDbbView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        mPaint = new Paint();
        mPaint.setColor(SkinUtils.getColor(R.color.ext_dsp_dbb_view_freq_curve_color));
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setStrokeWidth(STROKE_WIDTH);
        mPaint.setAntiAlias(true);

        if (DEBUG) {
            mPaintPoint = new Paint();
            mPaintPoint.setStyle(Paint.Style.STROKE);
            mPaintPoint.setStrokeWidth(10);
            mPaintPoint.setColor(Color.WHITE);
        }

        mPath = new Path();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        Log.d(TAG, "onMeasure");
        mWidth = getBackground().getIntrinsicWidth();
        mHeight = getBackground().getIntrinsicHeight();
        setMeasuredDimension(mWidth, mHeight);

        // 手动设置过位置，measure 时以手动设置的值为准
        if (initTouchX == mTouchX) {
            mTouchX = initTouchX;
        } else {
            mTouchX = mWidth / 2f;
        }
        if (initTouchY == mTouchY) {
            mTouchY = initTouchY;
        } else {
            mTouchY = mHeight;
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        mPath.reset();
        if (DEBUG) {
            canvas.drawPoint(mTouchX, mTouchY, mPaintPoint);
        }

        mPath.moveTo(0, mHeight);
        canvas.drawPath(mPath, mPaint);

        float offsetX = mWidth / PER; // 起始点/终点 与 触摸点 之间的 x 差值

        // 左边线的起始坐标
        float startX = mTouchX - offsetX;
        float startY = mHeight;
        mPath.lineTo(startX, startY);  //设置起点

        if (DEBUG) {
            canvas.drawPoint(startX, startY, mPaintPoint);
        }

        float quadX = mTouchX - offsetX / PER;
        float quadY = mHeight - ((mHeight - mTouchY) / PER);

        if (mTouchY < mHeight - STROKE_WIDTH) {
            if (DEBUG) {
                canvas.drawPoint(quadX, quadY, mPaintPoint);
            }

            // 顶点
            mPath.quadTo(quadX, quadY, mTouchX - 2, mTouchY + 2);
            canvas.drawPath(mPath, mPaint);

            mPath.quadTo(mTouchX, mTouchY, mTouchX + 2, mTouchY + 2);
            canvas.drawPath(mPath, mPaint);

            // 右边线
            quadX = mTouchX + offsetX / PER;

            if (DEBUG) {
                canvas.drawPoint(quadX, quadY, mPaintPoint);
            }
            mPath.quadTo(quadX, quadY, mTouchX + offsetX, mHeight);
            canvas.drawPath(mPath, mPaint);
        }

        mPath.lineTo(mWidth, mHeight);
        canvas.drawPath(mPath, mPaint);

        if (DEBUG) {
            canvas.drawPoint(mTouchX + offsetX, mHeight, mPaintPoint);
        }
        super.onDraw(canvas);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        float x = event.getX();
        float y = event.getY();

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                if (x >= 0 && x <= mWidth && y >= 0 && y <= mHeight) {
                    mTouchX = x;
                    mTouchY = y;
                    invalidate();
                }
                break;
            case MotionEvent.ACTION_MOVE:
                if (x >= 0 && x <= mWidth && y >= 0 && y <= mHeight) {
                    mTouchX = x;
                    mTouchY = y;
                    invalidate();
                    if (iExtDspDbbCallback != null) {
                        iExtDspDbbCallback.onActionMove(x, y);
                    }
                }
                break;
            case MotionEvent.ACTION_UP:
                if (x < 0) x = mTouchX;
                if (x > mWidth) x = mTouchX;
                if (y < 0) y = mTouchY;
                if (y > mHeight) y = mTouchY;

                if (iExtDspDbbCallback != null) {
                    iExtDspDbbCallback.onActionUp(x, y);
                }
                break;
        }
        return true;
    }

    public void setTouchPoint(float x, float y) {
        initTouchX = x;
        initTouchY = y;

        mTouchX = initTouchX;
        mTouchY = initTouchY;
        invalidate();
    }

    public void reset() {
        initTouchX = mWidth / 2f;
        initTouchY = mHeight;

        mTouchX = initTouchX;
        mTouchY = initTouchY;
        invalidate();
    }

    public float getTouchX() {
        return mTouchX;
    }

    public float getTouchY() {
        return mTouchY;
    }

    public void setExtDspDbbCallback(IExtDspDbbCallback iExtDspDbbCallback) {
        this.iExtDspDbbCallback = iExtDspDbbCallback;
    }

    public interface IExtDspDbbCallback {
        void onActionMove(float touchX, float touchY);

        void onActionUp(float touchX, float touchY);
    }
}
