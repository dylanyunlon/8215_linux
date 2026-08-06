package com.hcn.autoeq.view;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.animation.LinearInterpolator;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.fragment.app.FragmentManager;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.Band;
import com.hcn.autoeq.fragment.fydsp.FyDspFrequencyDialog;
import com.hcn.autoeq.fragment.fydsp.FyDspQValueDialog;
import com.hcn.autoeq.util.ConstantFyDsp;
import com.hcn.autoeq.util.SkinUtils;

import java.util.Locale;

public class FyDspBandSeekBar extends LinearLayout implements ConstantFyDsp {

    private SeekBar seekBar;
    private TextView tvFreq;
    private TextView tvGain, tvQValue;
    private SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;

    private Context context;
    private FragmentManager fragmentManager;
    private int freq;
    private int q;

    public FyDspBandSeekBar(Context context) {
        super(context);
        init();
    }

    public FyDspBandSeekBar(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        init();
        initStyle(attrs);
    }

    private void init() {
        LayoutInflater.from(context).inflate(R.layout.fydsp_band_seekbar, this);
        tvFreq = findViewById(R.id.tv_freq);
        tvGain = findViewById(R.id.tv_gain);
        tvQValue = findViewById(R.id.tv_q_value);
        seekBar = findViewById(R.id.sb_band);
        // FIXME, 换肤方式，在xml里设置 padding=0, offset=0，滑块依然会超出，通过代码设置后正常
        seekBar.setPadding((int) getResources().getDimension(R.dimen.fydsp_seekbar_padding), (int) getResources().getDimension(R.dimen.fydsp_seekbar_padding)
                , (int) getResources().getDimension(R.dimen.fydsp_seekbar_padding), (int) getResources().getDimension(R.dimen.fydsp_seekbar_padding));
        seekBar.setThumbOffset((int) getResources().getDimension(R.dimen.fydsp_seekbar_thumb_offset));
        seekBar.setTag(this.getTag());
        seekBar.setMax(DEF_GAIN_PROGRESS_MAX);
        seekBar.setOnSeekBarChangeListener(mSeekBarChangeListener);

        if (tvFreq != null) {
            tvFreq.setOnClickListener(v -> {
                if (fragmentManager != null) {
                    final Band band = new Band();
                    band.setIndex(Integer.parseInt((String) this.getTag()));
                    band.setFreq(this.freq);
                    band.setQ(this.q);
                    band.setGain(seekBar.getProgress() - DEF_GAIN_PROGRESS_MAX / 2);
                    FyDspFrequencyDialog fyDspFrequencyDialog = FyDspFrequencyDialog.newInstance(band);
                    fyDspFrequencyDialog.show(fragmentManager, "");
                }
            });
        }
        if (tvQValue != null) {
            // 点击q值，启动q值调节对话框
            tvQValue.setOnClickListener(v -> {
                if (fragmentManager != null) {
                    final Band band = new Band();
                    band.setIndex(Integer.parseInt((String) this.getTag()));
                    band.setFreq(this.freq);
                    band.setQ(this.q);
                    band.setGain(seekBar.getProgress() - DEF_GAIN_PROGRESS_MAX / 2);
                    FyDspQValueDialog qValueDialog = FyDspQValueDialog.newInstance(band);
                    qValueDialog.setCallback(qValue -> setQValue(qValue));
                    qValueDialog.show(fragmentManager, "");
                }
            });
        }
    }

    private void initStyle(AttributeSet attrs) {
        int count = attrs.getAttributeCount();
        for (int i = 0; i < count; i++) {
            int attributeNameResource = attrs.getAttributeNameResource(i);
            if (attributeNameResource == SkinUtils.getId(R.attr.band_seek_bar_height)) {
                int attributeResourceValue = attrs.getAttributeResourceValue(i, 0);
                seekBar.setLayoutParams(new LayoutParams((int) getResources().getDimension(attributeResourceValue)
                        , LayoutParams.WRAP_CONTENT));
            } else if (attributeNameResource == SkinUtils.getId(R.attr.band_seek_bar_enable_qvalue)) {
                int attributeIntValue = attrs.getAttributeIntValue(i, 0);
                if (tvQValue != null) {
                    tvQValue.setVisibility(attributeIntValue == 1 ? VISIBLE : GONE);
                }
            }
        }
    }

    public void setFragmentManager(FragmentManager fragmentManager) {
        this.fragmentManager = fragmentManager;
    }

    public void setProgress(int progress, boolean animation) {
        if (animation) {
            volumeGradient(seekBar, seekBar.getProgress(), progress + DEF_GAIN_PROGRESS_MAX / 2);
        } else {
            seekBar.setProgress(progress + DEF_GAIN_PROGRESS_MAX / 2);
        }
        tvGain.setText(String.valueOf(progress));
    }

    public void setFreq(int freq) {
        this.freq = freq;
        if (tvFreq != null) {
            if (freq < 1000) {
                tvFreq.setText(String.valueOf(freq));
            } else {
                tvFreq.setText(String.format(Locale.getDefault(), "%.1fK", freq / 1000f).replace(".0", ""));
            }
        }
    }

    public void setQValue(int qValue) {
        this.q = qValue;
        if (tvQValue != null) {
            tvQValue.setText(String.format(Locale.getDefault(), "%.1f", qValue / FyDspQValueDialog.Q_VALUE_PRECISION).replace(".0", ""));
        }
    }

    public void setSeekBarStatus(boolean canSeek) {
        if (seekBar != null) {
            seekBar.setAlpha(canSeek ? 1.0f : 0.9f);
            seekBar.setEnabled(canSeek);
        }
        // 预设模式不让修改 q 值
        if (tvQValue != null) {
            tvQValue.setEnabled(canSeek);
        }
    }

    public void setOnSeekBarChangeListener(SeekBar.OnSeekBarChangeListener onSeekBarChangeListener) {
        this.onSeekBarChangeListener = onSeekBarChangeListener;
    }

    SeekBar.OnSeekBarChangeListener mSeekBarChangeListener = new SeekBar.OnSeekBarChangeListener() {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            tvGain.setText(String.valueOf(progress - DEF_GAIN_PROGRESS_MAX / 2));
            if (fromUser && null != onSeekBarChangeListener) {
                onSeekBarChangeListener.onProgressChanged(seekBar, progress - DEF_GAIN_PROGRESS_MAX / 2, fromUser);
            }
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
            if (onSeekBarChangeListener != null) {
                onSeekBarChangeListener.onStartTrackingTouch(seekBar);
            }
        }

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
            if (onSeekBarChangeListener != null) {
                onSeekBarChangeListener.onStopTrackingTouch(seekBar);
            }
        }
    };

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
