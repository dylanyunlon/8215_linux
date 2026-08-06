package com.hcn.autoeq.fragment.ecdspt;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.ExtDspDelaySettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.ECDConstantExtDsp;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.ExtDspDelayView2;

public class ECDspDelayFragment extends BaseFragment
        implements ECDConstantExtDsp, View.OnClickListener, ExtDspDelayView2.Callback {

    private static final String TAG = ECDspDelayFragment.class.getSimpleName();
    private View mainView;
    private ExtDspDelayView2 rksbLF, rksbRF, rksbLR, rksbRR, rksbSub, rksbCen;
    private ImageView ivLF, ivRF, ivLR, ivRR, ivSub, ivCen;
    private Button btnResetDelay;

    private static final int maxDelayTime = 100;

    private ExtDspDelaySettings extDspDelaySettings;

    public ECDspDelayFragment() {
    }

    public static ECDspDelayFragment newInstance() {
        ECDspDelayFragment fragment = new ECDspDelayFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        if (checkLayoutExists(R.layout.ext_c_dsp_fragment_delay)) {
            return R.layout.ext_c_dsp_fragment_delay;
        } else {
            return R.layout.extdsp_fragment_delay;
        }
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
        rksbSub = mainView.findViewById(SkinUtils.getId(R.id.rksb_subwoofer));
        rksbCen = mainView.findViewById(SkinUtils.getId(R.id.rksb_center));

        ivLF = mainView.findViewById(SkinUtils.getId(R.id.iv_delay_speaker_lf));
        ivRF = mainView.findViewById(SkinUtils.getId(R.id.iv_delay_speaker_rf));
        ivLR = mainView.findViewById(SkinUtils.getId(R.id.iv_delay_speaker_lr));
        ivRR = mainView.findViewById(SkinUtils.getId(R.id.iv_delay_speaker_rr));
        ivSub = mainView.findViewById(SkinUtils.getId(R.id.iv_delay_speaker_subwoofer));
        ivCen = mainView.findViewById(SkinUtils.getId(R.id.iv_delay_speaker_center));

        if (ivSub != null && ivCen != null) {
            if (EqUtils.showCenter()) {
                ivSub.setVisibility(View.VISIBLE);
                ivCen.setVisibility(View.VISIBLE);
            } else {
                ivSub.setVisibility(View.GONE);
                ivCen.setVisibility(View.GONE);
            }
        }

        btnResetDelay = mainView.findViewById(SkinUtils.getId(R.id.btn_reset_delay));
        btnResetDelay.setOnClickListener(this);

        rksbLF.setCallback(this);
        rksbRF.setCallback(this);
        rksbLR.setCallback(this);
        rksbRR.setCallback(this);

        rksbLF.setMaxTime(maxDelayTime);
        rksbRF.setMaxTime(maxDelayTime);
        rksbLR.setMaxTime(maxDelayTime);
        rksbRR.setMaxTime(maxDelayTime);

        if (rksbSub != null) {
            rksbSub.setMaxTime(maxDelayTime);
            rksbSub.setCallback(this);
            if (EqUtils.showCenter()) {
                rksbSub.setVisibility(View.VISIBLE);
            } else {
                rksbSub.setVisibility(View.GONE);
            }
        }
        if (rksbCen != null) {
            rksbCen.setMaxTime(maxDelayTime);
            rksbCen.setCallback(this);
            if (EqUtils.showCenter()) {
                rksbCen.setVisibility(View.VISIBLE);
            } else {
                rksbCen.setVisibility(View.GONE);
            }
        }

        rksbLF.setTimeValue(extDspDelaySettings.getDelay((String) rksbLF.getTag()));
        rksbRF.setTimeValue(extDspDelaySettings.getDelay((String) rksbRF.getTag()));
        rksbLR.setTimeValue(extDspDelaySettings.getDelay((String) rksbLR.getTag()));
        rksbRR.setTimeValue(extDspDelaySettings.getDelay((String) rksbRR.getTag()));
        if (rksbSub != null) {
            rksbSub.setTimeValue(extDspDelaySettings.getDelay((String) rksbSub.getTag()));
        }
        if (rksbCen != null) {
            rksbCen.setTimeValue(extDspDelaySettings.getDelay((String) rksbCen.getTag()));
        }
    }

    @Override
    public void onClick(View view) {
        rksbLF.reset(false);
        rksbRF.reset(false);
        rksbLR.reset(false);
        rksbRR.reset(false);
        if (rksbSub != null) {
            rksbSub.reset(false);
        }
        if (rksbCen != null) {
            rksbCen.reset(false);
        }
        extDspDelaySettings.nativeDelay(0, 0, 0, 0, 0, 0);
        extDspDelaySettings.resetDelay();
    }

    @Override
    public void onValueChanged(ExtDspDelayView2 extDspDelayView2, int value, boolean needNativeData) {
        String channel = (String) extDspDelayView2.getTag();
        if ("LF".equals(channel)) {
            ivLF.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_lf_n : R.drawable.extdsp_delay_speaker_lf_p));
        }
        if ("RF".equals(channel)) {
            ivRF.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_rf_n : R.drawable.extdsp_delay_speaker_rf_p));
        }
        if ("LR".equals(channel)) {
            ivLR.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_lr_n : R.drawable.extdsp_delay_speaker_lr_p));
        }
        if ("RR".equals(channel)) {
            ivRR.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_rr_n : R.drawable.extdsp_delay_speaker_rr_p));
        }
        if ("SUBWOOFER".equals(channel)) {
            if (null != ivSub) {
                ivSub.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_dbb_speaker_center_n : R.drawable.extdsp_dbb_speaker_center_p));
                ivSub.setRotation(180);
            }
        }
        if ("CENTER".equals(channel)) {
            if (null != ivCen) {
                ivCen.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_dbb_speaker_subwoofer_n : R.drawable.extdsp_dbb_speaker_subwoofer_p));
                ivCen.setRotation(180);
            }
        }

        extDspDelaySettings.saveDelay((String) extDspDelayView2.getTag(), extDspDelayView2.getTimeValue());

        if (needNativeData) {
            if (rksbSub == null || rksbCen == null) {
                extDspDelaySettings.nativeDelay(rksbLF.getTimeValue(), rksbRF.getTimeValue(),
                        rksbLR.getTimeValue(), rksbRR.getTimeValue());
            } else {
                extDspDelaySettings.nativeDelay(rksbLF.getTimeValue(), rksbRF.getTimeValue(),
                        rksbLR.getTimeValue(), rksbRR.getTimeValue(), rksbSub.getTimeValue(),
                        rksbCen.getTimeValue());
            }

        }
    }

    @Override
    public void onRefreshLayout(ExtDspDelayView2 extDspDelayView2, int value) {

    }
}
