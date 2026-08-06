package com.hcn.autoeq.fragment.fydsp;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import com.blankj.utilcode.util.ToastUtils;
import com.hcn.autoeq.R;
import com.hcn.autoeq.adapter.FyDspFreqAdapter;
import com.hcn.autoeq.adapter.FyDspSlopeAdapter;
import com.hcn.autoeq.bean.EventMessage;
import com.hcn.autoeq.bean.FyDspHLPFFreq;
import com.hcn.autoeq.bean.FyDspHLPFSlope;
import com.hcn.autoeq.bean.FyDspOutputChannel;
import com.hcn.autoeq.bean.FyDspOutputMode;
import com.hcn.autoeq.data.FyDspHLPFSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.view.CustomSpinner;
import com.hcn.autoeq.view.rca.RCALineView;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.util.List;
import java.util.stream.IntStream;

public class FyDspHLPFFragment extends BaseFragment
        implements View.OnClickListener, AdapterView.OnItemSelectedListener, CustomSpinner.OnSpinnerEventsListener {

    private static final String TAG = FyDspHLPFFragment.class.getSimpleName();

    private View mainView;
    private RCALineView rcaLineView;

    private CustomSpinner spHPFFreq, spLPFFreq;
    private CustomSpinner spHPFSlope, spLPFSlope;

    private RadioGroup rgSpeakerWay2, rgSpeakerWay3, rgSpeakerChannel51, rgSpeakerWay6;
    private RadioGroup rgOutputMode;
    private Button btnReset;

    private FyDspHLPFSettings fyDspHLPFSettings;
    private FyDspOutputMode fyDspOutputMode;
    private FyDspOutputChannel fyDspOutputChannel;

    // 记录是不是手动选择列表项
    private boolean spHPFFreqFromUser, spLPFFreqFromUser, spHPFSlopeFromUser, spLPFSlopeFromUser;

    public FyDspHLPFFragment() {
    }

    public static FyDspHLPFFragment newInstance() {
        FyDspHLPFFragment fragment = new FyDspHLPFFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fydsp_fragment_hlpf;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        fyDspHLPFSettings = FyDspHLPFSettings.getInstance(mContext);
        return mainView;
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public void initView() {
        rcaLineView = mainView.findViewById(R.id.rca_line);

        spHPFFreq = mainView.findViewById(R.id.sp_hpf_freq);
        spLPFFreq = mainView.findViewById(R.id.sp_lpf_freq);
        spHPFFreq.setOnItemSelectedListener(this);
        spLPFFreq.setOnItemSelectedListener(this);
        spHPFFreq.setSpinnerEventsListener(this);
        spLPFFreq.setSpinnerEventsListener(this);
        spHPFFreq.setOnTouchListener((v, event) -> {
            spHPFFreqFromUser = true;
            return false;
        });
        spLPFFreq.setOnTouchListener((v, event) -> {
            spLPFFreqFromUser = true;
            return false;
        });

        spHPFSlope = mainView.findViewById(R.id.sp_hpf_slope);
        spLPFSlope = mainView.findViewById(R.id.sp_lpf_slope);
        spHPFSlope.setOnItemSelectedListener(this);
        spLPFSlope.setOnItemSelectedListener(this);
        spHPFSlope.setSpinnerEventsListener(this);
        spLPFSlope.setSpinnerEventsListener(this);
        spHPFSlope.setOnTouchListener((v, event) -> {
            spHPFSlopeFromUser = true;
            return false;
        });
        spLPFSlope.setOnTouchListener((v, event) -> {
            spLPFSlopeFromUser = true;
            return false;
        });

        rgSpeakerWay2 = mainView.findViewById(R.id.rg_hlpf_speaker_way2);
        rgSpeakerWay3 = mainView.findViewById(R.id.rg_hlpf_speaker_way3);
        rgSpeakerChannel51 = mainView.findViewById(R.id.rg_hlpf_speaker_channel51);
        rgSpeakerWay6 = mainView.findViewById(R.id.rg_hlpf_speaker_way6);
        rgSpeakerWay2.setOnCheckedChangeListener(this::speakerCheckedChanged);
        rgSpeakerWay3.setOnCheckedChangeListener(this::speakerCheckedChanged);
        rgSpeakerChannel51.setOnCheckedChangeListener(this::speakerCheckedChanged);
        rgSpeakerWay6.setOnCheckedChangeListener(this::speakerCheckedChanged);

        rgOutputMode = mainView.findViewById(R.id.rg_hlpf_output_mode);
        rgOutputMode.setOnCheckedChangeListener(this::outputModeCheckedChanged);

        btnReset = mainView.findViewById(R.id.btn_reset_hlpf);
        btnReset.setOnClickListener(this);
    }

    @Override
    public void initData() {
        super.initData();
        fyDspOutputMode = fyDspHLPFSettings.getOutputMode();
        fyDspOutputChannel = fyDspHLPFSettings.getOutputChannel(fyDspOutputMode);

        refreshOutputMode();
        refreshSpeaker();
        refreshSpFreq();
        refreshSpSlope();
        refreshRCALineView();
    }

    private void refreshSpFreq() {
        Log.d(TAG, "refreshSpeaker");
        List<FyDspHLPFFreq> freqList = fyDspHLPFSettings.getFreqList(fyDspOutputMode, fyDspOutputChannel);
        int hpfFreq = fyDspHLPFSettings.getHPFFreq(fyDspOutputMode, fyDspOutputChannel);
        int lpfFreq = fyDspHLPFSettings.getLPFFreq(fyDspOutputMode, fyDspOutputChannel);

        spHPFFreq.setAdapter(new FyDspFreqAdapter(mContext, freqList, spHPFFreq));
        spLPFFreq.setAdapter(new FyDspFreqAdapter(mContext, freqList, spLPFFreq));

        int hpfFreqIndex = IntStream.range(0, freqList.size()).filter(i -> hpfFreq == freqList.get(i).getFreq()).findFirst().orElse(0);
        int lpfFreqIndex = IntStream.range(0, freqList.size()).filter(i -> lpfFreq == freqList.get(i).getFreq()).findFirst().orElse(0);
        spHPFFreq.setSelection(hpfFreqIndex);
        spLPFFreq.setSelection(lpfFreqIndex);
    }

    private void refreshSpSlope() {
        Log.d(TAG, "refreshSpeaker");
        List<FyDspHLPFSlope> slopeList = fyDspHLPFSettings.getSlopeList();
        int hpfSlope = fyDspHLPFSettings.getHPFSlope(fyDspOutputMode, fyDspOutputChannel);
        int lpfSlope = fyDspHLPFSettings.getLPFSlope(fyDspOutputMode, fyDspOutputChannel);

        spHPFSlope.setAdapter(new FyDspSlopeAdapter(mContext, fyDspHLPFSettings.getSlopeList(), spHPFSlope));
        spLPFSlope.setAdapter(new FyDspSlopeAdapter(mContext, fyDspHLPFSettings.getSlopeList(), spLPFSlope));

        int hpfSlopeIndex = IntStream.range(0, slopeList.size()).filter(i -> hpfSlope == slopeList.get(i).getSlope()).findFirst().orElse(0);
        int lpfSlopeIndex = IntStream.range(0, slopeList.size()).filter(i -> lpfSlope == slopeList.get(i).getSlope()).findFirst().orElse(0);
        spHPFSlope.setSelection(hpfSlopeIndex);
        spLPFSlope.setSelection(lpfSlopeIndex);
    }

    private void refreshSpeaker() {
        Log.d(TAG, "refreshSpeaker");
        rgSpeakerWay2.setVisibility(fyDspOutputMode == FyDspOutputMode.WAY2 ? View.VISIBLE : View.GONE);
        rgSpeakerWay3.setVisibility(fyDspOutputMode == FyDspOutputMode.WAY3 ? View.VISIBLE : View.GONE);
        rgSpeakerChannel51.setVisibility(fyDspOutputMode == FyDspOutputMode.CHANNEL51 ? View.VISIBLE : View.GONE);
        rgSpeakerWay6.setVisibility(fyDspOutputMode == FyDspOutputMode.WAY6 ? View.VISIBLE : View.GONE);

        if (rgSpeakerWay2.getVisibility() == View.VISIBLE) { // 可见情况下才执行
            for (int i = 0; i < rgSpeakerWay2.getChildCount(); i++) {
                RadioButton radioButton = (RadioButton) rgSpeakerWay2.getChildAt(i);
                if (fyDspOutputChannel.name().equals(radioButton.getTag())) {
                    rgSpeakerWay2.check(radioButton.getId());
                    break;
                }
            }
        }
        if (rgSpeakerWay3.getVisibility() == View.VISIBLE) { // 可见情况下才执行
            for (int i = 0; i < rgSpeakerWay3.getChildCount(); i++) {
                RadioButton radioButton = (RadioButton) rgSpeakerWay3.getChildAt(i);
                if (fyDspOutputChannel.name().equals(radioButton.getTag())) {
                    rgSpeakerWay3.check(radioButton.getId());
                    break;
                }
            }
        }
        if (rgSpeakerChannel51.getVisibility() == View.VISIBLE) { // 可见情况下才执行
            for (int i = 0; i < rgSpeakerChannel51.getChildCount(); i++) {
                RadioButton radioButton = (RadioButton) rgSpeakerChannel51.getChildAt(i);
                if (fyDspOutputChannel.name().equals(radioButton.getTag())) {
                    rgSpeakerChannel51.check(radioButton.getId());
                    break;
                }
            }
        }
        if (rgSpeakerWay6.getVisibility() == View.VISIBLE) { // 可见情况下才执行
            for (int i = 0; i < rgSpeakerWay6.getChildCount(); i++) {
                RadioButton radioButton = (RadioButton) rgSpeakerWay6.getChildAt(i);
                if (fyDspOutputChannel.name().equals(radioButton.getTag())) {
                    rgSpeakerWay6.check(radioButton.getId());
                    break;
                }
            }
        }
    }

    private void refreshOutputMode() {
        for (int i = 0; i < rgOutputMode.getChildCount(); i++) {
            RadioButton radioButton = (RadioButton) rgOutputMode.getChildAt(i);
            if (fyDspOutputMode.name().equals(radioButton.getTag())) {
                rgOutputMode.check(radioButton.getId());
                break;
            }
        }
    }

    private void refreshRCALineView() {
        if (fyDspOutputMode == FyDspOutputMode.WAY6) {
            initSingleLine(RCALineView.RCA_LR_1, FyDspOutputChannel.WAY6_F);
            initSingleLine(RCALineView.RCA_LR_2, FyDspOutputChannel.WAY6_R);
            initSingleLine(RCALineView.RCA_LR_3, FyDspOutputChannel.WAY6_SUBWOOFER_CENTER);
            rcaLineView.getRCAPositionData(RCALineView.RCA_LR_0).setHide(true);
        } else if (fyDspOutputMode == FyDspOutputMode.CHANNEL51) {
            initSingleLine(RCALineView.RCA_LR_1, FyDspOutputChannel.CHANNEL51_F);
            initSingleLine(RCALineView.RCA_LR_2, FyDspOutputChannel.CHANNEL51_R);
            initSingleLine(RCALineView.RCA_LR_0, FyDspOutputChannel.CHANNEL51_CENTER);
            initSingleLine(RCALineView.RCA_LR_3, FyDspOutputChannel.CHANNEL51_SUBWOOFER);
            rcaLineView.getRCAPositionData(RCALineView.RCA_LR_0).setHide(false);
        } else if (fyDspOutputMode == FyDspOutputMode.WAY2) {
            initSingleLine(RCALineView.RCA_LR_1, FyDspOutputChannel.WAY2_F);
            initSingleLine(RCALineView.RCA_LR_2, FyDspOutputChannel.WAY2_R);
            initSingleLine(RCALineView.RCA_LR_3, FyDspOutputChannel.WAY2_SUBWOOFER_CENTER);
            rcaLineView.getRCAPositionData(RCALineView.RCA_LR_0).setHide(true);
        } else if (fyDspOutputMode == FyDspOutputMode.WAY3) {
            initSingleLine(RCALineView.RCA_LR_1, FyDspOutputChannel.WAY3_F);
            initSingleLine(RCALineView.RCA_LR_2, FyDspOutputChannel.WAY3_R);
            initSingleLine(RCALineView.RCA_LR_3, FyDspOutputChannel.WAY3_SUBWOOFER_CENTER);
            rcaLineView.getRCAPositionData(RCALineView.RCA_LR_0).setHide(true);
        }
    }

    private void initSingleLine(int idRCA, FyDspOutputChannel fyDspOutputChannel) {
        int hpfFreq = fyDspHLPFSettings.getHPFFreq(fyDspOutputMode, fyDspOutputChannel);
        int lpfFreq = fyDspHLPFSettings.getLPFFreq(fyDspOutputMode, fyDspOutputChannel);

        int hpfSlope = fyDspHLPFSettings.getHPFSlope(fyDspOutputMode, fyDspOutputChannel);
        int lpfSlope = fyDspHLPFSettings.getLPFSlope(fyDspOutputMode, fyDspOutputChannel);

        rcaLineView.getRCAPositionData(idRCA).setColorID(FyDspOutputChannel.getLineColor(fyDspOutputChannel));
        rcaLineView.setHighRate(idRCA, hpfFreq);
        rcaLineView.setLowRate(idRCA, lpfFreq);
        rcaLineView.setHighSlope(idRCA, hpfSlope);
        rcaLineView.setLowSlope(idRCA, lpfSlope);

        // 当前选中的喇叭对应的线条显示在最上层
        FyDspOutputChannel currentFyDspOutputChannel = fyDspHLPFSettings.getOutputChannel(fyDspOutputMode);
        if (currentFyDspOutputChannel == fyDspOutputChannel) {
            rcaLineView.setCurrentRCA(idRCA);
        }

        rcaLineView.invalidate();
    }

    private void speakerCheckedChanged(RadioGroup radioGroup, int checkedId) {
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

        Log.d(TAG, "speakerCheckedChanged checkedId : " + checkedId);

        String tag = (String) radioGroup.findViewById(radioGroup.getCheckedRadioButtonId()).getTag();
        fyDspOutputChannel = FyDspOutputChannel.valueOf(tag);
        fyDspHLPFSettings.saveOutputChannel(fyDspOutputChannel, fyDspOutputMode);

        EventMessage.anyChanged(mContext, TAG + "_" + "speakerCheckedChanged");
        refreshSpFreq();
        refreshSpSlope();
        refreshRCALineView();

        nativeHLPF(); // 设置音效
    }

    private void outputModeCheckedChanged(RadioGroup radioGroup, int checkedId) {
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

        Log.d(TAG, "outputModeCheckedChanged");
        String tag = (String) radioGroup.findViewById(radioGroup.getCheckedRadioButtonId()).getTag();
        fyDspOutputMode = FyDspOutputMode.valueOf(tag);
        fyDspOutputChannel = fyDspHLPFSettings.getOutputChannel(fyDspOutputMode);

        fyDspHLPFSettings.saveOutputMode(fyDspOutputMode);

        EventMessage.anyChanged(mContext, TAG + "_" + "outputModeCheckedChanged");
        refreshSpeaker();
        refreshSpFreq();
        refreshSpSlope();
        refreshRCALineView();

        fyDspHLPFSettings.nativeHLPFAllChannel(fyDspOutputMode); // 设置某输出模式下的所有通道

        EventBus.getDefault().postSticky(new EventMessage(EventMessage.MSG_STICKY_OUTPUT_MODE_CHANGED, fyDspOutputMode));
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_reset_hlpf:
                // 只还原当前输出模式、输出通道下的频率和斜率数据
                fyDspHLPFSettings.resetFreq(fyDspOutputMode, fyDspOutputChannel);
                fyDspHLPFSettings.resetSlope(fyDspOutputMode, fyDspOutputChannel);

                EventMessage.anyChanged(mContext, TAG + "_" + "btn_reset_hlpf");
                refreshSpFreq();
                refreshSpSlope();
                refreshRCALineView();

                nativeHLPF(); // 设置音效
                break;
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        if (spHPFFreqFromUser || spLPFFreqFromUser || spHPFSlopeFromUser || spLPFSlopeFromUser) {
            spHPFFreqFromUser = spLPFFreqFromUser = spHPFSlopeFromUser = spLPFSlopeFromUser = false;

            EventMessage.anyChanged(mContext, TAG + "_" + "onItemSelected");

            FyDspHLPFFreq fyDspHPFFreq = (FyDspHLPFFreq) spHPFFreq.getSelectedItem();
            FyDspHLPFFreq fyDspLPFFreq = (FyDspHLPFFreq) spLPFFreq.getSelectedItem();
            FyDspHLPFSlope fyDspHPFSlope = (FyDspHLPFSlope) spHPFSlope.getSelectedItem();
            FyDspHLPFSlope fyDspLPFSlope = (FyDspHLPFSlope) spLPFSlope.getSelectedItem();

            // 检查频率选择，HPF的值不能大于LPF
            // 如果选择错误，则让 Spinner 控件重新选择上次保存的值
            if (parent.getId() == R.id.sp_hpf_freq || parent.getId() == R.id.sp_lpf_freq) {
                Log.d(TAG, String.format("onItemSelected hpf freq : %d, lpf freq : %d", fyDspHPFFreq.getFreq(), fyDspLPFFreq.getFreq()));
                if (fyDspHPFFreq.getFreq() > fyDspLPFFreq.getFreq()) {
                    final List<FyDspHLPFFreq> freqList = fyDspHLPFSettings.getFreqList(fyDspOutputMode, fyDspOutputChannel);
                    if (parent.getId() == R.id.sp_hpf_freq) {
                        final int hpfFreq = fyDspHLPFSettings.getHPFFreq(fyDspOutputMode, fyDspOutputChannel);
                        int hpfFreqIndex = IntStream.range(0, freqList.size()).filter(i -> hpfFreq == freqList.get(i).getFreq()).findFirst().orElse(0);
                        spHPFFreq.setSelection(hpfFreqIndex);
                    } else {
                        final int lpfFreq = fyDspHLPFSettings.getLPFFreq(fyDspOutputMode, fyDspOutputChannel);
                        int hpfFreqIndex = IntStream.range(0, freqList.size()).filter(i -> lpfFreq == freqList.get(i).getFreq()).findFirst().orElse(0);
                        spLPFFreq.setSelection(hpfFreqIndex);
                    }
                    ToastUtils.showShort(R.string.fydsp_hlpf_freq_selection_error_msg);
                    return;
                }
            }

            switch (parent.getId()) {
                case R.id.sp_hpf_freq:
                case R.id.sp_lpf_freq: {
                    fyDspHLPFSettings.saveFreq(fyDspOutputMode, fyDspOutputChannel, fyDspHPFFreq, fyDspLPFFreq);
                    break;
                }
                case R.id.sp_hpf_slope:
                case R.id.sp_lpf_slope: {
                    fyDspHLPFSettings.saveSlope(fyDspOutputMode, fyDspOutputChannel, fyDspHPFSlope, fyDspLPFSlope);
                    break;
                }
            }
            refreshRCALineView();

            nativeHLPF(); // 设置音效
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {

    }

    @Override
    public void onSpinnerOpened(int lastSelectedItemPosition, int currentSelectedItemPosition) {
    }

    @Override
    public void onSpinnerClosed(int lastSelectedItemPosition, int currentSelectedItemPosition) {
    }

    private void nativeHLPF() {
        FyDspHLPFFreq fyDspHPFFreq = (FyDspHLPFFreq) spHPFFreq.getSelectedItem();
        FyDspHLPFFreq fyDspLPFFreq = (FyDspHLPFFreq) spLPFFreq.getSelectedItem();
        FyDspHLPFSlope fyDspHPFSlope = (FyDspHLPFSlope) spHPFSlope.getSelectedItem();
        FyDspHLPFSlope fyDspLPFSlope = (FyDspHLPFSlope) spLPFSlope.getSelectedItem();
        fyDspHLPFSettings.nativeHLPF(fyDspOutputMode, fyDspOutputChannel, fyDspHPFFreq, fyDspLPFFreq, fyDspHPFSlope, fyDspLPFSlope);
    }

    @Override
    @Subscribe(sticky = true)
    public void onEvent(EventMessage eventMessage) {
        super.onEvent(eventMessage);
        if (EventMessage.MSG_STICKY_USER_MODE_CHANGED.equals(eventMessage.getMessage())) {
            Log.d(TAG, "MSG_STICKY_USER_MODE_CHANGED");
            initData();
        }
    }
}
