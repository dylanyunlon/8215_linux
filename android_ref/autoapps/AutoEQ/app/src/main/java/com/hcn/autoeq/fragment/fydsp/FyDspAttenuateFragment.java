package com.hcn.autoeq.fragment.fydsp;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.ToggleButton;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.EventMessage;
import com.hcn.autoeq.bean.FyDspOutputMode;
import com.hcn.autoeq.data.FyDspAttenuateSettings;
import com.hcn.autoeq.data.FyDspHLPFSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.view.FyDspAttenuateSeekBar;

import org.greenrobot.eventbus.Subscribe;

import java.util.stream.Stream;

public class FyDspAttenuateFragment extends BaseFragment
        implements SeekBar.OnSeekBarChangeListener, CompoundButton.OnCheckedChangeListener, View.OnClickListener {

    private static final String TAG = FyDspAttenuateFragment.class.getSimpleName();

    private View mainView;
    private FyDspAttenuateSeekBar asbLF, asbRF, asbLR, asbRR, asbCenter, asbSubwoofer;
    private ToggleButton btnLinkLfRf, btnLinkLrRr, btnLinkCenterSubwoofer;
    private ImageView ivLinkLineCenterSubwoofer;
    private Button btnResetAttenuate;

    private boolean bLinkLfRf, bLinkLrRr, bLinkCenterSubwoofer; // 是否联动
    private FyDspAttenuateSettings fyDspAttenuateSettings;
    private FyDspHLPFSettings fyDspHLPFSettings;
    private FyDspOutputMode fyDspOutputMode;

    public FyDspAttenuateFragment() {
    }

    public static FyDspAttenuateFragment newInstance() {
        FyDspAttenuateFragment fragment = new FyDspAttenuateFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fydsp_fragment_attenuate;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        fyDspAttenuateSettings = FyDspAttenuateSettings.getInstance(mContext);
        fyDspHLPFSettings = FyDspHLPFSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        asbLF = mainView.findViewById(R.id.asb_lf);
        asbRF = mainView.findViewById(R.id.asb_rf);
        asbLR = mainView.findViewById(R.id.asb_lr);
        asbRR = mainView.findViewById(R.id.asb_rr);
        asbCenter = mainView.findViewById(R.id.asb_center);
        asbSubwoofer = mainView.findViewById(R.id.asb_subwoofer);

        asbLF.setOnSeekBarChangeListener(this);
        asbRF.setOnSeekBarChangeListener(this);
        asbLR.setOnSeekBarChangeListener(this);
        asbRR.setOnSeekBarChangeListener(this);
        asbCenter.setOnSeekBarChangeListener(this);
        asbSubwoofer.setOnSeekBarChangeListener(this);

        asbLF.setOnCheckedChangeListener(this);
        asbRF.setOnCheckedChangeListener(this);
        asbLR.setOnCheckedChangeListener(this);
        asbRR.setOnCheckedChangeListener(this);
        asbCenter.setOnCheckedChangeListener(this);
        asbSubwoofer.setOnCheckedChangeListener(this);

        btnLinkLfRf = mainView.findViewById(R.id.btn_link_lf_rf);
        btnLinkLrRr = mainView.findViewById(R.id.btn_link_lr_rr);
        btnLinkCenterSubwoofer = mainView.findViewById(R.id.btn_link_center_subwoofer);
        btnLinkLfRf.setOnCheckedChangeListener(this);
        btnLinkLrRr.setOnCheckedChangeListener(this);
        btnLinkCenterSubwoofer.setOnCheckedChangeListener(this);

        ivLinkLineCenterSubwoofer = mainView.findViewById(R.id.iv_link_line_center_subwoofer);

        btnResetAttenuate = mainView.findViewById(R.id.btn_reset_attenuate);
        btnResetAttenuate.setOnClickListener(this);
    }

    @Override
    public void initData() {
        super.initData();
        fyDspOutputMode = fyDspHLPFSettings.getOutputMode();
        initSeekBar();
        refreshSeekbarTitle();
        refreshLinkStatus();
    }

    private void initSeekBar() {
        Stream.of(asbLF, asbRF, asbLR, asbRR, asbCenter, asbSubwoofer).forEach(fyDspAttenuateSeekBar -> {
            String tag = (String) fyDspAttenuateSeekBar.getTag();
            fyDspAttenuateSeekBar.setProgress(fyDspAttenuateSettings.getAttenuate(tag), false);
            fyDspAttenuateSeekBar.setMuteStatus(fyDspAttenuateSettings.getMute(tag));
            fyDspAttenuateSeekBar.setInvertStatus(fyDspAttenuateSettings.getInvert(tag));
        });
    }

    private void refreshSeekbarTitle() {
        switch (fyDspOutputMode) {
            case WAY2:
                asbLF.setTitle(getString(R.string.fydsp_attenuate_way2_lf));
                asbRF.setTitle(getString(R.string.fydsp_attenuate_way2_rf));
                asbLR.setTitle(getString(R.string.fydsp_attenuate_way2_lr));
                asbRR.setTitle(getString(R.string.fydsp_attenuate_way2_rr));
                asbCenter.setTitle(getString(R.string.fydsp_attenuate_way2_subwoofer));
                asbSubwoofer.setTitle(getString(R.string.fydsp_attenuate_way2_center));
                break;
            case WAY3:
                asbLF.setTitle(getString(R.string.fydsp_attenuate_way3_lf));
                asbRF.setTitle(getString(R.string.fydsp_attenuate_way3_rf));
                asbLR.setTitle(getString(R.string.fydsp_attenuate_way3_lr));
                asbRR.setTitle(getString(R.string.fydsp_attenuate_way3_rr));
                asbCenter.setTitle(getString(R.string.fydsp_attenuate_way3_subwoofer));
                asbSubwoofer.setTitle(getString(R.string.fydsp_attenuate_way3_center));
                break;
            case CHANNEL51:
                asbLF.setTitle(getString(R.string.balance_field_lf));
                asbRF.setTitle(getString(R.string.balance_field_rf));
                asbLR.setTitle(getString(R.string.balance_field_lr));
                asbRR.setTitle(getString(R.string.balance_field_rr));
                asbCenter.setTitle(getString(R.string.fydsp_attenuate_channel51_center));
                asbSubwoofer.setTitle(getString(R.string.str_subwoofer));
                break;
            case WAY6:
                asbLF.setTitle(getString(R.string.fydsp_attenuate_way6_lf));
                asbRF.setTitle(getString(R.string.fydsp_attenuate_way6_rf));
                asbLR.setTitle(getString(R.string.fydsp_attenuate_way6_lr));
                asbRR.setTitle(getString(R.string.fydsp_attenuate_way6_rr));
                asbCenter.setTitle(getString(R.string.fydsp_attenuate_way6_center));
                asbSubwoofer.setTitle(getString(R.string.fydsp_attenuate_way6_subwoofer));
        }
    }

    private void refreshLinkStatus() {
        bLinkLfRf = fyDspAttenuateSettings.getLinkLfRf();
        bLinkLrRr = fyDspAttenuateSettings.getLinkLrRr();
        bLinkCenterSubwoofer = fyDspAttenuateSettings.getLinkCenterSubwoofer();
        btnLinkLfRf.setChecked(bLinkLfRf);
        btnLinkLrRr.setChecked(bLinkLrRr);
        btnLinkCenterSubwoofer.setChecked(bLinkCenterSubwoofer);

        // 5.1 声道时，中置和重低音分开调节
        ivLinkLineCenterSubwoofer.setVisibility(fyDspOutputMode == FyDspOutputMode.CHANNEL51 ? View.INVISIBLE : View.VISIBLE);
        btnLinkCenterSubwoofer.setVisibility(fyDspOutputMode == FyDspOutputMode.CHANNEL51 ? View.INVISIBLE : View.VISIBLE);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
//        Log.d(TAG, "onProgressChanged fromUser : " + fromUser);
        if (!fromUser) return; // 界面初始化时不需要执行，手动拖动才生效

        String channel = (String) seekBar.getTag();
        if (bLinkLfRf && ("LF".equals(channel) || "RF".equals(channel))) {
            linkSetProgress(progress, asbLF, asbRF);
            linkNativeAttenuate(asbLF, asbRF);
        } else if (bLinkLrRr && ("LR".equals(channel) || "RR".equals(channel))) {
            linkSetProgress(progress, asbLR, asbRR);
            linkNativeAttenuate(asbLR, asbRR);
        } else if (bLinkCenterSubwoofer && ("CENTER".equals(channel) || "SUBWOOFER".equals(channel))) {
            linkSetProgress(progress, asbCenter, asbSubwoofer);
            linkNativeAttenuate(asbCenter, asbSubwoofer);
        } else { // 非联动调节时，传入当前调节的控件
            FyDspAttenuateSeekBar fyDspAttenuateSeekBar = getFyDspAttenuateSeekBar(channel);
            linkNativeAttenuate(fyDspAttenuateSeekBar);
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        EventMessage.anyChanged(mContext, TAG + "_" + "onStopTrackingTouch");

        String channel = (String) seekBar.getTag();
        Log.d(TAG, "onStopTrackingTouch channel : " + channel);
        if (bLinkLfRf && ("LF".equals(channel) || "RF".equals(channel))) {
            linkSaveAttenuate(asbLF, asbRF); // 联动调节时，传入多个控件
        } else if (bLinkLrRr && ("LR".equals(channel) || "RR".equals(channel))) {
            linkSaveAttenuate(asbLR, asbRR); // 联动调节时，传入多个控件
        } else if (bLinkCenterSubwoofer && ("CENTER".equals(channel) || "SUBWOOFER".equals(channel))) {
            linkSaveAttenuate(asbCenter, asbSubwoofer); // 联动调节时，传入多个控件
        } else { // 非联动调节时，传入当前调节的控件
            FyDspAttenuateSeekBar fyDspAttenuateSeekBar = getFyDspAttenuateSeekBar(channel);
            linkSaveAttenuate(fyDspAttenuateSeekBar);
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (!buttonView.isPressed()) { // 界面初始化时不需要执行，手动点击才生效
            return;
        }
        EventMessage.anyChanged(mContext, TAG + "_" + "onCheckedChanged");

        int viewId = buttonView.getId();
        if (viewId == R.id.btn_link_lf_rf) {
            Log.d(TAG, "btnLinkLfRf isChecked : " + isChecked);
            bLinkLfRf = isChecked;
            fyDspAttenuateSettings.saveLink(bLinkLfRf, bLinkLrRr, bLinkCenterSubwoofer);
        } else if (viewId == R.id.btn_link_lr_rr) {
            Log.d(TAG, "btnLinkLrRr isChecked : " + isChecked);
            bLinkLrRr = isChecked;
            fyDspAttenuateSettings.saveLink(bLinkLfRf, bLinkLrRr, bLinkCenterSubwoofer);
        } else if (viewId == R.id.btn_link_center_subwoofer) {
            Log.d(TAG, "bLinkCenterSubwoofer isChecked : " + isChecked);
            bLinkCenterSubwoofer = isChecked;
            fyDspAttenuateSettings.saveLink(bLinkLfRf, bLinkLrRr, bLinkCenterSubwoofer);
        } else if (viewId == R.id.cb_mute) {
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
            } else if (bLinkCenterSubwoofer && ("CENTER".equals(channel) || "SUBWOOFER".equals(channel))) { // 联动调节时，传入多个控件
                linkSetMuteStatus(isChecked, asbCenter, asbSubwoofer);
                linkSaveAttenuate(asbCenter, asbSubwoofer);
                linkNativeAttenuate(asbCenter, asbSubwoofer);
            } else { // 非联动调节时，传入当前调节的控件
                FyDspAttenuateSeekBar fyDspAttenuateSeekBar = getFyDspAttenuateSeekBar(channel);
                linkSaveAttenuate(fyDspAttenuateSeekBar);
                linkNativeAttenuate(fyDspAttenuateSeekBar);
            }
        } else if (viewId == R.id.cb_invert) {
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
            } else if (bLinkCenterSubwoofer && ("CENTER".equals(channel) || "SUBWOOFER".equals(channel))) { // 联动调节时，传入多个控件
                linkSetInvertStatus(isChecked, asbCenter, asbSubwoofer);
                linkSaveAttenuate(asbCenter, asbSubwoofer);
                linkNativeAttenuate(asbCenter, asbSubwoofer);
            } else { // 非联动调节时，传入当前调节的控件
                FyDspAttenuateSeekBar fyDspAttenuateSeekBar = getFyDspAttenuateSeekBar(channel);
                linkSaveAttenuate(fyDspAttenuateSeekBar);
                linkNativeAttenuate(fyDspAttenuateSeekBar);
            }
        }
    }

    // 通过 channel 获取到对应的控件
    private FyDspAttenuateSeekBar getFyDspAttenuateSeekBar(final String tag) {
        return Stream.of(asbLF, asbRF, asbLR, asbRR, asbCenter, asbSubwoofer).filter(fyDspAttenuateSeekBar -> tag.equals(fyDspAttenuateSeekBar.getTag())).findFirst().get();
    }

    // 联动调节时，同时保存数据
    private void linkSaveAttenuate(FyDspAttenuateSeekBar... fyDspAttenuateSeekBars) {
        for (FyDspAttenuateSeekBar fyDspAttenuateSeekBar : fyDspAttenuateSeekBars) {
            String channel = (String) fyDspAttenuateSeekBar.getTag();
            int progress = fyDspAttenuateSeekBar.getProgress();
            boolean mute = fyDspAttenuateSeekBar.getAttenuateStatus();
            boolean invert = fyDspAttenuateSeekBar.getInvertStatus();
            fyDspAttenuateSettings.saveAttenuate(channel, progress, mute, invert);
        }
    }

    // 联动调节时，同时设置数据到底层
    private void linkNativeAttenuate(FyDspAttenuateSeekBar... fyDspAttenuateSeekBars) {
        for (FyDspAttenuateSeekBar fyDspAttenuateSeekBar : fyDspAttenuateSeekBars) {
            String channel = (String) fyDspAttenuateSeekBar.getTag();
            int progress = fyDspAttenuateSeekBar.getProgress();
            boolean mute = fyDspAttenuateSeekBar.getAttenuateStatus();
            boolean invert = fyDspAttenuateSeekBar.getInvertStatus();
            fyDspAttenuateSettings.nativeAttenuate(channel, progress, mute, invert);
        }
    }

    // 联动设置进度条进度
    private void linkSetProgress(int progress, FyDspAttenuateSeekBar... fyDspAttenuateSeekBars) {
        for (FyDspAttenuateSeekBar fyDspAttenuateSeekBar : fyDspAttenuateSeekBars) {
            fyDspAttenuateSeekBar.setProgress(progress, false);
        }
    }

    // 联动设置静音是否选中
    private void linkSetMuteStatus(boolean status, FyDspAttenuateSeekBar... fyDspAttenuateSeekBars) {
        for (FyDspAttenuateSeekBar fyDspAttenuateSeekBar : fyDspAttenuateSeekBars) {
            fyDspAttenuateSeekBar.setMuteStatus(status);
        }
    }

    // 联动设置反相是否选中
    private void linkSetInvertStatus(boolean status, FyDspAttenuateSeekBar... fyDspAttenuateSeekBars) {
        for (FyDspAttenuateSeekBar fyDspAttenuateSeekBar : fyDspAttenuateSeekBars) {
            fyDspAttenuateSeekBar.setInvertStatus(status);
        }
    }

    @Override
    @Subscribe(sticky = true)
    public void onEvent(EventMessage eventMessage) {
        super.onEvent(eventMessage);
        if (EventMessage.MSG_STICKY_OUTPUT_MODE_CHANGED.equals(eventMessage.getMessage())) {
            fyDspOutputMode = (FyDspOutputMode) eventMessage.getData();
            Log.d(TAG, "MSG_STICKY_OUTPUT_MODE_CHANGED change to " + fyDspOutputMode);

            if (fyDspOutputMode == FyDspOutputMode.CHANNEL51) { // 5.1声道时，中置和重低音不需要联动调节
                bLinkCenterSubwoofer = false;
                fyDspAttenuateSettings.saveLink(bLinkLfRf, bLinkLrRr, bLinkCenterSubwoofer);
            }
            refreshSeekbarTitle();
            refreshLinkStatus();
        } else if (EventMessage.MSG_STICKY_USER_MODE_CHANGED.equals(eventMessage.getMessage())) {
            Log.d(TAG, "MSG_STICKY_USER_MODE_CHANGED");
            initData();
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_reset_attenuate:
                fyDspAttenuateSettings.reset();
                EventMessage.anyChanged(mContext, TAG + "_" + "btn_reset_attenuate");

                initSeekBar();
                refreshLinkStatus();

                fyDspAttenuateSettings.nativeAll("LF", "RF", "LR", "RR", "CENTER", "SUBWOOFER");
                break;
        }
    }
}