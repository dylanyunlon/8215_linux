package com.hcn.media_view.lyrics;

import android.animation.ValueAnimator;
import android.animation.ValueAnimator.AnimatorUpdateListener;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;
import android.widget.Scroller;

import com.hcn.media_view.R;
import java.util.List;

/**
 * 歌词视图
 * <pre> 结合 LyricsManager 和 LyricsRow 两个类使用；
 *
 * @author 86158
 */
public class LyricsView extends View {
    public static final float MAX_SCALING_FACTOR = 1.5f;
    public static final float MIN_SCALING_FACTOR = 0.5f;

    /**
     * 没有歌词时的默认显示
     * <p> 尽可能不要和无歌词提示信息混合显示，体验不好；
     */
    private static final String DEFAULT_TEXT = "Lyrics";
    private static final float SIZE_FOR_DEFAULT_TEXT = 20;

    private static final float DEFAULT_SIZE_FOR_HIGH_LIGHT_LRC = 26;
    private static final int DEFAULT_COLOR_FOR_HIGH_LIGHT_LRC = 0xffffffff;
    private static final float DEFAULT_SIZE_FOR_OTHER_LRC = 20;
    private static final int DEFAULT_COLOR_FOR_OTHER_LRC = 0x14ffffff;
    private static final int COLOR_FOR_TIME_LINE = 0xffD02090;
    private static final int SIZE_FOR_TIME = 18;
    private static final float DEFAULT_PADDING = 10;
    private static final float DEFAULT_SCALING_FACTOR = 1.0f;
    private static final int DURATION_FOR_LRC_SCROLL = 1500;
    private static final int DURATION_FOR_ACTION_UP = 400;

    private List<LyricsRow> mLrcRows;
    private Paint mPaintForHighLightLrc;
    private float mCurSizeForHighLightLrc = DEFAULT_SIZE_FOR_HIGH_LIGHT_LRC;
    private int mCurColorForHighLightLrc = DEFAULT_COLOR_FOR_HIGH_LIGHT_LRC;
    private Paint mPaintForOtherLrc;
    private float mCurSizeForOtherLrc = DEFAULT_SIZE_FOR_OTHER_LRC;
    private int mCurColorForOtherLrc = DEFAULT_COLOR_FOR_OTHER_LRC;
    private Paint mPaintForTimeLine;
    private boolean mIsDrawTimeLine = false;
    private float mCurPadding = DEFAULT_PADDING;
    private float mCurScalingFactor = DEFAULT_SCALING_FACTOR;
    private Scroller mScroller;
    private float mCurFraction = 0;
    private int mTouchSlop;
    private int mTotalDrawRow = 0;

    private ValueAnimator mAnimator;
    private boolean canDrag = false;
    private float firstY;
    private float lastY;
    private float lastX;
    private int mCurRow = -1;
    private int mLastRow = -1;
    private float mCurTextXForHighLightLrc;
    private boolean alignLeft = false;

    AnimatorUpdateListener updateListener = new AnimatorUpdateListener() {
        @Override
        public void onAnimationUpdate(ValueAnimator animation) {
            mCurTextXForHighLightLrc = (Float) animation.getAnimatedValue();
            log("mCurTextXForHighLightLrc = " + mCurTextXForHighLightLrc);
            invalidate();
        }
    };

    private OnSeekToListener onSeekToListener;
    private OnLrcClickListener onLrcClickListener;

    public LyricsView(Context context) {
        super(context);
        init();
    }

    public LyricsView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();

        TypedArray ta = context.obtainStyledAttributes(attrs, R.styleable.LyricsView);
        int lrcSize = ta.getInteger(R.styleable.LyricsView_lyrics_other_lrc, (int) DEFAULT_SIZE_FOR_OTHER_LRC);
        int hightLightSize = ta.getInteger(R.styleable.LyricsView_lyrics_hightlight_lrc,(int) DEFAULT_SIZE_FOR_HIGH_LIGHT_LRC);
        int lrcLineSpacing = ta.getInteger(R.styleable.LyricsView_lyrics_lineSpacing_lrc,(int) DEFAULT_PADDING);
        ta.recycle();

        if (lrcSize == DEFAULT_SIZE_FOR_OTHER_LRC) {
            int resId = context.getResources().getIdentifier(
                    "lrc_text_size", "integer", context.getPackageName());
            if (resId > 0) {
                mCurSizeForOtherLrc = context.getResources().getInteger(resId);
            }
        }
        if (hightLightSize == DEFAULT_SIZE_FOR_HIGH_LIGHT_LRC) {
            int resId = context.getResources().getIdentifier(
                    "lrc_hightlight_text_size", "integer", context.getPackageName());
            if (resId > 0) {
                mCurSizeForHighLightLrc = context.getResources().getInteger(resId);
            }
        }
        if (lrcLineSpacing == DEFAULT_PADDING) {
            int resId = context.getResources().getIdentifier(
                    "lyrics_lineSpacing_lrc", "integer", context.getPackageName());
            if (resId > 0) {
                mCurPadding = context.getResources().getInteger(resId);
            }
        }
    }

    public void init() {
        mScroller = new Scroller(getContext());
        mPaintForHighLightLrc = new Paint();
        mPaintForHighLightLrc.setColor(mCurColorForHighLightLrc);
        mPaintForHighLightLrc.setTextSize(mCurSizeForHighLightLrc);
        mPaintForHighLightLrc.setTypeface(Typeface.SERIF);

        mPaintForOtherLrc = new Paint();
        mPaintForOtherLrc.setColor(mCurColorForOtherLrc);
        mPaintForOtherLrc.setTextSize(mCurSizeForOtherLrc);
        mPaintForOtherLrc.setTypeface(Typeface.SERIF);

        mPaintForTimeLine = new Paint();
        mPaintForTimeLine.setColor(COLOR_FOR_TIME_LINE);
        mPaintForTimeLine.setTextSize(SIZE_FOR_TIME);
        mPaintForTimeLine.setTypeface(Typeface.SERIF);

        mTouchSlop = ViewConfiguration.get(getContext()).getScaledTouchSlop();
    }

    public void SetPainTypeface(Typeface type) {
        if (mPaintForHighLightLrc != null) {
            mPaintForHighLightLrc.setTypeface(type);
        }

        if (mPaintForOtherLrc != null) {
            mPaintForOtherLrc.setTypeface(type);
        }

        if (mPaintForTimeLine != null) {
            mPaintForTimeLine.setTypeface(type);
        }
    }

    public void SetCurPaintColor(int color) {
        if (mPaintForHighLightLrc != null) {
            mPaintForHighLightLrc.setColor(color);
            mCurColorForHighLightLrc = color;
        }
    }

    public void SetNotCurPaintColor(int color) {
        if (mPaintForOtherLrc != null) {
            mPaintForOtherLrc.setColor(color);
            mCurColorForOtherLrc = color;
        }
    }

    public void SetNotTimeLinePaintColor(int color) {
        if (mPaintForTimeLine != null) {
            mPaintForTimeLine.setColor(color);
        }
    }

    public void setLrcScalingFactor(float scalingFactor) {
        mCurScalingFactor = scalingFactor;
        mCurSizeForHighLightLrc = DEFAULT_SIZE_FOR_HIGH_LIGHT_LRC * mCurScalingFactor;
        mCurSizeForOtherLrc = DEFAULT_SIZE_FOR_OTHER_LRC * mCurScalingFactor;
        mCurPadding = DEFAULT_PADDING * mCurScalingFactor;
        mTotalDrawRow = (int) (getHeight() / (mCurSizeForOtherLrc + mCurPadding)) + 3;

        log("mRowTotal = " + mTotalDrawRow);
        scrollTo(getScrollX(), (int) (mCurRow * (mCurSizeForOtherLrc + mCurPadding)));
        invalidate();

        mScroller.forceFinished(true);
    }

    public void setAlignLeft(boolean alignLeft) {
        this.alignLeft = alignLeft;
    }

    public boolean getAlignLeft() {
        return alignLeft;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        // 绘制默认提示信息
        if (mLrcRows == null || mLrcRows.size() == 0) {
            mPaintForOtherLrc.setTextSize(SIZE_FOR_DEFAULT_TEXT);
            float textWidth = mPaintForOtherLrc.measureText(DEFAULT_TEXT);
            float textX = (getWidth() - textWidth) / 2;
            canvas.drawText(DEFAULT_TEXT, textX, getHeight() / 2.0f, mPaintForOtherLrc);
            return;
        }

        if (mTotalDrawRow == 0) {
            mTotalDrawRow = (int) (getHeight() / (mCurSizeForOtherLrc + mCurPadding)) + 4;
        }

        int minRaw = mCurRow - (mTotalDrawRow - 1) / 2;
        int maxRaw = mCurRow + (mTotalDrawRow - 1) / 2;
        minRaw = Math.max(minRaw, 0);
        maxRaw = Math.min(maxRaw, mLrcRows.size() - 1);
        int count = Math.max(maxRaw - mCurRow, mCurRow - minRaw);
        int alpha = (0xFF);
        if (count != 0) {
            alpha = (0xFF) / count;
        }

        float rowY = getHeight() / 2.0f + minRaw * (mCurSizeForOtherLrc + mCurPadding);
        for (int i = minRaw; i <= maxRaw; i++) {
            if (i == mCurRow) {
                float textSize = mCurSizeForOtherLrc
                        + (mCurSizeForHighLightLrc - mCurSizeForOtherLrc) * mCurFraction;
                mPaintForHighLightLrc.setTextSize(textSize);

                String text = mLrcRows.get(i).getContent();
                float textWidth = mPaintForHighLightLrc.measureText(text);
                if (textWidth > getWidth()) {
                    canvas.drawText(text, mCurTextXForHighLightLrc, rowY, mPaintForHighLightLrc);
                } else {
                    float textX = (getWidth() - textWidth) / 2;
                    if (getAlignLeft()) {
                        textX = 0;
                    }
                    canvas.drawText(text, textX, rowY, mPaintForHighLightLrc);
                }
            } else {
                if (i == mLastRow) {
                    float textSize = mCurSizeForHighLightLrc
                            - (mCurSizeForHighLightLrc - mCurSizeForOtherLrc) * mCurFraction;
                    mPaintForOtherLrc.setTextSize(textSize);
                } else {
                    mPaintForOtherLrc.setTextSize(mCurSizeForOtherLrc);
                }

                String text = mLrcRows.get(i).getContent();
                float textWidth = mPaintForOtherLrc.measureText(text);
                float textX = (getWidth() - textWidth) / 2;

                textX = Math.max(textX, 0);
                int curAlpha = 0XFF - (Math.abs(i - mCurRow) - 1) * alpha;
                mPaintForOtherLrc.setColor(0x1000000 * curAlpha + mCurColorForOtherLrc);
                if (getAlignLeft()) {
                    textX = 0;
                }
                canvas.drawText(text, textX, rowY, mPaintForOtherLrc);
            }

            rowY += mCurSizeForOtherLrc + mCurPadding;
        }

        if (mIsDrawTimeLine) {
            float y = getHeight() / 2.0f + getScrollY();
            canvas.drawText(mLrcRows.get(mCurRow).getTimeInfo(), 0, y - 5, mPaintForTimeLine);
            canvas.drawLine(0, y, getWidth(), y, mPaintForTimeLine);
        }
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (mLrcRows == null || mLrcRows.size() == 0) {
            return false;
        }

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                firstY = event.getRawY();
                lastX = event.getRawX();
                break;
            case MotionEvent.ACTION_MOVE:
                if (!canDrag) {
                    if (Math.abs(event.getRawY() - firstY) > mTouchSlop
                            && Math.abs(event.getRawY() - firstY) > Math.abs(
                            event.getRawX() - lastX)) {
                        canDrag = true;
                        mIsDrawTimeLine = true;
                        mScroller.forceFinished(true);
                        stopScrollLrc();
                        mCurFraction = 1;
                    }

                    lastY = event.getRawY();
                }

                if (canDrag) {
                    float offset = event.getRawY() - lastY;
                    if (getScrollY() - offset < 0) {
                        if (offset > 0) {
                            offset = offset / 3;
                        }
                    } else if (getScrollY() - offset
                            > mLrcRows.size() * (mCurSizeForOtherLrc + mCurPadding) - mCurPadding) {
                        if (offset < 0) {
                            offset = offset / 3;
                        }
                    }

                    scrollBy(getScrollX(), -(int) offset);
                    lastY = event.getRawY();
                    int currentRow = (int) (getScrollY() / (mCurSizeForOtherLrc + mCurPadding));
                    currentRow = Math.min(currentRow, mLrcRows.size() - 1);
                    currentRow = Math.max(currentRow, 0);
                    seekTo(mLrcRows.get(currentRow).getTime(), false, false);
                    return true;
                }

                lastY = event.getRawY();
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                if (!canDrag) {
                    if (onLrcClickListener != null) {
                        onLrcClickListener.onClick();
                    }
                } else {
                    if (onSeekToListener != null && mCurRow != -1) {
                        onSeekToListener.onSeekTo(mLrcRows.get(mCurRow).getTime());
                    }
                    if (getScrollY() < 0) {
                        smoothScrollTo(0, DURATION_FOR_ACTION_UP);
                    } else if (getScrollY()
                            > mLrcRows.size() * (mCurSizeForOtherLrc + mCurPadding) - mCurPadding) {
                        smoothScrollTo((int) (mLrcRows.size() * (mCurSizeForOtherLrc + mCurPadding)
                                        - mCurPadding),
                                DURATION_FOR_ACTION_UP);
                    }

                    canDrag = false;
                    mIsDrawTimeLine = false;
                    invalidate();
                }
                break;
            default:
                break;
        }

        return true;
    }

    public void setLrcRows(List<LyricsRow> lrcRows) {
        reset();
        this.mLrcRows = lrcRows;
        invalidate();
    }

    public void seekTo(int progress, boolean fromSeekBar, boolean fromSeekBarByUser) {
        if (mLrcRows == null || mLrcRows.size() == 0) {
            return;
        }

        if (fromSeekBar && canDrag) {
            return;
        }

        for (int i = mLrcRows.size() - 1; i >= 0; i--) {
            if (progress >= mLrcRows.get(i).getTime()) {
                if (mCurRow != i) {
                    mLastRow = mCurRow;
                    mCurRow = i;

                    log("mCurRow = " + mCurRow);
                    if (fromSeekBarByUser) {
                        if (!mScroller.isFinished()) {
                            mScroller.forceFinished(true);
                        }

                        scrollTo(getScrollX(),
                                (int) (mCurRow * (mCurSizeForOtherLrc + mCurPadding)));
                    } else {
                        smoothScrollTo((int) (mCurRow * (mCurSizeForOtherLrc + mCurPadding)),
                                DURATION_FOR_LRC_SCROLL);
                    }

                    float textWidth = mPaintForHighLightLrc.measureText(
                            mLrcRows.get(mCurRow).getContent());
                    if (textWidth > getWidth()) {
                        if (fromSeekBarByUser) {
                            mScroller.forceFinished(true);
                        }

                        startScrollLrc(getWidth() - textWidth,
                                (long) (mLrcRows.get(mCurRow).getTotalTime() * 0.6));
                    }

                    invalidate();
                }

                break;
            }
        }
    }

    private void startScrollLrc(float endX, long duration) {
        if (mAnimator == null) {
            mAnimator = ValueAnimator.ofFloat(0, endX);
            mAnimator.addUpdateListener(updateListener);
        } else {
            mCurTextXForHighLightLrc = 0;
            mAnimator.cancel();
            mAnimator.setFloatValues(0, endX);
        }

        mAnimator.setDuration(duration);
        mAnimator.setStartDelay((long) (duration * 0.3));
        mAnimator.start();
    }

    private void stopScrollLrc() {
        if (mAnimator != null) {
            mAnimator.cancel();
        }

        mCurTextXForHighLightLrc = 0;
    }

    public void reset() {
        if (!mScroller.isFinished()) {
            mScroller.forceFinished(true);
        }

        mCurRow = -1;
        mLrcRows = null;
        scrollTo(getScrollX(), 0);
        invalidate();
    }

    private void smoothScrollTo(int dstY, int duration) {
        int oldScrollY = getScrollY();
        int offset = dstY - oldScrollY;
        mScroller.startScroll(getScrollX(), oldScrollY, getScrollX(), offset, duration);
        invalidate();
    }

    @Override
    public void computeScroll() {
        if (!mScroller.isFinished()) {
            if (mScroller.computeScrollOffset()) {
                int oldY = getScrollY();
                int y = mScroller.getCurrY();
                if (oldY != y && !canDrag) {
                    scrollTo(getScrollX(), y);
                }

                mCurFraction = mScroller.timePassed() * 3f / DURATION_FOR_LRC_SCROLL;
                mCurFraction = Math.min(mCurFraction, 1F);
                invalidate();
            }
        }
    }

    public float getCurScalingFactor() {
        return mCurScalingFactor;
    }

    public void setOnSeekToListener(OnSeekToListener onSeekToListener) {
        this.onSeekToListener = onSeekToListener;
    }

    public void setOnLrcClickListener(OnLrcClickListener onLrcClickListener) {
        this.onLrcClickListener = onLrcClickListener;
    }

    public void log(Object o) {
    }

    /**
     * 拖动监听
     */
    public interface OnSeekToListener {
        /**
         * 滑动步进事件
         *
         * @param progress
         */
        void onSeekTo(int progress);
    }

    /**
     * 点击监听
     */
    public interface OnLrcClickListener {
        /**
         * 点击处理事件
         */
        void onClick();
    }
}
