package com.hcn.autoeq.nine;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.SeekBar;

import com.hcn.autoeq.R;
import com.hcn.autoeq.view.NineDelaySeekBar;
import com.hcn_library.BaseFragment;
import com.hcn_library.data.NineDspAttenuateSettings;
import com.hcn_library.data.NineDspDelaySettings;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;
import com.hcn_library.util.SkinUtils;

public class NineSurroundDelayFragment extends BaseFragment implements SeekBar.OnSeekBarChangeListener, NineConstantExtDsp, NineSurroundFilterFragment.FragmentResetInterface {
    private static final String TAG = "NineSurroundDelayFragment";
    private static final int shiftPara = "gb05".equals(EqUtils.getSkinName()) ? 13 : 7; // 7604c最大200 * 0.1ms，si47925dts 最大100 * 0.1ms,对应从时间值换算到对应图片的下标，需要转换
    private NineDelaySeekBar asbCen;
    private NineDelaySeekBar asbLF;
    private NineDelaySeekBar asbLR;
    private NineDelaySeekBar asbRF;
    private NineDelaySeekBar asbRR;
    private NineDelaySeekBar asbSub;
    private ImageView ivForeFront;
    private ImageView ivFrontL;
    private ImageView ivFrontR;
    private ImageView ivRear;
    private ImageView ivRearL;
    private ImageView ivRearR;
    private View mainView;
    private NineDspDelaySettings nineDspDelaySettings;
    private int[] subDrawable = {R.drawable.icon_nine_surround_gain_audio_rear_00, R.drawable.icon_nine_surround_gain_audio_rear_01, R.drawable.icon_nine_surround_gain_audio_rear_02, R.drawable.icon_nine_surround_gain_audio_rear_03, R.drawable.icon_nine_surround_gain_audio_rear_04, R.drawable.icon_nine_surround_gain_audio_rear_05, R.drawable.icon_nine_surround_gain_audio_rear_06, R.drawable.icon_nine_surround_gain_audio_rear_07, R.drawable.icon_nine_surround_gain_audio_rear_08, R.drawable.icon_nine_surround_gain_audio_rear_09, R.drawable.icon_nine_surround_gain_audio_rear_10, R.drawable.icon_nine_surround_gain_audio_rear_11, R.drawable.icon_nine_surround_gain_audio_rear_12, R.drawable.icon_nine_surround_gain_audio_rear_13, R.drawable.icon_nine_surround_gain_audio_rear_14, R.drawable.icon_nine_surround_gain_audio_rear_15};
    private int[] centerDrawable = {R.drawable.icon_nine_surround_gain_audio_forefront_00, R.drawable.icon_nine_surround_gain_audio_forefront_01, R.drawable.icon_nine_surround_gain_audio_forefront_02, R.drawable.icon_nine_surround_gain_audio_forefront_03, R.drawable.icon_nine_surround_gain_audio_forefront_04, R.drawable.icon_nine_surround_gain_audio_forefront_05, R.drawable.icon_nine_surround_gain_audio_forefront_06, R.drawable.icon_nine_surround_gain_audio_forefront_07, R.drawable.icon_nine_surround_gain_audio_forefront_08, R.drawable.icon_nine_surround_gain_audio_forefront_09, R.drawable.icon_nine_surround_gain_audio_forefront_10, R.drawable.icon_nine_surround_gain_audio_forefront_11, R.drawable.icon_nine_surround_gain_audio_forefront_12, R.drawable.icon_nine_surround_gain_audio_forefront_13, R.drawable.icon_nine_surround_gain_audio_forefront_14, R.drawable.icon_nine_surround_gain_audio_forefront_15};
    private int[] leftDrawable = {R.drawable.icon_nine_surround_gain_audio_left_00, R.drawable.icon_nine_surround_gain_audio_left_01, R.drawable.icon_nine_surround_gain_audio_left_02, R.drawable.icon_nine_surround_gain_audio_left_03, R.drawable.icon_nine_surround_gain_audio_left_04, R.drawable.icon_nine_surround_gain_audio_left_05, R.drawable.icon_nine_surround_gain_audio_left_06, R.drawable.icon_nine_surround_gain_audio_left_07, R.drawable.icon_nine_surround_gain_audio_left_08, R.drawable.icon_nine_surround_gain_audio_left_09, R.drawable.icon_nine_surround_gain_audio_left_10, R.drawable.icon_nine_surround_gain_audio_left_11, R.drawable.icon_nine_surround_gain_audio_left_12, R.drawable.icon_nine_surround_gain_audio_left_13, R.drawable.icon_nine_surround_gain_audio_left_14, R.drawable.icon_nine_surround_gain_audio_left_15};
    private int[] rightDrawable = {R.drawable.icon_nine_surround_gain_audio_right_00, R.drawable.icon_nine_surround_gain_audio_right_01, R.drawable.icon_nine_surround_gain_audio_right_02, R.drawable.icon_nine_surround_gain_audio_right_03, R.drawable.icon_nine_surround_gain_audio_right_04, R.drawable.icon_nine_surround_gain_audio_right_05, R.drawable.icon_nine_surround_gain_audio_right_06, R.drawable.icon_nine_surround_gain_audio_right_07, R.drawable.icon_nine_surround_gain_audio_right_08, R.drawable.icon_nine_surround_gain_audio_right_09, R.drawable.icon_nine_surround_gain_audio_right_10, R.drawable.icon_nine_surround_gain_audio_right_11, R.drawable.icon_nine_surround_gain_audio_right_12, R.drawable.icon_nine_surround_gain_audio_right_13, R.drawable.icon_nine_surround_gain_audio_right_14, R.drawable.icon_nine_surround_gain_audio_right_15};

    public static NineSurroundDelayFragment newInstance() {
        return new NineSurroundDelayFragment();
    }

    @Override
    public int getLayoutRes() {
        return R.layout.nine_surround_fragment_delay;
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    @Override
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        mainView = super.onCreateView(layoutInflater, viewGroup, bundle);
        nineDspDelaySettings = NineDspDelaySettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        asbLF = (NineDelaySeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_delay_front_l));
        asbRF = (NineDelaySeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_delay_front_r));
        asbLR = (NineDelaySeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_delay_rear_l));
        asbRR = (NineDelaySeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_delay_rear_r));
        asbSub = (NineDelaySeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_delay_bass));
        asbCen = (NineDelaySeekBar) mainView.findViewById(SkinUtils.getId(R.id.sb_delay_center));
        ivForeFront = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_delay_forefront));
        ivFrontL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_delay_front_l));
        ivFrontR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_delay_front_r));
        ivRearL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_delay_rear_l));
        ivRearR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_delay_rear_r));
        ivRear = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_delay_rear));
        asbLF.setOnSeekBarChangeListener(this);
        asbRF.setOnSeekBarChangeListener(this);
        asbLR.setOnSeekBarChangeListener(this);
        asbRR.setOnSeekBarChangeListener(this);
        asbSub.setOnSeekBarChangeListener(this);
        asbCen.setOnSeekBarChangeListener(this);
    }

    @Override
    public void initData() {
        super.initData();
        refreshView(asbLF, asbRF, asbLR, asbRR, asbSub, asbCen);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        String str = (String) seekBar.getTag();
        Log.d(TAG, "onProgressChanged: progress = " + progress + " channel = " + str + " fromUser = " + fromUser + " seekBar.getTag(): " + seekBar.getTag());
        int channelId = 0;
        if ("LF".equals(str)) {
            channelId = 0;
            ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / shiftPara]));
        } else if ("RF".equals(str)) {
            channelId = 1;
            ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / shiftPara]));
        } else if ("LR".equals(str)) {
            channelId = 2;
            ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / shiftPara]));
        } else if ("RR".equals(str)) {
            channelId = 3;
            ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / shiftPara]));
        } else if ("CENTER".equals(str)) {
            channelId = 4;
            ivForeFront.setImageDrawable(SkinUtils.getDrawable(centerDrawable[seekBar.getProgress() / shiftPara]));
        } else if ("SUBWOOFER".equals(str)) {
            channelId = 5;
            ivRear.setImageDrawable(SkinUtils.getDrawable(subDrawable[seekBar.getProgress() / shiftPara]));
        }
        if (fromUser) {
            nineDspDelaySettings.saveDelay((String) seekBar.getTag(), progress); // 需要先保存数据，后续发送数据可能要用到保存的数据
            if ("gb05".equals(EqUtils.getSkinName()) || EqUtils.isChip7739()) {
                nineDspDelaySettings.nativeDelay7604C();
            } else {
                nineDspDelaySettings.nativeDelay(channelId, progress, NineDspAttenuateSettings.getInstance(mContext).getInvert(str) ? 1 : 0); // 进度条的值直接对应了指令需要的值
            }
        }
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        String str = (String) seekBar.getTag();
        Log.d(TAG, "onStopTrackingTouch: channel = " + str);
        if ("LF".equals(str)) {
            ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / shiftPara]));
        } else if ("RF".equals(str)) {
            ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / shiftPara]));
        } else if ("LR".equals(str)) {
            ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / shiftPara]));
        } else if ("RR".equals(str)) {
            ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / shiftPara]));
        } else if ("CENTER".equals(str)) {
            ivForeFront.setImageDrawable(SkinUtils.getDrawable(centerDrawable[seekBar.getProgress() / shiftPara]));
        } else if ("SUBWOOFER".equals(str)) {
            ivRear.setImageDrawable(SkinUtils.getDrawable(subDrawable[seekBar.getProgress() / shiftPara]));
        }
    }


    private void refreshView(NineDelaySeekBar... nineDelaySeekBarArr) {
        for (NineDelaySeekBar nineDelaySeekBar : nineDelaySeekBarArr) {
            if (nineDelaySeekBar != null) {
                nineDelaySeekBar.SUFFIX_1 = "ms";
                nineDelaySeekBar.SUFFIX_2 = "cm";
                nineDelaySeekBar.setProgress(nineDspDelaySettings.getDelay((String) nineDelaySeekBar.getTag()), false);
                nineDelaySeekBar.setTopNameOne(nineDspDelaySettings.getDelay((String) nineDelaySeekBar.getTag()));
            }
        }
        ivForeFront.setImageDrawable(SkinUtils.getDrawable(centerDrawable[nineDspDelaySettings.getDelay((String) asbCen.getTag()) / shiftPara]));
        ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[nineDspDelaySettings.getDelay((String) asbLF.getTag()) / shiftPara]));
        ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[nineDspDelaySettings.getDelay((String) asbRF.getTag()) / shiftPara]));
        ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[nineDspDelaySettings.getDelay((String) asbLR.getTag()) / shiftPara]));
        ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[nineDspDelaySettings.getDelay((String) asbRR.getTag()) / shiftPara]));
        ivRear.setImageDrawable(SkinUtils.getDrawable(subDrawable[nineDspDelaySettings.getDelay((String) asbSub.getTag()) / shiftPara]));
    }

    @Override
    public void onReset() {
        nineDspDelaySettings.resetDelay();
        refreshView(asbLF, asbRF, asbLR, asbRR, asbSub, asbCen);
    }
}