package com.hcn.autoeq.fragment.extdsp;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RadioGroup;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.ExtDspDbbSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.ConstantExtDsp;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.ExtDspDbbView;

public class ExtDspDbbFragment extends BaseFragment
        implements ConstantExtDsp, RadioGroup.OnCheckedChangeListener
        , View.OnClickListener, ExtDspDbbView.IExtDspDbbCallback {

    private static final String TAG = ExtDspDbbFragment.class.getSimpleName();

    private View mainView;
    private RadioGroup rgDbbMode;
    private ImageView ivDbbSpeakerLF, ivDbbSpeakerRF, ivDbbSpeakerLR, ivDbbSpeakerRR, ivDbbSpeakerSubwoofer;
    private Button btnDbbReset;
    private ExtDspDbbView extDspDbbView;

    private ExtDspDbbSettings extDspDbbSettings;
    private int channel = DEF_DBB_CHANNEL_FLFR;

    public ExtDspDbbFragment() {
    }

    public static ExtDspDbbFragment newInstance() {
        ExtDspDbbFragment fragment = new ExtDspDbbFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.extdsp_fragment_dbb;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        extDspDbbSettings = ExtDspDbbSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        Log.d(TAG, "initView");
        ivDbbSpeakerLF = mainView.findViewById(SkinUtils.getId(R.id.iv_dbb_speaker_lf));
        ivDbbSpeakerRF = mainView.findViewById(SkinUtils.getId(R.id.iv_dbb_speaker_rf));
        ivDbbSpeakerLR = mainView.findViewById(SkinUtils.getId(R.id.iv_dbb_speaker_lr));
        ivDbbSpeakerRR = mainView.findViewById(SkinUtils.getId(R.id.iv_dbb_speaker_rr));
        ivDbbSpeakerSubwoofer = mainView.findViewById(SkinUtils.getId(R.id.iv_dbb_speaker_subwoofer));

        btnDbbReset = mainView.findViewById(SkinUtils.getId(R.id.btn_reset_dbb));
        btnDbbReset.setOnClickListener(this);

        rgDbbMode = mainView.findViewById(SkinUtils.getId(R.id.rg_dbb_mode));
        rgDbbMode.setOnCheckedChangeListener(this);

        extDspDbbView = mainView.findViewById(SkinUtils.getId(R.id.v_dbb));
        extDspDbbView.setExtDspDbbCallback(this);
    }

    @Override
    public void initData() {
        super.initData();
        Log.d(TAG, "initData");

        int channel = extDspDbbSettings.getDbbChannel();
        if (channel == DBB_CHANNEL_FLFR) {
            rgDbbMode.check(SkinUtils.getId(R.id.rb_dbb_mode_fl_fr));
        } else if (channel == DBB_CHANNEL_RLRR) {
            rgDbbMode.check(SkinUtils.getId(R.id.rb_dbb_mode_rl_rr));
        } else {
            rgDbbMode.check(SkinUtils.getId(R.id.rb_dbb_mode_subwoofer));
        }
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (!hidden) {
            // 默认位置是底部一条直线，所以 x 是0，y 是图片的高度
            float touchX = extDspDbbSettings.getDbbTouchX(channel, 0);
            float touchY = extDspDbbSettings.getDbbTouchY(channel, extDspDbbView.getBackground().getIntrinsicHeight());
            extDspDbbView.setTouchPoint(touchX, touchY); // 更新view
        }
    }

    @Override
    public void onCheckedChanged(RadioGroup radioGroup, int checkedId) {
        Log.d(TAG, "onCheckedChanged");
        ivDbbSpeakerLF.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_dbb_mode_fl_fr)
                ? R.drawable.extdsp_dbb_speaker_lf_p : R.drawable.extdsp_dbb_speaker_lf_n));
        ivDbbSpeakerRF.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_dbb_mode_fl_fr)
                ? R.drawable.extdsp_dbb_speaker_rf_p : R.drawable.extdsp_dbb_speaker_rf_n));
        ivDbbSpeakerLR.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_dbb_mode_rl_rr)
                ? R.drawable.extdsp_dbb_speaker_lr_p : R.drawable.extdsp_dbb_speaker_lr_n));
        ivDbbSpeakerRR.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_dbb_mode_rl_rr)
                ? R.drawable.extdsp_dbb_speaker_rr_p : R.drawable.extdsp_dbb_speaker_rr_n));
        ivDbbSpeakerSubwoofer.setBackground(SkinUtils.getDrawable(radioGroup.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_dbb_mode_subwoofer)
                ? R.drawable.extdsp_dbb_speaker_subwoofer_p : R.drawable.extdsp_dbb_speaker_subwoofer_n));

        if (checkedId == SkinUtils.getId(R.id.rb_dbb_mode_fl_fr)) {
            channel = DBB_CHANNEL_FLFR;
        } else if (checkedId == SkinUtils.getId(R.id.rb_dbb_mode_rl_rr)) {
            channel = DBB_CHANNEL_RLRR;
        } else if (checkedId == SkinUtils.getId(R.id.rb_dbb_mode_subwoofer)) {
            channel = DBB_CHANNEL_SUBWOOFER;
        }

        // 默认位置是底部一条直线，所以 x 是0，y 是图片的高度
        float touchX = extDspDbbSettings.getDbbTouchX(channel, 0);
        float touchY = extDspDbbSettings.getDbbTouchY(channel, extDspDbbView.getBackground().getIntrinsicHeight());
        extDspDbbView.setTouchPoint(touchX, touchY); // 更新view

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

        int[] _data = touchPoint2Value(touchX, touchY); // 更新 native
        extDspDbbSettings.saveDbb(channel, touchX, touchY, _data[0], _data[1]); // 更新本地数据
    }

    @Override
    public void onClick(View view) {
        extDspDbbView.reset();
        float touchX = extDspDbbView.getTouchX();
        float touchY = extDspDbbView.getTouchY();
        int[] _data = touchPoint2Value(touchX, touchY); // 更新 native
        extDspDbbSettings.saveDbb(channel, touchX, touchY, _data[0], _data[1]); // 更新本地数据
    }

    @Override
    public void onActionMove(float touchX, float touchY) {
        touchPoint2Value(touchX, touchY);
    }

    @Override
    public void onActionUp(float touchX, float touchY) {
        int[] _data = touchPoint2Value(touchX, touchY);
        extDspDbbSettings.saveDbb(channel, touchX, touchY, _data[0], _data[1]);
    }

    // 根据背景图的宽高，横竖向的频点增益范围，计算每滑动多少对应的实际值
    private int[] touchPoint2Value(float touchX, float touchY) {
        float perWidth = (DBB_FREQ_MAX - DBB_FREQ_MIN) * 1f / extDspDbbView.getBackground().getIntrinsicWidth();
        float perHeight = (DBB_GAIN_MAX - DBB_GAIN_MIN) * 1f / extDspDbbView.getBackground().getIntrinsicHeight();
        float freq = perWidth * touchX;
        float gain = perHeight * (extDspDbbView.getBackground().getIntrinsicHeight() - touchY);

        if (freq < DBB_FREQ_MIN) freq = DBB_FREQ_MIN;
        if (freq > DBB_FREQ_MAX) freq = DBB_FREQ_MAX;

        if (gain < DBB_GAIN_MIN) gain = DBB_GAIN_MIN;
        if (gain > DBB_GAIN_MAX) gain = DBB_GAIN_MAX;

        Log.d(TAG, String.format("perWidth : %s, perHeight : %s, x : %s, y : %s", perWidth, perHeight, touchX, touchY));
        extDspDbbSettings.nativeDbb(channel, (int) freq, (int) (gain + 0.5)); // gain 四舍五入

        return new int[]{(int) freq, (int) (gain + 0.5)};
    }
}
