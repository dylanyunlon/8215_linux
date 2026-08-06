package com.hcn.autoeq.fragment.extdsp;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.ToggleButton;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.ExtDspAttenuateSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.ExtDspAttenuateSeekBar;

public class ExtDspAttenuateFragment extends BaseFragment
        implements SeekBar.OnSeekBarChangeListener, CompoundButton.OnCheckedChangeListener {

    private static final String TAG = ExtDspAttenuateFragment.class.getSimpleName();

    private View mainView;
    private ExtDspAttenuateSeekBar asbLF, asbRF, asbLR, asbRR, asbSubwoofer;
    private ToggleButton btnLinkLfRf, btnLinkLrRr;

    private ExtDspAttenuateSettings extDspAttenuateSettings;
    private boolean bLinkLfRf, bLinkLrRr; // 是否联动

    public ExtDspAttenuateFragment() {
    }

    public static ExtDspAttenuateFragment newInstance() {
        ExtDspAttenuateFragment fragment = new ExtDspAttenuateFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.extdsp_fragment_attenuate;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        extDspAttenuateSettings = ExtDspAttenuateSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        bLinkLfRf = extDspAttenuateSettings.getLinkLfRf();
        bLinkLrRr = extDspAttenuateSettings.getLinkLrRr();

        asbLF = mainView.findViewById(SkinUtils.getId(R.id.asb_lf));
        asbRF = mainView.findViewById(SkinUtils.getId(R.id.asb_rf));
        asbLR = mainView.findViewById(SkinUtils.getId(R.id.asb_lr));
        asbRR = mainView.findViewById(SkinUtils.getId(R.id.asb_rr));
        asbSubwoofer = mainView.findViewById(SkinUtils.getId(R.id.asb_subwoofer));

        asbLF.setOnSeekBarChangeListener(this);
        asbRF.setOnSeekBarChangeListener(this);
        asbLR.setOnSeekBarChangeListener(this);
        asbRR.setOnSeekBarChangeListener(this);
        asbSubwoofer.setOnSeekBarChangeListener(this);

        asbLF.setOnCheckedChangeListener(this);
        asbRF.setOnCheckedChangeListener(this);
        asbLR.setOnCheckedChangeListener(this);
        asbRR.setOnCheckedChangeListener(this);
        asbSubwoofer.setOnCheckedChangeListener(this);

        btnLinkLfRf = mainView.findViewById(SkinUtils.getId(R.id.btn_link_lf_rf));
        btnLinkLrRr = mainView.findViewById(SkinUtils.getId(R.id.btn_link_lr_rr));
        btnLinkLfRf.setOnCheckedChangeListener(this);
        btnLinkLrRr.setOnCheckedChangeListener(this);
    }

    @Override
    public void initData() {
        super.initData();
        initStatus(asbLF, asbRF, asbLR, asbRR, asbSubwoofer);
    }

    private void initStatus(ExtDspAttenuateSeekBar... extDspAttenuateSeekBars) {
        for (ExtDspAttenuateSeekBar extDspAttenuateSeekBar : extDspAttenuateSeekBars) {
            extDspAttenuateSeekBar.setProgress(extDspAttenuateSettings.getAttenuate((String) extDspAttenuateSeekBar.getTag()), false);
            extDspAttenuateSeekBar.setMuteStatus(extDspAttenuateSettings.getMute((String) extDspAttenuateSeekBar.getTag()));
            extDspAttenuateSeekBar.setInvertStatus(extDspAttenuateSettings.getInvert((String) extDspAttenuateSeekBar.getTag()));
        }

        btnLinkLfRf.setChecked(bLinkLfRf);
        btnLinkLrRr.setChecked(bLinkLrRr);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        Log.d(TAG, "onProgressChanged fromUser : " + fromUser);
        if (!fromUser) return; // 界面初始化时不需要执行，手动拖动才生效

        String channel = (String) seekBar.getTag();
        if (bLinkLfRf && ("LF".equals(channel) || "RF".equals(channel))) {
            linkSetProgress(progress, asbLF, asbRF);
            linkNativeAttenuate(asbLF, asbRF);
        } else if (bLinkLrRr && ("LR".equals(channel) || "RR".equals(channel))) {
            linkSetProgress(progress, asbLR, asbRR);
            linkNativeAttenuate(asbLR, asbRR);
        } else { // 非联动调节时，传入当前调节的控件
            ExtDspAttenuateSeekBar extDspAttenuateSeekBar = getExtDspAttenuateSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSubwoofer);
            linkNativeAttenuate(extDspAttenuateSeekBar);
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        String channel = (String) seekBar.getTag();
        Log.d(TAG, "onStopTrackingTouch channel : " + channel);
        if (bLinkLfRf && ("LF".equals(channel) || "RF".equals(channel))) {
            linkSaveAttenuate(asbLF, asbRF); // 联动调节时，传入多个控件
        } else if (bLinkLrRr && ("LR".equals(channel) || "RR".equals(channel))) {
            linkSaveAttenuate(asbLR, asbRR); // 联动调节时，传入多个控件
        } else { // 非联动调节时，传入当前调节的控件
            ExtDspAttenuateSeekBar extDspAttenuateSeekBar = getExtDspAttenuateSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSubwoofer);
            linkSaveAttenuate(extDspAttenuateSeekBar);
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (!buttonView.isPressed()) { // 界面初始化时不需要执行，手动点击才生效
            return;
        }

        int viewId = buttonView.getId();
        if (viewId == SkinUtils.getId(R.id.btn_link_lf_rf)) {
            Log.d(TAG, "btnLinkLfRf isChecked : " + isChecked);
            bLinkLfRf = isChecked;
            extDspAttenuateSettings.saveLink(bLinkLfRf, bLinkLrRr);
        } else if (viewId == SkinUtils.getId(R.id.btn_link_lr_rr)) {
            Log.d(TAG, "btnLinkLrRr isChecked : " + isChecked);
            bLinkLrRr = isChecked;
            extDspAttenuateSettings.saveLink(bLinkLfRf, bLinkLrRr);
        } else if (viewId == SkinUtils.getId(R.id.cb_mute)) {
            Log.d(TAG, "onCheckedChanged cb_mute isChecked ? " + isChecked);
            String channel = (String) buttonView.getTag();
            if (bLinkLfRf && ("LF".equals(channel) || "RF".equals(channel))) { // 联动调节时，传入多个控件
                linkSetMuteStatus(isChecked, asbLF, asbRF);
                linkSaveAttenuate(asbLF, asbRF);
                linkNativeAttenuate(asbLF, asbRF);
            } else if (bLinkLrRr && ("LR".equals(channel) || "RR".equals(channel))) { // 联动调节时，传入多个控件
                linkSetMuteStatus(isChecked, asbLR, asbRR);
                linkSaveAttenuate(asbLR, asbRR);
                linkNativeAttenuate(asbLR, asbRR);
            } else { // 非联动调节时，传入当前调节的控件
                ExtDspAttenuateSeekBar extDspAttenuateSeekBar = getExtDspAttenuateSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSubwoofer);
                linkSaveAttenuate(extDspAttenuateSeekBar);
                linkNativeAttenuate(extDspAttenuateSeekBar);
            }
        } else if (viewId == SkinUtils.getId(R.id.cb_invert)) {
            Log.d(TAG, "onCheckedChanged cb_invert isChecked ? " + isChecked);
            String channel = (String) buttonView.getTag();
            if (bLinkLfRf && ("LF".equals(channel) || "RF".equals(channel))) { // 联动调节时，传入多个控件
                linkSetInvertStatus(isChecked, asbLF, asbRF);
                linkSaveAttenuate(asbLF, asbRF);
                linkNativeAttenuate(asbLF, asbRF);
            } else if (bLinkLrRr && ("LR".equals(channel) || "RR".equals(channel))) { // 联动调节时，传入多个控件
                linkSetInvertStatus(isChecked, asbLR, asbRR);
                linkSaveAttenuate(asbLR, asbRR);
                linkNativeAttenuate(asbLR, asbRR);
            } else { // 非联动调节时，传入当前调节的控件
                ExtDspAttenuateSeekBar extDspAttenuateSeekBar = getExtDspAttenuateSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSubwoofer);
                linkSaveAttenuate(extDspAttenuateSeekBar);
                linkNativeAttenuate(extDspAttenuateSeekBar);
            }
        }
    }

    // 通过 channel 获取到对应的控件
    private ExtDspAttenuateSeekBar getExtDspAttenuateSeekBar(String tag, ExtDspAttenuateSeekBar... extDspAttenuateSeekBars) {
        for (ExtDspAttenuateSeekBar extDspAttenuateSeekBar : extDspAttenuateSeekBars) {
            if (tag.equals(extDspAttenuateSeekBar.getTag())) {
                return extDspAttenuateSeekBar;
            }
        }
        return null;
    }

    // 联动调节时，同时保存数据
    private void linkSaveAttenuate(ExtDspAttenuateSeekBar... extDspAttenuateSeekBars) {
        for (ExtDspAttenuateSeekBar extDspAttenuateSeekBar : extDspAttenuateSeekBars) {
            String channel = (String) extDspAttenuateSeekBar.getTag();
            int progress = extDspAttenuateSeekBar.getProgress();
            boolean mute = extDspAttenuateSeekBar.getAttenuateStatus();
            boolean invert = extDspAttenuateSeekBar.getInvertStatus();
            extDspAttenuateSettings.saveAttenuate(channel, progress, mute, invert);
        }
    }

    // 联动调节时，同时设置数据到底层
    private void linkNativeAttenuate(ExtDspAttenuateSeekBar... extDspAttenuateSeekBars) {
        for (ExtDspAttenuateSeekBar extDspAttenuateSeekBar : extDspAttenuateSeekBars) {
            String channel = (String) extDspAttenuateSeekBar.getTag();
            int progress = extDspAttenuateSeekBar.getProgress();
            boolean mute = extDspAttenuateSeekBar.getAttenuateStatus();
            boolean invert = extDspAttenuateSeekBar.getInvertStatus();
            extDspAttenuateSettings.nativeAttenuate(channel, progress, mute, invert);
        }
    }

    // 联动设置进度条进度
    private void linkSetProgress(int progress, ExtDspAttenuateSeekBar... extDspAttenuateSeekBars) {
        for (ExtDspAttenuateSeekBar extDspAttenuateSeekBar : extDspAttenuateSeekBars) {
            extDspAttenuateSeekBar.setProgress(progress, false);
        }
    }

    // 联动设置静音是否选中
    private void linkSetMuteStatus(boolean status, ExtDspAttenuateSeekBar... extDspAttenuateSeekBars) {
        for (ExtDspAttenuateSeekBar extDspAttenuateSeekBar : extDspAttenuateSeekBars) {
            extDspAttenuateSeekBar.setMuteStatus(status);
        }
    }

    // 联动设置反相是否选中
    private void linkSetInvertStatus(boolean status, ExtDspAttenuateSeekBar... extDspAttenuateSeekBars) {
        for (ExtDspAttenuateSeekBar extDspAttenuateSeekBar : extDspAttenuateSeekBars) {
            extDspAttenuateSeekBar.setInvertStatus(status);
        }
    }
}