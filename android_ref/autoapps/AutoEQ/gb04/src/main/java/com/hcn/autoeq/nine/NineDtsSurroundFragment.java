package com.hcn.autoeq.nine;

import static android.view.View.GONE;
import static android.view.View.VISIBLE;

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
import com.hcn_library.data.NineDspDtsSurroundSettings;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;
import com.hcn_library.util.SkinUtils;

import com.hcn.autoeq.view.NineCommonSeekBar;

public class NineDtsSurroundFragment extends BaseFragment implements SeekBar.OnSeekBarChangeListener, NineConstantExtDsp, NineDtsFilterFragment.EnableViewInterface {
    private static final String TAG = "NineDtsSurroundFragment";
    private NineDspDtsFilterSettings filterSettings;
    private ImageView ivCenterInner;
    private ImageView ivCenterOuter;
    private ImageView ivForeFront;
    private ImageView ivFrontL;
    private ImageView ivFrontR;
    private ImageView ivRearL;
    private ImageView ivRearR;
    private View mainView;
    private NineDspDtsSurroundSettings nineDspDtsSurroundSettings;
    private int progressCenterRear;
    private int progressFrontRear;
    private NineCommonSeekBar seekBarCenterRear;
    private NineCommonSeekBar seekBarFrontRear;
    private TextView tvReset;
    private int[] centerArcDrawable = {R.drawable.nine_audio_bar_05, R.drawable.nine_audio_bar_04, R.drawable.nine_audio_bar_03, R.drawable.nine_audio_bar_02, R.drawable.nine_audio_bar_01, R.drawable.nine_audio_bar_00};
    private int[] centerCircleDrawable = {R.drawable.nine_audio_field_08, R.drawable.nine_audio_field_07, R.drawable.nine_audio_field_06, R.drawable.nine_audio_field_05, R.drawable.nine_audio_field_04, R.drawable.nine_audio_field_03, R.drawable.nine_audio_field_02, R.drawable.nine_audio_field_01, R.drawable.nine_audio_field_00};

    @Override
    public int getLayoutRes() {
        return R.layout.nine_dts_fragment_surround;
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    public static NineDtsSurroundFragment newInstance() {
        return new NineDtsSurroundFragment();
    }

    @Override
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        mainView = super.onCreateView(layoutInflater, viewGroup, bundle);
        nineDspDtsSurroundSettings = NineDspDtsSurroundSettings.getInstance(mContext);
        filterSettings = NineDspDtsFilterSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        tvReset = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_dts_surround_reset));
        seekBarFrontRear = (NineCommonSeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_dts_surround_front_rear));
        seekBarCenterRear = (NineCommonSeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_dts_surround_center_rear));
        ivForeFront = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_surround_forefront));
        ivFrontL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_surround_front_l));
        ivFrontR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_surround_front_r));
        ivRearL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_surround_rear_l));
        ivRearR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_surround_rear_r));
        ivCenterInner = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_surround_circle_inner));
        ivCenterOuter = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_surround_circle_outer));
        tvReset.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                nineDspDtsSurroundSettings.reset();
                refreshView();
                Log.d(TAG, "onClick reset button");
            }
        });
        seekBarFrontRear.setOnSeekBarChangeListener(this);
        seekBarCenterRear.setOnSeekBarChangeListener(this);
        if (EqUtils.isChip7739()) {
            seekBarFrontRear.setVisibility(GONE);
            seekBarCenterRear.getTvDown().setText("");
        }
    }

    @Override
    public void initData() {
        super.initData();
        refreshView();
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            int channel = NineConstantExtDsp.NINE_DTS_FOCUS_FRONT_REAR_LV;
            String seekBarTag = (String) seekBar.getTag();
            if ("FrontRear".equals(seekBarTag)) {
                channel = NineConstantExtDsp.NINE_DTS_FOCUS_FRONT_REAR_LV;
                ivCenterInner.setImageDrawable(SkinUtils.getDrawable(centerCircleDrawable[seekBar.getProgress() / 12]));
            } else if ("CenterRear".equals(seekBarTag)) {
                channel = NineConstantExtDsp.NINE_DTS_FOCUS_CENTER_REAR_LV;
                ivCenterOuter.setImageDrawable(SkinUtils.getDrawable(centerArcDrawable[seekBar.getProgress() / 20]));
            }
            nineDspDtsSurroundSettings.nativeDTS(channel, progressToValue(progress));
        }
        Log.d(TAG, "onProgressChanged, fromUser " + fromUser + ", progress = " + progress);
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        String seekBarTag = (String) seekBar.getTag();
        if ("FrontRear".equals(seekBarTag)) {
            nineDspDtsSurroundSettings.saveDtsSurroundFrontFrontLv(progressToValue(seekBar.getProgress()));
            ivCenterInner.setImageDrawable(SkinUtils.getDrawable(centerCircleDrawable[seekBar.getProgress() / 12]));
        } else if ("CenterRear".equals(seekBarTag)) {
            nineDspDtsSurroundSettings.saveDtsSurroundCenterRearLv(progressToValue(seekBar.getProgress()));
            ivCenterOuter.setImageDrawable(SkinUtils.getDrawable(centerArcDrawable[seekBar.getProgress() / 20]));
        }
        Log.d(TAG, "onStopTrackingTouch, seekBar.getProgress = " + seekBar.getProgress());
    }

    public void refreshView() {
        setViewsEnableStatus();
        progressFrontRear = valueToProgress(nineDspDtsSurroundSettings.getDtsSurroundFrontFrontLv());
        progressCenterRear = valueToProgress(nineDspDtsSurroundSettings.getDtsSurroundCenterRearLv());
        seekBarFrontRear.setTopNameTwo(progressFrontRear);
        seekBarCenterRear.setTopNameTwo(progressCenterRear);
        seekBarFrontRear.setProgress(progressFrontRear, false);
        seekBarCenterRear.setProgress(progressCenterRear, false);
        ivCenterInner.setImageDrawable(SkinUtils.getDrawable(centerCircleDrawable[progressFrontRear / 12]));
        ivCenterOuter.setImageDrawable(SkinUtils.getDrawable(centerArcDrawable[progressCenterRear / 20]));
        Log.d(TAG, "refreshView progressFrontRear " + progressFrontRear + ", progressCenterRear " + progressCenterRear);
    }

    private int valueToProgress(int i) {
        if(EqUtils.isChip7739()) {
            return i;
        }
        Log.d(TAG, "valueToProgress i " + i + " " + Math.round((i * 100) / 32767.0d));
        return (int) Math.round((i * 100) / 32767.0d);
    }

    private int progressToValue(int i) {
        if(EqUtils.isChip7739()) {
            return i;
        }
        Log.d(TAG, "progressToValue i " + i + " " + Math.round((i * 32767) / 100.0d));
        return (int) Math.round((i * 32767) / 100.0d);
    }

    @Override
    public void updateView(boolean enable) {
        if (filterSettings == null) return;
        setViewsEnableStatus();
    }

    private void setViewsEnableStatus() {
        boolean dtsEnable = filterSettings.getDtsSwitch() == 0;
        boolean centerEnable = filterSettings.getCenterSwitch() == 1;
        if (EqUtils.isChip7739()) {
            boolean surroundEnable = filterSettings.getSurroundSwitchEnable();
            ivForeFront.setVisibility(surroundEnable ? VISIBLE : GONE);
            ivCenterInner.setVisibility(surroundEnable ? VISIBLE : GONE);
            ivCenterOuter.setVisibility(surroundEnable ? VISIBLE : GONE);
            ivForeFront.setAlpha(surroundEnable ? 1f : 0.4f);
            mainView.setAlpha(surroundEnable ? 1.0f : 0.4f);
            dtsEnable = surroundEnable;
            nineDspDtsSurroundSettings.nativeDTS(0, surroundEnable ? nineDspDtsSurroundSettings.getDtsSurroundCenterRearLv() : 0);
        } else {
            ivForeFront.setAlpha(centerEnable ? 1f : 0.4f);
            mainView.setAlpha(dtsEnable ? 1.0f : 0.4f);
        }
        seekBarFrontRear.setSeekBarStatus(dtsEnable);
        seekBarCenterRear.setSeekBarStatus(dtsEnable);
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