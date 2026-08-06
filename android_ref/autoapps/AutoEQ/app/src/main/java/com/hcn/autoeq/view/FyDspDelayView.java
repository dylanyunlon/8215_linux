package com.hcn.autoeq.view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.hcn.autoeq.R;

import java.util.Locale;

public class FyDspDelayView extends ConstraintLayout implements View.OnClickListener, View.OnLongClickListener, View.OnTouchListener {

    private static final String TAG = FyDspDelayView.class.getSimpleName();
    private static final int PER_MILLISECOND = 34; // 音速每毫秒 34 厘米
    private static final int WHAT_VALUES_REDUCE = 0;
    private static final int WHAT_VALUES_ADD = 1;
    private static final float DELAY_PRECISION = 100f; // delay值百分比，数值范围是0~1800，显示是0.01~18
    private static final int minTime = 0;
    private static final int maxTime = 1800;

    private Button btnReduce, btnAdd;
    private TextView tvValue, tvTitle;

    private Context context;
    private int timeValue = 0;
    private float distanceValue = 0f;
    private int unit = 0; // 0: time(ms), 1:distance(cm)
    private boolean allowQuickChange; // 配合 LongClick 使用，取消长按时，消息要清空
    private Callback callback;

    private ValuesHandler handler = new ValuesHandler();

    public FyDspDelayView(Context context) {
        super(context);
        init();
    }

    public FyDspDelayView(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        init();
    }

    private void init() {
        LayoutInflater.from(context).inflate(R.layout.fydsp_delay_item, this);
        btnReduce = findViewById(R.id.btn_reduce);
        btnAdd = findViewById(R.id.btn_add);
        tvValue = findViewById(R.id.tv_value);
        tvTitle = findViewById(R.id.tv_delay_item_title);

        btnReduce.setOnClickListener(this);
        btnAdd.setOnClickListener(this);
        btnReduce.setOnLongClickListener(this);
        btnAdd.setOnLongClickListener(this);
        btnReduce.setOnTouchListener(this);
        btnAdd.setOnTouchListener(this);
    }

    public void reset(boolean needNativeData) {
        this.timeValue = 0;
        this.distanceValue = 0;
        refreshValue(true, needNativeData);
    }

    public void setTitle(String title) {
        this.tvTitle.setText(title);
    }

    public void setTimeValue(int timeValue) {
        this.timeValue = timeValue;
        this.distanceValue = timeValue / DELAY_PRECISION * PER_MILLISECOND;
        refreshValue(false, false); // 初始化设置控件文本时，无需回调（保存值或设置音效）
    }

    public void setUnit(int unit) {
        this.unit = unit;
        refreshValue(false, false); // 切换单位时，无需回调（保存值或设置音效），只需要更新界面内容即可
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_reduce:
                reduce();
                break;
            case R.id.btn_add:
                add();
                break;
        }
    }

    @Override
    public boolean onLongClick(View v) {
        if (!allowQuickChange) return false;

        switch (v.getId()) {
            case R.id.btn_reduce:
                handler.sendEmptyMessageDelayed(WHAT_VALUES_REDUCE, 100);
                break;
            case R.id.btn_add:
                handler.sendEmptyMessageDelayed(WHAT_VALUES_ADD, 100);
                break;
        }
        return false;
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                allowQuickChange = true;
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                allowQuickChange = false;
                handler.removeCallbacksAndMessages(null);
                break;
        }
        return false;
    }

    @SuppressLint("HandlerLeak")
    class ValuesHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case WHAT_VALUES_REDUCE:
                    reduce();
                    sendEmptyMessageDelayed(WHAT_VALUES_REDUCE, 100);
                    break;
                case WHAT_VALUES_ADD:
                    add();
                    sendEmptyMessageDelayed(WHAT_VALUES_ADD, 100);
                    break;
            }
        }
    }

    private void reduce() {
        timeValue -= 2;
        if (timeValue < minTime) {
            timeValue = minTime;
        }
        distanceValue = timeValue / DELAY_PRECISION * PER_MILLISECOND;
        refreshValue(true, true);
    }

    private void add() {
        timeValue += 2;
        if (timeValue > maxTime) {
            timeValue = maxTime;
        }
        distanceValue = timeValue / DELAY_PRECISION * PER_MILLISECOND;
        refreshValue(true, true);
    }

    private void refreshValue(boolean needCallback, boolean needNativeData) {
        if (unit == 0) {
            tvValue.setText(String.format(Locale.ENGLISH, "%.2f", timeValue / DELAY_PRECISION).replace(".00", ""));
        } else {
            tvValue.setText(String.format(Locale.ENGLISH, "%.2f", distanceValue).replace(".00", ""));
        }
        if (callback != null && needCallback) {
            callback.onValueChanged(this, timeValue, needNativeData);
        }
    }

    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    public interface Callback {
        void onValueChanged(FyDspDelayView fyDspDelayView, int value, boolean needNativeData);
    }

}

