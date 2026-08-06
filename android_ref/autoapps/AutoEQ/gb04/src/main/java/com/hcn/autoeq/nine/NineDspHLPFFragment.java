package com.hcn.autoeq.nine;

import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.ToggleButton;

import com.hcn.autoeq.R;
import com.hcn.autoeq.adapter.NineFilterGirdAdapter;
import com.hcn.autoeq.view.NineHLFView;
import com.hcn_library.BaseFragment;
import com.hcn_library.data.NineDspHLPFSettings;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;
import com.hcn_library.util.SkinUtils;

import java.math.BigDecimal;

public class NineDspHLPFFragment extends BaseFragment
        implements NineConstantExtDsp, NineFilterFilterFragment.FragmentResetInterface, View.OnClickListener, View.OnLongClickListener, View.OnTouchListener {
    private static final int CLICK_DELAY = 100;
    private static final String TAG = "NineDspHLPFFragment";
    private NineFilterGirdAdapter adapterH;
    private NineFilterGirdAdapter adapterL;
    private Runnable clickRunnable;
    private ImageView currentBtn;
    private GridView grid_H;
    private GridView grid_L;
    private ImageView hArrowPlus;
    private ImageView hArrowReduce;
    private ImageView ivForeFront;
    private ImageView ivFrontL;
    private ImageView ivFrontR;
    private ImageView ivRear;
    private ImageView ivRearL;
    private ImageView ivRearR;
    private ImageView ivSeekbarBg;
    private ImageView lArrowPlus;
    private ImageView lArrowReduce;
    private View mainView;
    private NineDspHLPFSettings nineDspHLPFSettings;
    private NineHLFView nineHLFView;
    private RadioGroup rgHLPFMode;
    private ToggleButton slopeSwitchH;
    private ToggleButton slopeSwitchL;
    private TextView tvHpfFreq;
    private TextView tvLpfFreq;
    private Handler handler = new Handler();

    @Override
    public int getLayoutRes() {
        return R.layout.nine_dsp_fragment_hlpf;
    }

    public static NineDspHLPFFragment newInstance() {
        return new NineDspHLPFFragment();
    }

    @Override
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        mainView = super.onCreateView(layoutInflater, viewGroup, bundle);
        nineDspHLPFSettings = NineDspHLPFSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        hArrowReduce = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_h_arrow_delay));
        hArrowPlus = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_h_arrow_plus));
        lArrowReduce = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_l_arrow_delay));
        lArrowPlus = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_l_arrow_plus));
        hArrowReduce.setOnClickListener(this);
        hArrowPlus.setOnClickListener(this);
        lArrowReduce.setOnClickListener(this);
        lArrowPlus.setOnClickListener(this);
        hArrowReduce.setOnLongClickListener(this);
        hArrowPlus.setOnLongClickListener(this);
        lArrowReduce.setOnLongClickListener(this);
        lArrowPlus.setOnLongClickListener(this);
        hArrowReduce.setOnTouchListener(this);
        hArrowPlus.setOnTouchListener(this);
        lArrowReduce.setOnTouchListener(this);
        lArrowPlus.setOnTouchListener(this);
        ivSeekbarBg = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_seekbar_bg));
        nineHLFView = (NineHLFView) mainView.findViewById(R.id.nine_hlpf);
        // 设置折线图监听
        nineHLFView.setTouchLineMoveInterface(new NineHLFView.TouchLineMoveInterface() {
            @Override
            public void onStartMoveLine() {
            }

            @Override
            public void onLineMove(float f, float f2) {
                Log.d(TAG, "onLineMove freqHigh: " + f + " freqLow: " + f2);
                if (f > 1000.0f) {
                    tvHpfFreq.setText(new BigDecimal(f / 1000.0f).setScale(1, 4) + " KHz");
                } else {
                    tvHpfFreq.setText(getString(R.string.hlpf_freq_hz, Integer.valueOf((int) f)));
                }
                if (f2 > 1000.0f) {
                    tvLpfFreq.setText(new BigDecimal(f2 / 1000.0f).setScale(1, 4) + " KHz");
                } else {
                    tvLpfFreq.setText(getString(R.string.hlpf_freq_hz, Integer.valueOf((int) f2)));
                }
            }

            @Override
            public void onLineMoveStop(float freqHigh, float freqLow) {
                Log.d(TAG, "onLineMoveStop freqHigh: " + freqHigh + " freqLow: " + freqLow);

                int qValue = getQValue(adapterH);
                int qValue2 = getQValue(adapterL);
                nineDspHLPFSettings.saveHLPF(getChannel(true), (int) freqHigh, qValue);
                nineDspHLPFSettings.saveHLPF(getChannel(false), (int) freqLow, qValue2);
                if ("gb05".equals(EqUtils.getSkinName()) || EqUtils.isChip7739()) {
                    nineDspHLPFSettings.nativeHLPF(getChannel(false), qValue2, (int) freqLow, qValue, (int) freqHigh);
                    nineDspHLPFSettings.nativeHLPF(getChannel(true), qValue2, (int) freqLow, qValue, (int) freqHigh);
                } else {
                    nineDspHLPFSettings.nativeHLPF(getChannel(true), qValue2, (int) freqLow, qValue, (int) freqHigh);
                }
                if (freqHigh > 1000.0f) {
                    tvHpfFreq.setText(new BigDecimal(freqHigh / 1000.0f).setScale(1, 4) + " KHz");
                } else {
                    tvHpfFreq.setText(getString(R.string.hlpf_freq_hz, Integer.valueOf((int) freqHigh)));
                }
                if (freqLow > 1000.0f) {
                    tvLpfFreq.setText(new BigDecimal(freqLow / 1000.0f).setScale(1, 4) + " KHz");
                } else {
                    tvLpfFreq.setText(getString(R.string.hlpf_freq_hz, Integer.valueOf((int) freqLow)));
                }
            }
        });
        rgHLPFMode = (RadioGroup) mainView.findViewById(SkinUtils.getId(R.id.rg_hlpf_output_mode));
        if (rgHLPFMode != null) {
           initRgListener();
        }
        // 高通斜率面板
        grid_H = (GridView) mainView.findViewById(SkinUtils.getId(R.id.grid_view_h));
        adapterH = new NineFilterGirdAdapter(mContext, new String[]{"6DB/0", "12DB/0", "18DB/0", "24DB/0", "36DB/0", "48DB/0"});
        adapterH.setToggleButtonStateChangeListener(new NineFilterGirdAdapter.ToggleButtonStateChangeListener() {
            @Override
            public void onToggleButtonStateChanged(int position, boolean checked, boolean fromUser) {
                adapterH.setSelected(position);
                Log.d(TAG, "onToggleButtonStateChanged high  fromUser: " + (fromUser ? "true" : "false") + "状态变化，位置: " + position + ", 状态: " + checked);
                if (fromUser) {
                    Log.d(TAG, "grid_high item checked");
                    int channel = getChannel(true);
                    float freq = nineDspHLPFSettings.getFreq(getChannel(true));
                    float freq2 = nineDspHLPFSettings.getFreq(getChannel(false));
                    int qValue = getQValue(adapterH);
                    int i2 = (int) freq;
                    nineDspHLPFSettings.nativeHLPF(channel, getQValue(adapterL), (int) freq2, qValue, i2);
                    nineDspHLPFSettings.saveHLPF(channel, i2, qValue);
                    refreshHLFLine();
                }
            }
        });
        grid_H.setAdapter(adapterH);
        // 低通斜率面板
        grid_L = (GridView) mainView.findViewById(SkinUtils.getId(R.id.grid_view_l));
        adapterL = new NineFilterGirdAdapter(mContext, new String[]{"6DB/0", "12DB/0", "18DB/0", "24DB/0", "36DB/0", "48DB/0"});
        adapterL.setToggleButtonStateChangeListener(new NineFilterGirdAdapter.ToggleButtonStateChangeListener() {
            @Override
            public void onToggleButtonStateChanged(int i, boolean z, boolean z2) {
                adapterL.setSelected(i);
                Log.d(TAG, "onToggleButtonStateChanged low fromUser: " + (z2 ? "true" : "false") + "状态变化，位置: " + i + ", 状态: " + z);
                if (z2) {
                    Log.d(TAG, "grid_low item checked");
                    int channel = getChannel(false);
                    float freq = nineDspHLPFSettings.getFreq(getChannel(true));
                    float freq2 = nineDspHLPFSettings.getFreq(getChannel(false));
                    int qValue = getQValue(adapterH);
                    int qValue2 = getQValue(adapterL);
                    int i2 = (int) freq2;
                    nineDspHLPFSettings.nativeHLPF(channel, qValue2, i2, qValue, (int) freq);
                    nineDspHLPFSettings.saveHLPF(channel, i2, qValue2);
                    refreshHLFLine();
                }
            }
        });
        grid_L.setAdapter(adapterL);
        // 高通斜率开关
        slopeSwitchH = (ToggleButton) mainView.findViewById(SkinUtils.getId(R.id.bt_slope_h));
        slopeSwitchH.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean z) {
                grid_H.setAlpha(z ? 1f : 0.4f);
                setSlopeSwitch(z, compoundButton.isPressed(), true);
            }
        });
        // 低通斜率开关
        slopeSwitchL = (ToggleButton) mainView.findViewById(SkinUtils.getId(R.id.bt_slope_l));
        slopeSwitchL.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean z) {
                grid_L.setAlpha(z ? 1f : 0.4f);
                setSlopeSwitch(z, compoundButton.isPressed(), false);
            }
        });
        ivFrontL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_front_l));
        ivFrontR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_front_r));
        ivRearL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_rear_l));
        ivRearR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_rear_r));
        ivForeFront = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_forefront));
        ivRear = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_hlpf_rear));
        tvHpfFreq = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_freq_h));
        tvLpfFreq = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_freq_l));
    }

    public void setSlopeSwitch(boolean isChecked, boolean isPressed, boolean isHigh) {
        int channel = isHigh ? getChannel(true) : getChannel(false);
        float freq = nineDspHLPFSettings.getFreq(getChannel(true));
        float freq2 = nineDspHLPFSettings.getFreq(getChannel(false));
        if (isHigh) {
            if (isChecked) {
                adapterH.setSelected(getQValueSelect(nineDspHLPFSettings.getQValue(channel)));
            } else {
                adapterH.setAllUnselected();
            }
        } else if (isChecked) {
            adapterL.setSelected(getQValueSelect(nineDspHLPFSettings.getQValue(channel)));
        } else {
            adapterL.setAllUnselected();
        }

        nineDspHLPFSettings.saveSlopeSwitch(channel, isChecked);
        int qValue = nineDspHLPFSettings.getSlopeSwitch(channel) ? nineDspHLPFSettings.getQValue(channel) : 0;
        int qValue2 = nineDspHLPFSettings.getSlopeSwitch(channel) ? nineDspHLPFSettings.getQValue(channel) : 0;
        if (isPressed) {
            nineDspHLPFSettings.nativeHLPF(channel, qValue2, (int) freq2, qValue, (int) freq);
            refreshHLFLine();
        }
        Log.d(TAG, "setSlopeSwitch  isPressed: " + (isPressed ? "true" : "false") + ", 状态: " + isChecked + " isHigh:" + isHigh + " qValueHigh: " + qValue + " qValueLow: " + qValue2);
    }

    @Override
    public void initData() {
        super.initData();
        ivFrontL.setSelected(false);
        ivFrontR.setSelected(false);
        ivRearL.setSelected(false);
        ivRearR.setSelected(false);
        ivForeFront.setSelected(false);
        ivRear.setSelected(false);
        int channel = nineDspHLPFSettings.getHLPFChannel();
        if (channel == NineConstantExtDsp.NINE_CHANNEL_FRONT_HIGH || channel == NineConstantExtDsp.NINE_CHANNEL_FRONT_LOW) {
            if (rgHLPFMode != null) {
                rgHLPFMode.check(SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr));
            }
            ivFrontL.setSelected(true);
            ivFrontR.setSelected(true);
        } else if (channel == NineConstantExtDsp.NINE_CHANNEL_REAR_HIGH || channel == NineConstantExtDsp.NINE_CHANNEL_REAR_LOW) {
            if (rgHLPFMode != null) {
                rgHLPFMode.check(SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr));
            }
            ivRearL.setSelected(true);
            ivRearR.setSelected(true);
        } else if (channel == NineConstantExtDsp.NINE_CHANNEL_CENTER_HIGH || channel == NineConstantExtDsp.NINE_CHANNEL_CENTER_LOW) {
            if (rgHLPFMode != null) {
                rgHLPFMode.check(SkinUtils.getId(R.id.rb_hlpf_mode_center));
            }
            ivForeFront.setSelected(true);
        } else if (channel == NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_HIGH || channel == NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_LOW) {
            if (rgHLPFMode != null) {
                rgHLPFMode.check(SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer));
            }
            ivRear.setSelected(true);
        }
        refreshChannelView();
    }


    public void refreshHLFLine() {
        float qValue = nineDspHLPFSettings.getSlopeSwitch(getChannel(true)) ? nineDspHLPFSettings.getQValue(getChannel(true)) : 0.0f; // 开关开则设置斜率值，开关关则斜率为0
        float qValue2 = nineDspHLPFSettings.getSlopeSwitch(getChannel(false)) ? nineDspHLPFSettings.getQValue(getChannel(false)) : 0.0f;
        nineHLFView.setFreqHigh(nineDspHLPFSettings.getFreq(getChannel(true)));
        nineHLFView.setFreqLow(nineDspHLPFSettings.getFreq(getChannel(false)));
        nineHLFView.setSlopeH(qValue);
        nineHLFView.setSlopeL(qValue2);
        nineHLFView.reDraw();
    }

    private void initRgListener() {
        for (int i = 0; i < rgHLPFMode.getChildCount(); i++) {
            View child = rgHLPFMode.getChildAt(i);
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
        int checkedId = rgHLPFMode.getCheckedRadioButtonId();
        ivFrontL.setSelected(false);
        ivFrontR.setSelected(false);
        ivRearL.setSelected(false);
        ivRearR.setSelected(false);
        ivForeFront.setSelected(false);
        ivRear.setSelected(false);
        int freqHigh, freqLow;
        int qValueHigh, qValueLow;
        int chanelHigh, chanelLow;
        if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
            chanelHigh = NineConstantExtDsp.NINE_CHANNEL_FRONT_HIGH;
            chanelLow = NineConstantExtDsp.NINE_CHANNEL_FRONT_LOW;
            nineHLFView.setFreqLimit(NineConstantExtDsp.NINE_HLPF_FRONT_REAR_FREQ_MIN, NineConstantExtDsp.NINE_HLPF_FRONT_REAR_FREQ_MAX);
            freqHigh = nineDspHLPFSettings.getFreq(NineConstantExtDsp.NINE_CHANNEL_FRONT_HIGH);
            freqLow = nineDspHLPFSettings.getFreq(NineConstantExtDsp.NINE_CHANNEL_FRONT_LOW);
            qValueHigh = nineDspHLPFSettings.getQValue(NineConstantExtDsp.NINE_CHANNEL_FRONT_HIGH);
            qValueLow = nineDspHLPFSettings.getQValue(NineConstantExtDsp.NINE_CHANNEL_FRONT_LOW);
            ivFrontL.setSelected(true);
            ivFrontR.setSelected(true);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
            chanelHigh = NineConstantExtDsp.NINE_CHANNEL_REAR_HIGH;
            chanelLow = NineConstantExtDsp.NINE_CHANNEL_REAR_LOW;
            nineHLFView.setFreqLimit(NineConstantExtDsp.NINE_HLPF_FRONT_REAR_FREQ_MIN, NineConstantExtDsp.NINE_HLPF_FRONT_REAR_FREQ_MAX);
            freqHigh = nineDspHLPFSettings.getFreq(NineConstantExtDsp.NINE_CHANNEL_REAR_HIGH);
            freqLow = nineDspHLPFSettings.getFreq(NineConstantExtDsp.NINE_CHANNEL_REAR_LOW);
            qValueHigh = nineDspHLPFSettings.getQValue(NineConstantExtDsp.NINE_CHANNEL_REAR_HIGH);
            qValueLow = nineDspHLPFSettings.getQValue(NineConstantExtDsp.NINE_CHANNEL_REAR_LOW);
            ivRearL.setSelected(true);
            ivRearR.setSelected(true);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
            chanelHigh = NineConstantExtDsp.NINE_CHANNEL_CENTER_HIGH;
            chanelLow = NineConstantExtDsp.NINE_CHANNEL_CENTER_LOW;
            nineHLFView.setFreqLimit(NineConstantExtDsp.NINE_HLPF_FRONT_REAR_FREQ_MIN, NineConstantExtDsp.NINE_HLPF_FRONT_REAR_FREQ_MAX);
            freqHigh = nineDspHLPFSettings.getFreq(NineConstantExtDsp.NINE_CHANNEL_CENTER_HIGH);
            freqLow = nineDspHLPFSettings.getFreq(NineConstantExtDsp.NINE_CHANNEL_CENTER_LOW);
            qValueHigh = nineDspHLPFSettings.getQValue(NineConstantExtDsp.NINE_CHANNEL_CENTER_HIGH);
            qValueLow = nineDspHLPFSettings.getQValue(NineConstantExtDsp.NINE_CHANNEL_CENTER_LOW);
            ivForeFront.setSelected(true);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
            chanelHigh = NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_HIGH;
            chanelLow = NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_LOW;
            nineHLFView.setFreqLimit(NineConstantExtDsp.NINE_HLPF_SUBWOOFER_FREQ_MIN, NineConstantExtDsp.NINE_HLPF_SUBWOOFER_FREQ_MAX);
            freqHigh = nineDspHLPFSettings.getFreq(NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_HIGH);
            freqLow = nineDspHLPFSettings.getFreq(NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_LOW);
            qValueHigh = nineDspHLPFSettings.getQValue(NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_HIGH);
            qValueLow = nineDspHLPFSettings.getQValue(NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_LOW);
            ivRear.setSelected(true);
        } else {// clear 的时候，没有选中任何一个，直接返回
            return;
        }
        if (freqHigh > 1000) {
            float number = freqHigh / 1000;
            BigDecimal bd = new BigDecimal(number);
            bd = bd.setScale(1, BigDecimal.ROUND_HALF_UP);
            tvHpfFreq.setText(bd + " kHz");
        } else {
            tvHpfFreq.setText(freqHigh + " Hz");
        }
        if (freqLow > 1000) {
            float number = freqLow / 1000;
            BigDecimal bd = new BigDecimal(number);
            bd = bd.setScale(1, BigDecimal.ROUND_HALF_UP);
            tvLpfFreq.setText(bd + " kHz");
        } else {
            tvLpfFreq.setText(freqLow + " Hz");
        }


        boolean hasChildPressed = false;
        for (int i = 0; i < rgHLPFMode.getChildCount(); i++) {
            if (rgHLPFMode.getChildAt(i).isPressed()) {
                hasChildPressed = true;
            }
        }
        // 更新折线图
        refreshHLFLine();
        // 斜率开关和选项初始化
        slopeSwitchH.setChecked(nineDspHLPFSettings.getSlopeSwitch(chanelHigh));
        slopeSwitchL.setChecked(nineDspHLPFSettings.getSlopeSwitch(chanelLow));
        // 确保斜率面板数据更新，只更新斜率开关可能不会调用onCheckedChange方法
        boolean isSwitchHOpen = nineDspHLPFSettings.getSlopeSwitch(chanelHigh);
        boolean isSwitchLOpen = nineDspHLPFSettings.getSlopeSwitch(chanelLow);
        if (isSwitchHOpen) {
            adapterH.setSelected(getQValueSelect(nineDspHLPFSettings.getQValue(chanelHigh)));
        }
        if (isSwitchLOpen) {
            adapterL.setSelected(getQValueSelect(nineDspHLPFSettings.getQValue(chanelLow)));
        }
        // 控制面板透明度
        grid_H.setAlpha(isSwitchHOpen ? 1f : 0.4f);
        grid_L.setAlpha(isSwitchLOpen ? 1f : 0.4f);

        // 界面初始化时也会调用 onCheckedChanged 事件，这个时候不需要改变数据和实际音效
        // 手动点击时才往下执行
        if (!hasChildPressed) {
            return;
        }

        if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
            nineDspHLPFSettings.saveHLPF(NineConstantExtDsp.NINE_CHANNEL_FRONT_HIGH, freqHigh, qValueHigh);
            nineDspHLPFSettings.saveHLPF(NineConstantExtDsp.NINE_CHANNEL_FRONT_LOW, freqLow, qValueLow);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
            nineDspHLPFSettings.saveHLPF(NineConstantExtDsp.NINE_CHANNEL_REAR_HIGH, freqHigh, qValueHigh);
            nineDspHLPFSettings.saveHLPF(NineConstantExtDsp.NINE_CHANNEL_REAR_LOW, freqLow, qValueLow);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
            nineDspHLPFSettings.saveHLPF(NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_HIGH, freqHigh, qValueHigh);
            nineDspHLPFSettings.saveHLPF(NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_LOW, freqLow, qValueLow);
        } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
            nineDspHLPFSettings.saveHLPF(NineConstantExtDsp.NINE_CHANNEL_CENTER_HIGH, freqHigh, qValueHigh);
            nineDspHLPFSettings.saveHLPF(NineConstantExtDsp.NINE_CHANNEL_CENTER_LOW, freqLow, qValueLow);
        }

    }

    private int getChannel(boolean high) {
        if (rgHLPFMode != null) {
            int checkedId = rgHLPFMode.getCheckedRadioButtonId();
            if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
                return high ? NineConstantExtDsp.NINE_CHANNEL_FRONT_HIGH : NineConstantExtDsp.NINE_CHANNEL_FRONT_LOW;
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
                return high ? NineConstantExtDsp.NINE_CHANNEL_REAR_HIGH : NineConstantExtDsp.NINE_CHANNEL_REAR_LOW;
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
                return high ? NineConstantExtDsp.NINE_CHANNEL_CENTER_HIGH : NineConstantExtDsp.NINE_CHANNEL_CENTER_LOW;
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
                return high ? NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_HIGH : NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_LOW;
            }
            return high ? NineConstantExtDsp.NINE_CHANNEL_FRONT_HIGH : NineConstantExtDsp.NINE_CHANNEL_FRONT_LOW;
        }
        return -1;
    }

    private int getQValue(NineFilterGirdAdapter adapter) {
        int position = adapter.getSelectedPosition();
        // 关闭斜率，才为0
        switch (position) {
            case 0:
                return 1;
            case 1:
                return 2;
            case 2:
                return 3;
            case 3:
                return 4;
            case 4:
                return 5;
            case 5:
                return 6;
            default:
                return 1;
        }
    }

    private int getQValueSelect(int value) {
        Log.d(TAG, "getQValueSelect value: " + value);
        switch (value) {
            case 1:
                return 0;
            case 2:
                return 1;
            case 3:
                return 2;
            case 4:
                return 3;
            case 5:
                return 4;
            case 6:
                return 5;
            default:
                return 0;
        }
    }


    @Override
    public void onReset() {
        if (rgHLPFMode != null) {
            // 清除当前通道的数据，并设置到 native
            int checkedId = rgHLPFMode.getCheckedRadioButtonId();
            if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
                nineDspHLPFSettings.resetByChannel(NineConstantExtDsp.NINE_CHANNEL_FRONT_HIGH, NineConstantExtDsp.NINE_CHANNEL_FRONT_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
                nineDspHLPFSettings.resetByChannel(NineConstantExtDsp.NINE_CHANNEL_REAR_HIGH, NineConstantExtDsp.NINE_CHANNEL_REAR_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
                nineDspHLPFSettings.resetByChannel(NineConstantExtDsp.NINE_CHANNEL_CENTER_HIGH, NineConstantExtDsp.NINE_CHANNEL_CENTER_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
                nineDspHLPFSettings.resetByChannel(NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_HIGH, NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_LOW);
            }

            // 重新触发点击事件，刷新界面所有的 hcn.view
            rgHLPFMode.clearCheck();
            rgHLPFMode.check(checkedId);
            refreshChannelView();
        }

        if (rgHLPFMode != null) {
            int checkedId = rgHLPFMode.getCheckedRadioButtonId();
            // 只还原当前输出模式、输出通道下的频率和斜率数据
            if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_fl_fr)) {
                nineDspHLPFSettings.resetCurrentChannel(NineConstantExtDsp.NINE_CHANNEL_FRONT_HIGH, NineConstantExtDsp.NINE_CHANNEL_FRONT_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_rl_rr)) {
                nineDspHLPFSettings.resetCurrentChannel(NineConstantExtDsp.NINE_CHANNEL_REAR_HIGH, NineConstantExtDsp.NINE_CHANNEL_REAR_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_center)) {
                nineDspHLPFSettings.resetCurrentChannel(NineConstantExtDsp.NINE_CHANNEL_CENTER_HIGH, NineConstantExtDsp.NINE_CHANNEL_CENTER_LOW);
            } else if (checkedId == SkinUtils.getId(R.id.rb_hlpf_mode_subwoofer)) {
                nineDspHLPFSettings.resetCurrentChannel(NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_HIGH, NineConstantExtDsp.NINE_CHANNEL_SUBWOOFER_LOW);
            }
        }
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == SkinUtils.getId(R.id.iv_h_arrow_delay)) {
            freqHighStepClick(false);
            return;
        }
        if (view.getId() == SkinUtils.getId(R.id.iv_h_arrow_plus)) {
            freqHighStepClick(true);
        } else if (view.getId() == SkinUtils.getId(R.id.iv_l_arrow_delay)) {
            freqLowStepClick(false);
        } else if (view.getId() == SkinUtils.getId(R.id.iv_l_arrow_plus)) {
            freqLowStepClick(true);
        }
    }

    @Override
    public boolean onLongClick(View view) {
        if (view.getId() == SkinUtils.getId(R.id.iv_h_arrow_delay) || view.getId() == SkinUtils.getId(R.id.iv_h_arrow_plus) || view.getId() == SkinUtils.getId(R.id.iv_l_arrow_delay) || view.getId() == SkinUtils.getId(R.id.iv_l_arrow_plus)) {
            currentBtn = (ImageView) mainView.findViewById(SkinUtils.getId(view.getId()));
            stopContinuousClick();
            startContinuousClick();
            Log.d(TAG, "onLongClick");
        }
        return false;
    }

    private void freqHighStepClick(boolean isPlus) {
        nineHLFView.clickHighFreq(isPlus);
    }

    private void freqLowStepClick(boolean isPlus) {
        nineHLFView.clickLowFreq(isPlus);
    }

    @Override
    public boolean onTouch(View view, MotionEvent motionEvent) {
        Log.d(TAG, "onTouch hcn.view id: " + view.getId());
        if (view.getId() == SkinUtils.getId(R.id.iv_h_arrow_delay) || view.getId() == SkinUtils.getId(R.id.iv_h_arrow_plus) || view.getId() == SkinUtils.getId(R.id.iv_l_arrow_delay) || view.getId() == SkinUtils.getId(R.id.iv_l_arrow_plus)) {
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
                Log.d(TAG, "clickRunnable + currentButton id: " + currentBtn.getId());
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