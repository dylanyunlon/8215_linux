package com.hcn.autoeq.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import com.hcn.autoeq.R;

public class FyDspSeekBar extends LinearLayout {
    private TextView tvTitle, tvValue, tvMinText, tvMaxText;
    private SeekBar seekBar;

    private Context context;

    public FyDspSeekBar(Context context) {
        super(context);
        init();
    }

    public FyDspSeekBar(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        init();
    }

    private void init() {
        LayoutInflater.from(context).inflate(R.layout.fydsp_seekbar_item, this);
        tvTitle = findViewById(R.id.tv_title);
        tvValue = findViewById(R.id.tv_value);
        tvMinText = findViewById(R.id.tv_min_text);
        tvMaxText = findViewById(R.id.tv_max_text);
        seekBar = findViewById(R.id.sb_seekbar);
        // FIXME, 换肤方式，在xml里设置 padding=0, offset=0，滑块依然会超出，通过代码设置后正常
        seekBar.setPadding((int) getResources().getDimension(R.dimen.fydsp_seekbar_padding), (int) getResources().getDimension(R.dimen.fydsp_seekbar_padding)
                , (int) getResources().getDimension(R.dimen.fydsp_seekbar_padding), (int) getResources().getDimension(R.dimen.fydsp_seekbar_padding));
        seekBar.setThumbOffset((int) getResources().getDimension(R.dimen.fydsp_seekbar_thumb_offset));
    }

    public void setTitle(String title) {
        tvTitle.setText(title);
    }

    public void setProgress(int progress) {
        seekBar.setProgress(progress);
    }

    public void setValue(String value) {
        tvValue.setText(value);
    }

    public void setMinText(String minText) {
        tvMinText.setText(minText);
    }

    public void setMaxText(String maxText) {
        tvMaxText.setText(maxText);
    }

    public SeekBar getSeekBar() {
        return seekBar;
    }

}
