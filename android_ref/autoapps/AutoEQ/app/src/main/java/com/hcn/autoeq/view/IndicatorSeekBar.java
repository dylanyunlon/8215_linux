package com.hcn.autoeq.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.text.TextPaint;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.widget.SeekBar;

import androidx.appcompat.widget.AppCompatSeekBar;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.BitmapUtils;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.skin.support.resources.SkinCompatResources;

public class IndicatorSeekBar extends AppCompatSeekBar {

    // 画笔
    private Paint mPaint;
    // 进度文字位置信息
    private Rect mProgressTextRect = new Rect();
    // 滑块按钮宽度
    private int mThumbWidth = dp2px(26);
    // 进度指示器宽度
    private int mIndicatorWidth = dp2px(26);
    // 进度监听
    private OnIndicatorSeekBarChangeListener mIndicatorSeekBarChangeListener;
    private Bitmap bitmap = BitmapUtils.drawableToBitmap(SkinUtils.getDrawable(R.drawable.asp_seekbar_box));

    public IndicatorSeekBar(Context context) {
        this(context, null);
    }

    public IndicatorSeekBar(Context context, AttributeSet attrs) {
        this(context, attrs, R.attr.seekBarStyle);
    }

    public IndicatorSeekBar(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        if ("400".equals(EqUtils.getEThemeGod())
                && ("27".equals(EqUtils.getEThemeSub())
                || "027".equals(EqUtils.getEThemeSub()))
            ){

            mThumbWidth = dp2px(24);

        }
        mPaint = new TextPaint();
        mPaint.setAntiAlias(true);
        int skinColor =  SkinCompatResources.getInstance().getColor("indicator_seek_bar_textcolor");
        if (0 != skinColor){
            mPaint.setColor(skinColor);
        }else {
            mPaint.setColor(Color.WHITE);
        }

        mPaint.setTextSize(SkinUtils.getDimension(R.dimen.indicator_seek_tv_size));

        // 如果不设置padding，当滑动到最左边或最右边时，滑块会显示不全
        setPadding(mThumbWidth / 2, 0, mThumbWidth / 2, 0);

        // 设置滑动监听
        this.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (mIndicatorSeekBarChangeListener != null) {
                    mIndicatorSeekBarChangeListener.onProgressChanged(seekBar, progress, fromUser);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                if (mIndicatorSeekBarChangeListener != null) {
                    mIndicatorSeekBarChangeListener.onStartTrackingTouch(seekBar);
                }
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if (mIndicatorSeekBarChangeListener != null) {
                    mIndicatorSeekBarChangeListener.onStopTrackingTouch(seekBar);
                }
            }
        });
    }

    @Override
    protected synchronized void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        // 进度百分比
        float progressRatio = (float) getProgress() / getMax();
        float thumbX = getWidth() * progressRatio - mThumbWidth * progressRatio;
        float thumbY = 0;

        drawRotateBitmap(canvas, mPaint, bitmap, 90, thumbX, thumbY);

        // thumb偏移量
        float thumbOffset = mThumbWidth / 2 - mThumbWidth * progressRatio;
        thumbX = getWidth() * progressRatio + thumbOffset;

        String progressText = (getProgress() - 7) + "";
        mPaint.getTextBounds(progressText, 0, progressText.length(), mProgressTextRect);
        thumbY = mProgressTextRect.height();
        canvas.rotate(90, thumbX, thumbY);
        canvas.drawText(progressText, thumbX - mProgressTextRect.width() / 2, thumbY + mProgressTextRect.height() / 2, mPaint);

    }

    @Override
    public void setProgress(int progress) {
        super.setProgress(progress);
        invalidate(); // 强制触发重绘
    }

    private void drawRotateBitmap(Canvas canvas, Paint paint, Bitmap bitmap, float rotation, float posX, float posY) {
        Matrix matrix = new Matrix();
        int offsetX = bitmap.getWidth() / 2;
        int offsetY = bitmap.getHeight() / 2;
        matrix.postTranslate(-offsetX, -offsetY);
        matrix.postRotate(rotation);
        matrix.postTranslate(posX + offsetY, posY + offsetX);
        canvas.drawBitmap(bitmap, matrix, paint);
    }

    /**
     * 设置进度监听
     *
     * @param listener OnIndicatorSeekBarChangeListener
     */
    public void setOnSeekBarChangeListener(OnIndicatorSeekBarChangeListener listener) {
        this.mIndicatorSeekBarChangeListener = listener;
    }

    /**
     * 进度监听
     */
    public interface OnIndicatorSeekBarChangeListener {

        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser);

        /**
         * 开始拖动
         *
         * @param seekBar SeekBar
         */
        public void onStartTrackingTouch(SeekBar seekBar);

        /**
         * 停止拖动
         *
         * @param seekBar SeekBar
         */
        public void onStopTrackingTouch(SeekBar seekBar);
    }

    /**
     * dp转px
     *
     * @param dp dp值
     * @return px值
     */
    public int dp2px(float dp) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dp,
                getResources().getDisplayMetrics());
    }

    /**
     * sp转px
     *
     * @param sp sp值
     * @return px值
     */
    private int sp2px(float sp) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP, sp,
                getResources().getDisplayMetrics());
    }
}
