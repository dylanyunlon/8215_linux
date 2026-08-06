package com.hcn.autoeq.fragment.siextdsp;

import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.EventMessage;
import com.hcn.autoeq.data.SIExtDspHLPFSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.SIConstantExtDsp;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.CustomSpinner;
import com.hcn.skin.support.resources.SkinCompatResources;
import com.jaygoo.widget.OnRangeChangedListener;
import com.jaygoo.widget.RangeSeekBar;

import java.util.Arrays;

/**
 * 和普通extdsp的UI相似，但功能的实现上是不同的，存在一部分区别，
 * 所以应该另外创建setting以及Fragment，方便以后的扩展和修改；
 */
public class SIExtDspHLPFFragment extends BaseFragment
        implements SIConstantExtDsp
        , View.OnClickListener, RadioGroup.OnCheckedChangeListener, AdapterView.OnItemSelectedListener, OnRangeChangedListener {

    private static final String TAG = SIExtDspHLPFFragment.class.getSimpleName();

    private View mainView;
    private ImageView ivSeekbarBg;
    private RangeSeekBar rsbHLPF;
    private ImageView ivHLPFSpeakerLF, ivHLPFSpeakerRF, ivHLPFSpeakerLR, ivHLPFSpeakerRR, ivHLPFSpeakerSubwoofer, ivHLPFSpeakerCenter;
    private RadioGroup rgHLPFMode;
    private Button btnHLPFReset;
    private TextView tvHpfFreq, tvLpfFreq;
    private CustomSpinner spHPFSlope, spLPFSlope;

    private SIExtDspHLPFSettings siExtDspHLPFSettings;
    private boolean isLeft = false;
    private boolean spinnerHPFFromUser, spinnerLPFFromUser;

    public SIExtDspHLPFFragment() {
    }

    public static SIExtDspHLPFFragment newInstance() {
        SIExtDspHLPFFragment fragment = new SIExtDspHLPFFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.siextdsp_fragment_hlpf;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        siExtDspHLPFSettings = SIExtDspHLPFSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        ivSeekbarBg = mainView.findViewById(SkinUtils.getId(R.id.iv_seekbar_bg));
        rsbHLPF = mainView.findViewById(SkinUtils.getId(R.id.rsb_hlpf));
        rsbHLPF.setOnRangeChangedListener(this);

        ivHLPFSpeakerLF = mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_speaker_lf));
        ivHLPFSpeakerRF = mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_speaker_rf));
        ivHLPFSpeakerLR = mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_speaker_lr));
        ivHLPFSpeakerRR = mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_speaker_rr));
        ivHLPFSpeakerSubwoofer = mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_speaker_subwoofer));
        ivHLPFSpeakerCenter = mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_speaker_center));

        rgHLPFMode = mainView.findViewById(SkinUtils.getId(R.id.rg_hlpf_output_mode));
        if (rgHLPFMode != null) {
            rgHLPFMode.setOnCheckedChangeListener(this);
        }
        btnHLPFReset = mainView.findViewById(SkinUtils.getId(R.id.btn_reset_hlpf));
        btnHLPFReset.setOnClickListener(this);

        tvHpfFreq = mainView.findViewById(SkinUtils.getId(R.id.tv_hpf_freq));
        tvLpfFreq = mainView.findViewById(SkinUtils.getId(R.id.tv_lpf_freq));

        spHPFSlope = mainView.findViewById(SkinUtils.getId(R.id.sp_hpf_slope));
        spLPFSlope = mainView.findViewById(SkinUtils.getId(R.id.sp_lpf_slope));
        initSpHPFSlope();
        initSpLPFSlope();

        ivHLPFSpeakerSubwoofer.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_dbb_speaker_subwoofer_n));

        if (ivHLPFSpeakerCenter != null) {
            ivHLPFSpeakerCenter.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_dbb_speaker_center_n));

        }
    }

    @Override
    public void initData() {
        super.initData();

        int channel = siExtDspHLPFSettings.getHLPFChannel();
        if (channel == SI_CHANNEL_FRONT_HIGH || channel == SI_CHANNEL_FRONT_LOW) {
            if (rgHLPFMode != null) {
                rgHLPFMode.check(SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr));
            }
        } else if (channel == SI_CHANNEL_REAR_HIGH || channel == SI_CHANNEL_REAR_LOW) {
            if (rgHLPFMode != null) {
                rgHLPFMode.check(SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr));
            }
        } else if (channel == SI_CHANNEL_SUBWOOFER_HIGH || channel == SI_CHANNEL_SUBWOOFER_LOW) {
            if (rgHLPFMode != null) {
                rgHLPFMode.check(SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer));
            }
        } else if (channel == SI_CHANNEL_CENTER_HIGH || channel == SI_CHANNEL_CENTER_LOW) {
            if (rgHLPFMode != null) {
                rgHLPFMode.check(SkinUtils.getId(R.id.rb_hlpf_mode_center));
            }
        }
    }

    private void initSpHPFSlope() {
        ArrayAdapter<String> adapter = new ArrayAdapter(
                SkinCompatResources.getInstance().getSkinResId(R.layout.extdsp_slope_item, "layout") != 0
                        ? SkinUtils.getContext() : mContext
                , SkinUtils.getId(R.layout.extdsp_slope_item),
                SkinUtils.getId(R.id.tv_filter_slope), SkinUtils.getStringArray(R.array.extdsp_hlpf_qvalue));
        spHPFSlope.setAdapter(adapter);
        spHPFSlope.setSpinnerEventsListener(new CustomSpinner.OnSpinnerEventsListener() {
            @Override
            public void onSpinnerOpened(int lastSelectedItemPosition, int currentSelectedItemPosition) {
                Log.d(TAG, "spHPFSlope opened");
                spHPFSlope.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_slope_opened_selector));
            }

            @Override
            public void onSpinnerClosed(int lastSelectedItemPosition, int currentSelectedItemPosition) {
                Log.d(TAG, "spHPFSlope closed");
                spHPFSlope.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_slope_closed_selector));
            }
        });
        spHPFSlope.setOnItemSelectedListener(this);
        spHPFSlope.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                Log.d(TAG, "spHPFSlope onTouch");
                spinnerHPFFromUser = true;
                return false;
            }
        });
    }

    private void initSpLPFSlope() {
        ArrayAdapter<String> adapter = new ArrayAdapter(SkinCompatResources.getInstance().getSkinResId(R.layout.extdsp_slope_item, "layout") != 0
                ? SkinUtils.getContext() : mContext
                , SkinUtils.getId(R.layout.extdsp_slope_item),
                SkinUtils.getId(R.id.tv_filter_slope), SkinUtils.getStringArray(R.array.extdsp_hlpf_qvalue));
        spLPFSlope.setAdapter(adapter);
        spLPFSlope.setSpinnerEventsListener(new CustomSpinner.OnSpinnerEventsListener() {
            @Override
            public void onSpinnerOpened(int lastSelectedItemPosition, int currentSelectedItemPosition) {
                Log.d(TAG, "spLPFSlope opened");
                spLPFSlope.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_slope_opened_selector));
            }

            @Override
            public void onSpinnerClosed(int lastSelectedItemPosition, int currentSelectedItemPosition) {
                Log.d(TAG, "spLPFSlope closed");
                spLPFSlope.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_slope_closed_selector));
            }
        });
        spLPFSlope.setOnItemSelectedListener(this);
        spLPFSlope.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                Log.d(TAG, "spLPFSlope onTouch");
                spinnerLPFFromUser = true;
                return false;
            }
        });
    }

    @Override
    public void onCheckedChanged(RadioGroup radioGroup, int checkedId) {
        Log.d(TAG, "onCheckedChanged");
        ivHLPFSpeakerLF.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)
                ? R.drawable.extdsp_dbb_speaker_lf_p : R.drawable.extdsp_dbb_speaker_lf_n));
        ivHLPFSpeakerRF.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)
                ? R.drawable.extdsp_dbb_speaker_rf_p : R.drawable.extdsp_dbb_speaker_rf_n));
        ivHLPFSpeakerLR.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)
                ? R.drawable.extdsp_dbb_speaker_lr_p : R.drawable.extdsp_dbb_speaker_lr_n));
        ivHLPFSpeakerRR.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)
                ? R.drawable.extdsp_dbb_speaker_rr_p : R.drawable.extdsp_dbb_speaker_rr_n));
        ivHLPFSpeakerSubwoofer.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)
                ? R.drawable. extdsp_dbb_speaker_subwoofer_p: R.drawable.extdsp_dbb_speaker_subwoofer_n));

        if (ivHLPFSpeakerCenter != null) {
            ivHLPFSpeakerCenter.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_hlpf_mode_center)
                    ? R.drawable. extdsp_dbb_speaker_center_p: R.drawable.extdsp_dbb_speaker_center_n));

        }


        int freqProgressHigh, freqProgressLow;
        int freqUIProgressHigh, freqUIProgressLow;
        int freqHigh, freqLow;
        int qValueHigh, qValueLow;
        int chanelHigh, chanelLow;
        if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
            chanelHigh = SI_CHANNEL_FRONT_HIGH;
            chanelLow = SI_CHANNEL_FRONT_LOW;
            rsbHLPF.setRange(SI_HLPF_FRONT_REAR_FREQ_MIN, SI_HLPF_FRONT_REAR_FREQ_MAX); // 默认为前左右声道的调节范围
            ivSeekbarBg.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_seekbar_bg));
            freqProgressHigh = siExtDspHLPFSettings.getFreqProgress(SI_CHANNEL_FRONT_HIGH);
            freqProgressLow = siExtDspHLPFSettings.getFreqProgress(SI_CHANNEL_FRONT_LOW);
            freqHigh = siExtDspHLPFSettings.getFreq(SI_CHANNEL_FRONT_HIGH);
            freqLow = siExtDspHLPFSettings.getFreq(SI_CHANNEL_FRONT_LOW);
            qValueHigh = siExtDspHLPFSettings.getQValue(SI_CHANNEL_FRONT_HIGH);
            qValueLow = siExtDspHLPFSettings.getQValue(SI_CHANNEL_FRONT_LOW);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
            chanelHigh = SI_CHANNEL_REAR_HIGH;
            chanelLow = SI_CHANNEL_REAR_LOW;
            rsbHLPF.setRange(SI_HLPF_FRONT_REAR_FREQ_MIN, SI_HLPF_FRONT_REAR_FREQ_MAX); // 默认为后左右声道的调节范围
            ivSeekbarBg.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_seekbar_bg));
            freqProgressHigh = siExtDspHLPFSettings.getFreqProgress(SI_CHANNEL_REAR_HIGH);
            freqProgressLow = siExtDspHLPFSettings.getFreqProgress(SI_CHANNEL_REAR_LOW);
            freqHigh = siExtDspHLPFSettings.getFreq(SI_CHANNEL_REAR_HIGH);
            freqLow = siExtDspHLPFSettings.getFreq(SI_CHANNEL_REAR_LOW);
            qValueHigh = siExtDspHLPFSettings.getQValue(SI_CHANNEL_REAR_HIGH);
            qValueLow = siExtDspHLPFSettings.getQValue(SI_CHANNEL_REAR_LOW);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
            chanelHigh = SI_CHANNEL_SUBWOOFER_HIGH;
            chanelLow = SI_CHANNEL_SUBWOOFER_LOW;
            rsbHLPF.setRange(SI_HLPF_SUBWOOFER_FREQ_MIN, SI_HLPF_SUBWOOFER_FREQ_MAX); // 默认为低音左右声道的调节范围
            ivSeekbarBg.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_seekbar_bg_subwoofer));
            freqProgressHigh = siExtDspHLPFSettings.getFreqProgress(SI_CHANNEL_SUBWOOFER_HIGH);
            freqProgressLow = siExtDspHLPFSettings.getFreqProgress(SI_CHANNEL_SUBWOOFER_LOW);
            freqHigh = siExtDspHLPFSettings.getFreq(SI_CHANNEL_SUBWOOFER_HIGH);
            freqLow = siExtDspHLPFSettings.getFreq(SI_CHANNEL_SUBWOOFER_LOW);
            qValueHigh = siExtDspHLPFSettings.getQValue(SI_CHANNEL_SUBWOOFER_HIGH);
            qValueLow = siExtDspHLPFSettings.getQValue(SI_CHANNEL_SUBWOOFER_LOW);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
            chanelHigh = SI_CHANNEL_CENTER_HIGH;
            chanelLow = SI_CHANNEL_CENTER_LOW;
            rsbHLPF.setRange(SI_HLPF_FRONT_REAR_FREQ_MIN, SI_HLPF_FRONT_REAR_FREQ_MAX); // 默认为中置左右声道的调节范围
            ivSeekbarBg.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_seekbar_bg));
            freqProgressHigh = siExtDspHLPFSettings.getFreqProgress(SI_CHANNEL_CENTER_HIGH);
            freqProgressLow = siExtDspHLPFSettings.getFreqProgress(SI_CHANNEL_CENTER_LOW);
            freqHigh = siExtDspHLPFSettings.getFreq(SI_CHANNEL_CENTER_HIGH);
            freqLow = siExtDspHLPFSettings.getFreq(SI_CHANNEL_CENTER_LOW);
            qValueHigh = siExtDspHLPFSettings.getQValue(SI_CHANNEL_CENTER_HIGH);
            qValueLow = siExtDspHLPFSettings.getQValue(SI_CHANNEL_CENTER_LOW);
        } else {// clear 的时候，没有选中任何一个，直接返回
            return;
        }
        freqUIProgressHigh = (int) unconvertFreq(chanelHigh, (float) freqHigh);
        freqUIProgressLow = (int) unconvertFreq(chanelLow, (float) freqLow);
        rsbHLPF.setProgress(freqUIProgressHigh, freqUIProgressLow);

        if (freqHigh > 1000) {
            tvHpfFreq.setText(getString(R.string.hlpf_freq_khz, freqHigh / 1000));
        } else {
            tvHpfFreq.setText(getString(R.string.hlpf_freq_hz, freqHigh));
        }
        if (freqLow > 1000) {
            tvLpfFreq.setText(getString(R.string.hlpf_freq_khz, freqLow / 1000));
        } else {
            tvLpfFreq.setText(getString(R.string.hlpf_freq_hz, freqLow));
        }
        spHPFSlope.setSelection(getQValueSelect(qValueHigh));
        spLPFSlope.setSelection(getQValueSelect(qValueLow));

        boolean hasChildPressed = false;
        for (int i = 0; i < radioGroup.getChildCount(); i++) {
            if (radioGroup.getChildAt(i).isPressed()) {
                hasChildPressed = true;
            }
        }

        // 界面初始化时也会调用 onCheckedChanged 事件，这个时候不需要改变数据和实际音效
        // 手动点击时才往下执行
        if (!hasChildPressed) {
            return;
        }

        if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
            siExtDspHLPFSettings.saveHLPF(SI_CHANNEL_FRONT_HIGH, freqProgressHigh, freqHigh, qValueHigh);
            siExtDspHLPFSettings.saveHLPF(SI_CHANNEL_FRONT_LOW, freqProgressLow, freqLow, qValueLow);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
            siExtDspHLPFSettings.saveHLPF(SI_CHANNEL_REAR_HIGH, freqProgressHigh, freqHigh, qValueHigh);
            siExtDspHLPFSettings.saveHLPF(SI_CHANNEL_REAR_LOW, freqProgressLow, freqLow, qValueLow);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
            siExtDspHLPFSettings.saveHLPF(SI_CHANNEL_SUBWOOFER_HIGH, freqProgressHigh, freqHigh, qValueHigh);
            siExtDspHLPFSettings.saveHLPF(SI_CHANNEL_SUBWOOFER_LOW, freqProgressLow, freqLow, qValueLow);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
            siExtDspHLPFSettings.saveHLPF(SI_CHANNEL_CENTER_HIGH, freqProgressHigh, freqHigh, qValueHigh);
            siExtDspHLPFSettings.saveHLPF(SI_CHANNEL_CENTER_LOW, freqProgressLow, freqLow, qValueLow);
        }
    }

    @Override
    public void onClick(View view) {
        if (rgHLPFMode != null) {
            // 清除当前通道的数据，并设置到 native
            int checkedId = rgHLPFMode.getCheckedRadioButtonId();
            if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
                siExtDspHLPFSettings.resetByChannel(SI_CHANNEL_FRONT_HIGH, SI_CHANNEL_FRONT_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
                siExtDspHLPFSettings.resetByChannel(SI_CHANNEL_REAR_HIGH, SI_CHANNEL_REAR_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
                siExtDspHLPFSettings.resetByChannel(SI_CHANNEL_SUBWOOFER_HIGH, SI_CHANNEL_SUBWOOFER_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
                siExtDspHLPFSettings.resetByChannel(SI_CHANNEL_CENTER_HIGH, SI_CHANNEL_CENTER_LOW);
            }

            // 重新触发点击事件，刷新界面所有的 view
            rgHLPFMode.clearCheck();
            rgHLPFMode.check(checkedId);
        }

        switch (view.getId()) {
            case R.id.btn_reset_hlpf:
                if (rgHLPFMode != null) {
                    int checkedId = rgHLPFMode.getCheckedRadioButtonId();
                    // 只还原当前输出模式、输出通道下的频率和斜率数据
                    if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
                        siExtDspHLPFSettings.resetCurrentChannel(SI_CHANNEL_FRONT_HIGH, SI_CHANNEL_FRONT_LOW);
                    } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
                        siExtDspHLPFSettings.resetCurrentChannel(SI_CHANNEL_REAR_HIGH, SI_CHANNEL_REAR_LOW);
                    } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
                        siExtDspHLPFSettings.resetCurrentChannel(SI_CHANNEL_SUBWOOFER_HIGH, SI_CHANNEL_SUBWOOFER_LOW);
                    } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
                        siExtDspHLPFSettings.resetCurrentChannel(SI_CHANNEL_CENTER_HIGH, SI_CHANNEL_CENTER_LOW);
                    }
                }
                break;
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
        float freqProgressHigh, freqProgressLow;
        float freqHigh, freqLow;
        int qValueHigh, qValueLow;
        int channel;
        if (adapterView.getId() == SkinUtils.getId(R.id.sp_hpf_slope) && spinnerHPFFromUser) {
            Log.d(TAG, "spHPFSlope item selected");
            spinnerHPFFromUser = false;
            channel = getChannel(true);
            freqProgressHigh = rsbHLPF.getLeftSeekBar().getProgress();
            freqProgressLow = rsbHLPF.getRightSeekBar().getProgress();
            freqHigh = convertFreq(channel, freqProgressHigh);
            freqLow = convertFreq(channel, freqProgressLow);
            qValueHigh = getQValue(spHPFSlope);
            qValueLow = getQValue(spLPFSlope);
            siExtDspHLPFSettings.nativeHLPF(channel, qValueLow, (int) freqLow, qValueHigh, (int) freqHigh);
            siExtDspHLPFSettings.saveHLPF(channel, (int) freqProgressHigh, (int) freqHigh, qValueHigh);
        } else if (adapterView.getId() == SkinUtils.getId(R.id.sp_lpf_slope) && spinnerLPFFromUser) {
            Log.d(TAG, "spLPFSlope item selected");
            spinnerLPFFromUser = false;
            channel = getChannel(false);
            freqProgressHigh = rsbHLPF.getLeftSeekBar().getProgress();
            freqProgressLow = rsbHLPF.getRightSeekBar().getProgress();
            freqHigh = convertFreq(channel, freqProgressHigh);
            freqLow = convertFreq(channel, freqProgressLow);
            qValueHigh = getQValue(spHPFSlope);
            qValueLow = getQValue(spLPFSlope);
            siExtDspHLPFSettings.nativeHLPF(channel, qValueLow, (int) freqLow, qValueHigh, (int) freqHigh);
            siExtDspHLPFSettings.saveHLPF(channel, (int) freqProgressLow, (int) freqLow, qValueLow);
        }
        if ("600".equals(EqUtils.getEThemeGod())) {
            TextView textView = view.findViewById(SkinUtils.getId(R.id.tv_filter_slope));
            if (textView != null) {
                textView.setTextColor(Color.WHITE);
            }
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> adapterView) {
        Log.d(TAG, "onNothingSelected adapterView : " + adapterView.getId());
    }

    @Override
    public void onRangeChanged(RangeSeekBar view, float leftValue, float rightValue, boolean isFromUser) {
        Log.d(TAG, String.format("onRangeChanged leftValue : %f, rightValue : %f, isFromUser : %b", leftValue, rightValue, isFromUser));

        int channel = getChannel(isLeft);
        float freqProgress = (isLeft ? leftValue : rightValue);
        float freq = convertFreq(channel, freqProgress);

//        float freqHigh = convertFreq(channel, leftValue);
//        float freqLow = convertFreq(channel, rightValue);
//        int qValueHigh = getQValue(spHPFSlope);
//        int qValueLow = getQValue(spLPFSlope);

        if (isLeft) {
            if (freq > 1000) {
                tvHpfFreq.setText(getString(R.string.hlpf_freq_khz, (int) freq / 1000));
            } else {
                tvHpfFreq.setText(getString(R.string.hlpf_freq_hz, (int) freq));
            }
        } else {
            if (freq > 1000) {
                tvLpfFreq.setText(getString(R.string.hlpf_freq_khz, (int) freq / 1000));
            } else {
                tvLpfFreq.setText(getString(R.string.hlpf_freq_hz, (int) freq));
            }
        }

        if (!isFromUser) return;
//
//        siExtDspHLPFSettings.nativeHLPF(channel, qValueLow, (int) freqLow, qValueHigh, (int) freqHigh);
    }

    @Override
    public void onStartTrackingTouch(RangeSeekBar view, boolean isLeft) {
        Log.d(TAG, String.format("onStartTrackingTouch isLeft : %b", isLeft));

        this.isLeft = isLeft;
    }

    @Override
    public void onStopTrackingTouch(RangeSeekBar view, boolean isLeft) {
        Log.d(TAG, String.format("onStopTrackingTouch isLeft : %b", isLeft));
        int channel = getChannel(isLeft);
        float freqProgressHigh = view.getLeftSeekBar().getProgress();
        float freqProgressLow = view.getRightSeekBar().getProgress();
        float freqHigh = convertFreq(channel, freqProgressHigh);
        float freqLow = convertFreq(channel, freqProgressLow);
        int qValueHigh = getQValue(spHPFSlope);
        int qValueLow = getQValue(spLPFSlope);


        siExtDspHLPFSettings.saveHLPF(channel, isLeft ? (int) freqProgressHigh : (int) freqProgressLow,
                isLeft ? (int) freqHigh : (int) freqLow,
                isLeft ? qValueHigh : qValueLow);
        siExtDspHLPFSettings.nativeHLPF(channel, qValueLow, (int) freqLow, qValueHigh, (int) freqHigh);
    }

    private int getChannel(boolean high) {
        if (rgHLPFMode != null) {
            int checkedId = rgHLPFMode.getCheckedRadioButtonId();
            if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
                return high ? SI_CHANNEL_FRONT_HIGH : SI_CHANNEL_FRONT_LOW;
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
                return high ? SI_CHANNEL_REAR_HIGH : SI_CHANNEL_REAR_LOW;
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
                return high ? SI_CHANNEL_SUBWOOFER_HIGH : SI_CHANNEL_SUBWOOFER_LOW;
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
                return high ? SI_CHANNEL_CENTER_HIGH : SI_CHANNEL_CENTER_LOW;
            }
            return high ? SI_CHANNEL_FRONT_HIGH : SI_CHANNEL_FRONT_LOW;
        }
        return -1;
    }

    private int getQValue(Spinner spinner) {
        int position = spinner.getSelectedItemPosition();
        switch (position) {
            case 0:
                return 0;
            case 1:
                return 1;
            case 2:
                return 2;
            case 3:
                return 3;
            case 4:
                return 4;
            case 5:
                return 5;
            case 6:
                return 6;
            default:
                return 2;
        }
    }

    private int getQValueSelect(int value) {
        switch (value) {
            case 0:
                return 0;
            case 1:
                return 1;
            case 2:
                return 2;
            case 3:
                return 3;
            case 4:
                return 4;
            case 5:
                return 5;
            case 6:
                return 6;
            default:
                return 2;
        }
    }

    // seekbar 的进度值转换成真正的频点
    private float convertFreq(int channel, final float freqProgress) {
        float freq = freqProgress;
        if (channel == SI_CHANNEL_FRONT_HIGH || channel == SI_CHANNEL_FRONT_LOW || channel == SI_CHANNEL_REAR_HIGH || channel == SI_CHANNEL_REAR_LOW
                || channel == SI_CHANNEL_CENTER_HIGH || channel == SI_CHANNEL_CENTER_LOW) {
            if (freqProgress >= 20 && freqProgress <= 2600) {
                if (freqProgress >= 86) {
                    freq = 20 + (freqProgress / 86);
                } else {
                    freq = 20;
                }
            } else if (freqProgress > 2600 && freqProgress <= 4500) {
                freq = (freqProgress / 38) - 18;
            } else if (freqProgress > 4500 && freqProgress <= 6500) {
                freq = (freqProgress / 20) - 125;
            } else if (freqProgress > 6500 && freqProgress <= 9000) {
                freq = (freqProgress / 8.3f) - 584;
            } else if (freqProgress > 9000 && freqProgress <= 11000) {
                freq = (freqProgress / 4) - 1750;
            } else if (freqProgress > 11000 && freqProgress <= 12800) {
                freq = (freqProgress / 1.8f) - 5111;
            } else if (freqProgress > 12800 && freqProgress <= 15500) {
                freq = (freqProgress / 0.9f) - 12222;
            } else if (freqProgress > 15000 && freqProgress <= 17000) {
                freq = (freqProgress / 0.4f - 32500);
            } else if (freqProgress > 17000 && freqProgress <= 20000) {
                freq = (freqProgress / 0.3f - 46666);
            }

            if (freq > 20000) {
                freq = 20000;
            }
        }
        return freq;
    }

    // 真正的频点转换成进度值
    private float unconvertFreq(int channel, float freqValue) {
        float freqProgress = freqValue;
        if (channel == SI_CHANNEL_FRONT_HIGH || channel == SI_CHANNEL_FRONT_LOW || channel == SI_CHANNEL_REAR_HIGH || channel == SI_CHANNEL_REAR_LOW
                || channel == SI_CHANNEL_CENTER_HIGH || channel == SI_CHANNEL_CENTER_LOW) {
            if (freqValue > 20 && freqValue < 50) {
                freqProgress = 86 * (freqValue - 20);
            } else if (freqValue >= 50 && freqValue < 100) {
                freqProgress = 38 * (freqValue + 18);
            } else if (freqValue >= 100 && freqValue < 200) {
                freqProgress = 20 * (freqValue + 125);
            } else if (freqValue >= 200 && freqValue < 500) {
                freqProgress = 8 * (freqValue + 584);
            } else if (freqValue >= 500 && freqValue < 1000) {
                freqProgress = 4 * (freqValue + 1750);
            } else if (freqValue >= 1000 && freqValue < 2000) {
                freqProgress = 1.8f * (freqValue + 5111);
            } else if (freqValue >= 2000 && freqValue < 5000) {
                freqProgress = (freqValue + 12222) * 0.9f;
            } else if (freqValue >= 5000 && freqValue < 10000) {
                freqProgress = (freqValue + 32500) * 0.4f;
            } else if (freqValue >= 10000 && freqValue < 20000) {
                freqProgress = (freqValue + 46666) * 0.3f;
            }
        }
        return freqProgress;
    }
}
