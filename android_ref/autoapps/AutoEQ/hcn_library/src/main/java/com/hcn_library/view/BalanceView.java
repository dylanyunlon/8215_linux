package com.hcn_library.view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PaintFlagsDrawFilter;
import android.graphics.Point;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;

import com.hcn_library.hcn_library.R;
import com.hcn_library.util.BitmapUtils;
import com.hcn_library.util.EqUtils;

@SuppressLint("AppCompatCustomView")
public class BalanceView extends ImageView implements View.OnTouchListener, Runnable {

    private static String TAG = BalanceView.class.getSimpleName();
    private float mCurrentX = 0.0F;
    private float mCurrentY = 0.0F;

    private Point mCurrLevel = new Point(-1, -1);
    private Point mTempLevel = new Point(-1, -1);
    private Bitmap mThumbBitmap = null;

    private Drawable mThumbDrawable = null;
    private boolean mIsMoveBalance = false;
    private boolean mHavePresetValue = false;
    private PaintFlagsDrawFilter mDrawFilter = null;
    private EqBalanceDrag mEqBalanceDrag = null;
    private long mLastTime = 0;
    private boolean mAnimationFlag = false;
    private Thread mAnimationThread = null;
    private Context mContext = null;
    private RedrawHandle mRedrawHandle = null;
    private BalanceViewCallback mBalanceViewCallback = null;
    private int mInitBalanceX = 0;
    private int mInitBalanceY = 0;
    private int mThumbWidth = 0;
    private int mThumbHeight = 0;
    private float mThumbUnitGapX = 0.0F;
    private float mThumbUnitGapY = 0.0F;
    private int mBalanceMax = 0;
    private int mBalanceMin = 0;
    private boolean horizontalOnly = false; // 只允许横向移动

    //BALANCE_DEPTH长度：
    private int balanceDepth = 0;


    public BalanceView(Context context) {
        super(context);
    }

    public BalanceView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public BalanceView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mContext = context;
        mRedrawHandle = new RedrawHandle();
        mDrawFilter = new PaintFlagsDrawFilter(0, Paint.ANTI_ALIAS_FLAG
                | Paint.FILTER_BITMAP_FLAG);

        TypedArray typedArray = context.obtainStyledAttributes(attrs, R.styleable.AspBalanceView);
        mThumbDrawable = typedArray.getDrawable(R.styleable.AspBalanceView_thumb);
        mThumbBitmap = BitmapUtils.drawableToBitmap(mThumbDrawable);
        typedArray.recycle();
        if (null != mThumbBitmap) {
            mThumbWidth = mThumbBitmap.getWidth();
            mThumbHeight = mThumbBitmap.getHeight();
        }
        balanceDepth = EqUtils.ASP_CHIP_CSC37534.equals(EqUtils.getEqChipType()) || EqUtils.ASP_CHIP_ZL3560.equals(EqUtils.getEqChipType()) ?
                EqUtils.CSC_ASP_BALANCE_DEPTH : EqUtils.BALANCE_DEPTH;
        mBalanceMax = (balanceDepth - 1) / 2;
        mBalanceMin = -1 * mBalanceMax;

        setOnTouchListener(this);
    }

    @Override
    public void run() {
        long currTime = 0;
        long diffTime = 0;
        while ((!Thread.currentThread().isInterrupted() && mAnimationFlag)
                && (this.mAnimationFlag)) {
            if (null != mEqBalanceDrag) {
                if (mIsMoveBalance) {
                    currTime = System.currentTimeMillis();
                    diffTime = currTime - mLastTime;
                    if (diffTime >= 100L) {
                        mLastTime = currTime;
                        mRedrawHandle.sendEmptyMessage(RedrawHandle.WM_UPDATE_BALANCE);
                    } else {
                        try {
                            Thread.sleep(50L);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                } else {
                    try {
                        Thread.sleep(50L);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            } else {
                try {
                    Thread.sleep(50L);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public void monitorThreadStop() {
        mAnimationFlag = false;
    }

    public void monitorThreadStart() {
        mAnimationFlag = true;
        if ((mAnimationThread == null) || (!mAnimationThread.isAlive())) {
            mAnimationThread = new Thread(this);
            mAnimationThread.start();
        }
    }

    public void setEqBalanceListener(BalanceViewCallback balanceViewCallback) {
        if (null != balanceViewCallback) {
            mBalanceViewCallback = balanceViewCallback;
        }
    }

    public Point getBalanceThumbXY(Point level) {
        Point thumbPoint = new Point(0, 0);
        thumbPoint.x = mThumbWidth / 2
                + (int) (level.x * mThumbUnitGapX * 1.0F);
        thumbPoint.y = mThumbHeight / 2
                + (int) (level.y * mThumbUnitGapY * 1.0F);
        return thumbPoint;
    }


    public Point getBalanceLevel(float fx, float fy) {
        Point pointLevel = new Point(0, 0);
        int x = Math.round(fx);
        int y = Math.round(fy);
        int nDragRangeW = (getWidth() - mThumbWidth) + (int) mThumbUnitGapX;
        int nDragRangeH = (getHeight() - mThumbHeight) + (int) mThumbUnitGapY;
        int nMarginW = (getWidth() - nDragRangeW) / 2;
        int nMarginH = (getHeight() - nDragRangeH) / 2;
        x = (x < nMarginW) ? nMarginW : x;
        x = (x > nMarginW + nDragRangeW) ? nMarginW + nDragRangeW : x;
        y = (y < nMarginH) ? nMarginH : y;
        y = (y > nMarginH + nDragRangeH) ? nMarginH + nDragRangeH : y;
        x = x - nMarginW;
        y = y - nMarginH;
        double dLevelX = x / mThumbUnitGapX;
        double dLevelY = y / mThumbUnitGapY;
        pointLevel.x = (int) (dLevelX); // (0-20)
        pointLevel.y = (int) (dLevelY);
        return pointLevel;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (null != mEqBalanceDrag) {
            canvas.setDrawFilter(mDrawFilter);
            mEqBalanceDrag.drawDragThumb(canvas);
        }
    }

    public void setDragMotionBegin() {
        if (null != mBalanceViewCallback) {
            if (!mIsMoveBalance) {
                mBalanceViewCallback.onMotionBegin();
                mIsMoveBalance = true;
            }
        }
    }

    public void setDragMotionChanged(int x, int y, boolean bUpdate) {
        if (null != mBalanceViewCallback) {
            if (mCurrLevel.x != x || mCurrLevel.y != y) {
                mCurrLevel.x = x;
                mCurrLevel.y = y;
                mBalanceViewCallback.onMotionChanged(x, y, bUpdate);
            }
        }
    }

    public void setDragMotionFinished(int x, int y, boolean bUpdate) {
        if (null != mBalanceViewCallback) {
            mBalanceViewCallback.onMotionFinished(x, y, bUpdate);
            mIsMoveBalance = false;
        }
    }

    private void updateBalanceView() {
        if (null != mEqBalanceDrag) {
            mEqBalanceDrag.buildDrawParameter();
            invalidate();
        }
    }

    private void initEqBalanceDrag() {
        if (null == mEqBalanceDrag) {
            mThumbUnitGapX = (getWidth() - mThumbWidth)
                    / ((balanceDepth - 1) * 1.0F);
            mThumbUnitGapY = (getHeight() - mThumbHeight)
                    / ((balanceDepth - 1) * 1.0F);
            if (mHavePresetValue) {
                mInitBalanceX = getBalanceThumbXY(mTempLevel).x;
                mInitBalanceY = getBalanceThumbXY(mTempLevel).y;
            } else {
                mInitBalanceX = getWidth() / 2;
                mInitBalanceY = getHeight() / 2;
            }
            mCurrentX = mInitBalanceX;
            mCurrentY = mInitBalanceY;
            mEqBalanceDrag = new EqBalanceDrag(this, mThumbBitmap);
            mEqBalanceDrag.setThumbLocation(mInitBalanceX, mInitBalanceY);
            mEqBalanceDrag.buildDrawParameter();
        }
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right,
                            int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        initEqBalanceDrag();
    }

    public void reset() {
        if (null != mEqBalanceDrag) {
            mEqBalanceDrag.resetBalance();
            mTempLevel.x = (balanceDepth - 1) / 2;
            mTempLevel.y = (balanceDepth - 1) / 2;
            Point point = getBalanceThumbXY(mTempLevel);
            mCurrentX = point.x;
            mCurrentY = point.y;
            updateBalanceView();
        }
    }

    /**
     * 设置EqBalanceView x,y 值
     *
     * @param x       要设置到系统的值
     * @param y       要设置到系统的值
     * @param bUpdate true  表示执行监听事件，将最新的值更新到文件和系统同时刷新UI
     *                false 表示不执行监听事件，仅刷新UI不做其他任何操作
     */
    public void setBalanceLevel(int x, int y, boolean bUpdate) {
        if (x >= 0 && balanceDepth > x) {
            if (y >= 0 && balanceDepth > y) {
                if (null != mEqBalanceDrag) {
                    mEqBalanceDrag.resetBalance();
                    mTempLevel.x = x;
                    mTempLevel.y = y;
                    Point point = getBalanceThumbXY(mTempLevel);
                    mCurrentX = point.x;
                    mCurrentY = point.y;
                    if (bUpdate) {
                        mEqBalanceDrag.gotoThumbLocation((int) mCurrentX, (int) mCurrentY);
                    } else {
                        mEqBalanceDrag.setThumbLocation((int) mCurrentX, (int) mCurrentY);
                    }
                    mEqBalanceDrag.buildDrawParameter();
                    invalidate();
                } else {
                    mTempLevel.x = x;
                    mTempLevel.y = y;
                    mHavePresetValue = true;
                }
            }
        }
    }

    public void moveLeft() {
        if (null != mEqBalanceDrag) {
            mEqBalanceDrag.resetBalance();
            if (mCurrLevel.x > 0) {
                mTempLevel.set(mCurrLevel.x, mCurrLevel.y);
                mTempLevel.x -= 1;
                mCurrentX = mTempLevel.x;
                mCurrentY = mTempLevel.y;
                setBalanceLevel((int) mCurrentX, (int) mCurrentY, true);
            }
        }
    }

    public void moveRight() {
        if (null != mEqBalanceDrag) {
            mEqBalanceDrag.resetBalance();
            if (mCurrLevel.x < (balanceDepth - 1)) {
                mTempLevel.set(mCurrLevel.x, mCurrLevel.y);
                mTempLevel.x += 1;
                mCurrentX = mTempLevel.x;
                mCurrentY = mTempLevel.y;
                setBalanceLevel((int) mCurrentX, (int) mCurrentY, true);
            }
        }
    }

    public void moveFront() {
        if (null != mEqBalanceDrag) {
            mEqBalanceDrag.resetBalance();
            if (mCurrLevel.y > 0) {
                mTempLevel.set(mCurrLevel.x, mCurrLevel.y);
                mTempLevel.y -= 1;
                mCurrentX = mTempLevel.x;
                mCurrentY = mTempLevel.y;
                setBalanceLevel((int) mCurrentX, (int) mCurrentY, true);
            }
        }
    }

    public void moveRear() {
        if (null != mEqBalanceDrag) {
            mEqBalanceDrag.resetBalance();
            if (mCurrLevel.y < (balanceDepth - 1)) {
                mTempLevel.set(mCurrLevel.x, mCurrLevel.y);
                mTempLevel.y += 1;
                mCurrentX = mTempLevel.x;
                mCurrentY = mTempLevel.y;
                setBalanceLevel((int) mCurrentX, (int) mCurrentY, true);
            }
        }
    }

    public void moveFrontLeft() {
        if (null != mEqBalanceDrag) {
            mEqBalanceDrag.resetBalance();

            if (mCurrLevel.y > 0 || mCurrLevel.x > 0) {

                mTempLevel.set(mCurrLevel.x, mCurrLevel.y);

                if (mCurrLevel.y > 0) {
                    mTempLevel.y -= 1;
                }
                if (mCurrLevel.x > 0) {
                    mTempLevel.x -= 1;
                }

                Point point = getBalanceThumbXY(mTempLevel);
                mCurrentX = point.x;
                mCurrentY = point.y;
                updateBalanceView();
            }
        }
    }

    public void moveFrontRight() {
        if (null != mEqBalanceDrag) {
            mEqBalanceDrag.resetBalance();
            if (mCurrLevel.y > 0
                    || mCurrLevel.x < (balanceDepth - 1)) {
                mTempLevel.set(mCurrLevel.x, mCurrLevel.y);
                if (mCurrLevel.y > 0) {
                    mTempLevel.y -= 1;
                }
                if (mCurrLevel.x < (balanceDepth - 1)) {
                    mTempLevel.x += 1;
                }
                Point point = getBalanceThumbXY(mTempLevel);
                mCurrentX = point.x;
                mCurrentY = point.y;
                updateBalanceView();
            }
        }
    }

    public void moveRearLeft() {
        if (null != mEqBalanceDrag) {
            mEqBalanceDrag.resetBalance();
            if (mCurrLevel.y < (balanceDepth - 1)
                    || mCurrLevel.x > 0) {
                mTempLevel.set(mCurrLevel.x, mCurrLevel.y);
                if (mCurrLevel.y < (balanceDepth - 1)) {
                    mTempLevel.y += 1;
                }
                if (mCurrLevel.x > 0) {
                    mTempLevel.x -= 1;
                }
                Point point = getBalanceThumbXY(mTempLevel);
                mCurrentX = point.x;
                mCurrentY = point.y;
                updateBalanceView();
            }
        }
    }

    public void moveRearRight() {
        if (null != mEqBalanceDrag) {
            mEqBalanceDrag.resetBalance();
            if (mCurrLevel.y < (balanceDepth - 1)
                    || mCurrLevel.x < (balanceDepth - 1)) {
                mTempLevel.set(mCurrLevel.x, mCurrLevel.y);
                if (mCurrLevel.y < (balanceDepth - 1)) {
                    mTempLevel.y += 1;
                }
                if (mCurrLevel.x < (balanceDepth - 1)) {
                    mTempLevel.x += 1;
                }
                Point point = getBalanceThumbXY(mTempLevel);
                mCurrentX = point.x;
                mCurrentY = point.y;
                updateBalanceView();
            }
        }
    }

    public void recycle() {
        if ((mThumbBitmap != null) && (!mThumbBitmap.isRecycled())) {
            mThumbBitmap.recycle();
        }

        if (mEqBalanceDrag != null) {
            mEqBalanceDrag.recycle();
        }
    }

    @SuppressLint("HandlerLeak")
    private class RedrawHandle extends Handler {
        public static final int WM_UPDATE_BALANCE = 200;

        public void handleMessage(Message message) {
            super.handleMessage(message);

            switch (message.what) {
                case WM_UPDATE_BALANCE:
                    BalanceView.this.updateBalanceView();
                    break;
                default:
                    break;
            }
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {

        if (null != mBalanceViewCallback) {
            mBalanceViewCallback.onMotionBegin();
        }

        mCurrentX = event.getX();
        mCurrentY = event.getY();

        mCurrentX = (mCurrentX < mThumbWidth / 2) ? mThumbWidth / 2 : mCurrentX;
        mCurrentY = (mCurrentY < mThumbHeight / 2) ? mThumbHeight / 2
                : mCurrentY;
        mCurrentX = (mCurrentX > getWidth() - mThumbWidth / 2) ? getWidth()
                - mThumbWidth / 2 : mCurrentX;
        mCurrentY = (mCurrentY > getHeight() - mThumbHeight / 2) ? getHeight()
                - mThumbHeight / 2 : mCurrentY;

        if (this.horizontalOnly) {
            mCurrentY = getHeight() / 2;
        }

        mEqBalanceDrag.gotoThumbLocation((int) mCurrentX, (int) mCurrentY);

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                mIsMoveBalance = true;
                break;
            case MotionEvent.ACTION_MOVE:
                break;
            case MotionEvent.ACTION_UP:
                mBalanceViewCallback.onSaveData(getBalanceLevel((int) mCurrentX, (int) mCurrentY).x, getBalanceLevel((int) mCurrentX, (int) mCurrentY).y);
                break;
            case MotionEvent.ACTION_CANCEL:
                break;
        }
        return true;
    }

    public float getCurrentX() {
        return mCurrentX;
    }

    public float getCurrentY() {
        return mCurrentY;
    }

    public void setHorizontalOnly(boolean horizontalOnly) {
        this.horizontalOnly = horizontalOnly;
    }
    public void setmThumbDrawable(int i) {
        mEqBalanceDrag.setmThumbDrawable(i);
        updateBalanceView();
    }
}
