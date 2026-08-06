package com.hcn.autoeq.fragment;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import com.auto.hequalizer.EqualizerSurface;
import com.auto.hequalizer.OnCenterFreqChangedListener;
import com.auto.hequalizer.UIMode;
import com.hcn.autoeq.R;
import com.hcn.autoeq.data.DspInternalSettings;
import com.hcn.autoeq.util.ConstantDsp;
import com.hcn.autoeq.util.ConstantEq;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.common.misc.LogUtils;

import java.text.DecimalFormat;
import java.util.Arrays;

public class InternalDspFragment extends BaseFragment
        implements RadioGroup.OnCheckedChangeListener, ConstantDsp {

    private final static String TAG = InternalDspFragment.class.getSimpleName();
    private static final String ARG_PARAM1 = "param1";
    private static final String ARG_PARAM2 = "param2";

    private String mParam1;
    private String mParam2;
    private int[] mDspBandData;//user模式数据
    private View mInternalDspView;
    private RadioGroup mDspReverbGroup;
    private EqualizerSurface mEqualizerSurface;
    private DspInternalSettings mDspInternalSettings;
    private final int bandTotal = EqUtils.DSP_BAND_DEPTH;    // 中心频率个数
    private final int maxGainDB = 10;

    public InternalDspFragment() {
        // Required empty public constructor
    }

    public static InternalDspFragment newInstance(int[] param1, String param2) {
        InternalDspFragment fragment = new InternalDspFragment();
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
        return R.layout.internal_dsp_band_fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mInternalDspView = super.onCreateView(inflater, container, savedInstanceState);
        mDspInternalSettings = DspInternalSettings.getInstance(mContext);
        return mInternalDspView;
    }

    @Override
    public void initView() {
        //Dsp band flipper
        initEqualizerSurfaceView();
        int mDspReverb = mDspInternalSettings.getDspReverbType();
        mDspReverbGroup = mInternalDspView.findViewById(SkinUtils.getId(R.id.dsp_reverb_group));
        mDspReverbGroup.setOnCheckedChangeListener(this);
        for (int child = 0; child < mDspReverbGroup.getChildCount(); child++) {
            mDspReverbGroup.getChildAt(child).setOnFocusChangeListener(mFocusChangeListener);
        }
        ((RadioButton) mDspReverbGroup.getChildAt(mDspReverb)).requestFocus();
        ((RadioButton) mDspReverbGroup.getChildAt(mDspReverb)).setChecked(true);
        mInternalDspView.findViewById(SkinUtils.getId(R.id.dsp_reset)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                setDspReset();
            }
        });
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (!hidden) {
            int mDspReverb = mDspInternalSettings.getDspReverbType();
            ((RadioButton) mDspReverbGroup.getChildAt(mDspReverb)).requestFocus();
            ((RadioButton) mDspReverbGroup.getChildAt(mDspReverb)).setChecked(true);
        }

    }

    //添加焦点处理,用于字符串过长走马灯显示.
    View.OnFocusChangeListener mFocusChangeListener = new View.OnFocusChangeListener() {
        @Override
        public void onFocusChange(View v, boolean hasFocus) {
            if (null != mDspReverbGroup && hasFocus) {
                ((RadioButton) mDspReverbGroup.findViewById(v.getId())).setChecked(true);
            }
        }
    };

    public View initEqualizerSurfaceView() {
        mEqualizerSurface = mInternalDspView.findViewById(SkinUtils.getId(R.id.frequencyResponse));
        // 配置增益区间
        mEqualizerSurface.setGainRange(-maxGainDB, maxGainDB);

        // 配置中心频段个数
        mEqualizerSurface.setCenterFreqBands(bandTotal);

        // [测试数据]
        String[] arrCenterFreqs = SkinUtils.getStringArray(R.array.center_freq_16_segment);
        if(arrCenterFreqs != null){
            if (bandTotal == arrCenterFreqs.length) {
                float freq = 0.0f;
                for (int index = 0; index < bandTotal; index++) {
                    freq = Float.parseFloat(arrCenterFreqs[index]);
                    mEqualizerSurface.setCenterFreqBandsValue(index, freq);
                }
            }
        }else{
            LogUtils.vTag(TAG,"IESV get center_freq_16_segment fail!");
        }


        // 限制图表的增益值为整数
        mEqualizerSurface.setGainRound(true);

        // 配置曲线的光滑度, 也就是曲率, 取整范围: 0.0 ~ 0.3
        mEqualizerSurface.setCurveSmoothness(0.18f);

        // 配置均衡器曲线的显示模式
        mEqualizerSurface.setEqualizerUIMode(UIMode.BEZIER_EQUIDISTANT);
        // 初始化配置
        mEqualizerSurface.initialize();
        if (mDspInternalSettings.getDspReverbType() == ConstantEq.EQ_REVERB_USER) {
            setEqualizerSurface(mDspBandData, true);
        } else {
            setEqualizerSurface(DEF_DSP_BANDS[mDspInternalSettings.getDspReverbType()], false);
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
                    //单Band更新不需淡入淡出.
                    mDspInternalSettings.setupEqualizer(mDspBandData, false);
                }
            }
        }, 100);
        return null;
    }

    public void setEqualizerSurface(int[] mEqualVal, boolean mEnable) {
        if (null == mEqualizerSurface) return;
        mEqualizerSurface.setEnabled(mEnable);
        mEqualizerSurface.setAlpha(mEnable ? SkinUtils.getDimension(R.dimen.equalizerSurface_alpha) : (float) 0.6);
        for (int mUiBand = 0; mUiBand < mEqualVal.length; mUiBand++) {
            mEqualizerSurface.setBand(mUiBand, mEqualVal[mUiBand], false);
        }
    }

    private void setDspReset() {
        mEqualizerSurface.setEnabled(true);
        mEqualizerSurface.setAlpha(SkinUtils.getDimension(R.dimen.equalizerSurface_alpha));
        ((RadioButton) mDspReverbGroup.getChildAt(ConstantEq.EQ_REVERB_USER)).requestFocus();
        ((RadioButton) mDspReverbGroup.getChildAt(ConstantEq.EQ_REVERB_USER)).setChecked(true);
        mDspInternalSettings.setupEqualizer(DEF_DSP_BANDS[0], true);
        for (int mUiBand = 0; mUiBand < mDspBandData.length; mUiBand++) {
            mEqualizerSurface.setBand(mUiBand, DEF_DSP_BANDS[0][mUiBand], false);
            mDspBandData = Arrays.copyOf(DEF_DSP_BANDS[0], EqUtils.DSP_BAND_DEPTH);
        }
    }

    @Override
    public void onStop() {
        super.onStop();
        //退出时保存数据
        mDspInternalSettings.saveDspBandValue(Arrays.toString(mDspBandData));
    }

    @Override
    public void onCheckedChanged(RadioGroup radioGroup, int checkedId) {
        int soundMode = 0;
        if (checkedId == SkinUtils.getId(R.id.dsp_reverb_user)) {
            soundMode = ConstantEq.EQ_REVERB_USER;
        } else if (checkedId == SkinUtils.getId(R.id.dsp_reverb_news)) {
            soundMode = ConstantEq.EQ_REVERB_NEWS;
        } else if (checkedId == SkinUtils.getId(R.id.dsp_reverb_jazz)) {
            soundMode = ConstantEq.EQ_REVERB_JAZZ;
        } else if (checkedId == SkinUtils.getId(R.id.dsp_reverb_city)) {
            soundMode = ConstantEq.EQ_REVERB_CITY;
        } else if (checkedId == SkinUtils.getId(R.id.dsp_reverb_pop)) {
            soundMode = ConstantEq.EQ_REVERB_POP;
        } else if (checkedId == SkinUtils.getId(R.id.dsp_reverb_electronic)) {
            soundMode = ConstantEq.EQ_REVERB_ELECTRONIC;
        } else if (checkedId == SkinUtils.getId(R.id.dsp_reverb_classiz)) {
            soundMode = ConstantEq.EQ_REVERB_CLASSIZ;
        } else if (checkedId == SkinUtils.getId(R.id.dsp_reverb_movie)) {
            soundMode = ConstantEq.EQ_REVERB_MOVIE;
        } else if (checkedId == SkinUtils.getId(R.id.dsp_reverb_rock)) {
            soundMode = ConstantEq.EQ_REVERB_ROCK;
        } else if (checkedId == SkinUtils.getId(R.id.dsp_reverb_techno)) {
            soundMode = ConstantEq.EQ_REVERB_TECHNO;
        } else {
            soundMode = checkedId;
        }
        if (soundMode > ConstantEq.EQ_REVERB_USER) {
            //更新音效
            mDspInternalSettings.setDspReverbType(soundMode, DEF_DSP_BANDS[soundMode]);
            //刷新显示
            setEqualizerSurface(DEF_DSP_BANDS[soundMode], false);
        } else {
            mDspInternalSettings.setDspReverbType(soundMode, mDspBandData);
            setEqualizerSurface(mDspBandData, true);
        }
    }
}