package com.hcn.autoradio.view;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.hcn.autoradio.R;
import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.skin.SkinUtils;

import java.util.Locale;

/**
 * @author simon
 */
public class RoundKnobSeekBarView extends ConstraintLayout implements RoundKnobSeekBar.OnProgressChangeListener {

    private static final String TAG = RoundKnobSeekBarView.class.getSimpleName();

    private RoundKnobSeekBar roundKnobSeekBar;
    private ImageView ivThumb;
    private TextView tvFreqMin, tvFreqMax;
    private Context context;
    private RoundKnobSeekBar.OnProgressChangeListener onProgressChangeListener;

    public RoundKnobSeekBarView(Context context) {
        super(context);
        init();
    }

    public RoundKnobSeekBarView(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        init();
        initStyle(attrs);
    }

    private void init() {
        LayoutInflater.from(context).inflate(SkinUtils.getId(R.layout.round_knob_seekbar), this);
        roundKnobSeekBar = findViewById(SkinUtils.getId(R.id.rksb_main));
        ivThumb = findViewById(SkinUtils.getId(R.id.iv_thumb));
        tvFreqMin = findViewById(SkinUtils.getId(R.id.tv_freq_min));
        tvFreqMax = findViewById(SkinUtils.getId(R.id.tv_freq_max));

        roundKnobSeekBar.setTag(this.getTag());
        roundKnobSeekBar.setOnProgressChangeListener(this);
    }

    private void initStyle(AttributeSet attributeSet) {
    }

    /**
     * 设置滚动范围
     * @param isFmBand
     */
    public void setRoundKnobScrollValue(boolean isFmBand) {
        if (roundKnobSeekBar != null) {
            roundKnobSeekBar.initKnobScrollValue(isFmBand);
        }
        setFreqRangeText(isFmBand);
    }

    /**
     * 设置频点范围
     * @param isFmBand
     */
    private void setFreqRangeText(boolean isFmBand) {
        if (tvFreqMin == null || tvFreqMax == null) {
            return;
        }
        if (isFmBand) {
            tvFreqMin.setText(String.valueOf(formatFreq(FMDataControl.mRadioParameters.FmMin, false)));
            tvFreqMax.setText(String.valueOf(formatFreq(FMDataControl.mRadioParameters.FmMax, false)));
        } else {
            tvFreqMin.setText(String.valueOf(FMDataControl.mRadioParameters.AmMin));
            tvFreqMax.setText(String.valueOf(FMDataControl.mRadioParameters.AmMax));
        }
    }

    /**
     * 转换频点显示
     * @param freq
     * @param withUnit
     * @return
     */
    public String formatFreq(int freq, boolean withUnit) {
        StringBuilder strFreq = new StringBuilder();
        if (freq < 10000) {
            strFreq.append(freq);
            if (withUnit) {
                strFreq.append(" ").append(SkinUtils.getString(R.string.khz));
            }
        } else {
            strFreq.append(String.format(Locale.ENGLISH, "%.01f", freq * 0.001));
            if (withUnit) {
                strFreq.append(" ").append(SkinUtils.getString(R.string.mhz));
            }
        }
        return strFreq.toString();
    }

    /**
     * 获取当前进度
     * @return
     */
    public float getProgress() {
        return this.roundKnobSeekBar.getProgress();
    }

    /**
     * 设置当前滚动进度
     * @param progress
     */
    public void setProgress(int progress) {
        this.roundKnobSeekBar.setProgress(progress);
    }

    public void setOnProgressChangeListener(RoundKnobSeekBar.OnProgressChangeListener onProgressChangeListener) {
        this.onProgressChangeListener = onProgressChangeListener;
    }

    @Override
    public void onProgressChanged(RoundKnobSeekBar seekBar, float progress, boolean fromUser) {
        Log.d(TAG, "onProgressChanged progress : " + progress);
        ivThumb.setRotation(seekBar.getRotateAngle());
        if (this.onProgressChangeListener != null) {
            this.onProgressChangeListener.onProgressChanged(seekBar, progress, fromUser);
        }
    }

    @Override
    public void onStartTrackingTouch(RoundKnobSeekBar seekBar) {
        if (this.onProgressChangeListener != null) {
            this.onProgressChangeListener.onStartTrackingTouch(seekBar);
        }
    }

    @Override
    public void onStopTrackingTouch(RoundKnobSeekBar seekBar) {
        if (this.onProgressChangeListener != null) {
            this.onProgressChangeListener.onStopTrackingTouch(seekBar);
        }
    }
}
