package com.hcn.autoeq.nine;

import static android.view.View.GONE;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import com.hcn.autoeq.R;
import com.hcn_library.BaseFragment;
import com.hcn_library.data.NineDspDtsFilterSettings;
import com.hcn_library.data.NineDspDtsSoundFocusSettings;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;
import com.hcn_library.util.SkinUtils;

import com.hcn.autoeq.view.NineCommonSeekBar;

public class NineDtsSoundFocusFragment extends BaseFragment implements SeekBar.OnSeekBarChangeListener, NineConstantExtDsp, NineDtsFilterFragment.EnableViewInterface {
    private static final String TAG = "NineDtsSoundFocusFragment";
    private NineDspDtsFilterSettings filterSettings;
    private ImageView ivForeFront;
    private ImageView ivFrontL;
    private ImageView ivFrontR;
    private ImageView ivRearL;
    private ImageView ivRearR;
    private ImageView ivSoundFocus;
    private View mainView;
    private NineDspDtsSoundFocusSettings nineDspDtsSoundFocusSettings;
    private int progressCenter;
    private int progressFront;
    private int progressRear;
    private NineCommonSeekBar seekbarCenter;
    private NineCommonSeekBar seekbarFront;
    private NineCommonSeekBar seekbarRear;
    private TextView tvReset;
    private int[] centerDrawable = {R.drawable.icon_nine_audio_forefront_00, R.drawable.icon_nine_audio_forefront_01, R.drawable.icon_nine_audio_forefront_02, R.drawable.icon_nine_audio_forefront_03, R.drawable.icon_nine_audio_forefront_04, R.drawable.icon_nine_audio_forefront_05, R.drawable.icon_nine_audio_forefront_06, R.drawable.icon_nine_audio_forefront_07, R.drawable.icon_nine_audio_forefront_08, R.drawable.icon_nine_audio_forefront_09, R.drawable.icon_nine_audio_forefront_10};
    private int[] leftDrawable = {R.drawable.icon_nine_audio_left_00, R.drawable.icon_nine_audio_left_01, R.drawable.icon_nine_audio_left_02, R.drawable.icon_nine_audio_left_03, R.drawable.icon_nine_audio_left_04, R.drawable.icon_nine_audio_left_05, R.drawable.icon_nine_audio_left_06, R.drawable.icon_nine_audio_left_07, R.drawable.icon_nine_audio_left_08, R.drawable.icon_nine_audio_left_09, R.drawable.icon_nine_audio_left_10};
    private int[] rightDrawable = {R.drawable.icon_nine_audio_right_00, R.drawable.icon_nine_audio_right_01, R.drawable.icon_nine_audio_right_02, R.drawable.icon_nine_audio_right_03, R.drawable.icon_nine_audio_right_04, R.drawable.icon_nine_audio_right_05, R.drawable.icon_nine_audio_right_06, R.drawable.icon_nine_audio_right_07, R.drawable.icon_nine_audio_right_08, R.drawable.icon_nine_audio_right_09, R.drawable.icon_nine_audio_right_10};
    private int[] soundFocusDrawable = {R.drawable.icon_nine_sound_focus_06, R.drawable.icon_nine_sound_focus_05, R.drawable.icon_nine_sound_focus_04, R.drawable.icon_nine_sound_focus_03, R.drawable.icon_nine_sound_focus_02, R.drawable.icon_nine_sound_focus_01, R.drawable.icon_nine_sound_focus_00};

    @Override
    public int getLayoutRes() {
        return R.layout.nine_dts_fragment_sound_focus;
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    public static NineDtsSoundFocusFragment newInstance() {
        return new NineDtsSoundFocusFragment();
    }

    @Override
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        mainView = super.onCreateView(layoutInflater, viewGroup, bundle);
        nineDspDtsSoundFocusSettings = NineDspDtsSoundFocusSettings.getInstance(mContext);
        filterSettings = NineDspDtsFilterSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        tvReset = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_sound_reset));
        seekbarFront = (NineCommonSeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_sound_front));
        seekbarRear = (NineCommonSeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_sound_rear));
        seekbarCenter = (NineCommonSeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_sound_center));
        ivForeFront = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_sound_forefront));
        ivFrontL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_sound_front_l));
        ivFrontR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_sound_front_r));
        ivRearL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_sound_rear_l));
        ivRearR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_sound_rear_r));
        ivSoundFocus = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_sound_focus));
        tvReset.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                nineDspDtsSoundFocusSettings.reset();
                refreshView();
                Log.d(TAG, "onClick reset button");
            }
        });
        seekbarFront.setOnSeekBarChangeListener(this);
        seekbarRear.setOnSeekBarChangeListener(this);
        seekbarCenter.setOnSeekBarChangeListener(this);
        if (EqUtils.isChip7739()) {
            ivSoundFocus.setVisibility(filterSettings.getCenterSwitchEnable() ? View.VISIBLE : GONE);
            ivForeFront.setVisibility(filterSettings.getCenterSwitchEnable() ? View.VISIBLE : GONE);
            seekbarFront.setVisibility(GONE);
            seekbarRear.setVisibility(GONE);
            seekbarCenter.getTvDown().setText("");
        }
    }

    @Override
    public void initData() {
        super.initData();
        refreshView();
        if (EqUtils.isChip7739()) {
            updateView(filterSettings.getSoundFocusSwitchEnable());
        } else {
            seekbarCenter.setSeekBarStatus(filterSettings.getCenterSwitch() == 1 && filterSettings.getDtsSwitch() == 0);
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            String seekbarTag = (String) seekBar.getTag();
            int channel = NineConstantExtDsp.NINE_DTS_FOCUS_FRONT_LV;
            if ("Front".equals(seekbarTag)) {
                channel = NineConstantExtDsp.NINE_DTS_FOCUS_FRONT_LV;
                ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / 10]));
                ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / 10]));
            } else if ("Rear".equals(seekbarTag)) {
                channel = NineConstantExtDsp.NINE_DTS_FOCUS_REAR_LV;
                ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / 10]));
                ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / 10]));
            } else if ("Center".equals(seekbarTag)) {
                channel = NineConstantExtDsp.NINE_DTS_FOCUS_CENTER_LV;
                if (EqUtils.isChip7739()) {
                    ivSoundFocus.setImageDrawable(SkinUtils.getDrawable(soundFocusDrawable[seekBar.getProgress() / 16]));
                } else {
                    ivForeFront.setImageDrawable(SkinUtils.getDrawable(centerDrawable[seekBar.getProgress() / 10]));
                }
            }
            if (EqUtils.isChip7739()) {
                nineDspDtsSoundFocusSettings.nativeDTS(channel, progress);
            } else {
                nineDspDtsSoundFocusSettings.nativeDTS(channel, progressToValue(progress));
            }
        }
        Log.d(TAG, "onProgressChanged, fromUser " + fromUser + ", progress = " + progress + " seekBar.getTag(): " + seekBar.getTag());
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        String seekbarTag = (String) seekBar.getTag();
        if ("Front".equals(seekbarTag)) {
            nineDspDtsSoundFocusSettings.saveDtsFocusFrontLv(progressToValue(seekBar.getProgress()));
            ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / 10]));
            ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / 10]));
        } else if ("Rear".equals(seekbarTag)) {
            nineDspDtsSoundFocusSettings.saveDtsFocusRearLv(progressToValue(seekBar.getProgress()));
            ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / 10]));
            ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / 10]));
        } else if ("Center".equals(seekbarTag)) {
            if (EqUtils.isChip7739()) {
                ivSoundFocus.setImageDrawable(SkinUtils.getDrawable(soundFocusDrawable[seekBar.getProgress() / 16]));
                nineDspDtsSoundFocusSettings.saveDtsFocusCenterLv(seekBar.getProgress());
            } else {
                ivForeFront.setImageDrawable(SkinUtils.getDrawable(centerDrawable[seekBar.getProgress() / 10]));
                nineDspDtsSoundFocusSettings.saveDtsFocusCenterLv(progressToValue(seekBar.getProgress()));
            }
        }
        Log.d(TAG, "onStopTrackingTouch, seekBar.getProgress = " + seekBar.getProgress());
    }


    public void refreshView() {
        setViewsEnableStatus();
        progressFront = valueToProgress(nineDspDtsSoundFocusSettings.getDtsFocusFrontLv());
        progressRear = valueToProgress(nineDspDtsSoundFocusSettings.getDtsFocusRearLv());
        progressCenter = valueToProgress(nineDspDtsSoundFocusSettings.getDtsFocusCenterLv());
        seekbarFront.setTopNameTwo(progressFront);
        seekbarRear.setTopNameTwo(progressRear);
        seekbarCenter.setTopNameTwo(progressCenter);
        seekbarFront.setProgress(progressFront, false);
        seekbarRear.setProgress(progressRear, false);
        seekbarCenter.setProgress(progressCenter, false);
        ivForeFront.setImageDrawable(SkinUtils.getDrawable(centerDrawable[progressCenter / 10]));
        ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[progressFront / 10]));
        ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[progressFront / 10]));
        ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[progressRear / 10]));
        ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[progressRear / 10]));
        if (EqUtils.isChip7739() && filterSettings.getSoundFocusSwitchEnable()) {
            ivSoundFocus.setImageDrawable(SkinUtils.getDrawable(soundFocusDrawable[progressCenter / 16]));
        } else {
            ivForeFront.setImageDrawable(SkinUtils.getDrawable(centerDrawable[progressCenter / 10]));
        }
        Log.d(TAG, "refreshView progressFront " + progressFront + ", progressRear " + progressRear + ", progressCenter " + progressCenter);
    }

    private int valueToProgress(int i) {
        if(EqUtils.isChip7739()) {
            return i;
        }
        return (int) Math.round((i * 100) / 32767.0d);
    }

    private int progressToValue(int i) {
        if(EqUtils.isChip7739()) {
            return i;
        }
        return (int) Math.round((i * 32767) / 100.0d);
    }

    @Override
    public void updateView(boolean enable) {
        if (filterSettings == null) return;
        setViewsEnableStatus();
    }

    private void setViewsEnableStatus() {
        boolean enable = false;
        boolean dtsEnable = filterSettings.getDtsSwitch() == 0;
        boolean centerEnable = filterSettings.getCenterSwitch() == 1;
        mainView.setAlpha(dtsEnable ? 1.0f : 0.4f);
        ivForeFront.setAlpha(centerEnable ? 1.0f : 0.4f);
        seekbarFront.setSeekBarStatus(dtsEnable);
        seekbarRear.setSeekBarStatus(dtsEnable);
        if (dtsEnable && centerEnable) {
            enable = true;
        }
        if (EqUtils.isChip7739()) {
            enable = filterSettings.getSoundFocusSwitchEnable();
            dtsEnable = enable;
            ivSoundFocus.setVisibility(enable ? View.VISIBLE : GONE);
            ivForeFront.setVisibility(enable ? View.VISIBLE : GONE);
            mainView.setAlpha(enable ? 1.0f : 0.4f);
            ivForeFront.setAlpha(enable ? 1.0f : 0.4f);
            nineDspDtsSoundFocusSettings.nativeDTS(0, enable ? nineDspDtsSoundFocusSettings.getDtsFocusCenterLv() : 0);
        }
        seekbarCenter.setSeekBarStatus(enable);
        seekbarCenter.setProgressAlpha(enable ? 1.0f : 0.4f);
        tvReset.setEnabled(dtsEnable);
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (!hidden) {
            NineDspDtsFilterSettings.getInstance(mContext);
            setViewsEnableStatus();
        }
        Log.d(TAG, "onHiddenChanged  hidden: " + hidden);
    }
}