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
import com.hcn_library.data.NineDspDtsBassBoostSettings;
import com.hcn_library.data.NineDspDtsFilterSettings;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;
import com.hcn_library.util.SkinUtils;

import com.hcn.autoeq.view.NineSeekBarWithFrequency;

public class NineDtsBassBoostFragment extends BaseFragment implements SeekBar.OnSeekBarChangeListener, NineConstantExtDsp, NineDtsFilterFragment.EnableViewInterface {
    private static final String TAG = "NineDtsBassBoostFragment";
    private ImageView downFront;
    private ImageView downRear;
    private ImageView downSub;
    private NineDspDtsFilterSettings filterSettings;
    private ImageView ivFrontL;
    private ImageView ivFrontR;
    private ImageView ivRear;
    private ImageView ivForefront;
    private ImageView ivRearL;
    private ImageView ivRearR;
    private ImageView ivBassBoost;
    private View mainView;
    private ImageView moveDown;
    private ImageView moveUp;
    private NineDspDtsBassBoostSettings nineDspDtsBassBoostSettings;
    private int progressFront;
    private int progressRear;
    private int progressSub;
    private NineSeekBarWithFrequency seekbarFront;
    private NineSeekBarWithFrequency seekbarRear;
    private NineSeekBarWithFrequency seekbarSub;
    private TextView tvReset;
    private TextView tvFreqTitle;
    private ImageView upFront;
    private ImageView upRear;
    private ImageView upSub;
    private int currentIndexFront = 0;
    private int currentIndexRear = 0;
    private int currentIndexSub = 0;
    private int[] centerDrawable = {R.drawable.nine_bass_audio_rear_00, R.drawable.nine_bass_audio_rear_01, R.drawable.nine_bass_audio_rear_02, R.drawable.nine_bass_audio_rear_03, R.drawable.nine_bass_audio_rear_04, R.drawable.nine_bass_audio_rear_05, R.drawable.nine_bass_audio_rear_06, R.drawable.nine_bass_audio_rear_07, R.drawable.nine_bass_audio_rear_08, R.drawable.nine_bass_audio_rear_09, R.drawable.nine_bass_audio_rear_10};
    private int[] leftDrawable = {R.drawable.nine_bass_audio_left_00, R.drawable.nine_bass_audio_left_01, R.drawable.nine_bass_audio_left_02, R.drawable.nine_bass_audio_left_03, R.drawable.nine_bass_audio_left_04, R.drawable.nine_bass_audio_left_05, R.drawable.nine_bass_audio_left_06, R.drawable.nine_bass_audio_left_07, R.drawable.nine_bass_audio_left_08, R.drawable.nine_bass_audio_left_09, R.drawable.nine_bass_audio_left_10};
    private int[] rightDrawable = {R.drawable.nine_bass_audio_right_00, R.drawable.nine_bass_audio_right_01, R.drawable.nine_bass_audio_right_02, R.drawable.nine_bass_audio_right_03, R.drawable.nine_bass_audio_right_04, R.drawable.nine_bass_audio_right_05, R.drawable.nine_bass_audio_right_06, R.drawable.nine_bass_audio_right_07, R.drawable.nine_bass_audio_right_08, R.drawable.nine_bass_audio_right_09, R.drawable.nine_bass_audio_right_10};
    private int[] bassBoostDrawable = {R.drawable.icon_nine_bass_boost_00, R.drawable.icon_nine_bass_boost_01, R.drawable.icon_nine_bass_boost_02, R.drawable.icon_nine_bass_boost_03, R.drawable.icon_nine_bass_boost_04, R.drawable.icon_nine_bass_boost_05, R.drawable.icon_nine_bass_boost_06, R.drawable.icon_nine_bass_boost_07, R.drawable.icon_nine_bass_boost_08, R.drawable.icon_nine_bass_boost_09, R.drawable.icon_nine_bass_boost_10};

    private int currentFreq;

    @Override
    public int getLayoutRes() {
        return R.layout.nine_dts_fragment_bass_boost;
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    public static NineDtsBassBoostFragment newInstance() {
        return new NineDtsBassBoostFragment();
    }

    @Override
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        mainView = super.onCreateView(layoutInflater, viewGroup, bundle);
        nineDspDtsBassBoostSettings = NineDspDtsBassBoostSettings.getInstance(mContext);
        filterSettings = NineDspDtsFilterSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        tvReset = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_dts_bass_reset));
        tvFreqTitle = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_freq_title));
        seekbarFront = (NineSeekBarWithFrequency) mainView.findViewById(SkinUtils.getId(R.id.sb_dts_bass_front));
        seekbarRear = (NineSeekBarWithFrequency) mainView.findViewById(SkinUtils.getId(R.id.sb_dts_bass_rear));
        seekbarSub = (NineSeekBarWithFrequency) mainView.findViewById(SkinUtils.getId(R.id.sb_dts_bass_bass_boost));
        ivRear = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_bass_rear));
        ivForefront = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_bass_forefront));
        ivFrontL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_bass_front_l));
        ivFrontR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_bass_front_r));
        ivRearL = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_bass_rear_l));
        ivRearR = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_dts_bass_rear_r));
        ivBassBoost = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_bass_boost));
        upFront = seekbarFront.getMoveUp();
        upRear = seekbarRear.getMoveUp();
        upSub = seekbarSub.getMoveUp();
        downFront = seekbarFront.getMoveDown();
        downRear = seekbarRear.getMoveDown();
        downSub = seekbarSub.getMoveDown();
        seekbarFront.setScrollResultInterface(new NineSeekBarWithFrequency.ScrollResultInterface() {
            @Override
            public void itemChange(int currentIndex) {
                nineDspDtsBassBoostSettings.saveDtsBassFrontFREQ(currentIndex);
                nineDspDtsBassBoostSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_BASS_FRONT_FREQ, currentIndex);
            }
        });
        seekbarRear.setScrollResultInterface(new NineSeekBarWithFrequency.ScrollResultInterface() {
            @Override
            public void itemChange(int currentIndex) {
                nineDspDtsBassBoostSettings.saveDtsBassRearFREQ(currentIndex);
                nineDspDtsBassBoostSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_BASS_REAR_FREQ, currentIndex);
            }
        });
        seekbarSub.setScrollResultInterface(new NineSeekBarWithFrequency.ScrollResultInterface() {
            @Override
            public void itemChange(int currentIndex) {
                nineDspDtsBassBoostSettings.saveDtsBassBassFREQ(currentIndex);
                nineDspDtsBassBoostSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_BASS_SUB_FREQ, currentIndex);
            }
        });
        downFront.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                moveUpOrDown(nineDspDtsBassBoostSettings.getDtsBassFrontFREQ(), seekbarFront, false);
            }
        });
        downRear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                moveUpOrDown(nineDspDtsBassBoostSettings.getDtsBassRearFREQ(), seekbarRear, false);
            }
        });
        downSub.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                moveUpOrDown(nineDspDtsBassBoostSettings.getDtsBassBassFREQ(), seekbarSub, false);
            }
        });
        upFront.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                moveUpOrDown(nineDspDtsBassBoostSettings.getDtsBassFrontFREQ(), seekbarFront, true);
            }
        });
        upRear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                moveUpOrDown(nineDspDtsBassBoostSettings.getDtsBassRearFREQ(), seekbarRear, true);
            }
        });
        upSub.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                moveUpOrDown(nineDspDtsBassBoostSettings.getDtsBassBassFREQ(), seekbarSub, true);
            }
        });
        tvReset.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                nineDspDtsBassBoostSettings.reset();
                refreshView();
                Log.d(TAG, "onClick reset button");
            }
        });
        seekbarFront.setOnSeekBarChangeListener(this);
        seekbarRear.setOnSeekBarChangeListener(this);
        seekbarSub.setOnSeekBarChangeListener(this);

        if (EqUtils.isChip7739()) {
            tvFreqTitle.setVisibility(GONE);
            ivRear.setVisibility(GONE);
            ivForefront.setVisibility(filterSettings.getCenterSwitchEnable() ? VISIBLE : GONE);
            ivForefront.setImageDrawable(SkinUtils.getDrawable(R.drawable.nine_bass_audio_forefront_03));
            seekbarFront.setVisibility(GONE);
            seekbarRear.setVisibility(GONE);
            seekbarSub.getTvDown().setText("");
            seekbarSub.hideFreq();
            ivBassBoost.setVisibility(VISIBLE);
            mainView.findViewById(SkinUtils.getId(R.id.iv_dts_bass_bg_bottom)).setVisibility(GONE);
            mainView.findViewById(SkinUtils.getId(R.id.iv_dts_bass_bg_top)).setVisibility(GONE);
            seekbarSub.setFreqCallback(new NineSeekBarWithFrequency.FreqCallback() {
                @Override
                public void setFreqValue(int value) {
                    currentFreq = value;
                    nineDspDtsBassBoostSettings.saveDtsBassBassFREQ(value);
                    nineDspDtsBassBoostSettings.nativeDTS(seekbarSub.getProgress(), value);
                }
            });
            seekbarSub.initFreqStatus(nineDspDtsBassBoostSettings.getDtsBassBassFREQ());
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
            String seekBarTag = (String) seekBar.getTag();
            int channel = NineConstantExtDsp.NINE_DTS_BASS_FRONT_LV;
            if ("Front".equals(seekBarTag)) {
                channel = NineConstantExtDsp.NINE_DTS_BASS_FRONT_LV;
                ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / 10]));
                ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / 10]));
            } else if ("Rear".equals(seekBarTag)) {
                channel = NineConstantExtDsp.NINE_DTS_BASS_REAR_LV;
                ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / 10]));
                ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / 10]));
            } else if ("Subwoofer".equals(seekBarTag)) {
                channel = NineConstantExtDsp.NINE_DTS_BASS_SUB_LV;
                ivRear.setImageDrawable(SkinUtils.getDrawable(centerDrawable[seekBar.getProgress() / 10]));
                if (EqUtils.isChip7739()) {
                    ivBassBoost.setImageDrawable(SkinUtils.getDrawable(bassBoostDrawable[seekBar.getProgress() / 10]));
                }
            }
            if (EqUtils.isChip7739()) {
                nineDspDtsBassBoostSettings.nativeDTS(progressToValue(progress), currentFreq);
            } else {
                nineDspDtsBassBoostSettings.nativeDTS(channel, progressToValue(progress));
            }
        }
        Log.d(TAG, "onProgressChanged, fromUser " + fromUser + ", progress = " + progress);
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        String seekBarTag = (String) seekBar.getTag();
        if ("Front".equals(seekBarTag)) {
            nineDspDtsBassBoostSettings.saveDtsBassFrontLv(progressToValue(seekBar.getProgress()));
            ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / 10]));
            ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / 10]));
        } else if ("Rear".equals(seekBarTag)) {
            nineDspDtsBassBoostSettings.saveDtsBassRearLv(progressToValue(seekBar.getProgress()));
            ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[seekBar.getProgress() / 10]));
            ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[seekBar.getProgress() / 10]));
        } else if ("Subwoofer".equals(seekBarTag)) {
            nineDspDtsBassBoostSettings.saveDtsBassBassLv(progressToValue(seekBar.getProgress()));
            ivRear.setImageDrawable(SkinUtils.getDrawable(centerDrawable[seekBar.getProgress() / 10]));
            if (EqUtils.isChip7739()) {
                ivBassBoost.setImageDrawable(SkinUtils.getDrawable(bassBoostDrawable[seekBar.getProgress() / 10]));
            }
        }
        Log.d(TAG, "onStopTrackingTouch, seekBar.getProgress = " + seekBar.getProgress());
    }


    public void refreshView() {
        setViewsEnableStatus();
        currentIndexFront = nineDspDtsBassBoostSettings.getDtsBassFrontFREQ();
        currentIndexRear = nineDspDtsBassBoostSettings.getDtsBassRearFREQ();
        currentIndexSub = nineDspDtsBassBoostSettings.getDtsBassBassFREQ();
        seekbarFront.setTvFreq(currentIndexFront);
        seekbarRear.setTvFreq(currentIndexRear);
        seekbarSub.setTvFreq(currentIndexSub);
        progressFront = valueToProgress(nineDspDtsBassBoostSettings.getDtsBassFrontLv());
        progressRear = valueToProgress(nineDspDtsBassBoostSettings.getDtsBassRearLv());
        progressSub = valueToProgress(nineDspDtsBassBoostSettings.getDtsBassBassLv());
        seekbarFront.setTopNameTwo(progressFront + "");
        seekbarRear.setTopNameTwo(progressRear + "");
        seekbarSub.setTopNameTwo(progressSub + "");
        seekbarFront.setProgress(progressFront, false);
        seekbarRear.setProgress(progressRear, false);
        seekbarSub.setProgress(progressSub, false);
        ivRear.setImageDrawable(SkinUtils.getDrawable(centerDrawable[progressSub / 10]));
        if (EqUtils.isChip7739()) {
            ivBassBoost.setImageDrawable(SkinUtils.getDrawable(bassBoostDrawable[progressSub / 10]));
        }
        ivFrontL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[progressFront / 10]));
        ivFrontR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[progressFront / 10]));
        ivRearL.setImageDrawable(SkinUtils.getDrawable(leftDrawable[progressRear / 10]));
        ivRearR.setImageDrawable(SkinUtils.getDrawable(rightDrawable[progressRear / 10]));
        Log.d(TAG, "refreshView progressFront " + progressFront + ", progressRear " + progressRear + ", progressSub " + progressSub);
    }

    private int valueToProgress(int i) {
        if (EqUtils.isChip7739()) {
            return i;
        }
        return (int) Math.round((i * 100) / 32767.0d);
    }

    private int progressToValue(int i) {
        if (EqUtils.isChip7739()) {
            return i;
        }
        return (int) Math.round((i * 32767) / 100.0d);
    }


    public void moveUpOrDown(int currentIndex, NineSeekBarWithFrequency nineSeekBarWithFrequency, boolean isUp) {
        int indexLimit = nineDspDtsBassBoostSettings.optionsArray.length - 1;
        if (currentIndex >= 0 && currentIndex <= indexLimit) {
            int recycleIndex = isUp ? (currentIndex - 1) : (currentIndex + 1);
            if (recycleIndex < 0) {
                recycleIndex = indexLimit;
            }
            if (recycleIndex > indexLimit) {
                recycleIndex = 0;
            }
            nineSeekBarWithFrequency.setTvFreq(recycleIndex);
            if (nineSeekBarWithFrequency.getId() == SkinUtils.getId(R.id.sb_dts_bass_front)) {
                nineDspDtsBassBoostSettings.saveDtsBassFrontFREQ(recycleIndex);
                nineDspDtsBassBoostSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_BASS_FRONT_FREQ, recycleIndex);
            } else if (nineSeekBarWithFrequency.getId() == SkinUtils.getId(R.id.sb_dts_bass_rear)) {
                nineDspDtsBassBoostSettings.saveDtsBassRearFREQ(recycleIndex);
                nineDspDtsBassBoostSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_BASS_REAR_FREQ, recycleIndex);
            } else if (nineSeekBarWithFrequency.getId() == SkinUtils.getId(R.id.sb_dts_bass_bass_boost)) {
                nineDspDtsBassBoostSettings.saveDtsBassBassFREQ(recycleIndex);
                nineDspDtsBassBoostSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_BASS_SUB_FREQ, recycleIndex);
            }
        }
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
            centerEnable = filterSettings.getBassBoostSwitchEnable();
            ivForefront.setVisibility(centerEnable ? VISIBLE : GONE);
            ivBassBoost.setVisibility(centerEnable ? VISIBLE : GONE);
            dtsEnable = centerEnable;
            nineDspDtsBassBoostSettings.nativeDTS(centerEnable ? nineDspDtsBassBoostSettings.getDtsBassBassLv() : 0, centerEnable ? currentFreq : 0);
        }
        ivForefront.setAlpha(centerEnable ? 1f : 0.4f);
        mainView.setAlpha(dtsEnable ? 1.0f : 0.4f);
        seekbarFront.setSeekBarStatus(dtsEnable);
        seekbarRear.setSeekBarStatus(dtsEnable);
        seekbarSub.setSeekBarStatus(dtsEnable);
        upFront.setEnabled(dtsEnable);
        upRear.setEnabled(dtsEnable);
        upSub.setEnabled(dtsEnable);
        downFront.setEnabled(dtsEnable);
        downRear.setEnabled(dtsEnable);
        downSub.setEnabled(dtsEnable);
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