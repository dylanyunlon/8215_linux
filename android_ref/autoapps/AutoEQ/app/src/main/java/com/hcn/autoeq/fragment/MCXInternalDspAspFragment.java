package com.hcn.autoeq.fragment;

import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.auto.hequalizer.EqualizerSurface;
import com.auto.hequalizer.OnCenterFreqChangedListener;
import com.auto.hequalizer.UIMode;
import com.hcn.autoeq.R;
import com.hcn.autoeq.data.AspSettings;
import com.hcn.autoeq.data.DspInternalSettings;
import com.hcn.autoeq.util.ConstantDsp;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.ExtDspBandSeekBar;
import com.hcn.autoeq.view.SparkView;
import com.hcn.common.misc.LogUtils;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.text.DecimalFormat;
import java.util.Arrays;

public class MCXInternalDspAspFragment extends BaseFragment
        implements RadioGroup.OnCheckedChangeListener, ConstantDsp, SeekBar.OnSeekBarChangeListener {

    private final static String TAG = MCXInternalDspAspFragment.class.getSimpleName();
    private static final String ARG_PARAM1 = "param1";
    private static final String ARG_PARAM2 = "param2";
    private View mDspMainView;
    private RadioGroup mDspTabGroup;
    private ConstraintLayout layoutEqualizer, layoutSurround, layoutSubwoofer;
    private EqualizerSurface mEqualizerSurface;
    private SparkView sparkView;
    private CheckBox mSurroundCheck, mLoudCheck, mPowerCheck;
    private TextView mSourceText;
    private SeekBar mSubwooferSeek;
    private LinearLayout llSeekBar = null;

    private final int bandTotal = EqUtils.DSP_BAND_DEPTH;    // 中心频率个数
    private final int maxGainDB = 10;  // 频段最大增益
    private AspSettings mAspDataSettings;
    private DspInternalSettings mDspInternalSettings;
    private String mParam2;
    private int[] mDspBandData;

    public MCXInternalDspAspFragment() {
        // Required empty public constructor
    }

    public static MCXInternalDspAspFragment newInstance(int[] param1, String param2) {
        MCXInternalDspAspFragment fragment = new MCXInternalDspAspFragment();
        Bundle args = new Bundle();
        args.putIntArray(ARG_PARAM1, param1);
        args.putString(ARG_PARAM2, param2);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getArguments() != null) {
            mDspBandData = getArguments().getIntArray(ARG_PARAM1);
            mParam2 = getArguments().getString(ARG_PARAM2);
        }
    }

    @Override
    public int getLayoutRes() {
        return R.layout.internal_dsp_asp_fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mDspMainView = super.onCreateView(inflater, container, savedInstanceState);
        mAspDataSettings = AspSettings.getInstance(mContext);
        mDspInternalSettings = DspInternalSettings.getInstance(mContext);
        return mDspMainView;
    }

    @Override
    public void onPause() {
        super.onPause();
        // 这个保存动作不能放到 onStop 里，会大概率性保存不了（猜测：可能 Activity 销毁，对象没了）
        mDspInternalSettings.saveDspBandValue(Arrays.toString(mDspBandData));
        if (sparkView != null) {
            sparkView.stopDraw();
        }
    }

    @Override
    public void initView() {
        LinearLayout llBand = mDspMainView.findViewById(SkinUtils.getId(R.id.ll_band));
        llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.extdsp_fragment_band_seekbar_32);
        llBand.addView(llSeekBar);
        mSourceText = mDspMainView.findViewById(SkinUtils.getId(R.id.dsp_source_select));
        //Dsp band flipper
        layoutEqualizer = mDspMainView.findViewById(SkinUtils.getId(R.id.layout_equalizer));
        layoutSurround = mDspMainView.findViewById(SkinUtils.getId(R.id.layout_surround));
        layoutSubwoofer = mDspMainView.findViewById(SkinUtils.getId(R.id.layout_subwoofer));
        mDspTabGroup = mDspMainView.findViewById(SkinUtils.getId(R.id.dsp_tab_group));
        mDspTabGroup.setOnCheckedChangeListener(this);
        ((RadioButton) mDspTabGroup.getChildAt(0)).setChecked(true);
        initEqualizerSurfaceView();
        sparkView = mDspMainView.findViewById(SkinUtils.getId(R.id.sv_main));
        if (sparkView != null) {
            sparkView.setViewThatNeedTouch(mEqualizerSurface);
        }
        //surround flipper
        mSurroundCheck = mDspMainView.findViewById(SkinUtils.getId(R.id.dsp_ck_surround));
        mSurroundCheck.setChecked(mAspDataSettings.getAspSurround());
        mSurroundCheck.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean mCheck) {
                mAspDataSettings.setAspSurround(mCheck ? 1 : 0);
                mDspInternalSettings.setDspSurround(mCheck ? 10 : 0);
            }
        });
        mLoudCheck = mDspMainView.findViewById(SkinUtils.getId(R.id.dsp_ck_loudness));
        mLoudCheck.setChecked(mAspDataSettings.getAspLoudness());
        mLoudCheck.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean mCheck) {
                mAspDataSettings.setAspLoudness(mCheck ? 1 : 0);
            }
        });
        //DSP Power Check.
        boolean mPower = mDspInternalSettings.getDspPower();
        setDspPower(mPower);
        mPowerCheck = mDspMainView.findViewById(SkinUtils.getId(R.id.dsp_power_check));
        mPowerCheck.setChecked(mPower);
        mPowerCheck.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean mCheck) {
                setDspPower(mCheck);
            }
        });
        //subwoofer flipper
        mSubwooferSeek = mDspMainView.findViewById(SkinUtils.getId(R.id.dsp_seekbar_subwoofer));
        // FIXME, 换肤方式，在xml里设置 padding=0, offset=0，滑块依然会超出，通过代码设置后正常
        mSubwooferSeek.setPadding(0, 0, 0, 0);
        mSubwooferSeek.setThumbOffset(5);
        mSubwooferSeek.setProgress(mDspInternalSettings.getDspBassBoost());
        mSubwooferSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int mIndex, boolean forUser) {
                if (forUser) {
                    mDspInternalSettings.setDspBassBoost(mIndex, false);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                //停止滑动保存数值.
                mDspInternalSettings.setDspBassBoost(seekBar.getProgress(), true);
                if (seekBar.getProgress() > 0) {
                    //Chip3313 线路输出只支持开关，同步DSP低音调节时设置.
                    mAspDataSettings.setAspSubWoofer(1, true);
                } else {
                    mAspDataSettings.setAspSubWoofer(0, true);
                }
            }
        });

        refreshBandSeekBarStatus(mPower);
    }

    public View initEqualizerSurfaceView() {
        mEqualizerSurface = mDspMainView.findViewById(SkinUtils.getId(R.id.frequencyResponse));
        mEqualizerSurface.initConfig("freq_curve_color_1", SkinUtils.getColor(R.color.freq_curve_color_1)); // 曲线颜色
        mEqualizerSurface.initConfig("freq_curve_color_2", SkinUtils.getColor(R.color.freq_curve_color_2));
        mEqualizerSurface.initConfig("heq_bar_knob_color", SkinUtils.getColor(R.color.bar_knob_color)); // 曲线上的圆点颜色
        mEqualizerSurface.initConfig("gain_text_color", Color.WHITE); // 左侧 gain 文字颜色
        mEqualizerSurface.initConfig("bar_gain_text_color", Color.TRANSPARENT); // 底部 gain 文字颜色
        mEqualizerSurface.initConfig("bar_gain_text_shadow_color", Color.TRANSPARENT);
        mEqualizerSurface.initConfig("background_color", Color.TRANSPARENT); // 整个控件的背景颜色
        mEqualizerSurface.initConfig("bar_color", SkinUtils.getColor(R.color.bar_color)); // 竖向线条颜色
        mEqualizerSurface.initConfig("bar_text_size", (int) SkinUtils.getDimension(R.dimen.dsp_frequency_bar_tv_size)); // 顶部频点文字大小
        mEqualizerSurface.initConfig("freq_curve_width_1", (int) SkinUtils.getDimension(R.dimen.dsp_freq_curve_width_1)); // 曲线线条宽度
        mEqualizerSurface.initConfig("freq_curve_width_1", (int) SkinUtils.getDimension(R.dimen.dsp_freq_curve_width_2)); // 曲线线条宽度
        // 均衡器曲线被触动的行为，判断曲线是否UI刷新
        mEqualizerSurface.initConfig("equalizer_touch_action", (int) EqUtils.EQUALIZER_ACTION_SLIDE_HORIZONTAL);
        // 配置增益区间
        mEqualizerSurface.setGainRange(-maxGainDB, maxGainDB);

        // 配置中心频段个数
        mEqualizerSurface.setCenterFreqBands(bandTotal);

        // [测试数据]
        String[] arrCenterFreqs = SkinUtils.getStringArray(R.array.center_freq_32_segment);
        String[] arrCenterFreqsVisible = SkinUtils.getStringArray(R.array.center_freq_32_segment_visible);
        if(arrCenterFreqs != null && arrCenterFreqsVisible != null){
            if (bandTotal == arrCenterFreqs.length) {
                float freq = 0.0f;
                int visible = 1;
                for (int index = 0; index < bandTotal; index++) {
                    freq = Float.parseFloat(arrCenterFreqs[index]);
                    visible = Integer.parseInt(arrCenterFreqsVisible[index]);
                    mEqualizerSurface.setCenterFreqBandsValue(index, freq, visible);
                }
            }
        }else{
            LogUtils.vTag(TAG,"IESV get center_freq_32_segment or center_freq_32_segment_visible fail");
        }


        // 限制图表的增益值为整数
        mEqualizerSurface.setGainRound(true);

        // 配置曲线的光滑度, 也就是曲率, 取整范围: 0.0 ~ 0.3
        mEqualizerSurface.setCurveSmoothness(0.18f);

        // 配置均衡器曲线的显示模式
        mEqualizerSurface.setEqualizerUIMode(UIMode.BEZIER_EQUIDISTANT);
        // 初始化配置
        mEqualizerSurface.initialize();
        if (mDspInternalSettings.getDspPower()) {
            refreshEqualizerSurfaceView();
        }
        // 监听回调用来设置 DSP
        mEqualizerSurface.setCenterFreqChangedListener(new OnCenterFreqChangedListener() {
            @Override
            public void OnCenterFreqChanged(int index, double gain, boolean touch) {
                String szGain = (gain < 0 ? "" : "+") + (float) (Math.round(gain * 10) / 10.0f);
                Log.d(TAG, "center freq change: ["
                        + new DecimalFormat("00").format(index) + " & " + szGain + "]"
                        + " - [" + touch + "]");
                if (touch) {
                    mDspBandData[index] = (int) gain;
                    Log.d(TAG, "EqualizerSurface : " + Arrays.toString(mDspBandData));
                    mDspInternalSettings.setupEqualizer(mDspBandData, false);

                    refreshSingleBandSeekBarStatus(index);
                }
            }
        }, 100);
        return null;
    }

    private void setDspPower(boolean mPower) {
        mDspInternalSettings.setDspPower(mPower ? 1 : 0);
        if (!mPower) {
            mEqualizerSurface.setEnabled(false);
            mEqualizerSurface.setAlpha((float) 0.6);
            //off时数据还原.
            mDspBandData = Arrays.copyOf(DEF_DSP_BANDS_32[0], mDspBandData.length);
            mDspInternalSettings.setupEqualizer(DEF_DSP_BANDS_32[0], true);
            for (int mUiBand = 0; mUiBand < mDspBandData.length; mUiBand++) {
                mEqualizerSurface.setBand(mUiBand, DEF_DSP_BANDS_32[0][mUiBand], false);
            }
            mSourceText.setText(SkinUtils.getText(R.string.app_sound_off));
        } else {
            mEqualizerSurface.setEnabled(true);
            mEqualizerSurface.setAlpha(SkinUtils.getDimension(R.dimen.equalizerSurface_alpha));
            mDspInternalSettings.setupEqualizer(mDspBandData, true);
            refreshEqualizerSurfaceView();
            mSourceText.setText(SkinUtils.getText(R.string.app_sound_music));
        }

        refreshBandSeekBarStatus(mPower);
    }

    @Override
    public void onCheckedChanged(RadioGroup radioGroup, int checkedId) {
        layoutEqualizer.setVisibility(checkedId == SkinUtils.getId(R.id.dsp_tab_eq) ? View.VISIBLE : View.GONE);
        layoutSurround.setVisibility(checkedId == SkinUtils.getId(R.id.dsp_tab_surround) ? View.VISIBLE : View.GONE);
        layoutSubwoofer.setVisibility(checkedId == SkinUtils.getId(R.id.dsp_tab_subwoofer) ? View.VISIBLE : View.GONE);
    }

    private void refreshBandSeekBarStatus(boolean enable) {
        int[] _gainValue = mDspBandData;

        for (int i = 0; i < llSeekBar.getChildCount(); i++) {
            View child = llSeekBar.getChildAt(i);
            if (child instanceof ExtDspBandSeekBar) {
                ExtDspBandSeekBar bandSeekBarView = (ExtDspBandSeekBar) child;
                bandSeekBarView.setProgress(_gainValue[i], false);
                if (enable) { // 只有自定义的才能做修改操作
                    bandSeekBarView.setFragmentManager(getFragmentManager());
                    bandSeekBarView.setOnSeekBarChangeListener(this);
                }
                bandSeekBarView.setSeekBarStatus(enable);
            }
        }
    }

    private void refreshSingleBandSeekBarStatus(int index) {
        View child = llSeekBar.getChildAt(index);
        if (child instanceof ExtDspBandSeekBar) {
            ExtDspBandSeekBar bandSeekBarView = (ExtDspBandSeekBar) child;
            bandSeekBarView.setProgress(mDspBandData[index], false);
        }
    }

    private void refreshEqualizerSurfaceView() {
        for (int mUiBand = 0; mUiBand < mDspBandData.length; mUiBand++) {
            mEqualizerSurface.setBand(mUiBand, mDspBandData[mUiBand], false);
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            // 进度条拖动的时候，只修改底层音效和临时变量
            int _bandIndex = Integer.parseInt((String) seekBar.getTag());
            mDspBandData[_bandIndex] = progress;
            refreshEqualizerSurfaceView();

            mDspInternalSettings.setupEqualizer(mDspBandData, false);
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }
}