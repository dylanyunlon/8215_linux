package com.hcn.autoeq.view;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.animation.LinearInterpolator;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.annotation.Nullable;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.ConstantEq;
import com.hcn.autoeq.util.SkinUtils;

public class AspSeekBar extends RelativeLayout {
    private TextView mTvFreqView;
    private IndicatorSeekBar mIndicatorSeekBar;
    private int mSeekTag;
    private AspSeekBar.OnCustomSeekBarChangeListener mCustomSeekBarChangeListener;

    public AspSeekBar(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        LayoutInflater.from(context).inflate(SkinUtils.getId(R.layout.asp_seekbar), this, true);
        mIndicatorSeekBar = findViewById(SkinUtils.getId(R.id.indicator_seek_bar));
        mIndicatorSeekBar.setMax(ConstantEq.DEFINE_SEEKBAR_MAX);
        mTvFreqView = findViewById(SkinUtils.getId(R.id.tv_freq_asp));

        int count = attrs.getAttributeCount();
        for (int i = 0; i < count; i++) {
            int attributeNameResource = attrs.getAttributeNameResource(i);
            switch (attributeNameResource) {
                case R.attr.seekbar_freq_text:
                    int attributeResourceValue = attrs.getAttributeResourceValue(i, 0);
                    mTvFreqView.setText(getResources().getString(attributeResourceValue));
                    break;
                case R.attr.seekbar_band_tag:
                    int attributeIntValue = attrs.getAttributeIntValue(i, 0);
                    mSeekTag = attributeIntValue;
                    break;
                default:
                    break;
            }
        }

        initData();
    }

    public synchronized void initData() {
        mIndicatorSeekBar.setTag(mSeekTag);
        // FIXME, 换肤方式，在xml里设置 padding=0, offset=0，滑块依然会超出，通过代码设置后正常
        mIndicatorSeekBar.setPadding(0, 0, 0, 0);
        mIndicatorSeekBar.setThumbOffset(0);
        mIndicatorSeekBar.setOnSeekBarChangeListener(new IndicatorSeekBar.OnIndicatorSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (null != mCustomSeekBarChangeListener) {
                    mCustomSeekBarChangeListener.onProgressChanged(seekBar, progress, fromUser);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                if (null != mCustomSeekBarChangeListener) {
                    mCustomSeekBarChangeListener.onStartTrackingTouch(seekBar);
                }
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if (null != mCustomSeekBarChangeListener) {
                    mCustomSeekBarChangeListener.onStopTrackingTouch(seekBar);
                }
            }
        });
    }

    /**
     * 设置进度状态
     *
     * @param index seekbar Progress.
     */
    public void setProgress(int index) {
        mIndicatorSeekBar.setProgress(index);
    }

    public void setProgress(int val, boolean mAnimation) {
        if (mAnimation) {
            volumeGradient(mIndicatorSeekBar, mIndicatorSeekBar.getProgress(), val);
        } else {
            mIndicatorSeekBar.setProgress(val);
        }
    }

    /**
     * 设置进度状态
     *
     * @param mCanSeek 是否可以滑动
     */
    public void setCanSeek(boolean mCanSeek) {
        mIndicatorSeekBar.setAlpha(mCanSeek ? 1.0f : 0.9f);
        mIndicatorSeekBar.setEnabled(mCanSeek);
        mIndicatorSeekBar.setPressed(mCanSeek);
    }

    /**
     * 设置进度监听
     *
     * @param listener OnIndicatorSeekBarChangeListener
     */
    public void setOnSeekBarChangeListener(OnCustomSeekBarChangeListener listener) {
        mCustomSeekBarChangeListener = listener;
    }

    /**
     * 进度监听
     */
    public interface OnCustomSeekBarChangeListener {
        /**
         * 进度监听回调
         *
         * @param seekBar  SeekBar
         * @param progress 进度
         * @param fromUser 是否用户滑动
         */
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

    private void volumeGradient(final SeekBar mSeekBar,
                                final int from, final int to) {
        ValueAnimator animator = ValueAnimator.ofInt(from, to);
        animator.setDuration(300);
        animator.setInterpolator(new LinearInterpolator());
        animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                mSeekBar.setProgress((int) animation.getAnimatedValue());
            }
        });

        animator.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animation) {

            }

            @Override
            public void onAnimationEnd(Animator animation) {
                mSeekBar.setProgress(to);
            }

            @Override
            public void onAnimationCancel(Animator animation) {
                try {
                    mSeekBar.setProgress(to);
                } catch (Exception e) {
                }
            }

            @Override
            public void onAnimationRepeat(Animator animation) {

            }
        });
        animator.start();
    }

}
