package com.hcn.autoradio.view;

import android.R;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;

import com.hcn.autoradio.skin.SkinUtils;

public class FMSeekBar {
    private float mBeginOffset;
    private float mEndOffset;

    private Paint mPaint;
    private Matrix mThumbMatrix;
    private boolean mMotionFlag = false;
    private boolean mSettingFlag = true;

    private float mNumCurF = 0.0F;
    private int mNumMax = +14;
    private int mNumMin = -14;
    private float mNumTagF = 0.0F;

    private Bitmap mScrollThumb;
    private FMSeekBarView mFMSeekBarView;

    private float mScrollPointX;

    private float mSclBarWidth = 1.0F;
    private float mSclBarHeight = 1.0F;

    private float mSclThumbWidth = 1.0F;
    private float mSclThumbHeight = 1.0F;

    private final float mTextSize = 15.0F;

    private int mThumbOffsetY = 0;

    public FMSeekBar(FMSeekBarView eqSeekBar, Bitmap bmpThumb,
            float beginAngle, float endAngle, int nMin, int nMax, int numCur, int thumbOffsetY) {

        mFMSeekBarView = eqSeekBar;
        mScrollThumb = bmpThumb;

        mBeginOffset = beginAngle;
        mEndOffset = endAngle;
        mNumMin = nMin;
        mNumMax = nMax;

        mNumCurF = numCur;
        mNumTagF = numCur;

        mThumbOffsetY = thumbOffsetY; // thumb上下位置偏移，thumbOffsetY > 0 ? 向下:向上

        mPaint = new Paint();
        mPaint.setColor(Color.WHITE);
        mPaint.setTextAlign(Paint.Align.CENTER);
        mPaint.setTextSize(mTextSize);

        mThumbMatrix = new Matrix();

        if (null != mFMSeekBarView) {
            mSclBarWidth = mFMSeekBarView.getBkgWidth();
            mSclBarHeight = mFMSeekBarView.getBkgHeight();
        }

        if (null != mScrollThumb) {
            mSclThumbWidth = mScrollThumb.getWidth();
            mSclThumbHeight = mScrollThumb.getHeight();
        }
    }

    public void resetSeekBarData(float beginAngle, float endAngle, int nMin, int nMax,
            int numCur) {

        mBeginOffset = beginAngle;
        mEndOffset = endAngle;
        mNumMin = nMin;
        mNumMax = nMax;

        mNumCurF = numCur;
        mNumTagF = numCur;
    }

    public void drawEqScroller(Canvas canvas) {


        mPaint.setAntiAlias(false);


        canvas.drawBitmap(mScrollThumb, mThumbMatrix, mPaint);

        mPaint.setAntiAlias(true);
        mPaint.setColor(Color.WHITE);
    }

    public void setPos(int nNum) {
        mNumTagF = nNum;
        mNumCurF = nNum;
        mMotionFlag = false;
        mSettingFlag = false;

        mFMSeekBarView.scrollMotionBegin(false);
    }


    public void setNum(int nNum) {
        mNumTagF = nNum;
        mMotionFlag = true;
        mSettingFlag = false;

        mFMSeekBarView.scrollMotionBegin(true);
    }


    public void gotoNum(int nNum) {
        mNumTagF = nNum;
        mMotionFlag = true;
        mSettingFlag = true;

        mFMSeekBarView.scrollMotionBegin(true);
    }

    public void buildDrawParameter() {


        float interpolator = 2.8F;
        float diffNumF = Math.abs(mNumTagF - mNumCurF);

        if (diffNumF > 0.001D) {

            interpolator = (float) (Math.random()) * 1.5F + 1.5F;
            interpolator = interpolator < 2.8F ? 2.8F : interpolator;


            if (diffNumF > (mNumMax - mNumMin) / 5 * 4) {


                if (Math.abs(diffNumF - (mNumMax - mNumMin)) < 0.1F) {
                    mNumCurF = mNumTagF;
                    diffNumF = 0.0001F;
                } else {


                    if (mNumCurF > mNumTagF) {
                        mNumCurF += ((mNumMax - mNumCurF) + (mNumTagF - mNumMin))
                                / interpolator;
                        mNumCurF = mNumCurF > mNumMax ? mNumMin : mNumCurF;
                    } else {
                        mNumCurF -= ((mNumMax - mNumTagF) + (mNumCurF - mNumMin))
                                / interpolator;
                        mNumCurF = mNumCurF < mNumMin ? mNumMax : mNumCurF;
                    }
                }
            } else {
                mNumCurF += (mNumTagF - mNumCurF) / interpolator;
            }

            if (!mMotionFlag && diffNumF > 0.1F) {
                mMotionFlag = true;
                mFMSeekBarView.scrollMotionBegin(true);
            }
        }


        if (mMotionFlag && diffNumF < 0.1F) {
            mNumCurF = mNumTagF;
            mMotionFlag = false;
        }

        int nIndex = Math.round(mNumCurF);
        mFMSeekBarView.setScrollMotion(nIndex, mSettingFlag);

        mScrollPointX = ((mNumCurF - mNumMin) / (mNumMax - mNumMin)
                * (mEndOffset - mBeginOffset) + mBeginOffset);


        mFMSeekBarView.setScrollThumbCenterLocation((int) mScrollPointX,
                (int) (mSclBarHeight / 2));

        mThumbMatrix.reset();
        mThumbMatrix.postTranslate(mScrollPointX - (mSclThumbWidth + 1) / 2.0F,
                (mSclBarHeight - mSclThumbHeight) / 2.0F + mThumbOffsetY);

        if (!mMotionFlag) {
            mFMSeekBarView.scrollMotionFinished();
        }
    }

    public void recycle() {

        if ((mScrollThumb != null) && (!mScrollThumb.isRecycled())) {
            mScrollThumb.recycle();
            mScrollThumb = null;
        }
    }
}
