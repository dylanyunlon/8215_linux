package com.hcn.autoeq.view;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Point;
import android.util.Log;

import com.hcn.autoeq.util.BitmapUtils;
import com.hcn.autoeq.util.SkinUtils;

public class EqBalanceDrag {

    private final String TAG = EqBalanceDrag.class.getSimpleName();
    private Paint mPaint = null;
    private Matrix mThumbMatrix = null;
    private Bitmap mBmpThumb = null;

    private float mCurrentX = 0.0F;
    private float mCurrentY = 0.0F;

    private float mCurTargetX = 0.0F;
    private float mCurTargetY = 0.0F;
    private float mMoveDistance = 0.0F;

    private boolean mMotionFlag = false;
    private boolean mSettingFlag = true;

    private boolean mResetBalance = true;
    private BalanceView mEqBalanceView;

    public EqBalanceDrag(BalanceView view, Bitmap bitmap) {
        mBmpThumb = bitmap;
        mEqBalanceView = view;
        mPaint = new Paint();
        mPaint.setAntiAlias(true);
        mPaint.setColor(Color.WHITE);
        mPaint.setTextAlign(Paint.Align.CENTER);
        mPaint.setTextSize(18.0F);
        mPaint.setDither(true);
        mThumbMatrix = new Matrix();
    }

    public void drawDragThumb(Canvas canvas) {
        canvas.drawBitmap(mBmpThumb, mThumbMatrix, mPaint);
    }

    public void buildDrawParameter() {
        float interpolator = 2.9F;
        mMoveDistance = getMoveDistance();
        if (mMoveDistance > 0.001D) {
            interpolator = (float) (Math.random()) * 1.6F + 1.6F;
            interpolator = interpolator < 2.9F ? 2.9F : interpolator;
            float moveOffset = (mMoveDistance) / interpolator;
            float xOffset = Math.abs(mCurTargetX - mCurrentX) * moveOffset
                    / mMoveDistance;
            float yOffset = Math.abs(mCurTargetY - mCurrentY) * moveOffset
                    / mMoveDistance;
            mMoveDistance = mMoveDistance - moveOffset;
            mCurrentX = (mCurTargetX - mCurrentX) > 0 ? mCurrentX + xOffset : mCurrentX - xOffset;
            mCurrentY = (mCurTargetY - mCurrentY) > 0 ? mCurrentY + yOffset : mCurrentY - yOffset;
            if (!mMotionFlag) {
                mMotionFlag = true;
                if (null != mEqBalanceView) {
                    mEqBalanceView.setDragMotionBegin();
                }
            }
        }
        mThumbMatrix.reset();
        mThumbMatrix.postTranslate(-mBmpThumb.getWidth() / 2,
                -mBmpThumb.getHeight() / 2);
        mThumbMatrix.postTranslate(mCurrentX, mCurrentY);
        Point point = null;
        if (null != mEqBalanceView) {
            point = mEqBalanceView.getBalanceLevel(mCurrentX,
                    mCurrentY);
            mEqBalanceView.setDragMotionChanged(point.x, point.y, mSettingFlag);
        }
        if (mMotionFlag && mMoveDistance <= 0.5D) {
            mMotionFlag = false;
            mMoveDistance = 0.0F;
            mCurrentX = mCurTargetX;
            mCurrentY = mCurTargetY;
            if (null != mEqBalanceView) {
                mEqBalanceView.setDragMotionFinished(point.x, point.y, mSettingFlag);
            }
        }
    }

    public float getMoveDistance() {
        double _x = Math.abs(mCurTargetX - mCurrentX);
        double _y = Math.abs(mCurTargetY - mCurrentY);
        return (float) Math.sqrt(_x * _x + _y * _y);
    }

    public void resetBalance() {
        mResetBalance = true;
        mMotionFlag = false;
        mMoveDistance = 0.0F;
        setThumbLocation((int) mCurrentX, (int) mCurrentY);
        if (null != mEqBalanceView) {
            //mEqBalanceView.setDragMotionFinished();
        }
    }

    public void setThumbLocation(int x, int y) {
        mCurTargetX = x;
        mCurTargetY = y;
        mSettingFlag = false;
        if (mResetBalance) {
            mCurrentX = x;
            mCurrentY = y;
            mResetBalance = false;
        }
    }

    public void gotoThumbLocation(int x, int y) {
        mCurTargetX = x;
        mCurTargetY = y;
        mSettingFlag = true;
        if (mResetBalance) {
            mCurrentX = x;
            mCurrentY = y;
            mResetBalance = false;
        }
    }

    public void recycle() {
        if ((null != mBmpThumb) && (!mBmpThumb.isRecycled())) {
            mBmpThumb.recycle();
            mBmpThumb = null;
        }
    }
    public void setmThumbDrawable(int i) {
        recycle();
        mBmpThumb = BitmapUtils.drawableToBitmap(SkinUtils.getDrawable(i));
    }
}
