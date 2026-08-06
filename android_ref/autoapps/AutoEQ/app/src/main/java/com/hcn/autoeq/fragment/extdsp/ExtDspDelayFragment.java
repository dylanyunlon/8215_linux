package com.hcn.autoeq.fragment.extdsp;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.ExtDspDelaySettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.ConstantExtDsp;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.ExtDspDelayView2;

public class ExtDspDelayFragment extends BaseFragment
        implements ConstantExtDsp, View.OnClickListener, ExtDspDelayView2.Callback {

    private static final String TAG = ExtDspDelayFragment.class.getSimpleName();
    private View mainView;
    private ExtDspDelayView2 rksbLF, rksbRF, rksbLR, rksbRR;
    private ImageView ivLF, ivRF, ivLR, ivRR;
    private Button btnResetDelay;

    private ExtDspDelaySettings extDspDelaySettings;

    public ExtDspDelayFragment() {
    }

    public static ExtDspDelayFragment newInstance() {
        ExtDspDelayFragment fragment = new ExtDspDelayFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.extdsp_fragment_delay;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        extDspDelaySettings = ExtDspDelaySettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        rksbLF = mainView.findViewById(SkinUtils.getId(R.id.rksb_lf));
        rksbRF = mainView.findViewById(SkinUtils.getId(R.id.rksb_rf));
        rksbLR = mainView.findViewById(SkinUtils.getId(R.id.rksb_lr));
        rksbRR = mainView.findViewById(SkinUtils.getId(R.id.rksb_rr));

        ivLF = mainView.findViewById(SkinUtils.getId(R.id.iv_delay_speaker_lf));
        ivRF = mainView.findViewById(SkinUtils.getId(R.id.iv_delay_speaker_rf));
        ivLR = mainView.findViewById(SkinUtils.getId(R.id.iv_delay_speaker_lr));
        ivRR = mainView.findViewById(SkinUtils.getId(R.id.iv_delay_speaker_rr));

        btnResetDelay = mainView.findViewById(SkinUtils.getId(R.id.btn_reset_delay));
        btnResetDelay.setOnClickListener(this);

        rksbLF.setCallback(this);
        rksbRF.setCallback(this);
        rksbLR.setCallback(this);
        rksbRR.setCallback(this);

        rksbLF.setTimeValue(extDspDelaySettings.getDelay((String) rksbLF.getTag()));
        rksbRF.setTimeValue(extDspDelaySettings.getDelay((String) rksbRF.getTag()));
        rksbLR.setTimeValue(extDspDelaySettings.getDelay((String) rksbLR.getTag()));
        rksbRR.setTimeValue(extDspDelaySettings.getDelay((String) rksbRR.getTag()));
    }

    @Override
    public void onClick(View view) {
        rksbLF.reset(false);
        rksbRF.reset(false);
        rksbLR.reset(false);
        rksbRR.reset(false);
        extDspDelaySettings.nativeDelay(0, 0, 0, 0);
        extDspDelaySettings.resetDelay();
    }

    @Override
    public void onValueChanged(ExtDspDelayView2 extDspDelayView2, int value, boolean needNativeData) {
        String channel = (String) extDspDelayView2.getTag();
        if ("LF".equals(channel))
            ivLF.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_lf_n : R.drawable.extdsp_delay_speaker_lf_p));
        if ("RF".equals(channel))
            ivRF.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_rf_n : R.drawable.extdsp_delay_speaker_rf_p));
        if ("LR".equals(channel))
            ivLR.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_lr_n : R.drawable.extdsp_delay_speaker_lr_p));
        if ("RR".equals(channel))
            ivRR.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_rr_n : R.drawable.extdsp_delay_speaker_rr_p));

        extDspDelaySettings.saveDelay((String) extDspDelayView2.getTag(), extDspDelayView2.getTimeValue());

        if (needNativeData) {
            extDspDelaySettings.nativeDelay(rksbLF.getTimeValue(), rksbRF.getTimeValue(), rksbLR.getTimeValue(), rksbRR.getTimeValue());
        }
    }

    @Override
    public void onRefreshLayout(ExtDspDelayView2 extDspDelayView2, int value) {

    }
}
