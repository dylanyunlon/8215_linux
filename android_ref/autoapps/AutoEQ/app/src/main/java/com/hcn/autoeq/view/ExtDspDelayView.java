package com.hcn.autoeq.view;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.util.Locale;

public class ExtDspDelayView extends ConstraintLayout implements RoundKnobSeekBar.OnProgressChangeListener {

    private static final String TAG = ExtDspDelayView.class.getSimpleName();
    private int PER_MILLISECOND = 34;

    private RoundKnobSeekBar rkbgMain;
    private ImageView ivThumb;
    private TextView tvTime, tvDistance;

    private Context context;
    private RoundKnobSeekBar.OnProgressChangeListener onProgressChangeListener;

    public ExtDspDelayView(Context context) {
        super(context);
        init();
    }

    public ExtDspDelayView(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        init();
        initStyle(attrs);
    }

    private void init() {
        View view = LayoutInflater.from(context).inflate(SkinUtils.getId(R.layout.extdsp_delay_item), this);
        SkinCompatResources.getInstance().checkIfNeedBuildLayoutParams(view, SkinUtils.getId(R.layout.extdsp_delay_item));

        rkbgMain = findViewById(SkinUtils.getId(R.id.rksb_main));
        ivThumb = findViewById(SkinUtils.getId(R.id.iv_thumb));
        tvTime = findViewById(SkinUtils.getId(R.id.tv_time));
        tvDistance = findViewById(SkinUtils.getId(R.id.tv_distance));

        rkbgMain.setTag(this.getTag());
        rkbgMain.setOnProgressChangeListener(this);
    }

    private void initStyle(AttributeSet attributeSet) {
    }

    public float getProgress() {
        return this.rkbgMain.getProgress();
    }

    public void setProgress(int progress) {
        this.rkbgMain.setProgress(progress);
    }

    public void setOnProgressChangeListener(RoundKnobSeekBar.OnProgressChangeListener onProgressChangeListener) {
        this.onProgressChangeListener = onProgressChangeListener;
    }

    @Override
    public void onProgressChanged(RoundKnobSeekBar seekBar, float progress, boolean fromUser) {
        Log.d(TAG, "onProgressChanged progress : " + progress);

        ivThumb.setRotation(seekBar.getRotateAngle());
        tvTime.setText(String.format(Locale.ENGLISH, "%.1f %s", progress / 10, SkinUtils.getText(R.string.sound_field_ms)).replace(".0", ""));
        tvDistance.setText(String.format(Locale.ENGLISH, "%d %s", ((int) progress) / 10 * PER_MILLISECOND, SkinUtils.getText(R.string.sound_field_cm)));
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
