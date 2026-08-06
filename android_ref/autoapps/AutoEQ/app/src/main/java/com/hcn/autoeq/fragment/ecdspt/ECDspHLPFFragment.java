package com.hcn.autoeq.fragment.ecdspt;

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
import com.hcn.autoeq.data.ExtDspHLPFSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.ECDConstantExtDsp;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.CustomSpinner;
import com.hcn.skin.support.resources.SkinCompatResources;
import com.jaygoo.widget.OnRangeChangedListener;
import com.jaygoo.widget.RangeSeekBar;

public class ECDspHLPFFragment extends BaseFragment
        implements ECDConstantExtDsp
        , View.OnClickListener, RadioGroup.OnCheckedChangeListener, AdapterView.OnItemSelectedListener, OnRangeChangedListener {

    private static final String TAG = ECDspHLPFFragment.class.getSimpleName();

    private View mainView;
    private ImageView ivSeekbarBg;
    private RangeSeekBar rsbHLPF;
    private ImageView ivHLPFSpeakerLF, ivHLPFSpeakerRF,
            ivHLPFSpeakerLR, ivHLPFSpeakerRR, ivHLPFSpeakerSubwoofer,
            ivHLPFSpeakerCenter;
    private RadioGroup rgHLPFMode;
    private Button btnHLPFReset;
    private TextView tvHpfFreq, tvLpfFreq;
    private CustomSpinner spHPFSlope, spLPFSlope;

    private ExtDspHLPFSettings extDspHLPFSettings;
    private boolean isLeft = false;
    private boolean spinnerHPFFromUser, spinnerLPFFromUser;

    public ECDspHLPFFragment() {
    }

    public static ECDspHLPFFragment newInstance() {
        ECDspHLPFFragment fragment = new ECDspHLPFFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        if (checkLayoutExists(R.layout.ext_c_dsp_fragment_hlpf)) {
            return R.layout.ext_c_dsp_fragment_hlpf;
        } else {
            return R.layout.extdsp_fragment_hlpf;
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        extDspHLPFSettings = ExtDspHLPFSettings.getInstance(mContext);
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
    }

    @Override
    public void initData() {
        super.initData();

        int channel = extDspHLPFSettings.getHLPFChannel();
        if (channel == CHANNEL_FRONT_HIGH || channel == CHANNEL_FRONT_LOW) {
            if (rgHLPFMode != null) {
                rgHLPFMode.check(SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr));
            }
        } else if (channel == CHANNEL_REAR_HIGH || channel == CHANNEL_REAR_LOW) {
            if (rgHLPFMode != null) {
                rgHLPFMode.check(SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr));
            }
        } else if (channel == CHANNEL_SUBWOOFER_HIGH || channel == CHANNEL_SUBWOOFER_LOW) {
            if (rgHLPFMode != null) {
                rgHLPFMode.check(SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer));
            }
        } else if (channel == CHANNEL_CENTER_HIGH || channel == CHANNEL_CENTER_LOW) {
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
        if (EqUtils.isYuFeng()) {
            ivHLPFSpeakerSubwoofer.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)
                    ? R.drawable.extdsp_dbb_speaker_subwoofer_p : R.drawable.extdsp_dbb_speaker_subwoofer_n));
            if (null != ivHLPFSpeakerCenter) {
                ivHLPFSpeakerCenter.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_hlpf_mode_center)
                        ? R.drawable.extdsp_dbb_speaker_subwoofer2_p : R.drawable.extdsp_dbb_speaker_subwoofer2_n));
            }
        }else {
            ivHLPFSpeakerSubwoofer.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)
                    ? R.drawable.extdsp_dbb_speaker_subwoofer_p : R.drawable.extdsp_dbb_speaker_subwoofer_n));
            if (null != ivHLPFSpeakerCenter) {
                ivHLPFSpeakerCenter.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_hlpf_mode_center)
                        ? R.drawable.extdsp_hlpf_speaker_center_p : R.drawable.extdsp_hlpf_speaker_center_n));
            }
        }

        int freqProgressHigh, freqProgressLow;
        int freqUIProgressHigh, freqUIProgressLow;
        int freqHigh, freqLow;
        int qValueHigh, qValueLow;
        int chanelHigh, chanelLow;
        if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
            chanelHigh = CHANNEL_FRONT_HIGH;
            chanelLow = CHANNEL_FRONT_LOW;
            rsbHLPF.setRange(HLPF_FRONT_REAR_FREQ_MIN, HLPF_FRONT_REAR_FREQ_MAX); // 默认为前后左右声道的调节范围
            ivSeekbarBg.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_seekbar_bg));
            freqProgressHigh = extDspHLPFSettings.getFreqProgress(CHANNEL_FRONT_HIGH);
            freqProgressLow = extDspHLPFSettings.getFreqProgress(CHANNEL_FRONT_LOW);
            freqHigh = extDspHLPFSettings.getFreq(CHANNEL_FRONT_HIGH);
            freqLow = extDspHLPFSettings.getFreq(CHANNEL_FRONT_LOW);
            qValueHigh = extDspHLPFSettings.getQValue(CHANNEL_FRONT_HIGH);
            qValueLow = extDspHLPFSettings.getQValue(CHANNEL_FRONT_LOW);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
            chanelHigh = CHANNEL_REAR_HIGH;
            chanelLow = CHANNEL_REAR_LOW;
            rsbHLPF.setRange(HLPF_FRONT_REAR_FREQ_MIN, HLPF_FRONT_REAR_FREQ_MAX); // 默认为前后左右声道的调节范围
            ivSeekbarBg.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_seekbar_bg));
            freqProgressHigh = extDspHLPFSettings.getFreqProgress(CHANNEL_REAR_HIGH);
            freqProgressLow = extDspHLPFSettings.getFreqProgress(CHANNEL_REAR_LOW);
            freqHigh = extDspHLPFSettings.getFreq(CHANNEL_REAR_HIGH);
            freqLow = extDspHLPFSettings.getFreq(CHANNEL_REAR_LOW);
            qValueHigh = extDspHLPFSettings.getQValue(CHANNEL_REAR_HIGH);
            qValueLow = extDspHLPFSettings.getQValue(CHANNEL_REAR_LOW);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
            chanelHigh = CHANNEL_SUBWOOFER_HIGH;
            chanelLow = CHANNEL_SUBWOOFER_LOW;
            rsbHLPF.setRange(HLPF_SUBWOOFER_FREQ_MIN, HLPF_SUBWOOFER_FREQ_MAX); // 默认为前后左右声道的调节范围
            ivSeekbarBg.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_seekbar_bg_subwoofer));
            freqProgressHigh = extDspHLPFSettings.getFreqProgress(CHANNEL_SUBWOOFER_HIGH);
            freqProgressLow = extDspHLPFSettings.getFreqProgress(CHANNEL_SUBWOOFER_LOW);
            freqHigh = extDspHLPFSettings.getFreq(CHANNEL_SUBWOOFER_HIGH);
            freqLow = extDspHLPFSettings.getFreq(CHANNEL_SUBWOOFER_LOW);
            qValueHigh = extDspHLPFSettings.getQValue(CHANNEL_SUBWOOFER_HIGH);
            qValueLow = extDspHLPFSettings.getQValue(CHANNEL_SUBWOOFER_LOW);
        }  else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {

            if (EqUtils.isYuFeng()) {
                chanelHigh = CHANNEL_CENTER_HIGH;
                chanelLow = CHANNEL_CENTER_LOW;
                rsbHLPF.setRange(HLPF_SUBWOOFER_FREQ_MIN, HLPF_SUBWOOFER_FREQ_MAX); // 默认为中置左右声道的调节范围
                ivSeekbarBg.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_seekbar_bg_subwoofer));
                freqProgressHigh = extDspHLPFSettings.getFreqProgress(CHANNEL_CENTER_HIGH);
                freqProgressLow = extDspHLPFSettings.getFreqProgress(CHANNEL_CENTER_LOW);
                freqHigh = extDspHLPFSettings.getFreq(CHANNEL_CENTER_HIGH);
                freqLow = extDspHLPFSettings.getFreq(CHANNEL_CENTER_LOW);
                qValueHigh = extDspHLPFSettings.getQValue(CHANNEL_CENTER_HIGH);
                qValueLow = extDspHLPFSettings.getQValue(CHANNEL_CENTER_LOW);
            }else {
                chanelHigh = CHANNEL_CENTER_HIGH;
                chanelLow = CHANNEL_CENTER_LOW;
                rsbHLPF.setRange(HLPF_FRONT_REAR_FREQ_MIN, HLPF_FRONT_REAR_FREQ_MAX); // 默认为中置左右声道的调节范围
                ivSeekbarBg.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_hlpf_seekbar_bg));
                freqProgressHigh = extDspHLPFSettings.getFreqProgress(CHANNEL_CENTER_HIGH);
                freqProgressLow = extDspHLPFSettings.getFreqProgress(CHANNEL_CENTER_LOW);
                freqHigh = extDspHLPFSettings.getFreq(CHANNEL_CENTER_HIGH);
                freqLow = extDspHLPFSettings.getFreq(CHANNEL_CENTER_LOW);
                qValueHigh = extDspHLPFSettings.getQValue(CHANNEL_CENTER_HIGH);
                qValueLow = extDspHLPFSettings.getQValue(CHANNEL_CENTER_LOW);
            }

        } else {// clear 的时候，没有选中任何一个，直接返回
            return;
        }
        freqUIProgressHigh = (int) unConvertFreq(chanelHigh, (float) freqHigh);
        freqUIProgressLow = (int) unConvertFreq(chanelLow, (float) freqLow);
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
            extDspHLPFSettings.nativeHLPF(CHANNEL_FRONT_HIGH, freqHigh, spHPFSlope.getSelectedItemPosition());
            extDspHLPFSettings.nativeHLPF(CHANNEL_FRONT_LOW, freqLow, spLPFSlope.getSelectedItemPosition());
            extDspHLPFSettings.saveHLPF(CHANNEL_FRONT_HIGH, freqProgressHigh, freqHigh, qValueHigh);
            extDspHLPFSettings.saveHLPF(CHANNEL_FRONT_LOW, freqProgressLow, freqLow, qValueLow);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
            extDspHLPFSettings.nativeHLPF(CHANNEL_REAR_HIGH, freqHigh, spHPFSlope.getSelectedItemPosition());
            extDspHLPFSettings.nativeHLPF(CHANNEL_REAR_LOW, freqLow, spLPFSlope.getSelectedItemPosition());
            extDspHLPFSettings.saveHLPF(CHANNEL_REAR_HIGH, freqProgressHigh, freqHigh, qValueHigh);
            extDspHLPFSettings.saveHLPF(CHANNEL_REAR_LOW, freqProgressLow, freqLow, qValueLow);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
            extDspHLPFSettings.nativeHLPF(CHANNEL_SUBWOOFER_HIGH, freqHigh, spHPFSlope.getSelectedItemPosition());
            extDspHLPFSettings.nativeHLPF(CHANNEL_SUBWOOFER_LOW, freqLow, spLPFSlope.getSelectedItemPosition());
            extDspHLPFSettings.saveHLPF(CHANNEL_SUBWOOFER_HIGH, freqProgressHigh, freqHigh, qValueHigh);
            extDspHLPFSettings.saveHLPF(CHANNEL_SUBWOOFER_LOW, freqProgressLow, freqLow, qValueLow);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
            extDspHLPFSettings.nativeHLPF(CHANNEL_CENTER_HIGH, freqHigh, spHPFSlope.getSelectedItemPosition());
            extDspHLPFSettings.nativeHLPF(CHANNEL_CENTER_LOW, freqLow, spLPFSlope.getSelectedItemPosition());
            extDspHLPFSettings.saveHLPF(CHANNEL_CENTER_HIGH, freqProgressHigh, freqHigh, qValueHigh);
            extDspHLPFSettings.saveHLPF(CHANNEL_CENTER_LOW, freqProgressLow, freqLow, qValueLow);
        }
    }

    @Override
    public void onClick(View view) {
        if (rgHLPFMode != null) {
            // 清除当前通道的数据，并设置到 native
            int checkedId = rgHLPFMode.getCheckedRadioButtonId();
            if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
                extDspHLPFSettings.resetByChannel(CHANNEL_FRONT_HIGH);
                extDspHLPFSettings.resetByChannel(CHANNEL_FRONT_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
                extDspHLPFSettings.resetByChannel(CHANNEL_REAR_HIGH);
                extDspHLPFSettings.resetByChannel(CHANNEL_REAR_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
                extDspHLPFSettings.resetByChannel(CHANNEL_SUBWOOFER_HIGH);
                extDspHLPFSettings.resetByChannel(CHANNEL_SUBWOOFER_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
                extDspHLPFSettings.resetByChannel(CHANNEL_CENTER_HIGH);
                extDspHLPFSettings.resetByChannel(CHANNEL_CENTER_LOW);
            }

            // 重新触发点击事件，刷新界面所有的 view
            rgHLPFMode.clearCheck();
            rgHLPFMode.check(checkedId);
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
        float freqProgress, freq;
        int channel, qValue;
        if (adapterView.getId() == SkinUtils.getId(R.id.sp_hpf_slope) && spinnerHPFFromUser) {
            Log.d(TAG, "spHPFSlope item selected");
            spinnerHPFFromUser = false;
            channel = getChannel(true);
            freqProgress = rsbHLPF.getLeftSeekBar().getProgress();
            freq = convertFreq(channel, freqProgress);
            qValue = getQValue(spHPFSlope);
            extDspHLPFSettings.nativeHLPF(channel, (int) freq, spHPFSlope.getSelectedItemPosition());
            extDspHLPFSettings.saveHLPF(channel, (int) freqProgress, (int) freq, qValue);
        } else if (adapterView.getId() == SkinUtils.getId(R.id.sp_lpf_slope) && spinnerLPFFromUser) {
            Log.d(TAG, "spLPFSlope item selected");
            spinnerLPFFromUser = false;
            channel = getChannel(false);
            freqProgress = rsbHLPF.getRightSeekBar().getProgress();
            freq = convertFreq(channel, freqProgress);
            qValue = getQValue(spLPFSlope);
            extDspHLPFSettings.nativeHLPF(channel, (int) freq, spLPFSlope.getSelectedItemPosition());
            extDspHLPFSettings.saveHLPF(channel, (int) freqProgress, (int) freq, qValue);
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
        int qValue = getQValue(isLeft ? spHPFSlope : spLPFSlope);

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

        extDspHLPFSettings.nativeHLPF(channel, (int) freq, isLeft ? spHPFSlope.getSelectedItemPosition() : spLPFSlope.getSelectedItemPosition());
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
        float freqProgress, freq;
        int qValue;
        if (isLeft) {
            freqProgress = view.getLeftSeekBar().getProgress();
            qValue = getQValue(spHPFSlope);
        } else {
            freqProgress = view.getRightSeekBar().getProgress();
            qValue = getQValue(spLPFSlope);
        }
        freq = convertFreq(channel, freqProgress);
        extDspHLPFSettings.saveHLPF(channel, (int) freqProgress, (int) freq, qValue);
        extDspHLPFSettings.nativeHLPF(channel, (int) freq, isLeft ? spHPFSlope.getSelectedItemPosition() : spLPFSlope.getSelectedItemPosition());
    }

    private int getChannel(boolean high) {
        if (rgHLPFMode != null) {
            int checkedId = rgHLPFMode.getCheckedRadioButtonId();
            if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
                return high ? CHANNEL_FRONT_HIGH : CHANNEL_FRONT_LOW;
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
                return high ? CHANNEL_REAR_HIGH : CHANNEL_REAR_LOW;
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
                return high ? CHANNEL_SUBWOOFER_HIGH : CHANNEL_SUBWOOFER_LOW;
            }  else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
                return high ? CHANNEL_CENTER_HIGH : CHANNEL_CENTER_LOW;
            }
            return high ? CHANNEL_FRONT_HIGH : CHANNEL_FRONT_LOW;
        }
        return -1;
    }

    private int getQValue(Spinner spinner) {
        int position = spinner.getSelectedItemPosition();
        switch (position) {
            case 0:
                return 0;
            case 1:
                return 700;
            case 2:
                return 1000;
            case 3:
                return 1500;
            case 4:
                return 2000;
            case 5:
                return 2500;
            case 6:
                return 3000;
            default:
                return HLPF_QVALUE_DEFAULT;
        }
    }

    private int getQValueSelect(int value) {
        switch (value) {
            case 0:
                return 0;
            case 700:
                return 1;
            case 1000:
                return 2;
            case 1500:
                return 3;
            case 2000:
                return 4;
            case 2500:
                return 5;
            case 3000:
                return 6;
            default:
                return 1;
        }
    }

    // seekbar 的进度值转换成真正的频点
    private float convertFreq(int channel, final float freqProgress) {
        float freq = freqProgress;
        boolean isSpecialCustomer = EqUtils.isYuFeng();
        boolean isFrontOrRearChannel = channel == CHANNEL_FRONT_HIGH || channel == CHANNEL_FRONT_LOW ||
                channel == CHANNEL_REAR_HIGH || channel == CHANNEL_REAR_LOW;
        boolean isCenterChannel = channel == CHANNEL_CENTER_HIGH || channel == CHANNEL_CENTER_LOW;

        if ((isSpecialCustomer && isFrontOrRearChannel) || (!isSpecialCustomer && (isFrontOrRearChannel || isCenterChannel))) {
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

    private float unConvertFreq(int channel, float freqValue) {
        float freqProgress = freqValue;
        boolean isSpecialCustomer = EqUtils.isYuFeng();
        boolean isFrontOrRearChannel = channel == CHANNEL_FRONT_HIGH || channel == CHANNEL_FRONT_LOW ||
                channel == CHANNEL_REAR_HIGH || channel == CHANNEL_REAR_LOW;
        boolean isCenterChannel = channel == CHANNEL_CENTER_HIGH || channel == CHANNEL_CENTER_LOW;

        if ((isSpecialCustomer && isFrontOrRearChannel) || (!isSpecialCustomer && (isFrontOrRearChannel || isCenterChannel))) {
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
