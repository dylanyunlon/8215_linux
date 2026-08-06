package com.hcn.autoeq.view;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.animation.LinearInterpolator;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.SkinUtils;

public class FyDspAttenuateSeekBar extends ConstraintLayout implements CompoundButton.OnCheckedChangeListener, SeekBar.OnSeekBarChangeListener {
    private SeekBar seekBar;
    private TextView tvTop;
    private SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;
    private CompoundButton.OnCheckedChangeListener onCheckedChangeListener;
    private CheckBox cbMute, cbInvert;

    private Context context;
    private int seekBarMax = 15;
    private int progress;

    public FyDspAttenuateSeekBar(Context context) {
        super(context);
        init();
    }

    public FyDspAttenuateSeekBar(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        init();
        initStyle(attrs);
    }

    private void init() {
        LayoutInflater.from(context).inflate(R.layout.fydsp_attenuate_seekbar, this);
        tvTop = findViewById(R.id.tv_attenuate_top_text);
        seekBar = findViewById(R.id.sb_attenuate);
        seekBar.setMax(seekBarMax);
        seekBar.setOnSeekBarChangeListener(this);
        cbMute = findViewById(R.id.cb_mute);
        cbInvert = findViewById(R.id.cb_invert);

        cbMute.setOnCheckedChangeListener(this);
        cbInvert.setOnCheckedChangeListener(this);
    }

    private void initStyle(AttributeSet attributeSet) {
        TypedArray typedArray = this.getContext().obtainStyledAttributes(attributeSet, R.styleable.attenuate_seek_bar_attr);
        if (tvTop != null) {
            tvTop.setText(typedArray.getString(R.styleable.attenuate_seek_bar_attr_attenuate_seek_bar_top_name));
        }
        seekBar.setTag(this.getTag());
        cbMute.setTag(this.getTag());
        cbInvert.setTag(this.getTag());

        if (typedArray.hasValueOrEmpty(SkinUtils.getId(R.styleable.attenuate_seek_bar_attr_attenuate_seek_bar_height))) {
            seekBar.setLayoutParams(new LayoutParams((int) typedArray.getDimension(R.styleable.attenuate_seek_bar_attr_attenuate_seek_bar_height, LayoutParams.WRAP_CONTENT)
                    , LayoutParams.WRAP_CONTENT));
        }
        // FIXME, 换肤方式，在xml里设置 padding=0, offset=0，滑块依然会超出，通过代码设置后正常
        seekBar.setPadding((int) getResources().getDimension(R.dimen.fydsp_seekbar_padding), (int) getResources().getDimension(R.dimen.fydsp_seekbar_padding)
                , (int) getResources().getDimension(R.dimen.fydsp_seekbar_padding), (int) getResources().getDimension(R.dimen.fydsp_seekbar_padding));
        seekBar.setThumbOffset((int) getResources().getDimension(R.dimen.fydsp_seekbar_thumb_offset));
        seekBar.setMax(seekBarMax);
    }

    public void setTitle(String title) {
        tvTop.setText(title);
    }

    public void setProgress(int progress, boolean animation) {
        if (animation) {
            volumeGradient(seekBar, seekBar.getProgress(), progress + seekBarMax);
        } else {
            seekBar.setProgress(progress + seekBarMax);
        }
    }

    public void setSeekBarMax(int max) {
        if (seekBar != null) {
            seekBarMax = max;
            seekBar.setMax(max);
        }
    }

    public void setSeekBarStatus(boolean enabled) {
        if (seekBar != null) {
            seekBar.setAlpha(enabled ? 1.0f : 0.9f);
            seekBar.setEnabled(enabled);
        }
    }

    public void setMuteStatus(boolean checked) {
        cbMute.setChecked(checked);
    }

    public void setInvertStatus(boolean checked) {
        cbInvert.setChecked(checked);
    }

    public int getProgress() {
        return seekBar.getProgress() - seekBarMax;
    }

    public boolean getAttenuateStatus() {
        return cbMute.isChecked();
    }

    public boolean getInvertStatus() {
        return cbInvert.isChecked();
    }

    public void setOnSeekBarChangeListener(SeekBar.OnSeekBarChangeListener onSeekBarChangeListener) {
        this.onSeekBarChangeListener = onSeekBarChangeListener;
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (null != onSeekBarChangeListener) {
            onSeekBarChangeListener.onProgressChanged(seekBar, progress - seekBarMax, fromUser);
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        if (onSeekBarChangeListener != null) {
            progress = seekBar.getProgress();
            onSeekBarChangeListener.onStopTrackingTouch(seekBar);
        }
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

    public void setOnCheckedChangeListener(CompoundButton.OnCheckedChangeListener onCheckedChangeListener) {
        this.onCheckedChangeListener = onCheckedChangeListener;
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (buttonView.getId() == R.id.cb_mute) {
            setSeekBarStatus(!isChecked);
        }

        if (onCheckedChangeListener != null) {
            onCheckedChangeListener.onCheckedChanged(buttonView, isChecked);
        }
    }

}
