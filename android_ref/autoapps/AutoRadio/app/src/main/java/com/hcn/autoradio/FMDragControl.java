package com.hcn.autoradio;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnLongClickListener;
import android.view.View.OnTouchListener;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationSet;
import android.view.animation.ScaleAnimation;
import android.view.animation.TranslateAnimation;
import android.widget.Button;
import android.widget.Scroller;

import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.data.RadioData;
import com.hcn.autoradio.skin.SkinUtils;
import com.hcn.autoradio.view.DragContainer;
import com.hcn.autoradio.view.FMFreeLayout;
import com.hcn.autoradio.view.IScrollerListener;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.util.Locale;

public class FMDragControl {

    private static final String R_TAG = FMDragControl.class.getSimpleName();

    private DragContainer mAutoCell = null;
    private DragContainer mDragCell = null;
    private final int mMinMoveOffsetX = 4;
    private final int mMinMoveOffsetY = 2;

    private int mCenterDragCellX = 0;//long press freq position
    private int mCenterDragCellY = 0;
    private int mCenterDragCellW = 0;
    private int mCenterDragCellH = 0;

    private int mCurrDragCellTouchX = 0;
    private int mCurrDragCellTouchY = 0;

    private int mCurrFreqScaleCenterX = 0;//seekbar thumb pos
    private int mCurrFreqScaleCenterY = 0;

    private Context mContext = null;
    private FMFreeLayout mDragPanel = null;
    private FMDataControl mDataControl = null;
    private IFMCallBack mFMCallBack = null;
    private IPresetCallBack mPresetCallBack = null;
    private IDragControlEvent mDragControlEvent = null;

    private boolean mIsLongClick = false;
    private int mPresetStoreItem = -1;
    private boolean mIsDragStoreFM = false;
    private boolean mIsCollimation = false;

    private int CALIBRATE_OFFSET_X = 10;
    private int CALIBRATE_OFFSET_Y = 10;
    private int[] mPresetCX = {0, 0, 0, 0, 0, 0};//Preset Center X position for longpress Animation
    private int[] mPresetCY = {0, 0, 0, 0, 0, 0};//Preset Center Y position for longpress Animation
    private int mPresetWidth, mPresetHeight;

    private int[] mMinAimedRangeY = {0, 0, 0, 0, 0, 0};//Min Y position for drag match preset
    private int[] mMaxAimedRangeY = {0, 0, 0, 0, 0, 0};//Max Y position for drag match preset
    private int[] mMinAimedRangeX = {0, 0, 0, 0, 0, 0};//Min X position for drag match preset
    private int[] mMaxAimedRangeX = {0, 0, 0, 0, 0, 0};//Max X position for drag match preset
    private int mMatchingTargetIdx = -1;
    private boolean mFirstInitPresetCenterFixed = true;

    public FMDragControl(Context context, FMFreeLayout component,
            IFMCallBack fmCallBack, IPresetCallBack presetCallBack) {

        this.mContext = context;
        this.mDragPanel = component;
        this.mFMCallBack = fmCallBack;
        this.mPresetCallBack = presetCallBack;
        this.mDataControl = FMDataControl.getInstance();

        CALIBRATE_OFFSET_X =  SkinUtils.getInteger(
                R.integer.CALIBRATE_OFFSET_X);
        CALIBRATE_OFFSET_Y =  SkinUtils.getInteger(
                R.integer.CALIBRATE_OFFSET_Y);
        mCurrFreqScaleCenterX =  SkinUtils.getInteger(
                R.integer.AUTO_STROE_CENTER_POINT_X);
        mCurrFreqScaleCenterY =  SkinUtils.getInteger(
                R.integer.AUTO_STROE_CENTER_POINT_Y);

    }

    public void setDragControlEvent(IDragControlEvent eventCallback) {
        mDragControlEvent = eventCallback;
    }

    public void initPresetCenterFixed() {

        if (null != mPresetCallBack && mFirstInitPresetCenterFixed) {
            mFirstInitPresetCenterFixed = false;
            int nLocation[] = new int[2];
            View presets[] = mPresetCallBack.getCurrentPagePresets();
            for (int i = 0; i < FMDataControl.PAGE_STATION_NUM; i++) {
                if (presets[i] != null) {
                    mPresetWidth = presets[i].getWidth();
                    mPresetHeight = presets[i].getHeight();
                    presets[i].getLocationInWindow(nLocation);
                    nLocation[1] = nLocation[1];
                    mPresetCX[i] = nLocation[0] + (mPresetWidth >> 1);
                    mPresetCY[i] = nLocation[1] + (mPresetHeight >> 1);
                    mMinAimedRangeX[i] = nLocation[0];
                    mMaxAimedRangeX[i] = nLocation[0] + mPresetWidth;
                    mMinAimedRangeY[i] = nLocation[1] - (mPresetHeight >> 1);
                    mMaxAimedRangeY[i] = nLocation[1] + mPresetHeight;
                }
            }
        }
    }

    @SuppressLint("ClickableViewAccessibility")
    public DragContainer initDigitFreqCell() {
        if (null == mDragCell) {
            mDragCell = new DragContainer(this.mContext);
            mDragPanel.addComponentView(mDragCell);

            mDragCell.setSizeByCenter(mCenterDragCellW, mCenterDragCellH);
            mDragCell.setCenterPosition(mCenterDragCellX, mCenterDragCellY);
            mDragCell.setFocusable(false);

            mDragCell.setOnTouchListener(new OnDragTouchListener());
            mDragCell.setOnLongClickListener(new OnDragCellLongClickListener());

            mDragCell.setVisibility(View.VISIBLE);
            mDragCell.bringToFront();
        }
        return mDragCell;
    }

    public void initDigitFreqCellPos(int centerX, int centerY, int width, int height) {
        mCenterDragCellX = centerX;
        mCenterDragCellY = centerY;
        mCenterDragCellW = width;
        mCenterDragCellH = height;
        resetDigitFreqCell();
    }

    public void resetDigitFreqCell() {
        if (null != mDragCell) {
            mDragCell.removeAllViews();
            mDragCell.setSizeByCenter(mCenterDragCellW, mCenterDragCellH);
            mDragCell.setCenterPosition(mCenterDragCellX, mCenterDragCellY);
            mDragCell.setVisibility(View.VISIBLE);
            mDragCell.bringToFront();
            mDragCell.invalidate();
        }
    }

    public void createLable4DigitFreqCell(int cx, int cy) {
        if (null != mDragCell) {
            String freq = getStringCurrentFreq();
            FMFreeLayout.LayoutParams layoutParams = new FMFreeLayout.LayoutParams();
            mDragCell.setView(createLabel(freq), layoutParams);
            mDragCell.setSizeByCenter(mPresetWidth, mPresetHeight);
            mDragCell.setCenterPosition(cx, cy);
            mDragCell.setVisibility(View.VISIBLE);
            mDragCell.bringToFront();
            mDragCell.invalidate();
        }
    }

    public void setDigitFreqCell(int cx, int cy) {
        if (null != mDragCell) {
            mDragCell.setCenterPosition(cx, cy);
        }
    }

    /**
     * Screen XY to Window XY
     *
     * @param event
     */
    private void getTouchDragCellRawXY(MotionEvent event) {
        int DragPanelLocation[] = new int[2];
        mDragPanel.getLocationOnScreen(DragPanelLocation);
        mCurrDragCellTouchX = (int) (event.getRawX() - DragPanelLocation[0]);
        mCurrDragCellTouchY = (int) (event.getRawY() - DragPanelLocation[1]);
    }

    private boolean isCanMoveDragCell(MotionEvent event) {

        if (isDragStore()) {
            int nLastX = mCurrDragCellTouchX;
            int nLastY = mCurrDragCellTouchY;
            getTouchDragCellRawXY(event);
            if (Math.abs(nLastX - mCurrDragCellTouchX) >= mMinMoveOffsetX
                    || Math.abs(nLastY - mCurrDragCellTouchY) >= mMinMoveOffsetY) {
                return true;
            }
        }

        return false;
    }

    private final class OnDragTouchListener implements OnTouchListener {

        @SuppressLint("ClickableViewAccessibility")
        @Override
        public boolean onTouch(View v, MotionEvent event) {

            int action = event.getAction();
            switch (action) {
                case MotionEvent.ACTION_DOWN:
                    getTouchDragCellRawXY(event);
                    setCollimationFlag(false);
                    break;
                case MotionEvent.ACTION_MOVE:
                    if (isCanMoveDragCell(event)) {
                        setDigitFreqCell(mCurrDragCellTouchX, mCurrDragCellTouchY);
                        int index = matchingTarget(mCurrDragCellTouchX,
                                mCurrDragCellTouchY);
                        if (-1 != index && null != mPresetCallBack) {
                            mPresetCallBack.setPresetViewAimed(index, true);

                            if (-1 != mMatchingTargetIdx
                                    && mMatchingTargetIdx != index) {
                                int lastIdx = mMatchingTargetIdx;
                                mPresetCallBack.setPresetViewAimed(lastIdx, false);
                            }

                            mMatchingTargetIdx = index;
                            setCollimationFlag(true);
                        } else {
                            if (-1 != mMatchingTargetIdx && null != mPresetCallBack) {
                                int lastIdx = mMatchingTargetIdx;
                                mMatchingTargetIdx = -1;
                                mPresetCallBack.setPresetViewAimed(lastIdx, false);
                            }

                            setCollimationFlag(false);
                        }
                    }
                    break;
                case MotionEvent.ACTION_UP:
                    if (isDragStore()) {
                        getTouchDragCellRawXY(event);
                        if (isCollimation()) {
                            dragMatchingDisappear(mDragCell);
                        } else {
                            setPresetState(false);
                            setDragStoreFlag(false);
                            resetDigitFreqCell();
                        }
                    }
                    break;
                case MotionEvent.ACTION_CANCEL:
                    break;
                default:
                    break;
            }
            return false;
        }
    }

    private final class OnDragCellLongClickListener implements
            OnLongClickListener {

        @Override
        public boolean onLongClick(View v) {
            initPresetCenterFixed();
            if (getPresetState() || !mDragCell.canTouchEnabled()) {
                return false;
            }
            setPresetState(true);
            setDragStoreFlag(true);
            createLable4DigitFreqCell(mCurrDragCellTouchX, mCurrDragCellTouchY);
            autoStoreAnimation(mDragCell);
            return false;
        }
    }

    private Button createLabel(String freq) {
        Button butt = new Button(this.mContext);
        butt.setBackgroundResource(SkinUtils.getId(R.drawable.radio_dragitem));
        butt.setWidth(mPresetWidth);
        butt.setHeight(mPresetHeight);
        butt.setTextColor(android.graphics.Color.WHITE);
        if (ScreenSpec.getScreenStatus() != ScreenSpec.FULL_SCREEN) {
            butt.setTextSize(mContext.getResources().getDimension(SkinUtils.getId(R.dimen.PresetViewFreqSmallTextSize)));
        } else {
            butt.setTextSize(mContext.getResources().getDimension(SkinUtils.getId(R.dimen.PresetViewFreqTextSize)));
        }
        butt.setText(freq);
        //butt.setGravity(mContext.getResources().getInteger(R.integer.dragview_text_gravity));
        butt.setGravity(SkinUtils.getInteger(R.integer.dragview_text_gravity));

        butt.setPadding(0, 5, 0, 0);
        butt.setMaxLines(0x01);

        return butt;
    }

    private int matchingTarget(int x, int y) {
        int index = -1;

        if (mMinAimedRangeY[0] != mMinAimedRangeY[1]) {//Vertical
            if (x >= mMinAimedRangeX[0] && x <= mMaxAimedRangeX[0]) {
                for (int i = 0; i < FMDataControl.PAGE_STATION_NUM; i++) {//preset row1
                    if (mMinAimedRangeX[i] == mMinAimedRangeX[0] && y > mMinAimedRangeY[i]
                            && y < mMaxAimedRangeY[i]) {
                        index = i;
                        break;
                    }
                }
            }
        } else {//horizontal
            if (y >= mMinAimedRangeY[0] && y <= mMaxAimedRangeY[0]) {
                for (int i = 0; i < FMDataControl.PAGE_STATION_NUM; i++) {//preset row1
                    if (mMinAimedRangeY[i] == mMinAimedRangeY[0] && x > mMinAimedRangeX[i]
                            && x < mMaxAimedRangeX[i]) {
                        index = i;
                        break;
                    }
                }
            } else if (y >= mMinAimedRangeY[5] && y <= mMaxAimedRangeY[5]) {//preset row2
                for (int i = 0; i < FMDataControl.PAGE_STATION_NUM; i++) {
                    if (mMinAimedRangeY[i] == mMinAimedRangeY[5] && x > mMinAimedRangeX[i]
                            && x < mMaxAimedRangeX[i]) {
                        index = i;
                        break;
                    }
                }
            }
        }

        return index;
    }

    private String getStringCurrentFreq() {
        if (mFMCallBack.getCurrentBand() < RadioData.BAND_AM_1) {
            return String.format(Locale.ENGLISH, "%.02f", mFMCallBack.getCurrentFreq() * 0.001);
        }

        return Integer.toString(mFMCallBack.getCurrentFreq());
    }


    public boolean getPresetState() {
        return mIsLongClick;
    }

    public void setPresetState(boolean longClick) {
        mIsLongClick = longClick;

        if (null != mPresetCallBack) {
            mPresetCallBack.setPresetViewEnabled(!mIsLongClick);
        }
    }


    public int getPresetStoreItem() {
        return mPresetStoreItem;
    }

    public int getPresetItemVisibleIndex() {
        return mPresetStoreItem % FMDataControl.PAGE_STATION_NUM;
    }

    public boolean setPresetStoreItem(int nIndex) {

        if (nIndex >= -1 && nIndex <= 17) {
            mPresetStoreItem = nIndex;

            return true;
        }

        return false;
    }

    public boolean isDragStore() {
        return mIsDragStoreFM;
    }


    public void setDragStoreFlag(boolean mIsDragStoreFM) {
        this.mIsDragStoreFM = mIsDragStoreFM;
    }


    public boolean isCollimation() {
        return mIsCollimation;
    }

    public void setCollimationFlag(boolean mIsCollimation) {
        this.mIsCollimation = mIsCollimation;
    }

    public void autoStoreAnimation(View view) {

        if (null == view) {
            return;
        }

        AnimationSet animationSet = new AnimationSet(true);
        animationSet.addAnimation(new AlphaAnimation(0.0F, 1.0F));
        animationSet.addAnimation(new ScaleAnimation(2.5F, 1.0F, 2.0F, 1.0F, 1,
                0.5F, 1, 0.5F));

        animationSet.setDuration(500L);
        animationSet.setFillAfter(true);
        animationSet.setFillBefore(true);
        view.startAnimation(animationSet);

        if (!isDragStore()) {

            AutoStoreAnimationListener animationListener = new AutoStoreAnimationListener();
            animationListener.setTargetView(view);
            animationSet.setAnimationListener(animationListener);
        } else {
            DragStoreAnimationListener animationListener = new DragStoreAnimationListener();
            animationListener.setTargetView(view);
            animationSet.setAnimationListener(animationListener);
        }
    }

    private final class AutoStoreAnimationListener implements AnimationListener {

        private View mView = null;
        private int nPresetIndex = getPresetItemVisibleIndex();

        public void setTargetView(View view) {
            mView = view;
        }

        @Override
        public void onAnimationStart(Animation animation) {
        }

        @Override
        public void onAnimationEnd(Animation animation) {
            if (-1 != nPresetIndex && (mView instanceof DragContainer)) {

                OnScrollListener scrollListener = new OnScrollListener();
                scrollListener.setTargetView(mView);
                ((DragContainer) mView).setOnScrollListener(scrollListener);

                ((DragContainer) mView).setMoveDuration(300 + Math
                        .abs(nPresetIndex - 1) * 50);

                ((DragContainer) mView).setTargetCenterPosAnimated(
                        mPresetCX[nPresetIndex] + CALIBRATE_OFFSET_X,
                        mPresetCY[nPresetIndex] - CALIBRATE_OFFSET_Y); // Scroll

            } else {
                setPresetState(false);
                //使用handler避免从后台切换前台时异常
                new Handler().post(new Runnable() {
                    @Override
                    public void run() {
                        mDragPanel.removeView(mView);
                    }
                });
            }
        }

        @Override
        public void onAnimationRepeat(Animation animation) {
        }
    }

    private final class DragStoreAnimationListener implements AnimationListener {

        private View mView = null;

        public void setTargetView(View view) {
            mView = view;
        }

        @Override
        public void onAnimationStart(Animation animation) {
        }

        @Override
        public void onAnimationEnd(Animation animation) {

        }

        @Override
        public void onAnimationRepeat(Animation animation) {

        }

    }

    private final class OnScrollListener implements IScrollerListener {

        private View mView = null;
        private boolean mIsMoving = false;

        public void setTargetView(View view) {
            mView = view;
        }

        @Override
        public void scrollStart() {
            mIsMoving = true;
        }

        @Override
        public void scrollProcess(Scroller scroller) {
            mIsMoving = true;
        }

        @Override
        public void scrollEnd() {
            if (mIsMoving && (null != mView)) {
                autoMatchingDisappear(mView);
            }
        }
    }

    public void autoMatchingDisappear(View view) {
        AnimationSet animationSet = new AnimationSet(true);

        animationSet.addAnimation(new AlphaAnimation(1.0F, 0.0F));
        animationSet.addAnimation(new TranslateAnimation(0,
                -CALIBRATE_OFFSET_X, 0, CALIBRATE_OFFSET_Y));

        animationSet.setDuration(300L);
        animationSet.setFillAfter(true);
        animationSet.setFillBefore(true);
        view.startAnimation(animationSet);
        view.setVisibility(View.GONE);

        DisappearAnimationListener animationListener = new DisappearAnimationListener();
        animationListener.setTargetView(view);
        animationListener.setStoreStationIndex(getPresetStoreItem());
        animationSet.setAnimationListener(animationListener);
    }

    public void dragMatchingDisappear(View view) {
        AnimationSet animationSet = new AnimationSet(true);

        animationSet.addAnimation(new AlphaAnimation(1.0F, 0.0F));
        int toDeltaX = mPresetCX[mMatchingTargetIdx] - mCurrDragCellTouchX;
        int toDeltaY = mPresetCY[mMatchingTargetIdx] - mCurrDragCellTouchY;
        animationSet.addAnimation(new TranslateAnimation(0, toDeltaX, 0,
                toDeltaY));

        animationSet.setDuration(300L);
        animationSet.setFillAfter(true);
        animationSet.setFillBefore(true);
        view.startAnimation(animationSet);
        view.setVisibility(View.GONE);

        int nStationIndex = 0;
        nStationIndex = mMatchingTargetIdx;

        if (null != mPresetCallBack) {
            mPresetCallBack.setPresetViewAimed(mMatchingTargetIdx, false);
            mMatchingTargetIdx = -1;
        }

        DisappearAnimationListener animationListener = new DisappearAnimationListener();
        animationListener.setTargetView(view);
        animationListener.setStoreStationIndex(nStationIndex);
        animationSet.setAnimationListener(animationListener);
    }


    private final class DisappearAnimationListener implements AnimationListener {

        private View mView = null;
        private int mStoreIndex = 0;

        public void setStoreStationIndex(int index) {
            mStoreIndex = index;
        }

        public void setTargetView(View view) {
            mView = view;
        }

        @Override
        public void onAnimationStart(Animation animation) {
            if (null != mDragControlEvent) {
                mDragControlEvent.FMStoreStationEvent(mStoreIndex);
            }
        }

        @Override
        public void onAnimationRepeat(Animation animation) {
        }

        @Override
        public void onAnimationEnd(Animation animation) {
            if (null != mView) {
                if (isDragStore()) {
                    setPresetState(false);
                    setDragStoreFlag(false);
                    resetDigitFreqCell();
                } else {
                    setPresetState(false);
                    setPresetStoreItem(-1);
                    //使用handler避免从后台切换前台时异常
                    new Handler().post(new Runnable() {
                        @Override
                        public void run() {
                            mDragPanel.removeView(mView);
                        }
                    });
                }
            }
        }
    }

    public void setCurrFreqScaleCenterPos(int cx, int cy) {
        mCurrFreqScaleCenterX = cx;
        mCurrFreqScaleCenterY = cy;
    }

    private DragContainer createDragContainer(boolean isAuto) {

        setPresetState(true);
        String freq = getStringCurrentFreq();
        DragContainer dragCell = new DragContainer(this.mContext);
        mDragPanel.addComponentView(dragCell);

        FMFreeLayout.LayoutParams layoutParams = new FMFreeLayout.LayoutParams();
        dragCell.setView(createLabel(freq), layoutParams);

        if (isAuto) {
            dragCell.setSizeByCenter(mPresetWidth, mPresetHeight);
            dragCell.setCenterPosition(mCurrFreqScaleCenterX, mCurrFreqScaleCenterY);
        }

        dragCell.setFocusable(false);
        dragCell.setVisibility(View.VISIBLE);
        dragCell.bringToFront();

        return dragCell;
    }


    public boolean saveFavoriteFreq(int nPresetIndex) {

        initPresetCenterFixed();

        if (getPresetState()) {
            return false;
        }

        mAutoCell = createDragContainer(true);
        setDragStoreFlag(false);
        setPresetStoreItem(nPresetIndex);
        autoStoreAnimation(mAutoCell);

        return true;
    }

    //用于默认频点处于隐藏状态时
    public void setFirstInitPresetCenterFixed(boolean firstInitPresetCenterFixed) {
            mFirstInitPresetCenterFixed = firstInitPresetCenterFixed;
    }
}
