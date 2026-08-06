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
import com.hcn.autoeq.util.ConstantCscAsp;
import com.hcn.autoeq.util.ConstantEq;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.util.Locale;

public class CscAspSeekBar extends RelativeLayout {
    private TextView mTvFreqView;
    private SeekBar seekBar;

    private SeekBar.OnSeekBarChangeListener mCustomSeekBarChangeListener;
    private Context context;

    public CscAspSeekBar(Context context) {
        super(context);
        this.context = context;
        initView();

    }

    public CscAspSeekBar(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        initView();
    }


    public void initView() {
        LayoutInflater.from(SkinCompatResources.getInstance().getSkinResId(R.layout.csc_asp_seekbar, "layout") != 0
                        ? SkinUtils.getContext() : context)
                .inflate(SkinUtils.getId(R.layout.csc_asp_seekbar), this);
        seekBar = findViewById(SkinUtils.getId(R.id.csc_asp_indicator_seek_bar));
        seekBar.setMax(ConstantCscAsp.DEFINE_SEEKBAR_MAX);
        seekBar.setMin(ConstantCscAsp.DEFINE_SEEKBAR_MIN);
        seekBar.setThumbOffset(0);
        seekBar.setOnSeekBarChangeListener(mSeekBarChangeListener);
        mTvFreqView = findViewById(SkinUtils.getId(R.id.tv_freq_asp));

    }

    /**
     * 设置表示tag
     */
    public void setSeekBarTag(int tag) {
        seekBar.setTag(tag);
    }

    /**
     * 设置进度监听
     *
     * @param listener OnIndicatorSeekBarChangeListener
     */
    public void setOnSeekBarChangeListener(SeekBar.OnSeekBarChangeListener listener) {
        mCustomSeekBarChangeListener = listener;
    }


    SeekBar.OnSeekBarChangeListener mSeekBarChangeListener = new SeekBar.OnSeekBarChangeListener() {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            mCustomSeekBarChangeListener.onProgressChanged(seekBar, progress, fromUser);
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
            mCustomSeekBarChangeListener.onStartTrackingTouch(seekBar);
        }

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
            mCustomSeekBarChangeListener.onStopTrackingTouch(seekBar);
        }
    };

    /**
     * 设置进度
     */
    public void setProgress(float currentProgress) {
        if (seekBar != null) {
            seekBar.setProgress((int) currentProgress);
        }
    }

    /**
     * 设置Freq
     */
    public void setFreq(String freq) {
        if (mTvFreqView != null) {
            mTvFreqView.setText(freq);
        }
    }

}
