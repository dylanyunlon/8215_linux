package com.hcn.autoeq.nine;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.ToggleButton;

import com.hcn.autoeq.R;
import com.hcn_library.BaseFragment;
import com.hcn_library.data.NineDspAttenuateSettings;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.SkinUtils;

import com.hcn.autoeq.view.NineSurroundSeekBar;

public class NineSurroundGainFragment extends BaseFragment implements SeekBar.OnSeekBarChangeListener, CompoundButton.OnCheckedChangeListener
        , NineSurroundFilterFragment.FragmentResetInterface {
    private static final String TAG = "NineSurroundGainFragment";
    private NineSurroundSeekBar asbCen;
    private NineSurroundSeekBar asbLF;
    private NineSurroundSeekBar asbLR;
    private NineSurroundSeekBar asbRF;
    private NineSurroundSeekBar asbRR;
    private NineSurroundSeekBar asbSub;
    private boolean bLinkLfRf;
    private boolean bLinkLrRr;
    private ToggleButton btnLinkLfRf;
    private ToggleButton btnLinkLrRr;
    private ImageView ivForeFront;
    private ImageView ivFrontL;
    private ImageView ivFrontR;
    private ImageView ivLineF;
    private ImageView ivLineR;
    private ImageView ivRear;
    private ImageView ivRearL;
    private ImageView ivRearR;
    private View mainView;
    private NineDspAttenuateSettings nineDspAttenuateSettings;
    private int[] subDrawable = {R.drawable.icon_nine_surround_gain_audio_rear_00, R.drawable.icon_nine_surround_gain_audio_rear_01, R.drawable.icon_nine_surround_gain_audio_rear_02
            , R.drawable.icon_nine_surround_gain_audio_rear_03, R.drawable.icon_nine_surround_gain_audio_rear_04, R.drawable.icon_nine_surround_gain_audio_rear_05
            , R.drawable.icon_nine_surround_gain_audio_rear_06, R.drawable.icon_nine_surround_gain_audio_rear_07, R.drawable.icon_nine_surround_gain_audio_rear_08
            , R.drawable.icon_nine_surround_gain_audio_rear_09, R.drawable.icon_nine_surround_gain_audio_rear_10, R.drawable.icon_nine_surround_gain_audio_rear_11
            , R.drawable.icon_nine_surround_gain_audio_rear_12, R.drawable.icon_nine_surround_gain_audio_rear_13, R.drawable.icon_nine_surround_gain_audio_rear_14
            , R.drawable.icon_nine_surround_gain_audio_rear_15};
    private int[] centerDrawable = {R.drawable.icon_nine_surround_gain_audio_forefront_00, R.drawable.icon_nine_surround_gain_audio_forefront_01
            , R.drawable.icon_nine_surround_gain_audio_forefront_02, R.drawable.icon_nine_surround_gain_audio_forefront_03, R.drawable.icon_nine_surround_gain_audio_forefront_04
            , R.drawable.icon_nine_surround_gain_audio_forefront_05, R.drawable.icon_nine_surround_gain_audio_forefront_06, R.drawable.icon_nine_surround_gain_audio_forefront_07
            , R.drawable.icon_nine_surround_gain_audio_forefront_08, R.drawable.icon_nine_surround_gain_audio_forefront_09, R.drawable.icon_nine_surround_gain_audio_forefront_10
            , R.drawable.icon_nine_surround_gain_audio_forefront_11, R.drawable.icon_nine_surround_gain_audio_forefront_12, R.drawable.icon_nine_surround_gain_audio_forefront_13
            , R.drawable.icon_nine_surround_gain_audio_forefront_14, R.drawable.icon_nine_surround_gain_audio_forefront_15};
    private int[] leftDrawable = {R.drawable.icon_nine_surround_gain_audio_left_00, R.drawable.icon_nine_surround_gain_audio_left_01, R.drawable.icon_nine_surround_gain_audio_left_02
            , R.drawable.icon_nine_surround_gain_audio_left_03, R.drawable.icon_nine_surround_gain_audio_left_04, R.drawable.icon_nine_surround_gain_audio_left_05
            , R.drawable.icon_nine_surround_gain_audio_left_06, R.drawable.icon_nine_surround_gain_audio_left_07, R.drawable.icon_nine_surround_gain_audio_left_08
            , R.drawable.icon_nine_surround_gain_audio_left_09, R.drawable.icon_nine_surround_gain_audio_left_10, R.drawable.icon_nine_surround_gain_audio_left_11
            , R.drawable.icon_nine_surround_gain_audio_left_12, R.drawable.icon_nine_surround_gain_audio_left_13, R.drawable.icon_nine_surround_gain_audio_left_14
            , R.drawable.icon_nine_surround_gain_audio_left_15};
    private int[] rightDrawable = {R.drawable.icon_nine_surround_gain_audio_right_00, R.drawable.icon_nine_surround_gain_audio_right_01, R.drawable.icon_nine_surround_gain_audio_right_02
            , R.drawable.icon_nine_surround_gain_audio_right_03, R.drawable.icon_nine_surround_gain_audio_right_04, R.drawable.icon_nine_surround_gain_audio_right_05
            , R.drawable.icon_nine_surround_gain_audio_right_06, R.drawable.icon_nine_surround_gain_audio_right_07, R.drawable.icon_nine_surround_gain_audio_right_08
            , R.drawable.icon_nine_surround_gain_audio_right_09, R.drawable.icon_nine_surround_gain_audio_right_10, R.drawable.icon_nine_surround_gain_audio_right_11
            , R.drawable.icon_nine_surround_gain_audio_right_12, R.drawable.icon_nine_surround_gain_audio_right_13, R.drawable.icon_nine_surround_gain_audio_right_14
            , R.drawable.icon_nine_surround_gain_audio_right_15};

    @Override
    public int getLayoutRes() {
        return R.layout.nine_surround_fragment_gain;
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    public static NineSurroundGainFragment newInstance() {
        return new NineSurroundGainFragment();
    }

    @Override
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        mainView = super.onCreateView(layoutInflater, viewGroup, bundle);
        nineDspAttenuateSettings = NineDspAttenuateSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        bLinkLfRf = nineDspAttenuateSettings.getLinkLfRf();
        bLinkLrRr = nineDspAttenuateSettings.getLinkLrRr();
        asbLF = (NineSurroundSeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_gain_front_l));
        asbRF = (NineSurroundSeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_gain_front_r));
        asbLR = (NineSurroundSeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_gain_rear_l));
        asbRR = (NineSurroundSeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_gain_rear_r));
        asbSub = (NineSurroundSeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_gain_bass));
        asbCen = (NineSurroundSeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_gain_center));
        ivForeFront = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_gain_forefront));
        ivFrontL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_gain_front_l));
        ivFrontR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_gain_front_r));
        ivRearL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_gain_rear_l));
        ivRearR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_gain_rear_r));
        ivRear = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_gain_rear));
        asbLF.setOnSeekBarChangeListener(this);
        asbRF.setOnSeekBarChangeListener(this);
        asbLR.setOnSeekBarChangeListener(this);
        asbRR.setOnSeekBarChangeListener(this);
        asbSub.setOnSeekBarChangeListener(this);
        asbCen.setOnSeekBarChangeListener(this);
        asbLF.setOnCheckedChangeListener(this);
        asbRF.setOnCheckedChangeListener(this);
        asbLR.setOnCheckedChangeListener(this);
        asbRR.setOnCheckedChangeListener(this);
        asbSub.setOnCheckedChangeListener(this);
        asbCen.setOnCheckedChangeListener(this);
        ivLineF = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_gain_front_link_line));
        ivLineR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_gain_rear_link_line));
        btnLinkLfRf = (ToggleButton) mainView.findViewById(SkinUtils.getId(R.id.btn_gain_front_link));
        btnLinkLrRr = (ToggleButton) mainView.findViewById(SkinUtils.getId(R.id.btn_gain_rear_link));
        btnLinkLfRf.setOnCheckedChangeListener(this);
        btnLinkLrRr.setOnCheckedChangeListener(this);
    }

    @Override
    public void initData() {
        super.initData();
        refreshView(asbLF, asbRF, asbLR, asbRR, asbSub, asbCen);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        Log.d(TAG, "onProgressChanged fromUser : " + fromUser + " progress: " + progress + " seekBar.getProgress(): " + seekBar.getProgress());
        if (fromUser) {
            String channel = (String) seekBar.getTag();
            if (bLinkLfRf && ("LF".equals(channel) || "RF".equals(channel))) {
                linkSetProgress(progress, asbLF, asbRF);
                linkNativeAttenuate(asbLF, asbRF);
                ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress()]));
                ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress()]));
            } else if (bLinkLrRr && ("LR".equals(channel) || "RR".equals(channel))) {
                linkSetProgress(progress, asbLR, asbRR);
                linkNativeAttenuate(asbLR, asbRR);
                ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress()]));
                ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress()]));
            } else {
                linkNativeAttenuate(getNineSurroundSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSub, asbCen));
            }
            if ("LF".equals(channel)) {
                ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress()]));
            }
            if ("RF".equals(channel)) {
                ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress()]));
            }
            if ("LR".equals(channel)) {
                ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress()]));
            }
            if ("RR".equals(channel)) {
                ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress()]));
            }
            if ("SUBWOOFER".equals(channel)) {
                ivRear.setImageDrawable(SkinUtils.getDrawable(subDrawable[seekBar.getProgress()]));
            }
            if ("CENTER".equals(channel)) {
                ivForeFront.setImageDrawable(SkinUtils.getDrawable(centerDrawable[seekBar.getProgress()]));
            }
        }

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        String channel = (String) seekBar.getTag();
        Log.d(TAG, "onStopTrackingTouch channel : " + channel + " seekBar.getProgress(): " + seekBar.getProgress());
        if (bLinkLfRf && ("LF".equals(channel) || "RF".equals(channel))) {
            linkSaveAttenuate(asbLF, asbRF);
            ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress()]));
            ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress()]));
        } else if (bLinkLrRr && ("LR".equals(channel) || "RR".equals(channel))) {
            linkSaveAttenuate(asbLR, asbRR);
            ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress()]));
            ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress()]));
        } else {
            linkSaveAttenuate(getNineSurroundSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSub, asbCen));
        }
        if ("LF".equals(channel)) {
            ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress()]));
        }
        if ("RF".equals(channel)) {
            ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress()]));
        }
        if ("LR".equals(channel)) {
            ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress()]));
        }
        if ("RR".equals(channel)) {
            ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress()]));
        }
        if ("SUBWOOFER".equals(channel)) {
            ivRear.setImageDrawable(SkinUtils.getDrawable(subDrawable[seekBar.getProgress()]));
        }
        if ("CENTER".equals(channel)) {
            ivForeFront.setImageDrawable(SkinUtils.getDrawable(centerDrawable[seekBar.getProgress()]));
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton compoundButton, boolean checked) {
        if (compoundButton.isPressed()) {
            int id = compoundButton.getId();
            if (id == SkinUtils.getId(R.id.btn_gain_front_link)) {
                Log.d(TAG, "btnLinkLfRf isChecked : " + checked);
                bLinkLfRf = checked;
                ivLineF.setSelected(checked);
                nineDspAttenuateSettings.saveLink(bLinkLfRf, bLinkLrRr);
                return;
            }
            if (id == SkinUtils.getId(R.id.btn_gain_rear_link)) {
                Log.d(TAG, "btnLinkLrRr isChecked : " + checked);
                bLinkLrRr = checked;
                ivLineR.setSelected(checked);
                nineDspAttenuateSettings.saveLink(bLinkLfRf, bLinkLrRr);
                return;
            }
            if (id == SkinUtils.getId(R.id.cb_mute)) {
                Log.d(TAG, "onCheckedChanged cb_mute isChecked ? " + checked);
                String channel = (String) compoundButton.getTag();
                if (bLinkLfRf && ("LF".equals(channel) || "RF".equals(channel))) {
                    linkSetMuteStatus(checked, asbLF, asbRF);
                    linkSaveAttenuate(asbLF, asbRF);
                    linkNativeAttenuate(asbLF, asbRF);
                    ivFrontL.setAlpha(checked ? 0.5f : 1f);
                    ivFrontR.setAlpha(checked ? 0.5f : 1f);
                    return;
                } else if (bLinkLrRr && ("LR".equals(channel) || "RR".equals(channel))) {
                    linkSetMuteStatus(checked, asbLR, asbRR);
                    linkSaveAttenuate(asbLR, asbRR);
                    linkNativeAttenuate(asbLR, asbRR);
                    ivRearL.setAlpha(checked ? 0.5f : 1f);
                    ivRearR.setAlpha(checked ? 0.5f : 1f);
                    return;
                } else {
                    NineSurroundSeekBar nineSurroundSeekBar = getNineSurroundSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSub, asbCen);
                    linkSaveAttenuate(nineSurroundSeekBar);
                    linkNativeAttenuate(nineSurroundSeekBar);
                    if ("LF".equals(channel)) {
                        ivFrontL.setAlpha(checked ? 0.5f : 1f);
                    } else if ("RF".equals(channel)) {
                        ivFrontR.setAlpha(checked ? 0.5f : 1f);
                    } else if ("LR".equals(channel)) {
                        ivRearL.setAlpha(checked ? 0.5f : 1f);
                    } else if ("RR".equals(channel)) {
                        ivRearR.setAlpha(checked ? 0.5f : 1f);
                    } else if ("CENTER".equals(channel)) {
                        ivForeFront.setAlpha(checked ? 0.5f : 1f);
                    } else if ("SUBWOOFER".equals(channel)) {
                        ivRear.setAlpha(checked ? 0.5f : 1f);
                    }
                    return;
                }
            }
            if (id == SkinUtils.getId(R.id.cb_invert)) {
                Log.d(TAG, "onCheckedChanged cb_invert isChecked ? " + checked);
                String channel = (String) compoundButton.getTag();
                if (bLinkLfRf && ("LF".equals(channel) || "RF".equals(channel))) {
                    linkSetInvertStatus(checked, asbLF, asbRF);
                    linkSaveAttenuate(asbLF, asbRF);
                    linkNativeAttenuate(asbLF, asbRF, true);
                } else if (bLinkLrRr && ("LR".equals(channel) || "RR".equals(channel))) {
                    linkSetInvertStatus(checked, asbLR, asbRR);
                    linkSaveAttenuate(asbLR, asbRR);
                    linkNativeAttenuate(asbLR, asbRR, true);
                } else {
                    NineSurroundSeekBar nineSurroundSeekBar = getNineSurroundSeekBar(channel, asbLF, asbRF, asbLR, asbRR, asbSub, asbCen);
                    linkSaveAttenuate(nineSurroundSeekBar);
                    linkNativeAttenuate(nineSurroundSeekBar, true);
                }
            }
        }

    }

    private NineSurroundSeekBar getNineSurroundSeekBar(String str, NineSurroundSeekBar... nineSurroundSeekBarArr) {
        for (NineSurroundSeekBar nineSurroundSeekBar : nineSurroundSeekBarArr) {
            if (str.equals(nineSurroundSeekBar.getTag())) {
                return nineSurroundSeekBar;
            }
        }
        return null;
    }

    private void linkSaveAttenuate(NineSurroundSeekBar... nineSurroundSeekBarArr) {
        for (NineSurroundSeekBar nineSurroundSeekBar : nineSurroundSeekBarArr) {
            nineDspAttenuateSettings.saveAttenuate((String) nineSurroundSeekBar.getTag(), nineSurroundSeekBar.getProgress(), nineSurroundSeekBar.getAttenuateStatus(), nineSurroundSeekBar.getInvertStatus());
        }
    }

    private void linkNativeAttenuate(NineSurroundSeekBar... nineSurroundSeekBarArr) {
        for (NineSurroundSeekBar nineSurroundSeekBar : nineSurroundSeekBarArr) {
            nineDspAttenuateSettings.nativeAttenuate((String) nineSurroundSeekBar.getTag(), nineSurroundSeekBar.getProgress(), nineSurroundSeekBar.getAttenuateStatus(), nineSurroundSeekBar.getInvertStatus());
        }
    }

    private void linkNativeAttenuate(NineSurroundSeekBar nineSurroundSeekBar, boolean change) {
        if ("gb05".equals(EqUtils.getSkinName()) || EqUtils.isChip7739()) {
            nineDspAttenuateSettings.nativeAttenuate((String) nineSurroundSeekBar.getTag(), nineSurroundSeekBar.getProgress(), nineSurroundSeekBar.getAttenuateStatus(), nineSurroundSeekBar.getInvertStatus());
        } else {
            nineDspAttenuateSettings.nativeAttenuate((String) nineSurroundSeekBar.getTag(), nineSurroundSeekBar.getProgress(), nineSurroundSeekBar.getAttenuateStatus(), nineSurroundSeekBar.getInvertStatus(), change);

        }
    }

    private void linkNativeAttenuate(NineSurroundSeekBar nineSurroundSeekBar, NineSurroundSeekBar nineSurroundSeekBar2, boolean change) {
        if ("gb05".equals(EqUtils.getSkinName()) || EqUtils.isChip7739()) {
            nineDspAttenuateSettings.nativeAttenuate((String) nineSurroundSeekBar.getTag(), nineSurroundSeekBar.getProgress(), nineSurroundSeekBar.getAttenuateStatus(), nineSurroundSeekBar.getInvertStatus());
            nineDspAttenuateSettings.nativeAttenuate((String) nineSurroundSeekBar2.getTag(), nineSurroundSeekBar2.getProgress(), nineSurroundSeekBar2.getAttenuateStatus(), nineSurroundSeekBar2.getInvertStatus());
        } else {
            nineDspAttenuateSettings.nativeAttenuate((String) nineSurroundSeekBar.getTag(), nineSurroundSeekBar.getProgress(), nineSurroundSeekBar.getAttenuateStatus(), nineSurroundSeekBar.getInvertStatus(), change);
            nineDspAttenuateSettings.nativeAttenuate((String) nineSurroundSeekBar2.getTag(), nineSurroundSeekBar2.getProgress(), nineSurroundSeekBar2.getAttenuateStatus(), nineSurroundSeekBar2.getInvertStatus(), change);

        }
    }

    private void linkSetProgress(int i, NineSurroundSeekBar... nineSurroundSeekBarArr) {
        for (NineSurroundSeekBar nineSurroundSeekBar : nineSurroundSeekBarArr) {
            nineSurroundSeekBar.setProgress(i, false);
        }
    }

    private void linkSetMuteStatus(boolean checked, NineSurroundSeekBar... nineSurroundSeekBarArr) {
        for (NineSurroundSeekBar nineSurroundSeekBar : nineSurroundSeekBarArr) {
            nineSurroundSeekBar.setMuteStatus(checked);
        }
    }

    private void linkSetInvertStatus(boolean checked, NineSurroundSeekBar... nineSurroundSeekBarArr) {
        for (NineSurroundSeekBar nineSurroundSeekBar : nineSurroundSeekBarArr) {
            nineSurroundSeekBar.setInvertStatus(checked);
        }
    }

    private void refreshView(NineSurroundSeekBar... nineSurroundSeekBarArr) {
        for (NineSurroundSeekBar nineSurroundSeekBar : nineSurroundSeekBarArr) {
            if (nineSurroundSeekBar != null) {
                nineSurroundSeekBar.setTopNameTwo(nineDspAttenuateSettings.getAttenuate((String) nineSurroundSeekBar.getTag()));
                nineSurroundSeekBar.setProgress(nineDspAttenuateSettings.getAttenuate((String) nineSurroundSeekBar.getTag()), false);
                nineSurroundSeekBar.setMuteStatus(nineDspAttenuateSettings.getMute((String) nineSurroundSeekBar.getTag()));
                nineSurroundSeekBar.setInvertStatus(nineDspAttenuateSettings.getInvert((String) nineSurroundSeekBar.getTag()));
            }
        }
        bLinkLfRf = nineDspAttenuateSettings.getLinkLfRf();
        bLinkLrRr = nineDspAttenuateSettings.getLinkLrRr();
        btnLinkLfRf.setChecked(bLinkLfRf);
        btnLinkLrRr.setChecked(bLinkLrRr);
        ivLineF.setSelected(bLinkLfRf);
        ivLineR.setSelected(bLinkLrRr);
        ivForeFront.setImageDrawable(SkinUtils.getDrawable(centerDrawable[nineDspAttenuateSettings.getAttenuate((String) asbCen.getTag()) + 15]));
        ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[nineDspAttenuateSettings.getAttenuate((String) asbLF.getTag()) + 15]));
        ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[nineDspAttenuateSettings.getAttenuate((String) asbRF.getTag()) + 15]));
        ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[nineDspAttenuateSettings.getAttenuate((String) asbLR.getTag()) + 15]));
        ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[nineDspAttenuateSettings.getAttenuate((String) asbRR.getTag()) + 15]));
        ivRear.setImageDrawable(SkinUtils.getDrawable(subDrawable[nineDspAttenuateSettings.getAttenuate((String) asbSub.getTag()) + 15]));
        ivForeFront.setAlpha(nineDspAttenuateSettings.getMute((String) asbCen.getTag()) ? 0.5f : 1f);
        ivFrontL.setAlpha(nineDspAttenuateSettings.getMute((String) asbLF.getTag()) ? 0.5f : 1f);
        ivFrontR.setAlpha(nineDspAttenuateSettings.getMute((String) asbRF.getTag()) ? 0.5f : 1f);
        ivRearL.setAlpha(nineDspAttenuateSettings.getMute((String) asbLR.getTag()) ? 0.5f : 1f);
        ivRearR.setAlpha(nineDspAttenuateSettings.getMute((String) asbRR.getTag()) ? 0.5f : 1f);
        ivRear.setAlpha(nineDspAttenuateSettings.getMute((String) asbSub.getTag()) ? 0.5f : 1f);
    }

    @Override
    public void onReset() {
        nineDspAttenuateSettings.resetGain();
        refreshView(asbLF, asbRF, asbLR, asbRR, asbSub, asbCen);
        Log.d(TAG, "onReset");
    }
}