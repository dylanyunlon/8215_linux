package com.hcn.autoeq.fragment.extdsp;

import static com.hcn.autoeq.util.EqUtils.KEY_SKIN;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.TextView;

import com.auto.hequalizer.EqualizerSurface;
import com.auto.hequalizer.OnCenterFreqChangedListener;
import com.auto.hequalizer.UIMode;
import com.hcn.autoeq.R;
import com.hcn.autoeq.adapter.ExtDspBandAdapter;
import com.hcn.autoeq.bean.ExtDspBandItemBean;
import com.hcn.autoeq.data.ExtDspBandSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.ConstantExtDsp;
import com.hcn.autoeq.util.ECDConstantExtDsp;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.util.SystemUtils;
import com.hcn.autoeq.view.CustomSpinner;
import com.hcn.autoeq.view.ExtDspBandSeekBar;
import com.hcn.autoeq.view.SparkView;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.text.DecimalFormat;
import java.util.ArrayList;

public class ExtDspBandFragment extends BaseFragment
        implements ConstantExtDsp
        , SeekBar.OnSeekBarChangeListener, AdapterView.OnItemSelectedListener
        , View.OnClickListener, RadioGroup.OnCheckedChangeListener, CustomSpinner.OnSpinnerEventsListener {

    private static final String TAG = ExtDspBandFragment.class.getSimpleName();
    private View mainView;
    private RadioGroup rgBandUser;
    private LinearLayout llSeekBar = null;
    private Button btnResetBand;
    private CustomSpinner spReverb;
    private EqualizerSurface mEqualizerSurface;
    private SparkView sparkView;

    private TextView tvGainMax, tvGainMin;
    private HorizontalScrollView scrollView;
    /**
     * 超两屏幕专用
     */
    private Button moveScrollBtnNext;
    private Button moveScrollBtnPre;

    private ExtDspBandSettings extDspBandSettings;
    private boolean spinnerFromUser;
    private int[][] bandValue; // 保存当前界面各 gain value、qvalue

    private ArrayList<ExtDspBandItemBean> bandItemList = null;

    private String skinName;

    public ExtDspBandFragment() {
    }

    public static ExtDspBandFragment newInstance() {
        ExtDspBandFragment fragment = new ExtDspBandFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.extdsp_fragment_band;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        extDspBandSettings = ExtDspBandSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void onPause() {
        super.onPause();
        // 这个保存动作不能放到 onStop 里，会大概率性保存不了（猜测：可能 Activity 销毁，对象没了）
        if (sparkView != null) {
            sparkView.stopDraw();
        }
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public void initView() {
        rgBandUser = mainView.findViewById(SkinUtils.getId(R.id.rg_band_user));
        rgBandUser.setOnCheckedChangeListener(this);
        spReverb = mainView.findViewById(SkinUtils.getId(R.id.sp_reverb));
        bandItemList = new ArrayList<>();
        int count = SkinUtils.getStringArray(R.array.extdsp_band_reverb).length;
        for (int i = 0; i < count; i++) {
            bandItemList.add(new ExtDspBandItemBean(R.drawable.extdsp_band_icon_select, SkinUtils.getStringArray(R.array.extdsp_band_reverb)[i]));
        }
        ExtDspBandAdapter adapter = new ExtDspBandAdapter(bandItemList, mContext);
        spReverb.setAdapter(adapter);
        spReverb.setSpinnerEventsListener(this);
        spReverb.setOnItemSelectedListener(this);
        spReverb.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                spinnerFromUser = true;
                return false;
            }
        });

        LinearLayout llBand = mainView.findViewById(SkinUtils.getId(R.id.ll_band));
        int bandTotal = EqUtils.getBandTotal();
        if (bandTotal == EqUtils.BAND_TOTAL_14) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.extdsp_fragment_band_seekbar_14);
        } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.extdsp_fragment_band_seekbar_32);
        } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.extdsp_fragment_band_seekbar_48);
        }
        llBand.addView(llSeekBar);

        btnResetBand = mainView.findViewById(SkinUtils.getId(R.id.btn_reset_band));
        btnResetBand.setOnClickListener(this);

        tvGainMax = mainView.findViewById(SkinUtils.getId(R.id.tv_gain_max));
        tvGainMin = mainView.findViewById(SkinUtils.getId(R.id.tv_gain_min));
        int dspGainMax = EqUtils.getDspGainMax();
        tvGainMax.setText(String.valueOf(dspGainMax / 2));
        tvGainMin.setText(String.valueOf(dspGainMax / -2));

        boolean flipPageByBtn = EqUtils.flipPageByBtn(mContext);
        scrollView = mainView.findViewById(SkinUtils.getId(R.id.hsv_band));
        if (flipPageByBtn) { // 使用按钮翻页方式，则屏蔽触摸方式
            if (scrollView != null) {
                scrollView.setOnTouchListener((arg0, arg1) -> true);
            }
        }

        moveScrollBtnPre = mainView.findViewById(SkinUtils.getId(R.id.btn_move_scroll_pre));
        moveScrollBtnNext = mainView.findViewById(SkinUtils.getId(R.id.btn_move_scroll_next));
        if (moveScrollBtnPre != null) {
            moveScrollBtnPre.setOnClickListener(this);
            moveScrollBtnPre.setVisibility(flipPageByBtn ? View.VISIBLE : View.GONE);
        }
        if (moveScrollBtnNext != null) {
            moveScrollBtnNext.setOnClickListener(this);
            moveScrollBtnNext.setVisibility(flipPageByBtn ? View.VISIBLE : View.GONE);
        }
    }

    @Override
    public void initData() {
        super.initData();
        int reverb = extDspBandSettings.getReverb();
        bandValue = extDspBandSettings.getUserBandValue(reverb);
        initEqualizerSurfaceView();
        refreshBandSeekBarStatus();
        refreshSpReverbStatus();
        refreshBtnUserStatus();
    }

    @Override
    public void onSpinnerOpened(int lastSelectedItemPosition, int currentSelectedItemPosition) {
        spReverb.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_band_mode_opened_selector));
    }

    @Override
    public void onResume() {
        super.onResume();
        refreshSpReverbStatus();
    }

    @Override
    public void onSpinnerClosed(int lastSelectedItemPosition, int currentSelectedItemPosition) {
        spReverb.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_band_mode_closed_selector));

        // 如果选择的是相同项，也高亮控件
        if (lastSelectedItemPosition == currentSelectedItemPosition) {
            Log.d(TAG, "onSpinnerClosed current selected item same of last selected item");

            // 如果当前是用户模式，则切换模式
            if (extDspBandSettings.getReverb() > EXT_DSP_REVERB_SIZE) {
                extDspBandSettings.saveReverb(currentSelectedItemPosition);
                refreshData();
            } else { // 如果当前是预设模式，则仅作高亮处理
                refreshSpReverbStatus();
            }
        }
    }

    // 注意：界面初始化时会自动调用一次，所以需要判断是自动调用还是手动选择
    @Override
    public void onItemSelected(AdapterView<?> adapterView, View view, int position, long l) {
        Log.d(TAG, "onItemSelected position : " + position + ", spinnerFromUser : " + spinnerFromUser);
        if (spinnerFromUser) {
            spinnerFromUser = false;
            extDspBandSettings.saveReverb(position);
            refreshData();
        }
        for (int i = 0; i < spReverb.getCount(); i++) {
            ExtDspBandItemBean bandItemBean = (ExtDspBandItemBean) spReverb.getItemAtPosition(i);
            if (i == position) {
                bandItemBean.setIcon(R.drawable.extdsp_band_icon_select);
            } else {
                bandItemBean.setIcon(0);
            }
        }
        ImageView imageView = view.findViewById(R.id.tv_band_mode_select);
        if (imageView != null) {
            imageView.setVisibility(View.INVISIBLE);
        }
        spReverb.invalidate();
    }

    @Override
    public void onNothingSelected(AdapterView<?> adapterView) {
        Log.d(TAG, "onNothingSelected");
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            // 进度条拖动的时候，只修改底层音效和临时变量，拖动完毕后，再把临时变量值保存到 sp 文件中
            int reverb = extDspBandSettings.getReverb();
            bandValue = extDspBandSettings.getUserBandValue(reverb);
            int[] _gainValue = bandValue[0];
            int[] _qValue = bandValue[1];
            int _bandIndex = Integer.parseInt((String) seekBar.getTag());
            _gainValue[_bandIndex] = progress;

            refreshEqualizerSurfaceView();

            extDspBandSettings.nativeBand(_bandIndex, progress, _qValue[_bandIndex]);
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        extDspBandSettings.saveBandValue(bandValue);
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == SkinUtils.getId(R.id.btn_reset_band)) {
            extDspBandSettings.saveReverb(ConstantExtDsp.EXT_DSP_REVERB_USER0);
            extDspBandSettings.resetUserBand();
            refreshData();
        } else if (view.getId() == SkinUtils.getId(R.id.btn_move_scroll_next)) {
            if (scrollView != null) {
                int scrollContentWidth = scrollView.getChildAt(0).getWidth();
                scrollView.smoothScrollBy(scrollContentWidth / EqUtils.flipPageCount(mContext), 0);
            }
        } else if (view.getId() == SkinUtils.getId(R.id.btn_move_scroll_pre)) {
            if (scrollView != null) {
                int scrollContentWidth = scrollView.getChildAt(0).getWidth();
                scrollView.smoothScrollBy(-scrollContentWidth / EqUtils.flipPageCount(mContext), 0);
            }
        }
    }

    public void initEqualizerSurfaceView() {
        int bandTotal = EqUtils.getBandTotal();
        skinName = EqUtils.getSkinName();
        mEqualizerSurface = mainView.findViewById(SkinUtils.getId(R.id.es_freq));

        if (mEqualizerSurface != null){
            if (EqUtils.isRtL(getContext())){
                mEqualizerSurface.setScaleX(-1);
            }else {
                mEqualizerSurface.setScaleX(1);
            }
        }

        // 判定配置能拖动
        equalizerEnabled();
        mEqualizerSurface.initConfig("background", SkinUtils.getColor(R.color.extdsp_fragment_background));
        mEqualizerSurface.initConfig("background_color", SkinUtils.getColor(R.color.extdsp_fragment_background_color));
        mEqualizerSurface.initConfig("bar_color", SkinUtils.getColor(R.color.extdsp_fragment_bar_color));
        mEqualizerSurface.initConfig("bar_text_color", SkinUtils.getColor(R.color.extdsp_fragment_bar_text_color));
        mEqualizerSurface.initConfig("bar_text_shadow_color", SkinUtils.getColor(R.color.extdsp_fragment_bar_text_shadow_color));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_1", SkinUtils.getColor(R.color.extdsp_fragment_band_freq_curve_color_1));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_2", SkinUtils.getColor(R.color.extdsp_fragment_band_freq_curve_color_2));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_3", SkinUtils.getColor(R.color.extdsp_fragment_band_freq_curve_color_3));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_4", SkinUtils.getColor(R.color.extdsp_fragment_band_freq_curve_color_4));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_5", SkinUtils.getColor(R.color.extdsp_fragment_band_freq_curve_color_5));
        mEqualizerSurface.initConfig("freq_curve_color_1", SkinUtils.getColor(R.color.extdsp_fragment_freq_curve_color_1));
        mEqualizerSurface.initConfig("freq_curve_color_2", SkinUtils.getColor(R.color.extdsp_fragment_freq_curve_color_2));
        mEqualizerSurface.initConfig("gain_text_color", SkinUtils.getColor(R.color.extdsp_fragment_gain_text_color));
        mEqualizerSurface.initConfig("heq_bar_knob_color", SkinUtils.getColor(R.color.extdsp_fragment_heq_bar_knob_color));
        mEqualizerSurface.initConfig("freq_curve_width_1", (int) SkinUtils.getDimension(R.dimen.dsp_freq_curve_width_1));
        mEqualizerSurface.initConfig("freq_curve_width_2", (int) SkinUtils.getDimension(R.dimen.dsp_freq_curve_width_2));
        mEqualizerSurface.initConfig("bar_width", (int) SkinUtils.getDimension(R.dimen.dsp_frequency_bar_width));
        mEqualizerSurface.initConfig("bar_text_size", (int) SkinUtils.getDimension(R.dimen.extdsp_fragment_bar_text_size));
        mEqualizerSurface.initConfig("gain_text_size", (int) SkinUtils.getDimension(R.dimen.extdsp_fragment_gain_text_size));
        mEqualizerSurface.initConfig("padding_top", (int) SkinUtils.getDimension(R.dimen.extdsp_fragment_bar_padding_top));
        mEqualizerSurface.initConfig("padding_bottom", (int) SkinUtils.getDimension(R.dimen.extdsp_fragment_gain_padding_bottom));

        setEqualizerTouchAction();
        // 配置增益区间
        mEqualizerSurface.setGainRange(EqUtils.getDspGainMax() / 2 * -1, EqUtils.getDspGainMax() / 2);
        // 配置中心频段个数
        mEqualizerSurface.setCenterFreqBands(bandTotal);
        // [测试数据]

        if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())) {
            for (int i = 0; i < bandTotal; i++) {
                if (bandTotal == EqUtils.BAND_TOTAL_14) {
                    mEqualizerSurface.setCenterFreqBandsValue(i, ECDConstantExtDsp.DEF_EQ_14_FREQ_VALUES[i][2]);
                } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                    mEqualizerSurface.setCenterFreqBandsValue(i, ECDConstantExtDsp.DEF_EQ_32_FREQ_VALUES[i][2]);
                } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                    mEqualizerSurface.setCenterFreqBandsValue(i, ECDConstantExtDsp.DEF_EQ_48_FREQ_VALUES[i][2]);
                }
            }
        } else {
            for (int i = 0; i < bandTotal; i++) {
                if (bandTotal == EqUtils.BAND_TOTAL_14) {
                    mEqualizerSurface.setCenterFreqBandsValue(i, ConstantExtDsp.DEF_EQ_14_FREQ_VALUES[i][2]);
                } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                    mEqualizerSurface.setCenterFreqBandsValue(i, ConstantExtDsp.DEF_EQ_32_FREQ_VALUES[i][2]);
                } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                    mEqualizerSurface.setCenterFreqBandsValue(i, ConstantExtDsp.DEF_EQ_48_FREQ_VALUES[i][2]);
                }
            }
        }

        // 限制图表的增益值为整数
        mEqualizerSurface.setGainRound(true);
        // 配置曲线的光滑度, 也就是曲率, 取整范围: 0.0 ~ 0.3
        mEqualizerSurface.setCurveSmoothness(0.18f);
        // 配置均衡器曲线的显示模式
        mEqualizerSurface.setEqualizerUIMode(UIMode.BEZIER_EQUIDISTANT);
        // 初始化配置
        mEqualizerSurface.initialize();
        for (int mUiBand = 0; mUiBand < bandValue[0].length; mUiBand++) {
            mEqualizerSurface.setBand(mUiBand, bandValue[0][mUiBand], false);
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
                    int reverb = extDspBandSettings.getReverb();
                    bandValue = extDspBandSettings.getUserBandValue(reverb);
                    int[] _gainValue = bandValue[0];
                    int[] _qValue = bandValue[1];
                    int _bandIndex = index;
                    _gainValue[_bandIndex] = (int) gain;

                    extDspBandSettings.saveBandValue(bandValue);

                    refreshSingleBandSeekBarStatus(index);

                    extDspBandSettings.nativeBand(_bandIndex, _gainValue[_bandIndex], _qValue[_bandIndex]);
                }

            }
        }, 100);


        sparkView = mainView.findViewById(SkinUtils.getId(R.id.sv_main));
        if (sparkView != null) {
            sparkView.setViewThatNeedTouch(mEqualizerSurface);
        }

    }

    /**
     * 判断曲线图是否可以拖动
     */
    public void equalizerEnabled() {
        if (mEqualizerSurface == null) {
            return;
        }
        // 配置不能拖动
        if ("mcx_exdsp".equals(skinName)) {
            int reverb = extDspBandSettings.getReverb();
            boolean enable = reverb >= EXT_DSP_REVERB_SIZE;
            mEqualizerSurface.setEnabled(enable);
        } else {
            mEqualizerSurface.setEnabled(false);
        }

    }

    /**
     * 设置曲线触摸的行为模式
     */
    public void setEqualizerTouchAction() {
        if (mEqualizerSurface == null) {
            return;
        }
        if ("mcx_exdsp".equals(skinName)) {
            // 均衡器曲线被触动的行为，判断曲线是否UI刷新
            mEqualizerSurface.initConfig("equalizer_touch_action", (int) EqUtils.EQUALIZER_ACTION_SLIDE_HORIZONTAL);
        }
    }

    private void refreshData() {
        int reverb = extDspBandSettings.getReverb();
        bandValue = extDspBandSettings.getUserBandValue(reverb);
        refreshEqualizerSurfaceView();
        refreshBandSeekBarStatus();
        refreshSpReverbStatus();
        refreshBtnUserStatus();
    }

    private void refreshEqualizerSurfaceView() {
        for (int mUiBand = 0; mUiBand < bandValue[0].length; mUiBand++) {
            mEqualizerSurface.setBand(mUiBand, bandValue[0][mUiBand], false);
        }
        equalizerEnabled();
    }

    private void refreshSingleBandSeekBarStatus(int index) {
        int reverb = extDspBandSettings.getReverb();
        int[][] bandValue = extDspBandSettings.getUserBandValue(reverb);
        int[] _gainValue = bandValue[0];
        int[] _qValue = bandValue[1];
        boolean enable = reverb >= EXT_DSP_REVERB_SIZE;

        View child = llSeekBar.getChildAt(index);
        if (child instanceof ExtDspBandSeekBar) {
            ExtDspBandSeekBar bandSeekBarView = (ExtDspBandSeekBar) child;
            bandSeekBarView.setProgress(_gainValue[index], false);
            bandSeekBarView.setQValue(_qValue[index]);
            if (enable) { // 只有自定义的才能做修改操作
                bandSeekBarView.setFragmentManager(getFragmentManager());
                bandSeekBarView.setOnSeekBarChangeListener(this);
            }
            bandSeekBarView.setSeekBarStatus(enable);
        }
    }

    private void refreshBandSeekBarStatus() {
        int reverb = extDspBandSettings.getReverb();
        int[][] bandValue = extDspBandSettings.getUserBandValue(reverb);
        int[] _gainValue = bandValue[0];
        int[] _qValue = bandValue[1];
        boolean enable = reverb >= EXT_DSP_REVERB_SIZE;

        for (int i = 0; i < llSeekBar.getChildCount(); i++) {
            View child = llSeekBar.getChildAt(i);
            if (child instanceof ExtDspBandSeekBar) {
                ExtDspBandSeekBar bandSeekBarView = (ExtDspBandSeekBar) child;
                bandSeekBarView.setProgress(_gainValue[i], false);
                bandSeekBarView.setQValue(_qValue[i]);
                if (enable) { // 只有自定义的才能做修改操作
                    bandSeekBarView.setFragmentManager(getFragmentManager());
                    bandSeekBarView.setOnSeekBarChangeListener(this);
                }
                bandSeekBarView.setSeekBarStatus(enable);
            }
        }
    }

    private void refreshSpReverbStatus() {
        int reverb = extDspBandSettings.getReverb();
        Log.d(TAG, "refreshSpReverbStatus reverb : " + reverb);
        if (reverb < EXT_DSP_REVERB_SIZE) {
            spReverb.setSelection(reverb);
            spReverb.setPressed(true);
        } else {
            spReverb.setPressed(false);
        }
    }

    private void refreshBtnUserStatus() {
        int reverb = extDspBandSettings.getReverb();
        switch (reverb) {
            case ConstantExtDsp.EXT_DSP_REVERB_USER0:
                rgBandUser.check(SkinUtils.getId(R.id.rb_extdsp_band_user0));
                break;
            case ConstantExtDsp.EXT_DSP_REVERB_USER1:
                rgBandUser.check(SkinUtils.getId(R.id.rb_extdsp_band_user1));
                break;
            case ConstantExtDsp.EXT_DSP_REVERB_USER2:
                rgBandUser.check(SkinUtils.getId(R.id.rb_extdsp_band_user2));
                break;
            default:
                rgBandUser.clearCheck();
                break;
        }
    }

    @Override
    public void onCheckedChanged(RadioGroup radioGroup, int id) {
        boolean hasChildPressed = false;
        for (int i = 0; i < radioGroup.getChildCount(); i++) {
            if (radioGroup.getChildAt(i).isPressed()) {
                hasChildPressed = true;
            }
        }

        // 清除所有 check 的时候，也会调用 onCheckedChanged 事件，这个时候不需要改变数据
        // 手动点击时才往下执行
        if (!hasChildPressed) {
            return;
        }
        if (id == SkinUtils.getId(R.id.rb_extdsp_band_user0)) {
            extDspBandSettings.saveReverb(ConstantExtDsp.EXT_DSP_REVERB_USER0);
            refreshData();
        } else if (id == SkinUtils.getId(R.id.rb_extdsp_band_user1)) {
            extDspBandSettings.saveReverb(ConstantExtDsp.EXT_DSP_REVERB_USER1);
            refreshData();
        } else if (id == SkinUtils.getId(R.id.rb_extdsp_band_user2)) {
            extDspBandSettings.saveReverb(ConstantExtDsp.EXT_DSP_REVERB_USER2);
            refreshData();
        }
    }

}
