package com.hcn.autoeq;

import static com.hcn.autoeq.util.EqUtils.EQUALIZER_ACTION_TOUCHING;
import static com.hcn.autoeq.util.EqUtils.KEY_SKIN;

import android.annotation.SuppressLint;
import android.app.UiModeManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.drawable.BitmapDrawable;
import android.os.Build;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.RadioGroup;

import androidx.annotation.NonNull;
import androidx.core.view.ViewCompat;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;

import com.hcn.autoeq.data.AspSettings;
import com.hcn.autoeq.data.DspInternalSettings;
import com.hcn.autoeq.fragment.AspFragment;
import com.hcn.autoeq.fragment.BalanceFragment;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.fragment.InternalDspAspFragment;
import com.hcn.autoeq.fragment.InternalDspFragment;
import com.hcn.autoeq.fragment.InternalDspSurroundFragment;
import com.hcn.autoeq.fragment.MCXInternalDspAspFragment;
import com.hcn.autoeq.fragment.cscasp.CscAspBalanceFragment;
import com.hcn.autoeq.fragment.cscasp.CscAspEqualizerChartFragment;
import com.hcn.autoeq.fragment.cscasp.CscAspEqualizerFragment;
import com.hcn.autoeq.fragment.cscasp.CscAspQValueFragment;
import com.hcn.autoeq.fragment.cscasp.CscAspSubwooferFragment;
import com.hcn.autoeq.fragment.ecdspt.ECDspAttenuateFragment;
import com.hcn.autoeq.fragment.ecdspt.ECDspBalanceFragment;
import com.hcn.autoeq.fragment.ecdspt.ECDspBandFragment;
import com.hcn.autoeq.fragment.ecdspt.ECDspBandSecondFragment;
import com.hcn.autoeq.fragment.ecdspt.ECDspDelayFragment;
import com.hcn.autoeq.fragment.ecdspt.ECDspFilterFragment;
import com.hcn.autoeq.fragment.extdsp.ExtDspAttenuateFragment;
import com.hcn.autoeq.fragment.extdsp.ExtDspBalanceFragment;
import com.hcn.autoeq.fragment.extdsp.ExtDspBandFragment;
import com.hcn.autoeq.fragment.extdsp.ExtDspBandSecondFragment;
import com.hcn.autoeq.fragment.extdsp.ExtDspDelayFragment;
import com.hcn.autoeq.fragment.extdsp.ExtDspFilterFragment;
import com.hcn.autoeq.fragment.fydsp.FyDspAttenuateFragment;
import com.hcn.autoeq.fragment.fydsp.FyDspBalanceOrDelayFragment;
import com.hcn.autoeq.fragment.fydsp.FyDspBandFragment;
import com.hcn.autoeq.fragment.fydsp.FyDspHLPFFragment;
import com.hcn.autoeq.fragment.fydsp.FyDspSurroundFragment;
import com.hcn.autoeq.fragment.siextdsp.SIExtDspAttenuateFragment;
import com.hcn.autoeq.fragment.siextdsp.SIExtDspBalanceFragment;
import com.hcn.autoeq.fragment.siextdsp.SIExtDspBandFragment;
import com.hcn.autoeq.fragment.siextdsp.SIExtDspDelayFragment;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.util.SystemUtils;
import com.hcn.autoeq.view.DrawableCenterRadioButton;
import com.hcn.skin.support.SkinCompatManager;
import com.hcn.skin.support.app.SkinCompatActivity;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.util.ArrayList;

public class MainActivity extends SkinCompatActivity {

    static final String TAG = MainActivity.class.getSimpleName();

    // CSC asp 页面索引
    public static final int INDEX_CSC_ASP_BAND = 0;
    public static final int INDEX_CSC_ASP_BALANCE = 1;
    public static final int INDEX_CSC_ASP_ATTENUATE = 2;
    public static final int INDEX_CSC_ASP_FILTER = 3;
    public static final int INDEX_CSC_ASP_DELAY = 4;

    // asp 和内置 dsp 页面索引
    public static final int INDEX_DSP = 0;
    public static final int INDEX_ASP = 1;
    public static final int INDEX_SURROUND = 1; //内置DSP时页面为Surround
    public static final int INDEX_BALANCE = 2;

    // 外置 dsp 页面索引
    public static final int INDEX_EXTDSP_BAND = 0;
    public static final int INDEX_EXTDSP_BALANCE = 1;
    public static final int INDEX_EXTDSP_ATTENUATE = 2;
    public static final int INDEX_EXTDSP_FILTER = 3;
    public static final int INDEX_EXTDSP_DELAY = 4;



    private RadioGroup rb_main;
    private BaseFragment mNowFragment;
    private ArrayList<BaseFragment> fragments;
    private AspSettings mDataSettings;
    private DspInternalSettings mDspSettings;

    private int position = 0;
    private int[] mBalanceData;
    private int[] mBandData;
    private int[] mSurroundData;
    private int[] mDspBandData;

    private DrawableCenterRadioButton rb_dsp;
    private int lastIndex = -1;

    int current = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate: " + EqUtils.getEqChipType() + EqUtils.is6225());
        String skinName = EqUtils.getSkinName();
         if (EqUtils.ASP_CHIP_CSC37534.equals(EqUtils.getEqChipType()) || EqUtils.ASP_CHIP_ZL3560.equals(EqUtils.getEqChipType())) {
            initCscAspView();
        } else if (EqUtils.DSP_CHIP_FY7604.equals(EqUtils.getEqChipType())) {
            initFYDspView();
        } else if (EqUtils.DSP_CHIP_7604.equals(EqUtils.getEqChipType()) || EqUtils.DSP_CHIP_SI47925.equals(EqUtils.getEqChipType())
                || EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())) {
            initExtDspView();
        } else {
            initView();
        }

        initCurrent();
    }

    // app 的字体大小不跟随系统
    @Override
    public Resources getResources() {
        Resources res = super.getResources();
        Configuration config = new Configuration();
        config.setToDefaults();
        res.updateConfiguration(config, res.getDisplayMetrics());
        return res;
    }

    @SuppressLint("MissingSuperCall")
    @Override
    protected void onSaveInstanceState(@NonNull Bundle outState) {
        //super.onSaveInstanceState(outState);
    }
    // 飞音外置 dsp
    private void initCscAspView() {
        setContentView(R.layout.csc_asp_activity_main);
        SystemUtils.fullScreen(this);
        rb_main = findViewById(SkinUtils.getId(R.id.rg_main));
        initFragment();
        initListener();
        rb_main.check(SkinUtils.getId(R.id.rb_csc_asp_equalizer_chart));
    }

    // 飞音外置 dsp
    private void initFYDspView() {
        setContentView(R.layout.fydsp_activity_main);
        SystemUtils.fullScreen(this);
        rb_main = findViewById(SkinUtils.getId(R.id.rg_main));
        initFragment();
        initListener();
        rb_main.check(SkinUtils.getId(R.id.rb_fydsp_band));
    }

    // 外置 dsp
    private void initExtDspView() {
        setContentView(R.layout.extdsp_activity_main);
        SystemUtils.fullScreen(this);
        rb_main = findViewById(SkinUtils.getId(R.id.rg_main));
        initFragment();
        initListener();
        if (lastIndex != -1) {
            rb_main.check(lastIndex);
        } else {
            rb_main.check(SkinUtils.getId(R.id.rb_extdsp_band));
        }
    }

    // asp 或者内置 dsp
    private void initView() {
        setContentView(R.layout.activity_main);
        SystemUtils.fullScreen(this);
        rb_main = findViewById(SkinUtils.getId(R.id.rg_main));
        rb_dsp = findViewById(SkinUtils.getId(R.id.rb_dsp));
        mDataSettings = AspSettings.getInstance(getApplicationContext());
        mDspSettings = DspInternalSettings.getInstance(getApplicationContext());
        mBalanceData = mDataSettings.getAspBalance();
        mSurroundData = new int[]{mDspSettings.getDspBassBoost(), mDspSettings.getDspSurround() ? 1 : 0};
        if (rb_dsp != null && !"2".equals(EqUtils.getDspUI())) {
            String tabDspName = mDspSettings.getTabDspName();
            String value = SkinUtils.getString(R.string.main_tab_title_dsp);
            rb_dsp.setText(TextUtils.isEmpty(tabDspName) ? value : tabDspName);
        }
        if (EqUtils.hasAsp()) {
            mBandData = mDataSettings.getUserBandValue();
        }
        mDspBandData = mDspSettings.getDspBandValue();
        initFragment();
        initListener();
        String skinName = SystemUtils.getSystemProperty(KEY_SKIN, "");
        if (EqUtils.KEY_SKIN_RK02.equals(skinName)){
            if (lastIndex != -1) {
                rb_main.check(lastIndex);
            } else {
                rb_main.check(SkinUtils.getId(R.id.rb_asp));
            }
        }else {
            rb_main.check(SkinUtils.getId(R.id.rb_dsp));
        }

        if (!EqUtils.hasAsp()) {
            rb_main.findViewById(SkinUtils.getId(R.id.rb_asp)).setVisibility(View.GONE);
            //判断是否需要隐藏平衡功能
            if (mDspSettings != null && mDspSettings.hideEqBalance()) {
                rb_main.findViewById(SkinUtils.getId(R.id.rb_balance)).setVisibility(View.GONE);
            }
            //判断是否需要隐藏环绕功能
            if (mDspSettings != null && mDspSettings.hideEqSurround()) {
                rb_main.findViewById(SkinUtils.getId(R.id.rb_surround)).setVisibility(View.GONE);
            }
            //如果只存在一个功能，则菜单选项栏可以取消了
            int rbSurroundVisibility = rb_main.findViewById(SkinUtils.getId(R.id.rb_surround)).getVisibility();
            int rbBalanceVisibility = rb_main.findViewById(SkinUtils.getId(R.id.rb_balance)).getVisibility();
            if (rbSurroundVisibility != View.VISIBLE && rbBalanceVisibility != View.VISIBLE) {
                rb_main.findViewById(SkinUtils.getId(R.id.rg_main)).setVisibility(View.GONE);
            }
        } else {
            View rbSurround = rb_main.findViewById(SkinUtils.getId(R.id.rb_surround));
            if (rbSurround != null) {
                rbSurround.setVisibility(View.GONE);
            }
            // 8163海外头枕隐藏平衡功能
            if (EqUtils.isHeadRest()) {
                rb_main.findViewById(SkinUtils.getId(R.id.rb_balance)).setVisibility(View.GONE);
            }
        }
    }

    private void initListener() {
        rb_main.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup radioGroup, int mViewId) {
                if (mViewId == SkinUtils.getId(R.id.rb_csc_asp_equalizer_chart)) {
                    position = INDEX_CSC_ASP_BAND;
                } else if (mViewId == SkinUtils.getId(R.id.rb_csc_asp_equalizer)) {
                    position = INDEX_CSC_ASP_BALANCE;
                } else if (mViewId == SkinUtils.getId(R.id.rb_csc_asp_q_value)) {
                    position = INDEX_CSC_ASP_ATTENUATE;
                } else if (mViewId == SkinUtils.getId(R.id.rb_csc_asp_subwoofer)) {
                    position = INDEX_CSC_ASP_FILTER;
                } else if (mViewId == SkinUtils.getId(R.id.rb_csc_asp_balance)) {
                    position = INDEX_CSC_ASP_DELAY;
                } else if (mViewId == SkinUtils.getId(R.id.rb_fydsp_band)) {
                    position = 0;
                } else if (mViewId == SkinUtils.getId(R.id.rb_fydsp_balance)) {
                    position = 1;
                } else if (mViewId == SkinUtils.getId(R.id.rb_fydsp_surround)) {
                    position = 2;
                } else if (mViewId == SkinUtils.getId(R.id.rb_fydsp_attenuate)) {
                    position = 3;
                } else if (mViewId == SkinUtils.getId(R.id.rb_fydsp_filter)) {
                    position = 4;
                } else if (mViewId == SkinUtils.getId(R.id.rb_extdsp_band)) {
                    position = INDEX_EXTDSP_BAND;
                } else if (mViewId == SkinUtils.getId(R.id.rb_extdsp_balance)) {
                    position = INDEX_EXTDSP_BALANCE;
                } else if (mViewId == SkinUtils.getId(R.id.rb_extdsp_attenuate)) {
                    position = INDEX_EXTDSP_ATTENUATE;
                } else if (mViewId == SkinUtils.getId(R.id.rb_extdsp_filter)) {
                    position = INDEX_EXTDSP_FILTER;
                } else if (mViewId == SkinUtils.getId(R.id.rb_extdsp_delay)) {
                    position = INDEX_EXTDSP_DELAY;
                } else if (mViewId == SkinUtils.getId(R.id.rb_asp)) { //ASP
                    position = INDEX_ASP;
                } else if (mViewId == SkinUtils.getId(R.id.rb_surround)) { //SURROUND
                    position = INDEX_SURROUND;
                } else if (mViewId == SkinUtils.getId(R.id.rb_balance)) { //平衡
                    position = INDEX_BALANCE;
                } else {
                    position = INDEX_DSP;
                }
                setAppBackground(mViewId);
                performFragmentSwitch(position, mViewId);
            }
        });
    }

    public void setAppBackground(int i) {
        if (EqUtils.ASP_CHIP_CSC37534.equals(EqUtils.getEqChipType()) || EqUtils.ASP_CHIP_ZL3560.equals(EqUtils.getEqChipType())) {
            findViewById(SkinUtils.getId(R.id.main_layout)).setBackground(SkinUtils.getDrawable(R.drawable.csc_asp_background));
        } else if (EqUtils.DSP_CHIP_FY7604.equals(EqUtils.getEqChipType())) {
            findViewById(SkinUtils.getId(R.id.main_layout)).setBackground(SkinUtils.getDrawable(R.drawable.fydsp_main_background));
        } else if (EqUtils.DSP_CHIP_7604.equals(EqUtils.getEqChipType()) || EqUtils.DSP_CHIP_SI47925.equals(EqUtils.getEqChipType()) || EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())) {
            findViewById(SkinUtils.getId(R.id.main_layout)).setBackground(SkinUtils.getDrawable(R.drawable.extdsp_main_background));
        } else if (i == SkinUtils.getId(R.id.rb_surround)) {
            findViewById(SkinUtils.getId(R.id.main_layout)).setBackground(SkinUtils.getDrawable(R.drawable.surround_bg));
        } else {
            findViewById(SkinUtils.getId(R.id.main_layout)).setBackground(SkinUtils.getDrawable(R.drawable.eq_background));
        }
    }

    public void performFragmentSwitch(int positon, int mViewId) {
        BaseFragment fragment = getFragment(positon);
        lastIndex = mViewId;
        preAddFragment(fragment);
        switchFragment(mNowFragment, fragment);
    }

    private void initFragment() {
        Log.d(TAG, "initFragment: current eq chip type = " + EqUtils.getEqChipType());
        fragments = new ArrayList<>();
       if (EqUtils.hasAsp()) { // 内置 dsp 功能 + asp 功能
            if ("2".equals(EqUtils.getDspUI())) {
                fragments.add(MCXInternalDspAspFragment.newInstance(mDspBandData, ""));
            } else {
                fragments.add(InternalDspAspFragment.newInstance(mDspBandData, ""));
            }
            fragments.add(AspFragment.newInstance(mBandData, new int[]{mDataSettings.getAspBassBoost(), mDataSettings.getAspTreble()}));
            if (!EqUtils.isHeadRest()) {
                fragments.add(BalanceFragment.newInstance(mBalanceData, ""));
            }
        } else if (EqUtils.DSP_CHIP_FY7604.equals(EqUtils.getEqChipType())) { // 飞音外置 dsp 功能
            fragments.add(FyDspBandFragment.newInstance());
            fragments.add(FyDspBalanceOrDelayFragment.newInstance());
            fragments.add(FyDspSurroundFragment.newInstance());
            fragments.add(FyDspAttenuateFragment.newInstance());
            fragments.add(FyDspHLPFFragment.newInstance());
        } else if (EqUtils.DSP_CHIP_7604.equals(EqUtils.getEqChipType())) { // 外置 dsp 功能
            if (("400".equals(EqUtils.getEThemeGod()) && ("109".equals(EqUtils.getEThemeSub()) || "110".equals(EqUtils.getEThemeSub()) || "137".equals(EqUtils.getEThemeSub()) || "34".equals(EqUtils.getEThemeSub()) || "43".equals(EqUtils.getEThemeSub())))
                    || "3".equals(EqUtils.getDspUI())) {
                fragments.add(ExtDspBandSecondFragment.newInstance());
            } else {
                fragments.add(ExtDspBandFragment.newInstance());
            }
            fragments.add(ExtDspBalanceFragment.newInstance());
            fragments.add(ExtDspAttenuateFragment.newInstance());
            fragments.add(ExtDspFilterFragment.newInstance());
            fragments.add(ExtDspDelayFragment.newInstance());
        } else if (EqUtils.DSP_CHIP_SI47925.equals(EqUtils.getEqChipType())) {
            fragments.add(SIExtDspBandFragment.newInstance());
            fragments.add(SIExtDspBalanceFragment.newInstance());
            fragments.add(SIExtDspAttenuateFragment.newInstance());
            fragments.add(ExtDspFilterFragment.newInstance());
            fragments.add(SIExtDspDelayFragment.newInstance());
        } else if (EqUtils.ASP_CHIP_CSC37534.equals(EqUtils.getEqChipType()) || EqUtils.ASP_CHIP_ZL3560.equals(EqUtils.getEqChipType())) {
            fragments.add(CscAspEqualizerChartFragment.newInstance());
            fragments.add(CscAspEqualizerFragment.newInstance());
            fragments.add(CscAspQValueFragment.newInstance());
            fragments.add(CscAspSubwooferFragment.newInstance());
            fragments.add(CscAspBalanceFragment.newInstance());
        } else if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())) {
            if (("400".equals(EqUtils.getEThemeGod()) && ("109".equals(EqUtils.getEThemeSub()) || "110".equals(EqUtils.getEThemeSub()) || "137".equals(EqUtils.getEThemeSub()) || "34".equals(EqUtils.getEThemeSub()) || "43".equals(EqUtils.getEThemeSub())))
                    || "3".equals(EqUtils.getDspUI())) {
                fragments.add(ECDspBandSecondFragment.newInstance());
            } else {
                fragments.add(ECDspBandFragment.newInstance());
            }
            fragments.add(ECDspBalanceFragment.newInstance());
            fragments.add(ECDspAttenuateFragment.newInstance());
            fragments.add(ECDspFilterFragment.newInstance());
            fragments.add(ECDspDelayFragment.newInstance());
        } else { // 内置 dsp 功能
            fragments.add(InternalDspFragment.newInstance(mDspBandData, ""));
            fragments.add(InternalDspSurroundFragment.newInstance(mSurroundData, ""));
            fragments.add(BalanceFragment.newInstance(mBalanceData, ""));
        }
    }

    /**
     * 根据位置得到对应的 Fragment
     *
     * @param position
     * @return
     */
    private BaseFragment getFragment(int position) {
        if (fragments != null && fragments.size() > 0) {
            return fragments.get(position);
        }
        return null;
    }

    /**
     * 切换Fragment
     *
     * @param fragment
     * @param mNextFragment
     */
    private void switchFragment(Fragment fragment, BaseFragment mNextFragment) {
        if (mNowFragment != mNextFragment && !isDestroyed() && !isFinishing()) {
            mNowFragment = mNextFragment;
            if (mNextFragment != null) {
                FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
                if (fragment != null) {
                    transaction.hide(fragment);
                }
                transaction.show(mNextFragment).commit();
                Log.d(TAG, "switchFragment");
            }
        }
    }

    /**
     * 做一个预加载
     */
    public void preAddFragment(BaseFragment newFragment) {
        if (newFragment != null && !isDestroyed() && !isFinishing()) {
            if (!newFragment.isAdded()) {
                FragmentManager fragmentManager = getSupportFragmentManager();
                FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();
                fragmentTransaction.add(SkinUtils.getId(R.id.eq_frame), newFragment);
                fragmentTransaction.hide(newFragment).commit();
                fragmentManager.executePendingTransactions();
                Log.d(TAG, "preAddFragment");
            }
        }
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        UiModeManager uiModeManager = (UiModeManager) this.getSystemService(Context.UI_MODE_SERVICE);
        int mode = uiModeManager.getNightMode();
        if (EqUtils.supportDayAndNightMode() && mode != current) {
            fragments.clear();
            FragmentManager fragmentManager = getSupportFragmentManager();
            FragmentTransaction transaction = fragmentManager.beginTransaction();
            for (Fragment fragment : fragmentManager.getFragments()) {
                if (fragment != null) {
                    transaction.remove(fragment);
                }
            }
            transaction.commit();
            String skinName = EqUtils.getSkinName();
            int supportNewSkinSetting = EqUtils.getSupportNewSkin();
            if (null != skinName && supportNewSkinSetting == 1) {
                //新皮肤包不做处理
            } else if (EqUtils.ASP_CHIP_CSC37534.equals(EqUtils.getEqChipType()) || EqUtils.ASP_CHIP_ZL3560.equals(EqUtils.getEqChipType())) {
                initCscAspView();
            } else if (EqUtils.DSP_CHIP_FY7604.equals(EqUtils.getEqChipType())) {
                initFYDspView();
            } else if (EqUtils.DSP_CHIP_7604.equals(EqUtils.getEqChipType()) || EqUtils.DSP_CHIP_SI47925.equals(EqUtils.getEqChipType())
                    || EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())) {
                initExtDspView();
            } else {
                initView();
            }
            initMode();
        }
        current = uiModeManager.getNightMode();
        Log.d(TAG, "getNightMode:" + current);
    }

    /**
     *  初始化记录白天黑夜的 current，不然感觉当第一次 onConfigurationChanged 调用时，都会跑一次初始化
     */
    private void initCurrent() {
        UiModeManager uiModeManager = (UiModeManager) this.getSystemService(Context.UI_MODE_SERVICE);
        current = uiModeManager.getNightMode();
        initMode();
    }

    /**
     * 处理高版本状态栏黑白UI模式自适应, mcc602皮肤包支持黑夜白天模式
     */
    public void initMode() {
        Configuration configuration = getResources().getConfiguration();
        int currentNightMode = configuration.uiMode
                & Configuration.UI_MODE_NIGHT_MASK;
        if (Build.VERSION.SDK_INT >= 30) {
            ViewCompat.getWindowInsetsController(getWindow().getDecorView()).setAppearanceLightStatusBars(currentNightMode != Configuration.UI_MODE_NIGHT_YES);
        }
    }


}