package com.hcn.autoradio.view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;

import com.hcn.autoradio.FMResource;
import com.hcn.autoradio.R;
import com.hcn.autoradio.ScreenSpec;
import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.data.RadioData;
import com.hcn.autoradio.skin.SkinID;
import com.hcn.autoradio.skin.SkinUtils;
import com.hcn.autoradio.skin.ThemeID;
import com.hcn.autoradio.util.FMBitmapUnit;

public class FMSeekBarView extends ImageView implements View.OnTouchListener,
        Runnable {
    public FMSeekBar mFMScroller;
    long mTimeInterval = 60L;
    private Canvas mCanvas = null;
    private int mBgWidth = 0;
    private int mBgHeight = 0;
    private Bitmap mDrawCatchBuffer;
    private int mScrollUID = FMResource.REGION_CHINA_FM;
    private FMSeekBarListener mFMSeekBarListener;
    private boolean mMotionFlag = false;
    private Bitmap mBmpScrollThumb;
    private Drawable mScrollThumbDrawable;

    private float mBeginPos = 0;
    private float mEndPos = 0;
    private int mCenterX = 0;
    private int mCenterY = 0;

    private int mScrollCurNum;
    private int mScrollNumMax;
    private int mScrollNumMin;

    private Context mContext;
    private RedrawHandle mRedrawHandle;
    private boolean mDrawFlag;
    private Thread mDrawThread;

    private long mLastTime = 0L;

    private int mThumbOffsetY = 0;


    public FMSeekBarView(Context context, AttributeSet attrs) {

        this(context, attrs, 0);
    }


    public FMSeekBarView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

        mContext = context;
        mRedrawHandle = new RedrawHandle();

        TypedArray typedArray = context.obtainStyledAttributes(attrs,
                FMResource.styleable.EqScroll, defStyle, 0);

        mCenterX = typedArray.getInt(FMResource.styleable.center_x, 1);
        mCenterY = typedArray.getInt(FMResource.styleable.center_y, 1);
        mScrollThumbDrawable = typedArray
                .getDrawable(FMResource.styleable.scroll_item);
        mBeginPos = typedArray
                .getFloat(FMResource.styleable.scroll_begin, 0.0F);
        mEndPos = typedArray.getFloat(FMResource.styleable.scroll_end, 0.0F);
        int nSource = typedArray.getInt(FMResource.styleable.scroll_uid, 0);

        mThumbOffsetY = typedArray.getInt(FMResource.styleable.thumb_offset_y, 0);

        typedArray.recycle();

        mBmpScrollThumb = FMBitmapUnit.drawableToBitmap(mScrollThumbDrawable);

        initScrollInfo(nSource);
        setOnTouchListener(this);
    }

    private void initScrollInfo(int uid) {

        switch (uid) {
            case FMResource.REGION_CHINA_FM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.ChinaFMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.ChinaFMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.ChinaFMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.FmMax
                        - FMDataControl.mRadioParameters.FmMin)
                        / FMDataControl.mRadioParameters.FmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_fm_china);
                }
                break;
            case FMResource.REGION_CHINA_AM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.ChinaAMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.ChinaAMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.ChinaAMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.AmMax
                        - FMDataControl.mRadioParameters.AmMin)
                        / FMDataControl.mRadioParameters.AmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_am_china);
                }
                break;
            case FMResource.REGION_EUROPE_FM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.EuropeFMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.EuropeFMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.EuropeFMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.FmMax
                        - FMDataControl.mRadioParameters.FmMin)
                        / FMDataControl.mRadioParameters.FmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_fm_china);
                }
                break;
            case FMResource.REGION_EUROPE_AM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.EuropeAMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.EuropeAMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.EuropeAMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.AmMax
                        - FMDataControl.mRadioParameters.AmMin)
                        / FMDataControl.mRadioParameters.AmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_am_europe);
                }
                break;
            case FMResource.REGION_JAPAN_FM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.JapanFMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.JapanFMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.JapanFMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.FmMax
                        - FMDataControl.mRadioParameters.FmMin)
                        / FMDataControl.mRadioParameters.FmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_fm_japan);
                }
                break;
            case FMResource.REGION_JAPAN_AM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.JapanAMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.JapanAMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.JapanAMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.AmMax
                        - FMDataControl.mRadioParameters.AmMin)
                        / FMDataControl.mRadioParameters.AmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_am_japan);
                }
                break;
            case FMResource.REGION_LATIN_FM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.LatinFMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.LatinFMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.LatinFMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.FmMax
                        - FMDataControl.mRadioParameters.FmMin)
                        / FMDataControl.mRadioParameters.FmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_fm_china);
                }
                break;
            case FMResource.REGION_LATIN_AM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.LatinAMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.LatinAMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.LatinAMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.AmMax
                        - FMDataControl.mRadioParameters.AmMin)
                        / FMDataControl.mRadioParameters.AmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_am_latin);
                }
                break;
            case FMResource.REGION_USA_FM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.USAFMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.USAFMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.USAFMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.FmMax
                        - FMDataControl.mRadioParameters.FmMin)
                        / FMDataControl.mRadioParameters.FmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_fm_usa);
                }
                break;
            case FMResource.REGION_USA_AM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.USAAMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.USAAMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.USAAMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.AmMax
                        - FMDataControl.mRadioParameters.AmMin)
                        / FMDataControl.mRadioParameters.AmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_am_usa);
                }
                break;
            case FMResource.REGION_OIRT_FM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.OIRTFMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.OIRTFMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.OIRTFMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.FmMax
                        - FMDataControl.mRadioParameters.FmMin)
                        / FMDataControl.mRadioParameters.FmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_fm_oirt);
                }
                break;
            case FMResource.REGION_OIRT_AM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.OIRTAMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.OIRTAMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.OIRTAMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.AmMax
                        - FMDataControl.mRadioParameters.AmMin)
                        / FMDataControl.mRadioParameters.AmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_am_china);
                }
                break;

            case FMResource.REGION_LATIN2_FM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.Latin2FMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.Latin2FMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.Latin2FMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.FmMax
                        - FMDataControl.mRadioParameters.FmMin)
                        / FMDataControl.mRadioParameters.FmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_fm_latin2);
                }
                break;
            case FMResource.REGION_LATIN2_AM:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.TWO_THIRD_SCREEN) {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.Latin2AMBeginPosTwoThirdsScreen);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mBeginPos = SkinUtils.getInteger(
                            R.integer.Latin2AMBeginPosHalfScreen);
                } else {
                    mBeginPos =  SkinUtils.getInteger(
                            R.integer.Latin2AMBeginPosFullScreen);
                }
                mEndPos = ScreenSpec.mScreenWidth - mBeginPos;

                mScrollCurNum = 0;
                mScrollNumMin = 0;
                mScrollNumMax = (FMDataControl.mRadioParameters.AmMax
                        - FMDataControl.mRadioParameters.AmMin)
                        / FMDataControl.mRadioParameters.AmStep;

                if (mScrollUID != uid) {
                    setImageResource(R.drawable.radio_scroll_am_latin);
                }
                break;
            default:
                break;
        }

        mScrollUID = uid;
    }

    @Override
    public void setImageResource(int resId) {
        super.setImageResource(SkinUtils.getId(resId));
    }

    public int getSeekBarUID() {
        return mScrollUID;
    }

    public int getBkgWidth() {
        return mBgWidth;
    }

    public int getBkgHeight() {
        return mBgHeight;
    }

    public void setCurrentNum(int num) {

        if (mScrollNumMax >= num && num >= mScrollNumMin) {
            mScrollCurNum = num;
        }
    }

    private void buildEqScrollerCanvas() {

        if (null != mCanvas && null != mFMScroller) {

            // java.lang.IllegalStateException: Can't erase a recycled bitmap
            if (null != mDrawCatchBuffer && !mDrawCatchBuffer.isRecycled()) {

                mDrawCatchBuffer.eraseColor(Color.TRANSPARENT);


                mFMScroller.buildDrawParameter();

                mFMScroller.drawEqScroller(mCanvas);
            }
        }
    }

    private void updataSeekBar() {

        if (mMotionFlag) {

            buildEqScrollerCanvas();
            invalidate();
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        mLastTime = System.currentTimeMillis();

        if (mFMScroller != null) {


            canvas.drawBitmap(mDrawCatchBuffer, 0.0F, 0.0F, null);
        }
    }

    public void resetFMSeekbarData(int uid) {
        if (getSeekBarUID() != uid) {
            Log.v("Radio", "resetFMSeekbarData:" + uid);
            initScrollInfo(uid);

            if (null != mFMScroller) {
                initFMScrollerAlgorithm();
            }
        }
    }

    private void initFMScrollerAlgorithm() {
        if ((mScrollUID != -1) && (mBgWidth != 0) && (mBgHeight != 0)) {
            if (SkinUtils.useSkinPackage()) {
                switch (SkinUtils.getCurrentSkinID()) {
                    case SkinID.SKIN_ZA01:
                    case SkinID.SKIN_ZA03:
                    case SkinID.SKIN_ZA04:
                    case SkinID.SKIN_ZA05:
                    case SkinID.SKIN_ZA09:
                    case SkinID.SKIN_ZA10:
                    case SkinID.SKIN_ZA12:
                    case SkinID.SKIN_ZA33:
                    case SkinID.SKIN_ZA36:
                    case SkinID.SKIN_ZA37:
                    case SkinID.SKIN_ZA39:
                    case SkinID.SKIN_SA48:
                    case SkinID.SKIN_SA82:
                    case SkinID.SKIN_SA85:
                    case SkinID.SKIN_SA87:
                    case SkinID.SKIN_XT144:
                    case SkinID.SKIN_XT510:
                    case SkinID.SKIN_GB01:
                        mEndPos = getMeasuredWidth() - mBeginPos;
                        break;
                    default:
                        break;
                }
            } else {
                if (RadioData.E_THEME_GOD == ThemeID.E_THEME_ID_400
                        || RadioData.E_THEME_GOD == ThemeID.E_THEME_ID_202
                        || RadioData.E_THEME_GOD == ThemeID.E_THEME_ID_204
                        || RadioData.E_THEME_GOD == ThemeID.E_THEME_ID_401
                        || RadioData.E_THEME_GOD == ThemeID.E_THEME_ID_403
                        || RadioData.E_THEME_GOD == ThemeID.E_THEME_ID_404
                        || RadioData.E_THEME_GOD == ThemeID.E_THEME_ID_405
                        || RadioData.E_THEME_GOD == ThemeID.E_THEME_ID_408
                        || RadioData.E_THEME_GOD == ThemeID.E_THEME_ID_409) {
                    mEndPos = getMeasuredWidth() - mBeginPos;
                }
            }

            if (null == mFMScroller) {
                mFMScroller = new FMSeekBar(this, mBmpScrollThumb, mBeginPos,
                        mEndPos, mScrollNumMin, mScrollNumMax, mScrollCurNum, mThumbOffsetY);
            } else {
                mFMScroller.resetSeekBarData(mBeginPos, mEndPos, mScrollNumMin,
                        mScrollNumMax, mScrollCurNum);
            }
            buildEqScrollerCanvas();
        }
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        super.onLayout(changed, l, t, r, b);

        int nWidth = r - l;
        int nHeight = b - t;

        if (nWidth <= 0 || nHeight <= 0) {
            Log.e("FMSeekBarView", "nWidth=" + nWidth + " nHeight=" + nHeight);
            return;
        }

        if ((null == mCanvas) || (nWidth != mBgWidth) || (nHeight != mBgHeight)) {

            mBgWidth = nWidth;
            mBgHeight = nHeight;


            mDrawCatchBuffer = Bitmap.createBitmap(mBgWidth, mBgHeight,
                    Bitmap.Config.ARGB_8888);


            mCanvas = new Canvas(mDrawCatchBuffer);


            initFMScrollerAlgorithm();
        }
    }

    public void setFMSeekBarListener(FMSeekBarListener fmSeekBarListener) {
        if (null != fmSeekBarListener) {
            mFMSeekBarListener = fmSeekBarListener;
        }
    }

    public void scrollMotionBegin(boolean bAnimation) {

        if (null != mFMSeekBarListener) {

            if (!mMotionFlag) {
                mFMSeekBarListener.onMotionBegin();

                if (bAnimation) {
                    mTimeInterval = 60L;
                } else {
                    mTimeInterval = 20L;
                }

                mMotionFlag = true;
            }
        }
    }

    public void scrollMotionFinished() {

        if (null != mFMSeekBarListener) {
            mFMSeekBarListener.onMotionFinished();
            mMotionFlag = false;
        }
    }

    public void setScrollMotion(int nIndex, boolean bUpdate) {

        if (null != mFMSeekBarListener) {

            if (mScrollCurNum != nIndex) {
                mScrollCurNum = nIndex;
                mFMSeekBarListener.onMotionChanged(mScrollUID, nIndex, bUpdate);
            }
        }
    }

    public void setScrollThumbCenterLocation(int cx, int cy) {
        if (null != mFMSeekBarListener) {
            mFMSeekBarListener.onScrollThumbCenterLocation(cx, cy);
        }
    }

    public void recycle() {
        if ((mDrawCatchBuffer != null) && (!mDrawCatchBuffer.isRecycled())) {
            mDrawCatchBuffer.recycle();
        }

        if ((mBmpScrollThumb != null) && (!mBmpScrollThumb.isRecycled())) {
            mBmpScrollThumb.recycle();
        }

        if (mFMScroller != null) {
            mFMScroller.recycle();
        }
    }

    @Override
    public void run() {

        long currTime = 0;
        long diffTime = 0;


        while ((!Thread.currentThread().isInterrupted()) && (mDrawFlag)) {

            if (mFMScroller != null) {

                if (mMotionFlag) {
                    currTime = System.currentTimeMillis();
                    diffTime = currTime - mLastTime;

                    if (diffTime >= mTimeInterval) {
                        mLastTime = currTime;
                        mRedrawHandle
                                .sendEmptyMessage(RedrawHandle.WM_UPDATE_SEEKBAR);
                    } else {
                        try {
                            Thread.sleep(20L);
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
        mDrawFlag = false;
    }

    public void monitorThreadStart() {
        mDrawFlag = true;

        if ((mDrawThread == null) || (!mDrawThread.isAlive())) {
            mDrawThread = new Thread(this);
            mDrawThread.start();
        }
    }

    public void setPos(int num) {
        if (null != mFMScroller) {
            mFMScroller.setPos(num);
        } else {
            setCurrentNum(num);
        }
    }

    public void setNum(int num) {
        if (null != mFMScroller) {
            mFMScroller.setNum(num);
        } else {
            setCurrentNum(num);
        }
    }

    public void gotoNum(int num) {
        if (null != mFMScroller) {
            mFMScroller.gotoNum(num);
        }
    }

    private int getThumbNumByTouchX(float fx) {


        float fPerUnitLen = (mEndPos - mBeginPos)
                / (mScrollNumMax - mScrollNumMin);


        return Math.round((fx - mBeginPos) / fPerUnitLen) + mScrollNumMin;
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {

        if (null != mFMSeekBarListener) {
            mFMSeekBarListener.onMotionBegin();
        }

        float fx = event.getX();
        float fy = event.getY();


        fx = fx > mEndPos ? mEndPos : fx;
        fx = (fx < mBeginPos) ? mBeginPos : fx;

        int index = getThumbNumByTouchX(fx);
        mFMScroller.gotoNum(index);
        if (event.getAction() == MotionEvent.ACTION_UP
                || event.getAction() == MotionEvent.ACTION_DOWN) {
            if (null != mFMSeekBarListener) {
                mFMSeekBarListener.onSetValue(mScrollUID, Integer.valueOf(index),
                        false);
            }
        }

        return true;
    }

    @SuppressLint("HandlerLeak")
    private class RedrawHandle extends Handler {

        public static final int WM_UPDATE_SEEKBAR = 100;

        @Override
        public void handleMessage(Message message) {
            super.handleMessage(message);

            switch (message.what) {
                case WM_UPDATE_SEEKBAR:
                    FMSeekBarView.this.updataSeekBar();
                    break;
                default:
                    break;
            }
        }
    }
}
