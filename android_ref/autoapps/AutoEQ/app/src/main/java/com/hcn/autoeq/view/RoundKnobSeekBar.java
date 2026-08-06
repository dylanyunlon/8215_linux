package com.hcn.autoeq.view;

import static android.view.MotionEvent.ACTION_CANCEL;
import static android.view.MotionEvent.ACTION_DOWN;
import static android.view.MotionEvent.ACTION_MOVE;
import static android.view.MotionEvent.ACTION_UP;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Region;
import android.graphics.SweepGradient;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.SkinUtils;

/**
 * 圆形旋转的进度控件
 */
public class RoundKnobSeekBar extends View {
    private static final String TAG = RoundKnobSeekBar.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(RoundKnobSeekBar.class.getSimpleName(), Log.DEBUG);

    private static int DEFAULT_EDGE_LENGTH;              // 默认宽高，一般为底图的尺寸
    private static final float CIRCLE_ANGLE = 360;                  // 圆周角，一般不需要修改
    private static final int DEFAULT_ARC_WIDTH = 11;                // 默认宽度 dp
    public static final float DEFAULT_OPEN_ANGLE = 120;            // 开口角度
    private static final float DEFAULT_ROTATE_ANGLE = 90;           // 旋转角度，一般不需要修改
    private static final int DEFAULT_BORDER_WIDTH = 0;              // 默认描边宽度
    private static final int DEFAULT_SHADOW_RADIUS = 0;             // 默认阴影半径 dp
    private static final int DEFAULT_MAX_VALUE = 200;               // 默认最大数值
    private static final int DEFAULT_MIN_VALUE = 0;                 // 默认最小数值
    private static final int PIC_TRANSPARENT_DISTANCE = 0; // 默认偏离圆心的距离，数字越小，距离越远

    // 可配置数据
    private int[] mArcColors;       // Seek 颜色
    private float mArcWidth;        // Seek 宽度
    private float mOpenAngle;       // 开口的角度大小 0 - 360
    private float mRotateAngle;     // 旋转角度
    private int mBorderWidth;       // 描边宽度
    private int mPicTransparentDistance; // 偏离圆心的距离，数字越小，距离越远

    private int mShadowRadius;      // 阴影半径

    private int mMaxValue;          // 最大数值
    private int mMinValue;          // 最小数值

    private float mCenterX;         // 圆弧 SeekBar 中心点 X
    private float mCenterY;         // 圆弧 SeekBar 中心点 Y

    private Paint mArcPaint;

    private float[] mTempPos;
    private float[] mTempTan;
    private float mProgressPresent = 0;         // 当前进度百分比
    //    private boolean mCanDrag = false;           // 是否允许拖动
    private boolean mAllowTouchSkip = false;    // 是否允许越过边界
    private GestureDetector mDetector;
    private Matrix mInvertMatrix;               // 逆向 Matrix, 用于计算触摸坐标和绘制坐标的转换
    private Region mArcRegion;                  // ArcPath的实际区域大小,用于判定单击事件
    private RectF mCenterRectF;

    public RoundKnobSeekBar(Context context) {
        this(context, null);
    }

    public RoundKnobSeekBar(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public RoundKnobSeekBar(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        setSaveEnabled(true);
        setLayerType(LAYER_TYPE_SOFTWARE, null);
        initAttrs(context, attrs);
        initData();
        initPaint();
    }

    //--- 初始化 -----------------------------------------------------------------------------------

    // 初始化各种属性
    private void initAttrs(Context context, AttributeSet attrs) {
        TypedArray ta = context.obtainStyledAttributes(attrs, R.styleable.round_knob_seek_bar_attr);
        if (isInEditMode()) {
            mArcColors = new int[]{R.color.theme_color, R.color.theme_color}; // 获取 Arc 颜色数组
        } else {
            mArcColors = new int[]{SkinUtils.getColor(R.color.theme_color), SkinUtils.getColor(R.color.theme_color)}; // 获取 Arc 颜色数组
        }
//        mArcWidth = ta.getDimensionPixelSize(R.styleable.round_knob_seek_bar_attr_arc_width, dp2px(DEFAULT_ARC_WIDTH));
        if (isInEditMode()) {
            mArcWidth = R.dimen.extdsp_round_knob_seek_bar_arc_width;
        } else {
            mArcWidth = SkinUtils.getDimension(R.dimen.extdsp_round_knob_seek_bar_arc_width);
        }
//        mOpenAngle = ta.getFloat(R.styleable.round_knob_seek_bar_attr_arc_open_angle, DEFAULT_OPEN_ANGLE);
        mOpenAngle = 94;
        mRotateAngle = ta.getFloat(R.styleable.round_knob_seek_bar_attr_arc_rotate_angle, DEFAULT_ROTATE_ANGLE);
        mMaxValue = ta.getInt(R.styleable.round_knob_seek_bar_attr_arc_max, DEFAULT_MAX_VALUE);
        mMinValue = ta.getInt(R.styleable.round_knob_seek_bar_attr_arc_min, DEFAULT_MIN_VALUE);
        // 如果用户设置的最大值和最小值不合理，则直接按照默认进行处理
        if (mMaxValue <= mMinValue) {
            mMaxValue = DEFAULT_MAX_VALUE;
            mMinValue = DEFAULT_MIN_VALUE;
        }
        int progress = ta.getInt(R.styleable.round_knob_seek_bar_attr_arc_progress, mMinValue);
        setProgress(progress);
        mBorderWidth = ta.getDimensionPixelSize(R.styleable.round_knob_seek_bar_attr_arc_border_width, dp2px(DEFAULT_BORDER_WIDTH));
        mShadowRadius = ta.getDimensionPixelSize(R.styleable.round_knob_seek_bar_attr_arc_shadow_radius, dp2px(DEFAULT_SHADOW_RADIUS));
//        mPicTransparentDistance = ta.getInt(R.styleable.round_knob_seek_bar_attr_pic_transparent_distance, dp2px(PIC_TRANSPARENT_DISTANCE));
        if (isInEditMode()) {
            mPicTransparentDistance = R.integer.extdsp_round_knob_seek_bar_pic_transparent_distance;
        } else {
            mPicTransparentDistance = SkinUtils.getInteger(R.integer.extdsp_round_knob_seek_bar_pic_transparent_distance);
        }
        ta.recycle();
    }

    // 根据 resId 获取颜色数组
    private int[] getColorsByArrayResId(Context context, int resId) {
        int[] ret;
        TypedArray colorArray = context.getResources().obtainTypedArray(resId);
        ret = new int[colorArray.length()];
        for (int i = 0; i < colorArray.length(); i++) {
            ret[i] = colorArray.getColor(i, 0);
        }
        return ret;
    }

    // 初始化数据
    private void initData() {
        mTempPos = new float[2];
        mTempTan = new float[2];
        mDetector = new GestureDetector(getContext(), new OnClickListener());
        mInvertMatrix = new Matrix();
        mArcRegion = new Region();
        if (isInEditMode()) {
            DEFAULT_EDGE_LENGTH = (int) getResources().getDimension(R.dimen.extdsp_round_knob_seek_bar_width);
        } else {
            DEFAULT_EDGE_LENGTH = (int) SkinUtils.getDimension(R.dimen.extdsp_round_knob_seek_bar_width);
        }
    }

    // 初始化画笔
    private void initPaint() {
        initArcPaint();
    }

    // 初始化圆弧画笔
    private void initArcPaint() {
        mArcPaint = new Paint();
        mArcPaint.setAntiAlias(true);
        mArcPaint.setStrokeWidth(mArcWidth);
        mArcPaint.setStyle(Paint.Style.STROKE);
        mArcPaint.setStrokeCap(Paint.Cap.BUTT);
        mArcPaint.setColor(getColor());
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int ws = MeasureSpec.getSize(widthMeasureSpec);     //取出宽度的确切数值
        int wm = MeasureSpec.getMode(widthMeasureSpec);     //取出宽度的测量模式
        int hs = MeasureSpec.getSize(heightMeasureSpec);    //取出高度的确切数值
        int hm = MeasureSpec.getMode(heightMeasureSpec);    //取出高度的测量模

        if (wm == MeasureSpec.UNSPECIFIED) {
            wm = MeasureSpec.EXACTLY;
            ws = dp2px(DEFAULT_EDGE_LENGTH);
        } else if (wm == MeasureSpec.AT_MOST) {
            wm = MeasureSpec.EXACTLY;
            ws = Math.min(dp2px(DEFAULT_EDGE_LENGTH), ws);
        }
        if (hm == MeasureSpec.UNSPECIFIED) {
            hm = MeasureSpec.EXACTLY;
            hs = dp2px(DEFAULT_EDGE_LENGTH);
        } else if (hm == MeasureSpec.AT_MOST) {
            hm = MeasureSpec.EXACTLY;
            hs = Math.min(dp2px(DEFAULT_EDGE_LENGTH), hs);
        }
        setMeasuredDimension(MeasureSpec.makeMeasureSpec(ws, wm), MeasureSpec.makeMeasureSpec(hs, hm));
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        // 计算在当前大小下,内容应该显示的大小和起始位置
        int safeW = w - getPaddingLeft() - getPaddingRight() - mPicTransparentDistance;
        int safeH = h - getPaddingTop() - getPaddingBottom() - mPicTransparentDistance;
        float edgeLength, startX, startY;
        float fix = mArcWidth / 2 + mBorderWidth + mShadowRadius * 2;  // 修正距离,画笔宽度的修正
        if (DEBUG) {
            Log.d(TAG, "fix : " + fix + ", arc width : " + mArcWidth + ", border width :" + mBorderWidth
                    + ", shadow radius : " + mShadowRadius + ", safeW : " + safeW + ", safeH : " + safeH
                    + ", w : " + w + ", h : " + h + ", padding left : " + getPaddingLeft() + ", padding top : " + getPaddingTop());
        }
        if (safeW < safeH) {
            // 宽度小于高度,以宽度为准
            edgeLength = safeW - fix;
            startX = getPaddingLeft();
            startY = (safeH - safeW) / 2.0f + getPaddingTop();
        } else {
            // 宽度大于高度,以高度为准
            edgeLength = safeH - fix;
            startX = (safeW - safeH) / 2.0f + getPaddingLeft() + mPicTransparentDistance / 2;
            startY = getPaddingTop() + mPicTransparentDistance / 2;
        }

        // 得到显示区域和中心点
        mCenterRectF = new RectF(startX + fix + 3, startY + fix + 3, startX + edgeLength - 3, startY + edgeLength - 3);
        mCenterX = mCenterRectF.centerX();
        mCenterY = mCenterRectF.centerY();
        resetShaderColor();

        mInvertMatrix.reset();
        mInvertMatrix.preRotate(-mRotateAngle, mCenterX, mCenterY);
    }

    // 重置 shader 颜色
    private void resetShaderColor() {
        // 计算渐变数组
        float startPos = (mOpenAngle / 2) / CIRCLE_ANGLE;
        float stopPos = (CIRCLE_ANGLE - (mOpenAngle / 2)) / CIRCLE_ANGLE;
        int len = mArcColors.length - 1;
        float distance = (stopPos - startPos) / len;
        float pos[] = new float[mArcColors.length];
        for (int i = 0; i < mArcColors.length; i++) {
            pos[i] = startPos + (distance * i);
        }
        SweepGradient gradient = new SweepGradient(mCenterX, mCenterY, mArcColors, pos);
        mArcPaint.setShader(gradient);
    }

    // 具体绘制
    @Override
    protected void onDraw(Canvas canvas) {
        canvas.save();
        canvas.rotate(mRotateAngle, mCenterX, mCenterY);
        canvas.drawArc(mCenterRectF, mOpenAngle / 2, (CIRCLE_ANGLE - mOpenAngle) * mProgressPresent, false, mArcPaint);
        canvas.restore();
    }

    private boolean moved = false;
    private float lastProgress = -1;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        super.onTouchEvent(event);
        if (mOnSeekBarEnableListener != null) {
            if (!mOnSeekBarEnableListener.setSeekBarEnable(this)) {
                return true;
            }
        }
        int action = event.getActionMasked();
        switch (action) {
            case ACTION_DOWN:
                moved = false;
                judgeCanDrag(event);
                if (null != mOnProgressChangeListener) {
                    mOnProgressChangeListener.onStartTrackingTouch(this);
                }
                break;
            case ACTION_MOVE:
                float tempProgressPresent = getCurrentProgress(event.getX(), event.getY());
                if (!mAllowTouchSkip) {
                    // 不允许突变
                    if (Math.abs(tempProgressPresent - mProgressPresent) > 0.5f) {
                        break;
                    }
                }
                // 允许突变 或者非突变
                mProgressPresent = tempProgressPresent;
//                computeThumbPos(mProgressPresent);
                // 事件回调
                if (null != mOnProgressChangeListener && getProgress() != lastProgress) {
                    mOnProgressChangeListener.onProgressChanged(this, getProgress(), true);
                    lastProgress = getProgress();
                }
                moved = true;
                break;
            case ACTION_UP:
            case ACTION_CANCEL:
                if (null != mOnProgressChangeListener && moved) {
                    mOnProgressChangeListener.onStopTrackingTouch(this);
                }
                break;
        }
        mDetector.onTouchEvent(event);
        invalidate();
        return true;
    }

    // 判断是否允许拖动
    private void judgeCanDrag(MotionEvent event) {
        float[] pos = {event.getX(), event.getY()};
        mInvertMatrix.mapPoints(pos);
    }

    private class OnClickListener extends GestureDetector.SimpleOnGestureListener {
        @Override
        public boolean onSingleTapUp(MotionEvent e) {
            // 判断是否点击在了进度区域
            if (!isInArcProgress(e.getX(), e.getY())) return false;
            // 点击允许突变
            mProgressPresent = getCurrentProgress(e.getX(), e.getY());
//            computeThumbPos(mProgressPresent);
            // 事件回调
            if (null != mOnProgressChangeListener) {
                mOnProgressChangeListener.onProgressChanged(RoundKnobSeekBar.this, getProgress(), true);
                mOnProgressChangeListener.onStopTrackingTouch(RoundKnobSeekBar.this);
            }
            return true;
        }
    }

    // 判断该点是否在进度条上面
    private boolean isInArcProgress(float px, float py) {
        float[] pos = {px, py};
        mInvertMatrix.mapPoints(pos);
        return mArcRegion.contains((int) pos[0], (int) pos[1]);
    }

    // 获取当前进度理论进度数值
    private float getCurrentProgress(float px, float py) {
        float diffAngle = getDiffAngle(px, py);
        float progress = diffAngle / (CIRCLE_ANGLE - mOpenAngle);
        if (progress < 0) progress = 0;
        if (progress > 1) progress = 1;
        return progress;
    }

    // 获得当前点击位置所成角度与开始角度之间的数值差
    private float getDiffAngle(float px, float py) {
        float angle = getAngle(px, py);
        float diffAngle;
        diffAngle = angle - mRotateAngle;
        if (diffAngle < 0) {
            diffAngle = (diffAngle + CIRCLE_ANGLE) % CIRCLE_ANGLE;
        }
        diffAngle = diffAngle - mOpenAngle / 2;
        if (DEBUG) {
            Log.d(TAG, "diffAngle : " + diffAngle + ", angle : " + angle);
        }
        return diffAngle;
    }

    // 计算指定位置与内容区域中心点的夹角
    private float getAngle(float px, float py) {
        float angle = (float) ((Math.atan2(py - mCenterY, px - mCenterX)) * 180 / 3.14f);
        if (angle < 0) {
            angle += 360;
        }
        if (DEBUG) {
            Log.d(TAG, "angle : " + angle);
        }
        return angle;
    }

    private int dp2px(int dp) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dp, getContext().getResources().getDisplayMetrics());
    }

    /**
     * 获取当前进度的具体颜色
     *
     * @return 当前进度在渐变中的颜色
     */
    public int getColor() {
        return getColor(mProgressPresent);
    }

    /**
     * 获取某个百分比位置的颜色
     *
     * @param radio 取值[0,1]
     * @return 最终颜色
     */
    private int getColor(float radio) {
        float diatance = 1.0f / (mArcColors.length - 1);
        int startColor;
        int endColor;
        if (radio >= 1) {
            return mArcColors[mArcColors.length - 1];
        }
        for (int i = 0; i < mArcColors.length; i++) {
            if (radio <= i * diatance) {
                if (i == 0) {
                    return mArcColors[0];
                }
                startColor = mArcColors[i - 1];
                endColor = mArcColors[i];
                float areaRadio = getAreaRadio(radio, diatance * (i - 1), diatance * i);
                return getColorFrom(startColor, endColor, areaRadio);
            }
        }
        return -1;
    }

    /**
     * 计算当前比例在子区间的比例
     *
     * @param radio         总比例
     * @param startPosition 子区间开始位置
     * @param endPosition   子区间结束位置
     * @return 自区间比例[0, 1]
     */
    private float getAreaRadio(float radio, float startPosition, float endPosition) {
        return (radio - startPosition) / (endPosition - startPosition);
    }

    /**
     * 取两个颜色间的渐变区间 中的某一点的颜色
     *
     * @param startColor 开始的颜色
     * @param endColor   结束的颜色
     * @param radio      比例 [0, 1]
     * @return 选中点的颜色
     */
    private int getColorFrom(int startColor, int endColor, float radio) {
        int redStart = Color.red(startColor);
        int blueStart = Color.blue(startColor);
        int greenStart = Color.green(startColor);
        int redEnd = Color.red(endColor);
        int blueEnd = Color.blue(endColor);
        int greenEnd = Color.green(endColor);

        int red = (int) (redStart + ((redEnd - redStart) * radio + 0.5));
        int greed = (int) (greenStart + ((greenEnd - greenStart) * radio + 0.5));
        int blue = (int) (blueStart + ((blueEnd - blueStart) * radio + 0.5));
        return Color.argb(255, red, greed, blue);
    }

    //region 对外接口 -------------------------------------------------------------------------------

    /**
     * 设置进度
     *
     * @param progress 进度值
     */
    public void setProgress(int progress) {
        if (progress > mMaxValue) progress = mMaxValue;
        if (progress < mMinValue) progress = mMinValue;
        mProgressPresent = (progress - mMinValue) * 1.0f / (mMaxValue - mMinValue);
        if (null != mOnProgressChangeListener) {
            mOnProgressChangeListener.onProgressChanged(this, progress, false);
        }
//        computeThumbPos(mProgressPresent);
        postInvalidate();
    }

    /**
     * 获取当前进度数值
     * 返回 float 类型，以便前端精确的旋转图片
     *
     * @return 当前进度数值
     */
    public float getProgress() {
        return (mProgressPresent * (mMaxValue - mMinValue)) + mMinValue;
    }

    public float getRotateAngle() {
        return (CIRCLE_ANGLE - mOpenAngle) * mProgressPresent;
    }

    /**
     * 设置颜色
     *
     * @param colors 颜色
     */
    public void setArcColors(int[] colors) {
        mArcColors = colors;
        resetShaderColor();
        postInvalidate();
    }

    /**
     * 设置最大数值
     *
     * @param max 最大数值
     */
    public void setMaxValue(int max) {
        mMaxValue = max;
    }

    /**
     * 设置最小数值
     *
     * @param min 最小数值
     */
    public void setMinValue(int min) {
        mMinValue = min;
    }
    // endregion -----------------------------------------------------------------------------------

    // region 状态回调 ------------------------------------------------------------------------------
    private OnProgressChangeListener mOnProgressChangeListener;

    public void setOnProgressChangeListener(OnProgressChangeListener onProgressChangeListener) {
        mOnProgressChangeListener = onProgressChangeListener;
    }

    public interface OnProgressChangeListener {
        /**
         * 进度发生变化
         *
         * @param seekBar  拖动条
         * @param progress 当前进度数值
         * @param fromUser 是否是用户操作, true 表示用户拖动, false 表示通过代码设置
         */
        void onProgressChanged(RoundKnobSeekBar seekBar, float progress, boolean fromUser);

        /**
         * 用户开始拖动
         *
         * @param seekBar 拖动条
         */
        void onStartTrackingTouch(RoundKnobSeekBar seekBar);

        /**
         * 用户结束拖动
         *
         * @param seekBar 拖动条
         */
        void onStopTrackingTouch(RoundKnobSeekBar seekBar);
    }

    private OnSeekBarEnableListener mOnSeekBarEnableListener;

    public void setOnSeekBarEnableListener(OnSeekBarEnableListener onSeekBarEnableListener) {
        mOnSeekBarEnableListener = onSeekBarEnableListener;
    }

    public interface OnSeekBarEnableListener {
        /**
         * 设置BalanceSeekBar 是否可以拖动
         *
         * @param seekBar 拖动条
         */
        boolean setSeekBarEnable(RoundKnobSeekBar seekBar);
    }
    // endregion -----------------------------------------------------------------------------------
}
