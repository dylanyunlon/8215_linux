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
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.SkinUtils;

import java.math.BigDecimal;


public class NineDelaySeekBar extends ConstraintLayout implements SeekBar.OnSeekBarChangeListener {
    private static int DEFAULT_HEIGHT = 0;
    public static final float DELAY_PRECISION = 10.0f;
    private static final int PER_MILLISECOND = 34;

    private int minTime = 0;
    private int maxTime = "gb05".equals(EqUtils.getSkinName()) ? 200 : 100;
    public String SUFFIX_1 = "";
    public String SUFFIX_2 = "";
    private Context context;
    private ImageView ivScale;
    private SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;
    private int progress;
    private SeekBar seekBar;
    private TextView tvDown;
    private TextView tvTop1;
    private TextView tvTop2;

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }


    public NineDelaySeekBar(Context context) {
        super(context);
        init();
    }

    public NineDelaySeekBar(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        this.context = context;
        init();
        initStyle(attributeSet);
    }

    private void init() {
        SkinCompatResources.getInstance().checkIfNeedBuildLayoutParams(LayoutInflater.from(context).inflate(SkinUtils.getId(R.layout.nine_delay_seekbar), this), SkinUtils.getId(R.layout.nine_delay_seekbar));
        DEFAULT_HEIGHT = (int) SkinUtils.getDimension(R.dimen.y530);
        tvTop1 = (TextView) findViewById(SkinUtils.getId(R.id.tv_top1));
        tvTop2 = (TextView) findViewById(SkinUtils.getId(R.id.tv_top2));
        tvDown = (TextView) findViewById(SkinUtils.getId(R.id.tv_down));
        seekBar = (SeekBar) findViewById(SkinUtils.getId(R.id.sb_nine));
        ivScale = (ImageView) findViewById(SkinUtils.getId(R.id.iv_scale));
        seekBar.setMax(maxTime);
        seekBar.setOnSeekBarChangeListener(this);
    }

    private void initStyle(AttributeSet attributeSet) {
        TypedArray obtainStyledAttributes = getContext().obtainStyledAttributes(attributeSet, R.styleable.nine_seek_bar_attr);
        if (obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_seek_bar_top_name_one) != null) {
            tvTop1.setText(obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_seek_bar_top_name_one));
        }
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

    public void setProgress(int i, boolean z) {
        if (z) {
            volumeGradient(seekBar, seekBar.getProgress(), i);
        } else {
            seekBar.setProgress(i);
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

    public void setTopNameOne(int i) {
        int distance = timeToDistance(i);
        tvTop1.setText(new BigDecimal(i / DELAY_PRECISION).setScale(1, 4) + SUFFIX_1);
        tvTop2.setText(distance + SUFFIX_2);
        Log.d("NineCommonSeekBar", "setTomeNameOne " + i / DELAY_PRECISION + " setTomeNameTwo " + distance);

    }

    public void setOnSeekBarChangeListener(SeekBar.OnSeekBarChangeListener onSeekBarChangeListener) {
        this.onSeekBarChangeListener = onSeekBarChangeListener;
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int i, boolean z) {
        if (onSeekBarChangeListener != null) {
            setTopNameOne(i);
            onSeekBarChangeListener.onProgressChanged(seekBar, i, z);
        }
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        if (onSeekBarChangeListener != null) {
            int progress = seekBar.getProgress();
            setTopNameOne(progress);
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


    private int timeToDistance(int i) {
        return Math.round((i / DELAY_PRECISION) * PER_MILLISECOND);
    }

    public void setMaxTime(int maxTime) {
        this.maxTime = maxTime;
    }
}