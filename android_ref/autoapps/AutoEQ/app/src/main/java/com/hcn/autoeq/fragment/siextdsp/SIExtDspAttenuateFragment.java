package com.hcn.autoeq.fragment.siextdsp;

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
import com.hcn.autoeq.data.SIExtDspAttenuateSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.ExtDspAttenuateSeekBar;

/**
 * 和普通extdsp的UI相似，但功能的实现上是不同的，存在一部分区别，
 * 所以应该另外创建setting以及Fragment，方便以后的扩展和修改；
 */
public class SIExtDspAttenuateFragment extends BaseFragment
        implements SeekBar.OnSeekBarChangeListener, CompoundButton.OnCheckedChangeListener {

    private static final String TAG = SIExtDspAttenuateFragment.class.getSimpleName();

    private View mainView;
    private ExtDspAttenuateSeekBar asbLF, asbRF, asbLR, asbRR, asbSubwoofer, asbCenter;
    private ToggleButton btnLinkLfRf, btnLinkLrRr;

    private SIExtDspAttenuateSettings siExtDspAttenuateSettings;
    private boolean bLinkLfRf, bLinkLrRr; // 是否联动

    public SIExtDspAttenuateFragment() {
    }

    public static SIExtDspAttenuateFragment newInstance() {
        SIExtDspAttenuateFragment fragment = new SIExtDspAttenuateFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.siextdsp_fragment_attenuate;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        siExtDspAttenuateSettings = SIExtDspAttenuateSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        bLinkLfRf = siExtDspAttenuateSettings.getLinkLfRf();
        bLinkLrRr = siExtDspAttenuateSettings.getLinkLrRr();

        asbLF = mainView.findViewById(SkinUtils.getId(R.id.asb_lf));
        asbRF = mainView.findViewById(SkinUtils.getId(R.id.asb_rf));
        asbLR = mainView.findViewById(SkinUtils.getId(R.id.asb_lr));
        asbRR = mainView.findViewById(SkinUtils.getId(R.id.asb_rr));
        asbSubwoofer = mainView.findViewById(SkinUtils.getId(R.id.asb_subwoofer));
        asbCenter = mainView.findViewById(SkinUtils.getId(R.id.asb_centcer));

        asbLF.setOnSeekBarChangeListener(this);
        asbRF.setOnSeekBarChangeListener(this);
        asbLR.setOnSeekBarChangeListener(this);
        asbRR.setOnSeekBarChangeListener(this);
        asbSubwoofer.setOnSeekBarChangeListener(this);
        if (asbCenter != null) {
            asbCenter.setOnSeekBarChangeListener(this);
        }

        asbLF.setOnCheckedChangeListener(this);
        asbRF.setOnCheckedChangeListener(this);
        asbLR.setOnCheckedChangeListener(this);
        asbRR.setOnCheckedChangeListener(this);
        asbSubwoofer.setOnCheckedChangeListener(this);
        if (asbCenter != null) {
            asbCenter.setOnCheckedChangeListener(this);
        }


        btnLinkLfRf = mainView.findViewById(SkinUtils.getId(R.id.btn_link_lf_rf));
        btnLinkLrRr = mainView.findViewById(SkinUtils.getId(R.id.btn_link_lr_rr));
        btnLinkLfRf.setOnCheckedChangeListener(this);
        btnLinkLrRr.setOnCheckedChangeListener(this);

    }

    @Override
    public void initData() {
        super.initData();
        initStatus(asbLF, asbRF, asbLR, asbRR, asbSubwoofer, asbCenter);
    }

    private void initStatus(ExtDspAttenuateSeekBar... extDspAttenuateSeekBars) {
        for (ExtDspAttenuateSeekBar extDspAttenuateSeekBar : extDspAttenuateSeekBars) {
            if (extDspAttenuateSeekBar==null){
                continue;
            }
            extDspAttenuateSeekBar.setProgress(siExtDspAttenuateSettings.getAttenuate((String) extDspAttenuateSeekBar.getTag()), false);
            extDspAttenuateSeekBar.setMuteStatus(siExtDspAttenuateSettings.getMute((String) extDspAttenuateSeekBar.getTag()));
            extDspAttenuateSeekBar.setInvertStatus(siExtDspAttenuateSettings.getInvert((String) extDspAttenuateSeekBar.getTag()));
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
            ExtDspAttenuateSeekBar extDspAttenuateSeekBar = getExtDspAttenuateSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSubwoofer, asbCenter);
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
            ExtDspAttenuateSeekBar extDspAttenuateSeekBar = getExtDspAttenuateSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSubwoofer, asbCenter);
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
            siExtDspAttenuateSettings.saveLink(bLinkLfRf, bLinkLrRr);
        } else if (viewId == SkinUtils.getId(R.id.btn_link_lr_rr)) {
            Log.d(TAG, "btnLinkLrRr isChecked : " + isChecked);
            bLinkLrRr = isChecked;
            siExtDspAttenuateSettings.saveLink(bLinkLfRf, bLinkLrRr);
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
                ExtDspAttenuateSeekBar extDspAttenuateSeekBar = getExtDspAttenuateSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSubwoofer, asbCenter);
                linkSaveAttenuate(extDspAttenuateSeekBar);
                linkNativeAttenuate(extDspAttenuateSeekBar);
            }
        } else if (viewId == SkinUtils.getId(R.id.cb_invert)) {
            Log.d(TAG, "onCheckedChanged cb_invert isChecked ? " + isChecked);
            String channel = (String) buttonView.getTag();
            if (bLinkLfRf && ("LF".equals(channel) || "RF".equals(channel))) { // 联动调节时，传入多个控件
                linkSetInvertStatus(isChecked, asbLF, asbRF);
                linkSaveAttenuate(asbLF, asbRF);
                linkNativeAttenuate(asbLF, asbRF, true);
            } else if (bLinkLrRr && ("LR".equals(channel) || "RR".equals(channel))) { // 联动调节时，传入多个控件
                linkSetInvertStatus(isChecked, asbLR, asbRR);
                linkSaveAttenuate(asbLR, asbRR);
                linkNativeAttenuate(asbLR, asbRR, true);
            } else { // 非联动调节时，传入当前调节的控件
                ExtDspAttenuateSeekBar extDspAttenuateSeekBar = getExtDspAttenuateSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSubwoofer, asbCenter);
                linkSaveAttenuate(extDspAttenuateSeekBar);
                linkNativeAttenuate(extDspAttenuateSeekBar, true);
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
            boolean isInvert = extDspAttenuateSeekBar.getInvertStatus();
            siExtDspAttenuateSettings.saveAttenuateSI(channel, progress, mute, isInvert);
        }
    }

    // 联动调节时，同时设置数据到底层
    private void linkNativeAttenuate(ExtDspAttenuateSeekBar... extDspAttenuateSeekBars) {
        for (ExtDspAttenuateSeekBar extDspAttenuateSeekBar : extDspAttenuateSeekBars) {
            String channel = (String) extDspAttenuateSeekBar.getTag();
            int progress = extDspAttenuateSeekBar.getProgress();
            boolean mute = extDspAttenuateSeekBar.getAttenuateStatus();
            boolean isInvert = extDspAttenuateSeekBar.getInvertStatus();
            siExtDspAttenuateSettings.nativeAttenuateSI(channel, progress, mute, isInvert);
        }
    }

    private void linkNativeAttenuate(ExtDspAttenuateSeekBar extDspAttenuateSeekBar, boolean change) {
        String channel = (String) extDspAttenuateSeekBar.getTag();
        int progress = extDspAttenuateSeekBar.getProgress();
        boolean mute = extDspAttenuateSeekBar.getAttenuateStatus();
        boolean isInvert = extDspAttenuateSeekBar.getInvertStatus();
        siExtDspAttenuateSettings.nativeAttenuateSI(channel, progress, mute, isInvert, change);
    }

    private void linkNativeAttenuate(ExtDspAttenuateSeekBar extDspAttenuateSeekBarL, ExtDspAttenuateSeekBar extDspAttenuateSeekBarR, boolean change) {
        String channel = (String) extDspAttenuateSeekBarL.getTag();
        int progress = extDspAttenuateSeekBarL.getProgress();
        boolean mute = extDspAttenuateSeekBarL.getAttenuateStatus();
        boolean isInvert = extDspAttenuateSeekBarL.getInvertStatus();
        siExtDspAttenuateSettings.nativeAttenuateSI(channel, progress, mute, isInvert, change);


        channel = (String) extDspAttenuateSeekBarR.getTag();
        progress = extDspAttenuateSeekBarR.getProgress();
        mute = extDspAttenuateSeekBarR.getAttenuateStatus();
        isInvert = extDspAttenuateSeekBarR.getInvertStatus();
        siExtDspAttenuateSettings.nativeAttenuateSI(channel, progress, mute, isInvert, change);
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