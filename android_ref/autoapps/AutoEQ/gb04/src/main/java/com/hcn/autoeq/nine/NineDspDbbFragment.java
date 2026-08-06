package com.hcn.autoeq.nine;

import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import com.hcn.autoeq.R;
import com.hcn.autoeq.view.NineDspDbbView;
import com.hcn_library.BaseFragment;
import com.hcn_library.data.NineDspDbbSettings;
import com.hcn_library.util.ConstantExtDsp;
import com.hcn_library.util.NineConstantExtDsp;
import com.hcn_library.util.SkinUtils;


public class NineDspDbbFragment extends BaseFragment
        implements NineConstantExtDsp, NineDspDbbView.IExtDspDbbCallback, NineFilterFilterFragment.FragmentResetInterface, View.OnClickListener, View.OnLongClickListener, View.OnTouchListener {
    private static final int CLICK_DELAY = 100;
    private static final String TAG = "NineDspDbbFragment";
    private Runnable clickRunnable;
    private ImageView currentBtn;
    private NineDspDbbView nineDspDbbView;
    private ImageView ivBassPlus;
    private ImageView ivBassReduce;
    private ImageView ivForeFront;
    private ImageView ivFreReduce;
    private ImageView ivFreqPlus;
    private ImageView ivFrontL;
    private ImageView ivFrontR;
    private ImageView ivRear;
    private ImageView ivRearL;
    private ImageView ivRearR;
    private View mainView;
    private NineDspDbbSettings nineDspDbbSettings;
    private RadioGroup rgDbbMode;
    private TextView tvBassGain;
    private TextView tvCenterFreq;
    private int channel = NINE_DEF_DBB_CHANNEL_FLFR;
    private Handler handler = new Handler();

    @Override
    public int getLayoutRes() {
        return R.layout.nine_dsp_fragment_dbb;
    }

    public static NineDspDbbFragment newInstance() {
        return new NineDspDbbFragment();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        nineDspDbbSettings = NineDspDbbSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        Log.d(TAG, "initView");
        rgDbbMode = (RadioGroup) mainView.findViewById(SkinUtils.getId(R.id.rg_dbb_mode));
        initRgListener();
        nineDspDbbView = (NineDspDbbView) mainView.findViewById(SkinUtils.getId(R.id.v_dbb));
        nineDspDbbView.setExtDspDbbCallback(this);
        ivFrontL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_filter_bass_front_l));
        ivFrontR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_filter_bass_front_r));
        ivForeFront = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_filter_bass_forefront));
        ivRearL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_filter_bass_rear_l));
        ivRearR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_filter_bass_rear_r));
        ivRear = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_filter_bass_rear));
        tvCenterFreq = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_freq_center));
        tvBassGain = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_freq_bass));
        ivFreReduce = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_nine_dbb_freq_delay));
        ivFreqPlus = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_nine_dbb_freq_plus));
        ivBassReduce = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_nine_dbb_bass_delay));
        ivBassPlus = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_nine_dbb_bass_plus));
        ivFreReduce.setOnClickListener(this);
        ivFreqPlus.setOnClickListener(this);
        ivBassReduce.setOnClickListener(this);
        ivBassPlus.setOnClickListener(this);
        ivFreReduce.setOnLongClickListener(this);
        ivFreqPlus.setOnLongClickListener(this);
        ivBassReduce.setOnLongClickListener(this);
        ivBassPlus.setOnLongClickListener(this);
        ivFreReduce.setOnTouchListener(this);
        ivFreqPlus.setOnTouchListener(this);
        ivBassReduce.setOnTouchListener(this);
        ivBassPlus.setOnTouchListener(this);
    }

    @Override
    public void initData() {
        super.initData();
        nineDspDbbView.setPaintColor(R.color.nine_curve_line_color);
        refreshView();
        Log.d(TAG, "initData");
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (!hidden) {
            // 默认位置是底部一条直线，所以 x 是0，y 是图片的高度
            float touchX = nineDspDbbSettings.getDbbTouchX(channel, nineDspDbbView.getDbbHorizontalPadding());
            float touchY = nineDspDbbSettings.getDbbTouchY(channel, nineDspDbbView.getBackground().getIntrinsicHeight());
            nineDspDbbView.setTouchPoint(touchX, touchY); // 更新view
        }
    }


    private void initRgListener() {
        for (int i = 0; i < rgDbbMode.getChildCount(); i++) {
            View child = rgDbbMode.getChildAt(i);
            if (child instanceof RadioButton) {
                RadioButton radioButton = (RadioButton) child;
                radioButton.setOnTouchListener(new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        if (event.getAction() == MotionEvent.ACTION_DOWN) {
                            radioButton.setChecked(true);
                            refreshChannelView();
                        }
                        return false;
                    }
                });
            }
        }
    }

    private void refreshChannelView() {
        int checkedId = rgDbbMode.getCheckedRadioButtonId();
        ivFrontL.setSelected(false);
        ivFrontR.setSelected(false);
        ivRearL.setSelected(false);
        ivRearR.setSelected(false);
        ivForeFront.setSelected(false);
        ivRear.setSelected(false);
        if (checkedId == SkinUtils.getId(R.id.nine_rb_dbb_mode_fl_fr)) {
            channel = NineConstantExtDsp.NINE_DBB_CHANNEL_FLFR;
            ivFrontL.setSelected(true);
            ivFrontR.setSelected(true);
        } else if (checkedId == SkinUtils.getId(R.id.nine_rb_dbb_mode_rl_rr)) {
            channel = NineConstantExtDsp.NINE_DBB_CHANNEL_RLRR;
            ivRearL.setSelected(true);
            ivRearR.setSelected(true);
        } else if (checkedId == SkinUtils.getId(R.id.nine_rb_dbb_mode_center)) {
            channel = NineConstantExtDsp.NINE_DBB_CHANNEL_CEN;
            ivForeFront.setSelected(true);
        } else {
            channel = NineConstantExtDsp.NINE_DBB_CHANNEL_SUBWOOFER;
            ivRear.setSelected(true);
        }
        tvCenterFreq.setText(nineDspDbbSettings.getDbbFreq(channel) + " Hz");
        tvBassGain.setText(nineDspDbbSettings.getDbbGain(channel) + " dB");

        // 默认位置是底部一条直线，所以 x 是0，y 是图片的高度
        float touchX = nineDspDbbSettings.getDbbTouchX(channel, nineDspDbbView.getDbbHorizontalPadding());
        float touchY = nineDspDbbSettings.getDbbTouchY(channel, nineDspDbbView.getBackground().getIntrinsicHeight());
        nineDspDbbView.setTouchPoint(touchX, touchY); // 更新view

        boolean hasChildPressed = false;
        for (int i = 0; i < rgDbbMode.getChildCount(); i++) {
            if (rgDbbMode.getChildAt(i).isPressed()) {
                hasChildPressed = true;
            }
        }

        // 界面初始化时也会调用 onCheckedChanged 事件，这个时候不需要改变数据和实际音效
        // 手动点击时才往下执行
        if (!hasChildPressed) {
            return;
        }

        int[] _data = touchPoint2Value(touchX, touchY); // 更新 native
        nineDspDbbSettings.saveDbb(channel, touchX, touchY, _data[0], _data[1]); // 更新本地数据
    }


    @Override
    public void onActionMove(float touchX, float touchY) {
        touchPoint2Value(touchX, touchY);
    }

    @Override
    public void onActionUp(float touchX, float touchY) {
        int[] _data = touchPoint2Value(touchX, touchY);
        nineDspDbbSettings.saveDbb(channel, touchX, touchY, _data[0], _data[1]);
        tvCenterFreq.setText(nineDspDbbSettings.getDbbFreq(channel) + " Hz");
        tvBassGain.setText(nineDspDbbSettings.getDbbGain(channel) + " dB");
    }

    // 根据背景图的宽高，横竖向的频点增益范围，计算每滑动多少对应的实际值
    private int[] touchPoint2Value(float touchX, float touchY) {
        float perWidth = (ConstantExtDsp.DBB_FREQ_MAX - ConstantExtDsp.DBB_FREQ_MIN) * 1f / (nineDspDbbView.getBackground().getIntrinsicWidth() - nineDspDbbView.getDbbHorizontalPadding() * 2);
        float perHeight = (ConstantExtDsp.DBB_GAIN_MAX - ConstantExtDsp.DBB_GAIN_MIN) * 1f / nineDspDbbView.getBackground().getIntrinsicHeight();
        float freq = perWidth * (touchX - nineDspDbbView.getDbbHorizontalPadding()) + 20;
        float gain = perHeight * (nineDspDbbView.getBackground().getIntrinsicHeight() - touchY);

        if (freq < ConstantExtDsp.DBB_FREQ_MIN) freq = ConstantExtDsp.DBB_FREQ_MIN;
        if (freq > ConstantExtDsp.DBB_FREQ_MAX) freq = ConstantExtDsp.DBB_FREQ_MAX;

        if (gain < ConstantExtDsp.DBB_GAIN_MIN) gain = ConstantExtDsp.DBB_GAIN_MIN;
        if (gain > ConstantExtDsp.DBB_GAIN_MAX) gain = ConstantExtDsp.DBB_GAIN_MAX;

        Log.d(TAG, String.format("touch perWidth : %s, perHeight : %s, x : %s, y : %s", perWidth, perHeight, touchX, touchY));
        Log.d(TAG, String.format("touch freq : %s, gain : %s", freq, gain));
        nineDspDbbSettings.nativeDbb(channel, (int) freq, (int) (gain + 0.5)); // gain 四舍五入

        return new int[]{(int) freq , (int) (gain + 0.5)};
    }

    private void freqStepClick(boolean isPlus) {
        float intrinsicWidth = (nineDspDbbView.getBackground().getIntrinsicWidth() * 1.0f) / 100.0f;
        float dbbTouchX = nineDspDbbSettings.getDbbTouchX(channel, nineDspDbbView.getDbbHorizontalPadding());
        float dbbTouchY = nineDspDbbSettings.getDbbTouchY(channel, nineDspDbbView.getBackground().getIntrinsicHeight());
        float f = isPlus ? dbbTouchX + intrinsicWidth : dbbTouchX - intrinsicWidth;
        if (f > nineDspDbbView.getBackground().getIntrinsicWidth()) {
            f = nineDspDbbView.getBackground().getIntrinsicWidth();
        }
        float f2 = f < 0.0f ? 0.0f : f;
        nineDspDbbView.setTouchPoint(f2, dbbTouchY);
        int[] iArr = touchPoint2Value(f2, dbbTouchY);
        nineDspDbbSettings.saveDbb(channel, f2, dbbTouchY, iArr[0], iArr[1]);
    }

    private void bassStepClick(boolean isPlus) {
        float intrinsicHeight = (nineDspDbbView.getBackground().getIntrinsicHeight() * 1.0f) / 15.0f;
        float dbbTouchX = nineDspDbbSettings.getDbbTouchX(channel, nineDspDbbView.getDbbHorizontalPadding());
        float dbbTouchY = nineDspDbbSettings.getDbbTouchY(channel, nineDspDbbView.getBackground().getIntrinsicHeight());
        float f = isPlus ? dbbTouchY - intrinsicHeight : dbbTouchY + intrinsicHeight;
        if (f > nineDspDbbView.getBackground().getIntrinsicHeight()) {
            f = nineDspDbbView.getBackground().getIntrinsicHeight();
        }
        float f2 = f < 0.0f ? 0.0f : f;
        nineDspDbbView.setTouchPoint(dbbTouchX, f2);
        int[] iArr = touchPoint2Value(dbbTouchX, f2);
        nineDspDbbSettings.saveDbb(channel, dbbTouchX, f2, iArr[0], iArr[1]);
    }

    @Override
    public void onReset() {
        nineDspDbbView.reset();
        float touchX = nineDspDbbView.getTouchX();
        float touchY = nineDspDbbView.getTouchY();
        int[] iArr = touchPoint2Value(touchX, touchY);
        nineDspDbbSettings.saveDbb(channel, touchX, touchY, iArr[0], iArr[1]);
        nineDspDbbSettings.nativeAll(nineDspDbbSettings.getDbbChannel());
        refreshView();
    }

    private void refreshView() {
        ivFrontL.setSelected(false);
        ivFrontR.setSelected(false);
        ivRearL.setSelected(false);
        ivRearR.setSelected(false);
        ivForeFront.setSelected(false);
        ivRear.setSelected(false);
        int channel = nineDspDbbSettings.getDbbChannel();
        if (channel == NineConstantExtDsp.NINE_DBB_CHANNEL_FLFR) {
            rgDbbMode.check(SkinUtils.getId(R.id.nine_rb_dbb_mode_fl_fr));
            ivFrontL.setSelected(true);
            ivFrontR.setSelected(true);
        } else if (channel == NineConstantExtDsp.NINE_DBB_CHANNEL_RLRR) {
            rgDbbMode.check(SkinUtils.getId(R.id.nine_rb_dbb_mode_rl_rr));
            ivRearL.setSelected(true);
            ivRearR.setSelected(true);
        } else if (channel == NineConstantExtDsp.NINE_DBB_CHANNEL_CEN) {
            rgDbbMode.check(SkinUtils.getId(R.id.nine_rb_dbb_mode_center));
            ivForeFront.setSelected(true);
        } else if (channel == NineConstantExtDsp.NINE_DBB_CHANNEL_SUBWOOFER) {
            rgDbbMode.check(SkinUtils.getId(R.id.nine_rb_dbb_mode_subwoofer));
            ivRear.setSelected(true);
        }
        tvCenterFreq.setText(nineDspDbbSettings.getDbbFreq(channel) + " Hz");
        tvBassGain.setText(nineDspDbbSettings.getDbbGain(channel) + " dB");
        refreshChannelView();
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_freq_delay)) {
            freqStepClick(false);
        } else if (view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_freq_plus)) {
            freqStepClick(true);
        } else if (view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_bass_delay)) {
            bassStepClick(false);
        } else if (view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_bass_plus)) {
            bassStepClick(true);
        }
        tvCenterFreq.setText(nineDspDbbSettings.getDbbFreq(channel) + " Hz");
        tvBassGain.setText(nineDspDbbSettings.getDbbGain(channel) + " dB");
        Log.d(TAG, "onClick");
    }

    @Override
    public boolean onLongClick(View view) {
        if (view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_freq_delay) || view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_freq_plus) || view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_bass_delay) || view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_bass_plus)) {
            currentBtn = (ImageView) mainView.findViewById(SkinUtils.getId(view.getId()));
            stopContinuousClick();
            startContinuousClick();
            Log.d(TAG, "onLongClick");
        }
        return false;
    }

    @Override
    public boolean onTouch(View view, MotionEvent motionEvent) {
        if (view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_freq_delay) || view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_freq_plus) || view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_bass_delay) || view.getId() == SkinUtils.getId(R.id.iv_nine_dbb_bass_plus)) {
            stopContinuousClick();
            Log.d(TAG, "onTouch");
        }
        return false;
    }

    private void startContinuousClick() {
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                if (currentBtn != null) {
                    currentBtn.performClick();
                    handler.postDelayed(this, CLICK_DELAY);
                }
            }
        };
        clickRunnable = runnable;
        handler.post(runnable);
    }

    private void stopContinuousClick() {
        handler.removeCallbacks(clickRunnable);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        handler.removeCallbacksAndMessages(null);
    }

}