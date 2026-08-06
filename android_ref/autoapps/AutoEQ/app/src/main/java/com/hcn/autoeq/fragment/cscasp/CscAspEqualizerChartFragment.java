package com.hcn.autoeq.fragment.cscasp;


import static com.hcn.autoeq.data.CscAspEqualizerChartSettings.MAX_GAIN_DB;
import static com.hcn.autoeq.data.CscAspEqualizerChartSettings.MIN_GAIN_DB;
import static com.hcn.autoeq.util.ConstantCscAsp.BAND_TOTAL;
import static com.hcn.autoeq.util.ConstantCscAsp.EXT_CSC_ASP_REVERB_SIZE;
import static com.hcn.autoeq.util.ConstantCscAsp.EXT_CSC_ASP_REVERB_USER0;

import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;

import com.auto.hequalizer.EqualizerSurface;
import com.auto.hequalizer.EqualizerTransaction;
import com.auto.hequalizer.OnCenterFreqChangedListener;
import com.auto.hequalizer.UIMode;
import com.hcn.autoeq.R;
import com.hcn.autoeq.data.CscAspEqualizerChartSettings;
import com.hcn.autoeq.data.CscAspQValueSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.CscAspQValueBtnView;
import com.hcn.autoeq.view.CustomAspTopTabView;
import com.hcn.autoeq.view.SparkView;
import com.hcn.common.misc.LogUtils;
import com.hcn.skin.support.SkinCompatManager;
import com.hcn.skin.support.resources.SkinCompatResources;
import com.hcn.skin2.Skin2;

import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.Locale;


/**
 * hcn_asp乐谱
 */
public class CscAspEqualizerChartFragment extends BaseFragment {

    private static final String TAG = CscAspEqualizerChartFragment.class.getSimpleName();
    private View mainView;
    /**
     * 音频效果设定类
     */
    private CscAspEqualizerChartSettings cscAspEqualizerChartSettings;
    /**
     * 音频Q值设定类
     */
    private CscAspQValueSettings cscAspQValueSettings;

    /**
     * 音谱曲线图
     */
    private EqualizerSurface mEqualizerSurface;


    LinearLayout llBand;

    /**
     * Q值按钮
     */
    CscAspQValueBtnView cscAspQValueBtnView;
    CustomAspTopTabView customAspTopTabView;

    private SparkView sparkView;

    //段数数量
    private int bandNumber = BAND_TOTAL;

    /**
     * 保存当前界面各 gain centerFre
     * 必须是float值，因为拖动条的拖动虽然是整数，但是音谱滑动的值不可能是只有整数，两者统一协调时会有冲突
     */
    private float[][] bandValue;

    /**
     * 低中高音Q值
     */
    private int[] qValue = {1, 1, 1};

    /**
     * 弹窗
     */
    CscAspUserModeResetDialog cscAspUserModeResetDialog;

    @Override
    public int getLayoutRes() {
        return R.layout.csc_asp_equalizer_chart_fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        cscAspEqualizerChartSettings = CscAspEqualizerChartSettings.getInstance(mContext);
        cscAspQValueSettings = CscAspQValueSettings.getInstance(mContext);
        return mainView;
    }

    public static CscAspEqualizerChartFragment newInstance() {
        CscAspEqualizerChartFragment fragment = new CscAspEqualizerChartFragment();
        return fragment;
    }


    @Override
    public void initView() {
        //顶部导航栏
        customAspTopTabView = mainView.findViewById(SkinUtils.getId(R.id.first_csc_asp_top_tab));
        customAspTopTabView.setOnDspPopupListener(new CustomAspTopTabView.OnCscAspTopTabViewListener() {
            @Override
            public void updateModeContent() {
                int reverb = cscAspEqualizerChartSettings.getReverb();
                bandValue = cscAspEqualizerChartSettings.getUserBandValue(reverb);
                refreshEqualizerSurfaceView(false);
            }

            @Override
            public void showUserResetDialog(int mode) {
                cscAspUserModeResetDialog = CscAspUserModeResetDialog.newInstance(mode);
                cscAspUserModeResetDialog.show(getParentFragmentManager(), "");
                cscAspUserModeResetDialog.setOnDialogListener(new CscAspUserModeResetDialog.OnDialogListener() {
                    @Override
                    public void isOk(int mode) {
                        //重置数据
                        int reverb = cscAspEqualizerChartSettings.getReverb();
                        if (reverb < EXT_CSC_ASP_REVERB_SIZE) {
                            return;
                        }
                        //拖动结束后，要遍历获取所有增益值，存储起来，作为自定义模式的值，并且更新状态栏；
                        for (int mUiBand = 0; mUiBand < bandNumber; mUiBand++) {
                            bandValue[0][mUiBand] = 0;
                        }
                        qValue[0] = 1;
                        qValue[1] = 1;
                        qValue[2] = 1;
                        cscAspQValueSettings.setCscQValue(qValue[0], qValue[1], qValue[2]);
                        cscAspEqualizerChartSettings.saveBandValue(bandValue);
                        //更新view
                        refreshView();
                    }
                });
            }
        });

        llBand = mainView.findViewById(SkinUtils.getId(R.id.ll_csc_asp_band));
    }

    @Override
    public void initData() {
        super.initData();
        //获取总段数
        String[] stringArray = SkinUtils.getStringArray(R.array.center_csc_asp_freq_36_segment);
        if(stringArray != null){
            bandNumber = stringArray.length;
        }
        //获取音谱模式
        int reverb = cscAspEqualizerChartSettings.getReverb();
        //初始化，获取具体音谱模式的 gain centerFre；
        bandValue = cscAspEqualizerChartSettings.getUserBandValue(reverb);
        //qValue
        qValue = cscAspQValueSettings.getCscQValue(reverb);
        initQValueBtnView();
        initEqualizerSurfaceView();
    }

    private void refreshView() {
        //获取音谱模式
        int reverb = cscAspEqualizerChartSettings.getReverb();
        //初始化，获取具体音谱模式的 gain centerFre；
        bandValue = cscAspEqualizerChartSettings.getUserBandValue(reverb);
        //qValue
        qValue = cscAspQValueSettings.getCscQValue(reverb);
        refreshEqualizerSurfaceView(false);
        refreshQValueBtnView();
    }


    /**
     * 初始化横轴Q值按钮
     *
     * @return
     */
    public void initQValueBtnView() {

        String[] fc = SkinUtils.getStringArray(R.array.center_csc_asp_freq_36_segment);
        if(fc == null){
            LogUtils.vTag(TAG,"get center_csc_asp_freq_36_segment fail!");
            return;
        }
        int paragraph = BAND_TOTAL / 3;
        for (int i = 0; i < bandNumber; i++) {
            cscAspQValueBtnView = new CscAspQValueBtnView(mContext);
            if (i < paragraph) {
                cscAspQValueBtnView.setQValue(qValue[0]);
            } else if (i < paragraph * 2) {
                cscAspQValueBtnView.setQValue(qValue[1]);
            } else {
                cscAspQValueBtnView.setQValue(qValue[2]);
            }
            String freq = "";
            int freqValue = Integer.parseInt(fc[i]);
            if (freqValue >= 1000) {
                freq = String.format(Locale.getDefault(), "%.1f", freqValue / 1000f) + "K";
            } else {
                freq = String.format(Locale.getDefault(), "%d", freqValue);
            }
            cscAspQValueBtnView.setCscCenterFre(freq);

            llBand.addView(cscAspQValueBtnView);
        }
    }

    /**
     * 刷新 QValueBtnView
     */
    public void refreshQValueBtnView() {
        int paragraph = BAND_TOTAL / 3;
        for (int i = 0; i < bandNumber; i++) {
            View child = llBand.getChildAt(i);
            if (child instanceof CscAspQValueBtnView) {
                CscAspQValueBtnView btn = (CscAspQValueBtnView) child;
                if (i < paragraph) {
                    btn.setQValue(qValue[0]);
                    Log.d(TAG, "qValue[0] " + i + ", " + paragraph + "," + qValue[0]);
                } else if (i < paragraph * 2) {
                    btn.setQValue(qValue[1]);
                    Log.d(TAG, "qValue[0] " + i + ", " + paragraph + "," + qValue[1]);
                } else {
                    btn.setQValue(qValue[2]);
                    Log.d(TAG, "qValue[0] " + i + ", " + paragraph + "," + qValue[2]);
                }
            }

        }
    }

    @Override
    public void onPause() {
        super.onPause();
        // 这个保存动作不能放到 onStop 里，会大概率性保存不了（猜测：可能 Activity 销毁，对象没了）
        cscAspEqualizerChartSettings.saveBandValue(bandValue);
        if (sparkView != null) {
            sparkView.stopDraw();
        }
    }

    public void initEqualizerSurfaceView() {
//        skinName = SystemUtils.getSystemProperty(KEY_SKIN, "");
        mEqualizerSurface = mainView.findViewById(SkinUtils.getId(R.id.csc_asp_es_freq));
        //设置曲线触摸的行为模式
        setEqualizerTouchAction();
        // 配置增益区间
        mEqualizerSurface.setGainRange(MIN_GAIN_DB, MAX_GAIN_DB);
        // 配置中心频段个数
        mEqualizerSurface.setCenterFreqBands(bandNumber);

        // 限制图表的增益值为整数
        mEqualizerSurface.setGainRound(false);

        // 配置曲线的光滑度, 也就是曲率, 取整范围: 0.0 ~ 0.3
        mEqualizerSurface.setCurveSmoothness(0.18f);

        // 配置均衡器曲线的显示模式
        mEqualizerSurface.setEqualizerUIMode(UIMode.BEZIER_EQUIDISTANT);
        //Android 13及以上，设置zOrderOnTop为true，可以解决绘制时，出现黑色背景的问题
        if (Build.VERSION.SDK_INT >= 30) {
            try {
                // 获取指定类的声明类数组
                Class<?>[] declaredClasses = Class.forName("com.auto.hequalizer.EqualizerSurface").getDeclaredClasses();

                // 用于存储最终找到的类
                Class<?> cls = null;
                // 遍历声明类数组，查找名为 RConfig 的类
                for (Class<?> clazz : declaredClasses) {
                    if (clazz.getSimpleName().equals("RConfig")) {
                        cls = clazz;
                        break;
                    }
                }
                if (cls != null) {
                    // 获取 RConfig 类中的 mControlBarShadowColor 字段
                    Field declaredField = cls.getDeclaredField("mControlBarShadowColor");
                    // 设置该字段为可访问
                    declaredField.setAccessible(true);
                    // 将 mControlBarShadowColor 字段的值设置为 0
                    declaredField.set(null, 0);
                    // 获取该字段的值
                    Object obj = declaredField.get(null);
                    // 打印修改后的属性值
                    Log.d(TAG, "修改后的属性值为: " + obj);
                }
            } catch (ClassNotFoundException | NoSuchFieldException | IllegalAccessException e) {
                e.printStackTrace();
            }
            mEqualizerSurface.setZOrderOnTop(true);
            //mEqualizerSurface.invalidate();
        }
        // 判定配置能拖动
        Log.d(TAG, "mEqualizerSurface.getBackground() before" + mEqualizerSurface.getBackground().getAlpha());
        mEqualizerSurface.initConfig("background_color", SkinUtils.getColor(R.color.csc_asp_fragment_background_color)); // 整个控件的背景颜色
        mEqualizerSurface.initConfig("bar_color", SkinUtils.getColor(R.color.csc_asp_fragment_bar_color));
        mEqualizerSurface.initConfig("bar_text_color", SkinUtils.getColor(R.color.csc_asp_fragment_bar_text_color));
        mEqualizerSurface.initConfig("bar_text_shadow_color", SkinUtils.getColor(R.color.csc_asp_fragment_bar_text_shadow_color));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_1", SkinUtils.getColor(R.color.csc_asp_fragment_band_freq_curve_color_1));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_2", SkinUtils.getColor(R.color.csc_asp_fragment_band_freq_curve_color_2));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_3", SkinUtils.getColor(R.color.csc_asp_fragment_band_freq_curve_color_3));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_4", SkinUtils.getColor(R.color.csc_asp_fragment_band_freq_curve_color_4));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_5", SkinUtils.getColor(R.color.csc_asp_fragment_band_freq_curve_color_5));
        mEqualizerSurface.initConfig("gridlines_color", SkinUtils.getColor(R.color.csc_asp_fragment_gridlines_color));
        mEqualizerSurface.initConfig("freq_curve_color_1", SkinUtils.getColor(R.color.csc_asp_fragment_freq_curve_color_1));
        mEqualizerSurface.initConfig("freq_curve_color_2", SkinUtils.getColor(R.color.csc_asp_fragment_freq_curve_color_2));
        mEqualizerSurface.initConfig("gain_text_color", SkinUtils.getColor(R.color.csc_asp_fragment_gain_text_color));
        mEqualizerSurface.initConfig("heq_bar_knob_color", SkinUtils.getColor(R.color.csc_asp_fragment_heq_bar_knob_color));
        mEqualizerSurface.initConfig("freq_curve_width_1", (int) SkinUtils.getDimension(R.dimen.csc_asp_freq_curve_width_1));
        mEqualizerSurface.initConfig("freq_curve_width_2", (int) SkinUtils.getDimension(R.dimen.csc_asp_freq_curve_width_2));
        mEqualizerSurface.initConfig("bar_width", (int) SkinUtils.getDimension(R.dimen.csc_asp_bar_width));
        mEqualizerSurface.initConfig("bar_text_size", (int) SkinUtils.getDimension(R.dimen.csc_asp_bar_text_size));
        mEqualizerSurface.initConfig("gain_text_size", (int) SkinUtils.getDimension(R.dimen.csc_asp_gain_text_size));
        mEqualizerSurface.initConfig("bar_gain_text_shadow_color", SkinUtils.getColor(R.color.csc_asp_fragment_bar_gain_text_shadow_color));
        mEqualizerSurface.initConfig("bar_gain_text_color", SkinUtils.getColor(R.color.csc_asp_fragment_bar_gain_text_color));
        mEqualizerSurface.initConfig("heq_attr_bar_color", SkinUtils.getColor(R.color.csc_asp_fragment_heq_attr_bar_color));
        mEqualizerSurface.initConfig("shell_Color", SkinUtils.getColor(R.color.csc_asp_fragment_gridlines_color));
        mEqualizerSurface.initConfig("padding_top", (int) SkinUtils.getDimension(R.dimen.equalizer_csc_asp_es_freq_padding_top));
        mEqualizerSurface.initConfig("padding_bottom", (int) SkinUtils.getDimension(R.dimen.equalizer_csc_asp_es_freq_padding_bottom));
        Log.d(TAG, "mEqualizerSurface.getBackground() after" + mEqualizerSurface.getBackground().getAlpha());
        //设置横轴，即中心频率值
        Log.i(TAG, "initEqualizerSurfaceView: "+bandValue[1].length + " "+bandNumber);
        for (int i = 0; i < bandNumber; i++) {
            mEqualizerSurface.setCenterFreqBandsValue(i, bandValue[1][i]);
        }

        // 初始化配置
        mEqualizerSurface.initialize();

        //设置竖轴，即增益值
        for (int mUiBand = 0; mUiBand < bandNumber; mUiBand++) {
            mEqualizerSurface.setBand(mUiBand, bandValue[0][mUiBand], false);
        }

        //这里时为了改变，主动滑动轴时的变化情况
        mEqualizerSurface.setCenterFreqChangedListener(new OnCenterFreqChangedListener() {
            @Override
            public void OnCenterFreqChanged(int index, double gain, boolean touch) {
                if (touch) {
                    //被触摸时，要更新bandValue值;
                    bandValue[0][index] = (float) gain;
                }
            }
        }, 0);

        //滑动特效区域
        sparkView = mainView.findViewById(SkinUtils.getId(R.id.sv_main));
        if (sparkView != null) {
            sparkView.setViewThatNeedTouch(mEqualizerSurface);
            sparkView.setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View v, MotionEvent event) {
                    switch (event.getAction()) {
                        case MotionEvent.ACTION_UP:
                            int reverb = cscAspEqualizerChartSettings.getReverb();
                            //拖动结束后，要遍历获取所有增益值，存储起来，作为自定义模式的值，并且更新状态栏；
                            for (int mUiBand = 0; mUiBand < bandNumber; mUiBand++) {
                                bandValue[0][mUiBand] = mEqualizerSurface.getBand(mUiBand);
                            }
                            if (reverb < EXT_CSC_ASP_REVERB_SIZE) {
                                cscAspEqualizerChartSettings.saveReverb(EXT_CSC_ASP_REVERB_USER0);
                            }
                            cscAspEqualizerChartSettings.saveBandValue(bandValue);
                            if (customAspTopTabView != null) {
                                customAspTopTabView.initViewStatus();
                            }
                            break;
                    }

                    return false;
                }
            });
        }
    }


    /**
     * 设置曲线触摸的行为模式
     */
    public void setEqualizerTouchAction() {
        if (mEqualizerSurface == null) {
            return;
        }
        // 均衡器曲线被触动的行为，判断曲线是否UI刷新
        mEqualizerSurface.initConfig("equalizer_touch_action", (int) EqUtils.EQUALIZER_ACTION_TOUCHING);
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (hidden && customAspTopTabView != null) {
            customAspTopTabView.highGallery();
        }
        if (!hidden) {
            //view出现后，获取当前模式的值，并且更新view
            refreshView();
        }
    }

    private void refreshEqualizerSurfaceView(boolean commit) {
        if (mEqualizerSurface == null) {
            return;
        }

        // [开始事务]
        EqualizerTransaction transaction = mEqualizerSurface.beginTransaction();
        if (transaction != null) {
            for (int i = 0; i < bandValue[0].length; i++) {
                transaction.setCenterFreqGainValue(i, bandValue[0][i]);
            }
            // [提交事务]
            if (commit) {
                // [初始化直接提交]
                transaction.commit();
            } else {
                // [非初始化做淡入处理, 提高视觉听觉体验]
                transaction.commitAllowFadeIn(1.0f, 50);
            }
        }
    }
}
