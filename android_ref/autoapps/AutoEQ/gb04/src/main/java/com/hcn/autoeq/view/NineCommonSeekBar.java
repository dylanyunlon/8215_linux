package com.hcn.autoeq.view;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.view.animation.LinearInterpolator;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.hcn.autoeq.R;
import com.hcn.skin.support.resources.SkinCompatResources;
import com.hcn_library.util.SkinUtils;


public class NineCommonSeekBar extends ConstraintLayout implements SeekBar.OnSeekBarChangeListener {
    private static int DEFAULT_HEIGHT = 0;
    public String SUFFIX_1;
    public String SUFFIX_2;
    private Context context;
    private ImageView ivScale;
    private SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;
    private SeekBar seekBar;
    private int seekBarMax;
    private TextView tvDown;
    private TextView tvTop2;

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }


    public NineCommonSeekBar(Context context) {
        super(context);
        seekBarMax = 100;
        SUFFIX_1 = "";
        SUFFIX_2 = "";
        init();
    }

    public NineCommonSeekBar(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        seekBarMax = 100;
        SUFFIX_1 = "";
        SUFFIX_2 = "";
        this.context = context;
        init();
        initStyle(attributeSet);
    }

    private void init() {
        SkinCompatResources.getInstance().checkIfNeedBuildLayoutParams(LayoutInflater.from(context).inflate(SkinUtils.getId(R.layout.nine_common_seekbar), this), SkinUtils.getId(R.layout.nine_common_seekbar));
        DEFAULT_HEIGHT = (int) SkinUtils.getDimension(R.dimen.y530);
        tvTop2 = (TextView) findViewById(SkinUtils.getId(R.id.tv_top2));
        tvDown = (TextView) findViewById(SkinUtils.getId(R.id.tv_down));
        seekBar = (SeekBar) findViewById(SkinUtils.getId(R.id.sb_nine));
        ivScale = (ImageView) findViewById(SkinUtils.getId(R.id.iv_scale));
        seekBar.setMax(seekBarMax);
        seekBar.setOnSeekBarChangeListener(this);
    }

    private void initStyle(AttributeSet attributeSet) {
        TypedArray obtainStyledAttributes = getContext().obtainStyledAttributes(attributeSet, R.styleable.nine_seek_bar_attr);
        if (obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_seek_bar_top_name_two) != null) {
            tvTop2.setText(obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_seek_bar_top_name_two));
        }
        if (obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_seek_bar_down_name) != null) {
            tvDown.setText(obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_seek_bar_down_name));
        }
        int i = obtainStyledAttributes.getInt(R.styleable.nine_seek_bar_attr_nine_seek_bar_height, DEFAULT_HEIGHT);
        if (i != DEFAULT_HEIGHT) {
            seekBar.setLayoutParams(new LinearLayout.LayoutParams(i, LinearLayout.LayoutParams.WRAP_CONTENT));
            ViewGroup.LayoutParams layoutParams = ivScale.getLayoutParams();
            layoutParams.height = i;
            ivScale.setLayoutParams(layoutParams);
        }
        seekBar.setTag(getTag());
    }

    public void setProgress(int progress, boolean showAnim) {
        if (showAnim) {
            volumeGradient(seekBar, seekBar.getProgress(), progress);
        } else {
            seekBar.setProgress(progress);
        }
    }

    public void setSeekBarMax(int i) {
        if (seekBar != null) {
            seekBarMax = i;
            seekBar.setMax(i);
        }
    }

    public void setSeekBarStatus(boolean enable) {
        if (seekBar != null) {
            if (enable) {
                seekBar.setThumb(SkinUtils.getDrawable(R.drawable.nine_seek_bar_thumb));
            } else {
                seekBar.setThumb(SkinUtils.getDrawable(R.drawable.nine_seek_bar_thumb_default));
            }
            seekBar.setEnabled(enable);
        }
    }

    public void setTopNameTwo(int i) {
        tvTop2.setText(i + SUFFIX_2);
        Log.d("NineCommonSeekBar", "setTomeNameTwo " + i);
    }

    public void setOnSeekBarChangeListener(SeekBar.OnSeekBarChangeListener onSeekBarChangeListener) {
        this.onSeekBarChangeListener = onSeekBarChangeListener;
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int i, boolean z) {
        if (onSeekBarChangeListener != null) {
            setTopNameTwo(i);
            onSeekBarChangeListener.onProgressChanged(seekBar, i, z);
        }
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        if (onSeekBarChangeListener != null) {
            int progress = seekBar.getProgress();
            setTopNameTwo(progress);
            onSeekBarChangeListener.onStopTrackingTouch(seekBar);
        }
    }

    private void volumeGradient(final SeekBar seekBar, int i, final int i2) {
        ValueAnimator ofInt = ValueAnimator.ofInt(i, i2);
        ofInt.setDuration(300L);
        ofInt.setInterpolator(new LinearInterpolator());
        ofInt.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator valueAnimator) {
                seekBar.setProgress(((Integer) valueAnimator.getAnimatedValue()).intValue());
            }
        });
        ofInt.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationRepeat(Animator animator) {
            }

            @Override
            public void onAnimationStart(Animator animator) {
            }

            @Override
            public void onAnimationEnd(Animator animator) {
                seekBar.setProgress(i2);
            }

            @Override
            public void onAnimationCancel(Animator animator) {
                try {
                    seekBar.setProgress(i2);
                } catch (Exception unused) {
                }
            }
        });
        ofInt.start();
    }

    public void setProgressAlpha(float alpha) {
        tvTop2.setAlpha(alpha);
    }

    public TextView getTvDown() {
        return tvDown;
    }
}