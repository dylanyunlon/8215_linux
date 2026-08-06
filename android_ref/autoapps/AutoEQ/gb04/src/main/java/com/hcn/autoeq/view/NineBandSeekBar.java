package com.hcn.autoeq.view;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.animation.LinearInterpolator;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.fragment.app.FragmentManager;

import com.hcn.autoeq.R;
import com.hcn.skin.support.resources.SkinCompatResources;
import com.hcn_library.util.SkinUtils;

import java.util.Locale;

import com.hcn.autoeq.nine.NineDspQValueFragment;

public class NineBandSeekBar extends ConstraintLayout implements SeekBar.OnSeekBarChangeListener {
    private static int DEFAULT_HEIGHT;
    private Context context;
    private SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;

    private SeekBar seekBar;
    private int seekBarMax = 24;
    private TextView tvGValue;
    private TextView tvQValue;
    private FragmentManager fragmentManager;

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }


    public NineBandSeekBar(Context context) {
        super(context);
        init();
    }

    public NineBandSeekBar(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        this.context = context;
        init();
        initStyle(attributeSet);
    }

    private void init() {
        SkinCompatResources.getInstance().checkIfNeedBuildLayoutParams(LayoutInflater.from(context).inflate(SkinUtils.getId(R.layout.nine_band_seekbar), this), SkinUtils.getId(R.layout.nine_band_seekbar));
        DEFAULT_HEIGHT = (int) SkinUtils.getDimension(R.dimen.y400);
        tvGValue = (TextView) findViewById(SkinUtils.getId(R.id.tv_g));
        tvQValue = (TextView) findViewById(SkinUtils.getId(R.id.tv_q));
        seekBar = (SeekBar) findViewById(SkinUtils.getId(R.id.sb_nine));
        seekBar.setMax(seekBarMax);
        seekBar.setOnSeekBarChangeListener(this);
        if (tvQValue != null) {
            // 点击q值，启动q值调节对话框
            tvQValue.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (fragmentManager != null) {
                        NineDspQValueFragment qValueFragment = NineDspQValueFragment.newInstance(Integer.valueOf((String) seekBar.getTag()), seekBar.getProgress() - seekBarMax / 2, (Integer) v.getTag());
                        qValueFragment.setCallback(new NineDspQValueFragment.Callback() {
                            @Override
                            public void onQValueChanged(int position, int qValue) {
                                setQValue(qValue);
                                callback.onDialogQValueChanged(position, qValue);
                                Log.d("qvalue", "onQValueChanged: " + position + " " + qValue);
                            }
                        });
                        qValueFragment.show(fragmentManager, "qvalue");
                    }
                }
            });
        }
        Drawable thumb = seekBar.getThumb();
        if (thumb != null) {
            thumb.setBounds(0, 0, thumb.getIntrinsicWidth(), thumb.getIntrinsicHeight());
            seekBar.setThumb(thumb);
        }
    }

    private void initStyle(AttributeSet attributeSet) {
        TypedArray obtainStyledAttributes = getContext().obtainStyledAttributes(attributeSet, R.styleable.nine_seek_bar_attr);
        if (obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_band_seek_bar_enable_gvalue) != null) {
            tvGValue.setText(obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_band_seek_bar_enable_gvalue));
        }
        if (obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_band_seek_bar_enable_qvalue) != null) {
            tvQValue.setText(obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_band_seek_bar_enable_qvalue));
        }
        int i = obtainStyledAttributes.getInt(R.styleable.nine_seek_bar_attr_nine_seek_bar_height, DEFAULT_HEIGHT);
        if (i != DEFAULT_HEIGHT) {
            seekBar.setLayoutParams(new LinearLayout.LayoutParams(i, LinearLayout.LayoutParams.WRAP_CONTENT));
        }
        seekBar.setTag(getTag());
    }

    public void setFragmentManager(FragmentManager fragmentManager) {
        this.fragmentManager = fragmentManager;
    }

    public void setProgress(int progress, boolean showAnim) {
        if (showAnim) {
            volumeGradient(seekBar, seekBar.getProgress(), progress + seekBarMax / 2);
        } else {
            seekBar.setProgress(progress + seekBarMax / 2);
        }
    }

    public void setSeekBarStatus(boolean canSeek) {
        if (seekBar != null) {
            seekBar.setAlpha(canSeek ? 1.0f : 0.9f);
            seekBar.setEnabled(canSeek);
        }
        if (tvQValue != null) {
            tvQValue.setEnabled(canSeek);
        }
    }

    public void setGValue(int i) {
        tvGValue.setText(i + "");
    }

    public void setQValue(int qValue) {
        if (tvQValue != null) {
            tvQValue.setText(String.format(Locale.getDefault(), "%.1f", qValue / 1000f).replace(".0", ""));
            tvQValue.setTag(qValue);
        }
    }

    public int getProgress() {
        return seekBar.getProgress() - seekBarMax / 2;
    }

    public void setOnSeekBarChangeListener(SeekBar.OnSeekBarChangeListener onSeekBarChangeListener) {
        this.onSeekBarChangeListener = onSeekBarChangeListener;
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (onSeekBarChangeListener != null) {
            setGValue(progress - seekBarMax / 2);
            onSeekBarChangeListener.onProgressChanged(seekBar, progress - seekBarMax / 2, fromUser);
        }
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        if (onSeekBarChangeListener != null) {
            setGValue(seekBar.getProgress() - seekBarMax / 2);
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

    private QValueCallback callback;

    public void setCallback(QValueCallback callback) {
        this.callback = callback;
    }

    public interface QValueCallback {
        void onDialogQValueChanged(int position, int qValue);
    }
}