package com.hcn.autoeq.view;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.animation.LinearInterpolator;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.fragment.app.FragmentManager;

import com.hcn.autoeq.R;
import com.hcn.autoeq.fragment.extdsp.ExtDspQValueFragment;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.skin2.Skin2;

import java.util.Locale;

public class ExtDspBandSeekBar extends LinearLayout {
    private SeekBar seekBar;
    private int progress;
    private TextView tvFreq;
    private TextView tvGain, tvQValue;
    private SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;
    private Context context;
    // mcx 内置dsp+asp，也用到此控件
    private int seekBarMax = "2".equals(EqUtils.getDspUI()) ? 20 : EqUtils.getDspGainMax();
    private int seekBarTag = 0;
    private FragmentManager fragmentManager;
    private boolean flipPageByBtn;

    public ExtDspBandSeekBar(Context context) {
        super(context);
        init();
    }

    public ExtDspBandSeekBar(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        init();
        initStyle(attrs);
    }

    private void init() {
        LayoutInflater.from(context).inflate(SkinUtils.getId(R.layout.extdsp_band_seekbar), this);
        flipPageByBtn = EqUtils.flipPageByBtn(context);
        if (flipPageByBtn) { // 飞音客户使用翻页按钮，每页放12段，所以最小宽度做宽点，没有mcc编号使用xml区分，在代码里重新设值
            setMinimumWidth((int) SkinUtils.getDimension(R.dimen.extdsp_band_seekbar_width_fy));
        }
        tvFreq = findViewById(SkinUtils.getId(R.id.tv_freq));
        tvGain = findViewById(SkinUtils.getId(R.id.tv_gain));
        tvQValue = findViewById(SkinUtils.getId(R.id.tv_q_value));
        seekBar = findViewById(SkinUtils.getId(R.id.sb_band));
        // FIXME, 换肤方式，在xml里设置 padding=0, offset=0，滑块依然会超出，通过代码设置后正常
        seekBar.setPadding((int) SkinUtils.getDimension(R.dimen.extdsp_seekbar_padding)
                , (int) SkinUtils.getDimension(R.dimen.extdsp_seekbar_padding)
                , (int) SkinUtils.getDimension(R.dimen.extdsp_seekbar_padding)
                , (int) SkinUtils.getDimension(R.dimen.extdsp_seekbar_padding));
        seekBar.setThumbOffset((int) SkinUtils.getDimension(R.dimen.extdsp_seekbar_thumb_offset));
        seekBar.setTag(this.getTag());
        seekBar.setMax(seekBarMax);
        seekBar.setOnSeekBarChangeListener(mSeekBarChangeListener);
        if (tvQValue != null) {
            // 点击q值，启动q值调节对话框
            tvQValue.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (fragmentManager != null) {
                        ExtDspQValueFragment qValueFragment = ExtDspQValueFragment.newInstance(Integer.valueOf((String) seekBar.getTag()), seekBar.getProgress() - seekBarMax / 2, (Integer) v.getTag());
                        qValueFragment.setCallback(new ExtDspQValueFragment.Callback() {
                            @Override
                            public void onQValueChanged(int qValue) {
                                setQValue(qValue);
                            }
                        });
                        qValueFragment.show(fragmentManager, "qvalue");
                    }
                }
            });
        }
    }

    private void initStyle(AttributeSet attrs) {
        int count = attrs.getAttributeCount();
        for (int i = 0; i < count; i++) {
            int attributeNameResource = attrs.getAttributeNameResource(i);
            if (attributeNameResource == SkinUtils.getId(R.attr.band_seek_bar_top_name)) {
                if (tvFreq != null) {
                    int attributeResourceValue = attrs.getAttributeResourceValue(i, 0);
                    int localId = Skin2.xAppId(attributeResourceValue);
                    if ("2".equals(EqUtils.getDspUI())) {
                        tvFreq.setText(SkinUtils.getText(localId).toString().replace("Hz", ""));
                    } else {
                        tvFreq.setText(SkinUtils.getText(localId));
                    }
                }
            } else if (attributeNameResource == SkinUtils.getId(R.attr.band_seek_bar_height)) {
                int attributeResourceValue = attrs.getAttributeResourceValue(i, 0);
                if (flipPageByBtn) {
                    seekBar.setLayoutParams(new LayoutParams((int) getResources().getDimension(attributeResourceValue)
                            , (int) SkinUtils.getDimension(R.dimen.extdsp_band_seekbar_width_fy)));
                } else {
                    seekBar.setLayoutParams(new LayoutParams((int) getResources().getDimension(attributeResourceValue)
                            , LayoutParams.MATCH_PARENT));
                }
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
            volumeGradient(seekBar, seekBar.getProgress(), progress + seekBarMax / 2);
        } else {
            seekBar.setProgress(progress + seekBarMax / 2);
        }
        tvGain.setText(String.valueOf(progress));
    }

    public void setQValue(int qValue) {
        if (tvQValue != null) {
            tvQValue.setText(String.format(Locale.getDefault(), "%.1f", qValue / 1000f).replace(".0", ""));
            tvQValue.setTag(qValue);
        }
    }

    public void setSeekBarMax(int max) {
        if (seekBar != null) {
            seekBarMax = max;
            seekBar.setMax(max);
        }
    }

    public void setSeekBarTag(int tag) {
        if (seekBar != null) {
            seekBarTag = tag;
            seekBar.setTag(seekBarTag);
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
            tvGain.setText(String.valueOf(progress - seekBarMax / 2));
            if (fromUser && null != onSeekBarChangeListener) {
                onSeekBarChangeListener.onProgressChanged(seekBar, progress - seekBarMax / 2, fromUser);
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
