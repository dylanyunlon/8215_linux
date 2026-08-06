package com.hcn.autoeq.fragment.ecdspt;

import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.TextView;

import com.auto.hequalizer.EqualizerSurface;
import com.auto.hequalizer.UIMode;
import com.hcn.autoeq.R;
import com.hcn.autoeq.adapter.GalleryAdapter;
import com.hcn.autoeq.data.ExtDspBandSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.ECDConstantExtDsp;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.ScalePageTransformer;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.DspPopupWindow;
import com.hcn.autoeq.view.ExtDspBandSeekBar;
import com.hcn.autoeq.view.GalleryViewPager;
import com.hcn.common.misc.LogUtils;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.util.ArrayList;
import java.util.List;

/**
 * describe：dsp band音频界面-类型2
 * 整体布局有区别常用布局：场景模式的选择效果，通过画廊方式实现；
 * 目前应用：mcc400-mnc109，mcc400-mnc110
 * <p>
 * author：zjk
 */
public class ECDspBandSecondFragment extends BaseFragment
        implements ECDConstantExtDsp
        , SeekBar.OnSeekBarChangeListener
        , View.OnClickListener {

    private static final String TAG = ECDspBandSecondFragment.class.getSimpleName();
    private View mainView;
    /**
     * 音频情景用户自定义模式-按钮
     */
    private Button btnUserModeBand;

    /**
     * 音频控制拖动条-视图
     */
    private LinearLayout llSeekBar = null;

    /**
     * 音频默认重置-按钮
     */
    private Button btnResetBand;

    /**
     * 音频场景模式-按钮
     */
    private Button btnBandReverbMode;
    private LinearLayout llBandReverb;

    /**
     * 音频效果曲线图-自定义view
     */
    private EqualizerSurface mEqualizerSurface;
    private TextView tvGainMax, tvGainMin;

    /**
     * 音频效果设定类
     */
    private ExtDspBandSettings extDspBandSettings;

    /**
     * 保存当前界面各 gain value、qvalue
     */
    private int[][] bandValue;

    /**
     * 场景模式回廊-自定义view(继承是ViewPage)
     */
    GalleryViewPager gvpBandReverb;
    /**
     * 场景模式回廊适配器
     */
    GalleryAdapter gvpBandReverbAdapter;
    /**
     * 用户自定义模式悬浮框-popupWindow
     */
    private DspPopupWindow mPopWindow;

    /**
     * 用户自定义模式图标
     */
    Drawable userReverbIconClose;
    Drawable userReverbIconOpen;

    RadioGroup rgBandTopTab;

    public ECDspBandSecondFragment() {
    }

    public static ECDspBandSecondFragment newInstance() {
        ECDspBandSecondFragment fragment = new ECDspBandSecondFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        if (checkLayoutExists(R.layout.ext_c_dsp_fragment_band_second)) {
            return R.layout.ext_c_dsp_fragment_band_second;
        } else {
            return R.layout.extdsp_fragment_band_second;
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        extDspBandSettings = ExtDspBandSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        rgBandTopTab = mainView.findViewById(SkinUtils.getId(R.id.rg_band_top_tab));

        //音频默认重置
        initResetModeView();

        //音频场景模式
        initReverbModeView();

        //音频用户自定义弹窗
        initUserModePopupWindow();

        //音频自定义模式
        initUserModeView();

        //音频控制拖动条
        initSeekBar();

        tvGainMax = mainView.findViewById(SkinUtils.getId(R.id.tv_gain_max));
        tvGainMin = mainView.findViewById(SkinUtils.getId(R.id.tv_gain_min));
        int dspGainMax = EqUtils.getDspGainMax();
        tvGainMax.setText(String.valueOf(dspGainMax / 2));
        tvGainMin.setText(String.valueOf(dspGainMax / -2));
    }

    /**
     * 音频默认重置
     */
    public void initResetModeView() {
        btnResetBand = mainView.findViewById(SkinUtils.getId(R.id.btn_band_reset));
        btnResetBand.setOnClickListener(this);
    }

    /**
     * 音频场景模式
     */
    public void initReverbModeView() {
        llBandReverb = mainView.findViewById(SkinUtils.getId(R.id.ll_band_reverb));
        llBandReverb.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                return gvpBandReverb.dispatchTouchEvent(event);
            }
        });
        //场景模式按钮
        btnBandReverbMode = mainView.findViewById(SkinUtils.getId(R.id.btn_band_reverb_mode));
        btnBandReverbMode.setOnClickListener(this);
        //情景回廊
        gvpBandReverb = mainView.findViewById(SkinUtils.getId(R.id.gvp_band_reverb));
        // 设置每项的间距
        gvpBandReverb.setPageMargin(15);
        // 设置缩放和移动动画
        gvpBandReverb.setPageTransformer(true, new ScalePageTransformer());

        /*
         设置需要缓存的数量，最好>=集合数量
         多缓存一倍，可以防止快速滑动时，来不及刷新导致界面异常的问题
          */
        String[] reverbList = SkinUtils.getStringArray(R.array.extdsp_band_reverb);
        List<Drawable> drawableList = getReverbResource();
        if(reverbList != null){
            gvpBandReverb.setOffscreenPageLimit(reverbList.length * 2);

            //情景回廊适配器
            gvpBandReverbAdapter = new GalleryAdapter(mContext, drawableList, reverbList);
            gvpBandReverb.setOnItemClickListener(new GalleryViewPager.OnItemClickListener() {
                @Override
                public void onItemClick(View view, int position) {
                    chooseReverb(reverbList.length, position);
                }

                @Override
                public void onItemInvalidClick() {
                    llBandReverb.setVisibility(View.GONE);
                    int reverb = extDspBandSettings.getReverb();
                    refreshBtnTopReverbActivityStatus(reverb);
                }
            });
            gvpBandReverb.setAdapter(gvpBandReverbAdapter);
            gvpBandReverb.setCurrentItem(Short.MAX_VALUE / 2 - 1); // 默认选中中间（必须在 setAdapter 之后）
        }else{
            LogUtils.vTag(TAG,"IRMV get reverbList fail!");
        }

    }

    /**
     * 选择场景模式
     */
    public void chooseReverb(int amount, int position) {
        extDspBandSettings.saveReverb(position % (amount));
        initData();
        llBandReverb.setVisibility(View.GONE);
    }


    /**
     * 获取本地存储的场景模式图片资源
     */
    public List<Drawable> getReverbResource() {
        List<Drawable> drawableList = new ArrayList<>();
        drawableList.add(SkinUtils.getDrawable(R.drawable.extdsp_band_reverb_news_selector));
        drawableList.add(SkinUtils.getDrawable(R.drawable.extdsp_band_reverb_jazz_selector));
        drawableList.add(SkinUtils.getDrawable(R.drawable.extdsp_band_reverb_city_selector));
        drawableList.add(SkinUtils.getDrawable(R.drawable.extdsp_band_reverb_pop_selector));
        drawableList.add(SkinUtils.getDrawable(R.drawable.extdsp_band_reverb_electronic_selector));
        drawableList.add(SkinUtils.getDrawable(R.drawable.extdsp_band_reverb_classiz_selector));
        drawableList.add(SkinUtils.getDrawable(R.drawable.extdsp_band_reverb_movie_selector));
        drawableList.add(SkinUtils.getDrawable(R.drawable.extdsp_band_reverb_rock_selector));
        drawableList.add(SkinUtils.getDrawable(R.drawable.extdsp_band_reverb_techno_selector));
        return drawableList;
    }

    /**
     * 音频用户自定义弹窗
     */
    public void initUserModePopupWindow() {
        View popup_view = SkinUtils.inflate(R.layout.extdsp_band_user_reverb_popup);
        mPopWindow = new DspPopupWindow(popup_view, LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, true);
        RadioGroup rgBand_user = popup_view.findViewById(SkinUtils.getId(R.id.rg_band_user));
        RadioButton btn1 = popup_view.findViewById(SkinUtils.getId(R.id.rb_extdsp_band_user0));
        RadioButton btn2 = popup_view.findViewById(SkinUtils.getId(R.id.rb_extdsp_band_user1));
        RadioButton btn3 = popup_view.findViewById(SkinUtils.getId(R.id.rb_extdsp_band_user2));
        btn1.setOnClickListener(this);
        btn2.setOnClickListener(this);
        btn3.setOnClickListener(this);

        mPopWindow.setOnDspPopupListener(new DspPopupWindow.OnDspPopupListener() {
            @Override
            public void UpdatePopupContent() {
                int reverb = extDspBandSettings.getReverb();
                switch (reverb) {
                    case ECDConstantExtDsp.EXT_DSP_REVERB_USER0:
                        rgBand_user.check(SkinUtils.getId(R.id.rb_extdsp_band_user0));
                        break;
                    case ECDConstantExtDsp.EXT_DSP_REVERB_USER1:
                        rgBand_user.check(SkinUtils.getId(R.id.rb_extdsp_band_user1));
                        break;
                    case ECDConstantExtDsp.EXT_DSP_REVERB_USER2:
                        rgBand_user.check(SkinUtils.getId(R.id.rb_extdsp_band_user2));
                        break;
                    default:
                        rgBand_user.clearCheck();
                        break;
                }
            }

            @Override
            public void openOrCloseListener(boolean isOpenStatus) {
                refreshBtnUserIconStatus(isOpenStatus);
            }
        });
    }

    //

    /**
     * 音频用户自定义模式
     */
    public void initUserModeView() {
        btnUserModeBand = mainView.findViewById(SkinUtils.getId(R.id.btn_band_user_reverb_mode));
        btnUserModeBand.setOnClickListener(this);
        userReverbIconClose = SkinUtils.getDrawable(R.drawable.extdsp_icon_close);
        userReverbIconOpen = SkinUtils.getDrawable(R.drawable.extdsp_icon_open);
        refreshBtnUserIconStatus(false);
    }

    /**
     * 音频控制拖动条
     */
    public void initSeekBar() {
        LinearLayout llBand = mainView.findViewById(SkinUtils.getId(R.id.ll_band));
        int bandTotal = EqUtils.getBandTotal();
        if (bandTotal == EqUtils.BAND_TOTAL_14) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.extdsp_fragment_band_seekbar_14);
        } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.extdsp_fragment_band_seekbar_16);
        } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.extdsp_fragment_band_seekbar_32);
        } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.extdsp_fragment_band_seekbar_48);
        }
        llBand.addView(llSeekBar);
    }

    @Override
    public void initData() {
        super.initData();
        int reverb = extDspBandSettings.getReverb();
        bandValue = extDspBandSettings.getUserBandValue(reverb);
        initEqualizerSurfaceView();
        refreshBandSeekBarStatus();
        refreshBtnUserStatus(reverb);
        refreshBtnReverbModeStatus(reverb);
        refreshBtnTopReverbActivityStatus(reverb);
    }

    /**
     * 刷新图标
     */
    public void refreshBtnUserIconStatus(boolean isOpenStatus) {
        if (userReverbIconOpen == null) {
            userReverbIconOpen = SkinUtils.getDrawable(R.drawable.extdsp_icon_open);
        }
        if (userReverbIconClose == null) {
            userReverbIconClose = SkinUtils.getDrawable(R.drawable.extdsp_icon_close);
        }
        btnUserModeBand.setCompoundDrawablesWithIntrinsicBounds(null, null, isOpenStatus ? userReverbIconOpen : userReverbIconClose, null);
    }


    @Override
    public void onResume() {
        super.onResume();
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

            //需要获取最新的处理数据，然后再针对相应位置进行数据处理；
            int _Gain = extDspBandSettings.getDealBandValue(_gainValue, _bandIndex);
            int _Q = extDspBandSettings.getDealBandValue(_qValue, _bandIndex);
            int _Index = extDspBandSettings.getDealBandIndex(_gainValue, _bandIndex);
            extDspBandSettings.nativeBand(_Index, _Gain, _Q);
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
        int id = view.getId();
        if (id == SkinUtils.getId(R.id.btn_band_reset)) {
            extDspBandSettings.saveReverb(ECDConstantExtDsp.EXT_DSP_REVERB_USER0);
            extDspBandSettings.resetUserBand();
            initData();
        } else if (id == SkinUtils.getId(R.id.btn_band_reverb_mode)) {
            llBandReverb.setVisibility(View.VISIBLE);
        } else if (id == SkinUtils.getId(R.id.btn_band_user_reverb_mode)) {//显示popWindow
            if (mPopWindow != null) {
                mPopWindow.showAsDropDown(view.findViewById(SkinUtils.getId(R.id.btn_band_user_reverb_mode))
                        , SkinUtils.getInteger(R.integer.band_user_reverb_mode_x)
                        , SkinUtils.getInteger(R.integer.band_user_reverb_mode_y),
                        Gravity.BOTTOM);
            }
        } else if (id == SkinUtils.getId(R.id.rb_extdsp_band_user0)) {
            extDspBandSettings.saveReverb(ECDConstantExtDsp.EXT_DSP_REVERB_USER0);
            initData();
            mPopWindow.close(mPopWindow);
        } else if (id == SkinUtils.getId(R.id.rb_extdsp_band_user1)) {
            extDspBandSettings.saveReverb(ECDConstantExtDsp.EXT_DSP_REVERB_USER1);
            initData();
            mPopWindow.close(mPopWindow);
        } else if (id == SkinUtils.getId(R.id.rb_extdsp_band_user2)) {
            extDspBandSettings.saveReverb(ECDConstantExtDsp.EXT_DSP_REVERB_USER2);
            initData();
            mPopWindow.close(mPopWindow);
        }
    }

    public void initEqualizerSurfaceView() {
        int bandTotal = EqUtils.getBandTotal();

        mEqualizerSurface = mainView.findViewById(SkinUtils.getId(R.id.es_freq));
        // 配置不能拖动
        mEqualizerSurface.setEnabled(false);
        // 配置增益区间
        mEqualizerSurface.setGainRange(EqUtils.getDspGainMax() / 2 * -1, EqUtils.getDspGainMax() / 2);
        // 配置中心频段个数
        mEqualizerSurface.setCenterFreqBands(bandTotal);
        // [测试数据]

        for (int i = 0; i < bandTotal; i++) {
            if (bandTotal == EqUtils.BAND_TOTAL_14) {
                mEqualizerSurface.setCenterFreqBandsValue(i, ECDConstantExtDsp.DEF_EQ_14_FREQ_VALUES[i][2]);
            } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
                mEqualizerSurface.setCenterFreqBandsValue(i, ECDConstantExtDsp.DEF_EQ_16_FREQ_VALUES[i][2]);
            } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                mEqualizerSurface.setCenterFreqBandsValue(i, ECDConstantExtDsp.DEF_EQ_32_FREQ_VALUES[i][2]);
            } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                mEqualizerSurface.setCenterFreqBandsValue(i, ECDConstantExtDsp.DEF_EQ_48_FREQ_VALUES[i][2]);
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
    }

    private void refreshEqualizerSurfaceView() {
        for (int mUiBand = 0; mUiBand < bandValue[0].length; mUiBand++) {
            mEqualizerSurface.setBand(mUiBand, bandValue[0][mUiBand], false);
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

    /**
     * 刷新顶部音频效果按钮激活状态
     */
    private void refreshBtnTopReverbActivityStatus(int reverb) {
        rgBandTopTab.clearCheck();
        switch (reverb) {
            case ECDConstantExtDsp.EXT_DSP_REVERB_NEWS:
            case ECDConstantExtDsp.EXT_DSP_REVERB_JAZZ:
            case ECDConstantExtDsp.EXT_DSP_REVERB_CITY:
            case ECDConstantExtDsp.EXT_DSP_REVERB_POP:
            case ECDConstantExtDsp.EXT_DSP_REVERB_ELECTRONIC:
            case ECDConstantExtDsp.EXT_DSP_REVERB_CLASSIZ:
            case ECDConstantExtDsp.EXT_DSP_REVERB_MOVIE:
            case ECDConstantExtDsp.EXT_DSP_REVERB_ROCK:
            case ECDConstantExtDsp.EXT_DSP_REVERB_TECHNO:
                rgBandTopTab.check(SkinUtils.getId(R.id.btn_band_reverb_mode));
                break;
            case ECDConstantExtDsp.EXT_DSP_REVERB_USER0:
            case ECDConstantExtDsp.EXT_DSP_REVERB_USER1:
            case ECDConstantExtDsp.EXT_DSP_REVERB_USER2:
                rgBandTopTab.check(SkinUtils.getId(R.id.btn_band_user_reverb_mode));
                break;
            default:
                break;
        }
    }

    /**
     * 刷新用户自定义场景模式状态
     */
    private void refreshBtnReverbModeStatus(int reverb) {
        if (reverb < EXT_DSP_REVERB_SIZE) {

            String[] reverbList = SkinUtils.getStringArray(R.array.extdsp_band_reverb);
            if(reverbList != null){
                String reverbMode = SkinUtils.getText(R.string.extdsp_band_reverb_mode_title) + ":" + reverbList[reverb];
                btnBandReverbMode.setText(reverbMode);
            }else{
                LogUtils.vTag(TAG,"get reverbList fail!");
            }
        } else {
            btnBandReverbMode.setText(SkinUtils.getText(R.string.extdsp_band_reverb_mode_title));
        }
    }

    /**
     * 刷新用户自定义场景模式状态
     */
    private void refreshBtnUserStatus(int reverb) {
        switch (reverb) {
            case ECDConstantExtDsp.EXT_DSP_REVERB_USER0:
                btnUserModeBand.setText(SkinUtils.getText(R.string.band_user0));
                break;
            case ECDConstantExtDsp.EXT_DSP_REVERB_USER1:
                btnUserModeBand.setText(SkinUtils.getText(R.string.band_user1));
                break;
            case ECDConstantExtDsp.EXT_DSP_REVERB_USER2:
                btnUserModeBand.setText(SkinUtils.getText(R.string.band_user2));
                break;
            default:
                btnUserModeBand.setText(SkinUtils.getText(R.string.band_user0));
                break;
        }
    }

}
