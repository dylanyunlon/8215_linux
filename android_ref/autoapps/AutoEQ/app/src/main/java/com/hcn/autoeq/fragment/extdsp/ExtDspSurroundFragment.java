package com.hcn.autoeq.fragment.extdsp;

import android.graphics.drawable.AnimationDrawable;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.ToggleButton;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.ExtDspSurroundSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.SkinUtils;

public class ExtDspSurroundFragment extends BaseFragment implements CompoundButton.OnCheckedChangeListener {

    private static final String TAG = ExtDspSurroundFragment.class.getSimpleName();

    private View mainView;
    private ToggleButton btnSurroundEnable, btnLoudnessEnable;
    private ImageView ivSpeakerAnim, ivLoudnessSpeaker;
    private AnimationDrawable animationDrawable;

    private ExtDspSurroundSettings extDspSurroundSettings;

    public ExtDspSurroundFragment() {
    }

    public static ExtDspSurroundFragment newInstance() {
        ExtDspSurroundFragment fragment = new ExtDspSurroundFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.extdsp_fragment_surround;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        extDspSurroundSettings = ExtDspSurroundSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        btnSurroundEnable = mainView.findViewById(SkinUtils.getId(R.id.btn_surround_enable));
        btnLoudnessEnable = mainView.findViewById(SkinUtils.getId(R.id.btn_loudness_enable));
        ivSpeakerAnim = mainView.findViewById(SkinUtils.getId(R.id.iv_speaker_anim));
        animationDrawable = (AnimationDrawable) ivSpeakerAnim.getBackground();
        ivLoudnessSpeaker = mainView.findViewById(SkinUtils.getId(R.id.iv_loudness_speaker));

        btnSurroundEnable.setOnCheckedChangeListener(this);
        btnLoudnessEnable.setOnCheckedChangeListener(this);
    }

    @Override
    public void initData() {
        super.initData();
        btnSurroundEnable.setChecked(extDspSurroundSettings.getSurround() == 1);
        btnLoudnessEnable.setChecked(extDspSurroundSettings.getLoudness() == 1);
    }

    @Override
    public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
        if (compoundButton.getId() == SkinUtils.getId(R.id.btn_surround_enable)) {
            if (isChecked) {
                animationDrawable.start();
            } else {
                animationDrawable.stop();
                animationDrawable.selectDrawable(0); // 关闭后，回到第一帧的关闭状态图
            }

            // 手动点击时才往下执行
            if (compoundButton.isPressed()) {
                extDspSurroundSettings.nativeSurround(isChecked ? 1 : 0);
                extDspSurroundSettings.saveSurround(isChecked ? 1 : 0);
            }
        } else if (compoundButton.getId() == SkinUtils.getId(R.id.btn_loudness_enable)) {
            ivLoudnessSpeaker.setVisibility(isChecked ? View.VISIBLE : View.GONE);

            // 手动点击时才往下执行
            if (compoundButton.isPressed()) {
                extDspSurroundSettings.nativeLoudness(isChecked ? 1 : 0);
                extDspSurroundSettings.saveLoudness(isChecked ? 1 : 0);
            }
        }

    }
}
