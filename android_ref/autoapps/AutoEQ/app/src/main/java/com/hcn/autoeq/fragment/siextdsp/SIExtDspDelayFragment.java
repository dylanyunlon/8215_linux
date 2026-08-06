package com.hcn.autoeq.fragment.siextdsp;

import static com.hcn.autoeq.data.SIExtDspDelaySettings.CHANEL_CEN;
import static com.hcn.autoeq.data.SIExtDspDelaySettings.CHANEL_FL;
import static com.hcn.autoeq.data.SIExtDspDelaySettings.CHANEL_FR;
import static com.hcn.autoeq.data.SIExtDspDelaySettings.CHANEL_RL;
import static com.hcn.autoeq.data.SIExtDspDelaySettings.CHANEL_RR;
import static com.hcn.autoeq.data.SIExtDspDelaySettings.CHANEL_SUB;
import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_CEN;
import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_FL;
import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_FR;
import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_RL;
import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_RR;
import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_SUB;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.ExtDspDelaySettings;
import com.hcn.autoeq.data.SIExtDspAttenuateSettings;
import com.hcn.autoeq.data.SIExtDspDelaySettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.SIConstantExtDsp;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.ExtDspDelayView2;

/**
 * 和普通extdsp的UI相似，但功能的实现上是不同的，存在一部分区别，
 * 所以应该另外创建setting以及Fragment，方便以后的扩展和修改；
 */
public class SIExtDspDelayFragment extends BaseFragment
        implements SIConstantExtDsp, View.OnClickListener, ExtDspDelayView2.Callback {

    private static final String TAG = SIExtDspDelayFragment.class.getSimpleName();

    //延迟时间最大值
    private static final int maxDelayTime = 100;
    private View mainView;
    private ExtDspDelayView2 rksbLF, rksbRF, rksbLR, rksbRR, rksbSub, rksbCen;
    private ImageView ivLF, ivRF, ivLR, ivRR, ivSub, ivCen;
    private Button btnResetDelay;

    private SIExtDspDelaySettings siExtDspDelaySettings;

    public SIExtDspDelayFragment() {
    }

    public static SIExtDspDelayFragment newInstance() {
        SIExtDspDelayFragment fragment = new SIExtDspDelayFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.siextdsp_fragment_delay;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        siExtDspDelaySettings = SIExtDspDelaySettings.getInstance(mContext);
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


        btnResetDelay = mainView.findViewById(SkinUtils.getId(R.id.btn_reset_delay));
        btnResetDelay.setOnClickListener(this);

        rksbLF.setMaxTime(maxDelayTime);
        rksbLF.setCallback(this);
        rksbRF.setMaxTime(maxDelayTime);
        rksbRF.setCallback(this);
        rksbLR.setMaxTime(maxDelayTime);
        rksbLR.setCallback(this);
        rksbRR.setMaxTime(maxDelayTime);
        rksbRR.setCallback(this);
        if (rksbSub != null) {
            rksbSub.setMaxTime(maxDelayTime);
            rksbSub.setCallback(this);
        }
        if (rksbCen != null) {
            rksbCen.setMaxTime(maxDelayTime);
            rksbCen.setCallback(this);
        }

        rksbLF.setTimeValue(siExtDspDelaySettings.getDelay((String) rksbLF.getTag()));
        rksbRF.setTimeValue(siExtDspDelaySettings.getDelay((String) rksbRF.getTag()));
        rksbLR.setTimeValue(siExtDspDelaySettings.getDelay((String) rksbLR.getTag()));
        rksbRR.setTimeValue(siExtDspDelaySettings.getDelay((String) rksbRR.getTag()));
        if (rksbSub != null) {
            rksbSub.setTimeValue(siExtDspDelaySettings.getDelay((String) rksbSub.getTag()));
        }
        if (rksbCen != null) {
            rksbCen.setTimeValue(siExtDspDelaySettings.getDelay((String) rksbCen.getTag()));
        }
    }

    @Override
    public void initData() {
        super.initData();
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
        siExtDspDelaySettings.resetDelay();
        siExtDspDelaySettings.nativeAll(CHANEL_FL, CHANEL_FR, CHANEL_RL, CHANEL_RR, CHANEL_SUB, CHANEL_CEN);
    }

    @Override
    public void onValueChanged(ExtDspDelayView2 extDspDelayView2, int value, boolean needNativeData) {
        String channel = (String) extDspDelayView2.getTag();
        Log.d(TAG, "onValueChanged: value = " + value + " channel = " + channel + " needNativeData = " + needNativeData);
        int channelId = SI47925_INDEX_CHANEL_FL;
        if ("LF".equals(channel)) {
            channelId = SI47925_INDEX_CHANEL_FL;
            ivLF.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_lf_n : R.drawable.extdsp_delay_speaker_lf_p));
        }
        if ("RF".equals(channel)) {
            channelId = SI47925_INDEX_CHANEL_FR;
            ivRF.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_rf_n : R.drawable.extdsp_delay_speaker_rf_p));
        }
        if ("LR".equals(channel)) {
            channelId = SI47925_INDEX_CHANEL_RL;
            ivLR.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_lr_n : R.drawable.extdsp_delay_speaker_lr_p));
        }
        if ("RR".equals(channel)) {
            channelId = SI47925_INDEX_CHANEL_RR;
            ivRR.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_rr_n : R.drawable.extdsp_delay_speaker_rr_p));
        }
        if ("SUBWOOFER".equals(channel)) {
            channelId = SI47925_INDEX_CHANEL_SUB;
            ivSub.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable. extdsp_delay_speaker_subwoofer_n: R.drawable.extdsp_delay_speaker_subwoofer_p));

        }
        if ("CENTER".equals(channel)) {
            channelId = SI47925_INDEX_CHANEL_CEN;
            ivCen.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable.extdsp_delay_speaker_center_n : R.drawable.extdsp_delay_speaker_center_p));

        }
        siExtDspDelaySettings.saveDelay((String) extDspDelayView2.getTag(), extDspDelayView2.getTimeValue());
        if (needNativeData) {
            SIExtDspAttenuateSettings extDspAttenuateSettings = SIExtDspAttenuateSettings.getInstance(mContext);
            int polarity = extDspAttenuateSettings.getInvert(channel) ? 1 : 0;
            siExtDspDelaySettings.nativeDelay(channelId, extDspDelayView2.getTimeValue(), polarity);
        }
    }

    @Override
    public void onRefreshLayout(ExtDspDelayView2 extDspDelayView2, int value) {
        String channel = (String) extDspDelayView2.getTag();
        Log.d(TAG, "onRefreshLayout: value = " + value + " channel = " + channel);
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
            ivSub.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable. extdsp_delay_speaker_subwoofer_n: R.drawable.extdsp_delay_speaker_subwoofer_p));

        }
        if ("CENTER".equals(channel)) {
            ivCen.setBackground(SkinUtils.getDrawable(value == 0 ? R.drawable. extdsp_delay_speaker_center_n: R.drawable.extdsp_delay_speaker_center_p));

        }
    }
}
