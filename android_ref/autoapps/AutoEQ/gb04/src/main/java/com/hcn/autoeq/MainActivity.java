package com.hcn.autoeq;

import static com.hcn_library.util.EqUtils.KEY_SKIN;

import android.annotation.SuppressLint;
import android.app.UiModeManager;
import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.drawable.BitmapDrawable;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import androidx.annotation.NonNull;
import androidx.core.view.ViewCompat;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;

import com.hcn.autoeq.nine.NineDspBalanceFragment;
import com.hcn.autoeq.nine.NineDspBandFragment;
import com.hcn.autoeq.nine.NineDspUserModeExitDialog;
import com.hcn.autoeq.nine.NineDtsFilterFragment;
import com.hcn.autoeq.nine.NineFilterFilterFragment;
import com.hcn.autoeq.nine.NineSurroundFilterFragment;
import com.hcn.skin.support.SkinCompatManager;
import com.hcn.skin.support.app.SkinCompatActivity;
import com.hcn.skin.support.resources.SkinCompatResources;
import com.hcn_library.BaseFragment;
import com.hcn_library.data.NineDspBandSettings;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.FastBlurUtils;
import com.hcn_library.util.SkinUtils;
import com.hcn_library.util.SystemUtils;

import java.util.ArrayList;

public class MainActivity extends SkinCompatActivity {

    static final String TAG = MainActivity.class.getSimpleName();

    // 九品主页索引
    public static final int INDEX_NINE_DTS = 0;
    public static final int INDEX_NINE_EQ = 1;
    public static final int INDEX_NINE_MODEL = 2;
    public static final int INDEX_NINE_SURROUND = 3;
    public static final int INDEX_NINE_FILTER = 4;


    private RadioGroup rb_main;
    private RadioButton dts;
    private BaseFragment mNowFragment;
    private ArrayList<BaseFragment> fragments;

    private int position = 0;
    private int[] mBalanceData;
    private int[] mBandData;
    private int[] mSurroundData;
    private int[] mDspBandData;

    private int lastIndex = -1;

    int current = -1;
    String skinName;
    private int[] reverbBackgroundDrawables = {R.drawable.nine_scene_default, R.drawable.nine_scene_user, R.drawable.nine_scene_user, R.drawable.nine_scene_user
            , R.drawable.nine_scene_classical, R.drawable.nine_scene_news, R.drawable.nine_scene_popular, R.drawable.nine_scene_city
            , R.drawable.nine_scene_cinema, R.drawable.nine_scene_electronic, R.drawable.nine_scene_rock, R.drawable.nine_scene_high_tech, R.drawable.nine_scene_jazz};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        if (EqUtils.isChip7739()) {
            disableNightMode();
        }
        super.onCreate(savedInstanceState);
        skinName = EqUtils.getSkinName();
        Log.d(TAG, "onCreate: " + EqUtils.getEqChipType() + EqUtils.is6225());
        initNineView();
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
    //九品
    private void initNineView() {
        Log.d(TAG,"dsp chip=" + SystemUtils.getSystemProperty("persist.sys.dspui", "NULL"));
        Log.d(TAG,"skinName=" + SystemUtils.getSystemProperty(KEY_SKIN, ""));
        SystemUtils.fullScreen(MainActivity.this);
        setContentView(R.layout.nine_activity_main);
        rb_main = findViewById(SkinUtils.getId(R.id.rg_main));
        dts = findViewById(SkinUtils.getId(R.id.rb_nine_dts));
        if (EqUtils.DSP_CHIP_SI47925_DTS.equals(EqUtils.getEqChipType())
                || EqUtils.isChip7739()) {
            rb_main.check(SkinUtils.getId(R.id.rb_nine_dts));
            dts.setVisibility(View.VISIBLE);
            initDtsMargin();
            if (EqUtils.isChip7739()) {
                dts.setText(R.string.dsp_tab_title_surround);
                dts.setBackground(getDrawable(R.drawable.nine_main_radio_bg_selector1_gb02));
//                        ((Ak7739) NativeHelper.getEq()).setResultTv(findViewById(SkinUtils.getId(R.id.tv_result)));
            }
        } else {
            rb_main.check(SkinUtils.getId(R.id.rb_nine_eq));
            rb_main.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
                @Override
                public void onGlobalLayout() {
                    int measuredWidth = rb_main.getMeasuredWidth();
                    int measuredHeight = rb_main.getMeasuredHeight();
                    View[] views = new View[]{rb_main};
                    boolean[] ifBlur = new boolean[]{true};
                    boolean[] ifWindow = new boolean[]{false};
                    int[] blurRadii = {25};
                    int[] sampleSizes = {4};
                    int[] drawableIds = {R.drawable.nine_main_radio_group_bg_eq};
                    float[] cornerRadii = {SkinUtils.getDimension(R.dimen.x24)};
                    int currentDrawableId = reverbBackgroundDrawables[NineDspBandSettings.getInstance(MainActivity.this).getReverb()];
                    if (measuredWidth > 0 && measuredHeight > 0) {
                        FastBlurUtils.applyGlassEffectOptimized(ifBlur, views, blurRadii, sampleSizes, ifWindow, cornerRadii, drawableIds, currentDrawableId, null, MainActivity.this);
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                            rb_main.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                        } else {
                            rb_main.getViewTreeObserver().removeGlobalOnLayoutListener(this);
                        }
                    }
                    Log.d(TAG, "onGlobalLayout rg_main  width: " + measuredWidth + "  height: " + measuredHeight);
                }
            });
        }
        initFragment();
        initListener();
        refreshCheckedFragment();

    }
    private void initListener() {
        for (int i = 0; i < rb_main.getChildCount(); i++) {
            View child = rb_main.getChildAt(i);
            if (child instanceof RadioButton) {
                RadioButton radioButton = (RadioButton) child;
                radioButton.setOnTouchListener(new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        if (event.getAction() == MotionEvent.ACTION_DOWN) {
                            radioButton.setChecked(true);
                            refreshCheckedFragment();
                        }
                        return false;
                    }

                });
            }
        }
    }

    private void refreshCheckedFragment() {
        int mViewId = rb_main.getCheckedRadioButtonId();
        if (mViewId == SkinUtils.getId(R.id.rb_nine_dts)) {
            position = INDEX_NINE_DTS;
        } else if (mViewId == SkinUtils.getId(R.id.rb_nine_eq)) {
            position = INDEX_NINE_EQ;
        } else if (mViewId == SkinUtils.getId(R.id.rb_nine_model)) {
            position = INDEX_NINE_MODEL;
        } else if (mViewId == SkinUtils.getId(R.id.rb_nine_surround)) {
            position = INDEX_NINE_SURROUND;
        } else {
            position = INDEX_NINE_FILTER;
        }
        if (lastIndex != SkinUtils.getId(R.id.rb_nine_eq) || mViewId == SkinUtils.getId(R.id.rb_nine_eq) || !NineDspBandFragment.isEditing) {
            setAppBackground(mViewId);
            performFragmentSwitch(position, mViewId);
        } else {
            Log.d(MainActivity.TAG, "showExitDialog mode:  3");
            dialog = NineDspUserModeExitDialog.newInstance(3);
            dialog.setUserName(NineDspBandSettings.getInstance(MainActivity.this).getCurrentCustomReverbName());
            dialog.setOnDialogListener(new NineDspUserModeExitDialog.OnDialogListener() {
                @Override
                public void isOk(int i2) {
                    if (getFragment(1) != null) {
                        ((NineDspBandFragment) getFragment(1)).applyBandEdit();
                        setAppBackground(mViewId);
                        performFragmentSwitch(position, mViewId);
                        Log.d(MainActivity.TAG, "isOK, exit and not save user band data");
                    }
                }

                @Override
                public void isCancel(int i2) {
                    if (getFragment(1) != null) {
                        ((NineDspBandFragment) getFragment(1)).recycleOldData();
                        setAppBackground(mViewId);
                        performFragmentSwitch(position, mViewId);
                        Log.d(MainActivity.TAG, " isCancel, stop switch fragment");
                    }
                }
            });
            dialog.show(getSupportFragmentManager(), "exit_dialog");
        }
        Log.d(TAG, "refreshCheckedFragment, position: " + position + "  mViewId: " + mViewId);
    }

    public void setAppBackground(int i) {
        if (i == SkinUtils.getId(R.id.rb_nine_eq)) {
            int currentDrawableId = reverbBackgroundDrawables[NineDspBandSettings.getInstance(this).getReverb()];
            findViewById(SkinUtils.getId(R.id.main_layout)).setBackground(SkinUtils.getDrawable(currentDrawableId));
            View[] views = new View[]{rb_main};
            boolean[] ifBlur = new boolean[]{true};
            boolean[] ifWindow = new boolean[]{false};
            int[] blurRadii = {12};
            int[] sampleSizes = {8};
            int[] drawableIds = {R.drawable.nine_main_radio_group_bg_eq};
            float[] cornerRadii = {SkinUtils.getDimension(R.dimen.x24)};
            FastBlurUtils.applyGlassEffectOptimized(ifBlur, views, blurRadii, sampleSizes, ifWindow, cornerRadii, drawableIds, currentDrawableId, null, MainActivity.this);
        } else if (i != SkinUtils.getId(R.id.rb_nine_eq)) {
            findViewById(SkinUtils.getId(R.id.main_layout)).setBackground(SkinUtils.getDrawable(R.drawable.nine_main_background));
            findViewById(SkinUtils.getId(R.id.rg_main)).setBackground(SkinUtils.getDrawable(R.drawable.nine_main_radio_group_bg));
        }
    }

    public void performFragmentSwitch(int positon, int mViewId) {
        Log.d(TAG, "performFragmentSwitch: position = " + positon + "  mViewId = " + mViewId);
        BaseFragment fragment = getFragment(positon);
        lastIndex = mViewId;
        preAddFragment(fragment);
        switchFragment(mNowFragment, fragment);
    }

    private void initFragment() {
        Log.d(TAG, "initFragment: current eq chip type = " + EqUtils.getEqChipType());
        fragments = new ArrayList<>();
        if (dts.getVisibility() == View.VISIBLE) {
            fragments.add(NineDtsFilterFragment.newInstance());
        } else {
            fragments.add(null);
        }
        fragments.add(NineDspBandFragment.newInstance());
        fragments.add(NineDspBalanceFragment.newInstance());
        fragments.add(NineSurroundFilterFragment.newInstance());
        fragments.add(NineFilterFilterFragment.newInstance());
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
        disableNightMode();
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
            initNineView();
            initMode();
        }
        current = uiModeManager.getNightMode();
        Log.d(TAG, "getNightMode:" + current);
    }

    // 该方法用于根据当前系统的暗黑模式状态设置状态栏颜色
    public void initMode() {
        if (EqUtils.isChip7739()) {
            if (SkinCompatResources.getInstance().getSkinResources() != null) {
                Log.d(TAG, "updateConfiguration getSkinResources");
                SkinCompatResources.getInstance().getSkinResources().updateConfiguration(disableNightMode(), getResources().getDisplayMetrics());
            } else {
                Log.d(TAG, "updateConfiguration getAppContextResources");
                SkinCompatManager.getInstance().getContext().getResources().updateConfiguration(disableNightMode(), getResources().getDisplayMetrics());
            }
            return;
        }
        Configuration configuration = getResources().getConfiguration();
        int currentNightMode = configuration.uiMode
                & Configuration.UI_MODE_NIGHT_MASK;
        if (Build.VERSION.SDK_INT >= 30) {
            ViewCompat.getWindowInsetsController(getWindow().getDecorView()).setAppearanceLightStatusBars(currentNightMode != Configuration.UI_MODE_NIGHT_YES);
        }
    }

    /**
     *  初始化记录白天黑夜的 current，不然感觉当第一次 onConfigurationChanged 调用时，都会跑一次初始化
     */
    private void initCurrent() {
        UiModeManager uiModeManager = (UiModeManager) this.getSystemService(Context.UI_MODE_SERVICE);
        current = uiModeManager.getNightMode();
        initMode();
    }

    private boolean isConfirmed = true;
    private NineDspUserModeExitDialog dialog;

    @Override
    public void onBackPressed() {
        Log.d(TAG, "onBackPressed");
        if (lastIndex == SkinUtils.getId(R.id.rb_nine_eq) && NineDspBandFragment.isEditing) {
            isConfirmed = false;
        }
        if (!isConfirmed) {
            dialog = NineDspUserModeExitDialog.newInstance(4);
            dialog.setUserName(NineDspBandSettings.getInstance(MainActivity.this).getCurrentCustomReverbName());
            dialog.setOnDialogListener(new NineDspUserModeExitDialog.OnDialogListener() {
                @Override
                public void isOk(int i) {
                    if (getFragment(1) != null) {
                        ((NineDspBandFragment) getFragment(1)).applyBandEdit();
                        isConfirmed = true;
                        finish();
                        Log.d(MainActivity.TAG, "isOK, onPause and save user band data");
                    }
                }

                @Override
                public void isCancel(int i) {
                    if (getFragment(1) != null) {
                        ((NineDspBandFragment) getFragment(1)).recycleOldData();
                        isConfirmed = true;
                        finish();
                    }
                    Log.d(MainActivity.TAG, " isCancel, onBackPressed");
                }
            });
            dialog.show(getSupportFragmentManager(), "exit_dialog");
            return;
        }
        super.onBackPressed();
    }
    // 根据芯片型号判断是否显示DTS功能，同时调节间距
    private void initDtsMargin() {
        int marginInPx = (int) SkinUtils.getDimension(R.dimen.y20);
        int childCount = rb_main.getChildCount();
        for (int i = 1; i < childCount; i++) {
            // 获取子 RadioButton
            RadioButton child = (RadioButton) rb_main.getChildAt(i);
            if (child instanceof RadioButton && i != 0) {
                // 获取 RadioButton 的布局参数
                ViewGroup.MarginLayoutParams params = (ViewGroup.MarginLayoutParams) child.getLayoutParams();
                // 设置 marginTop 的值
                params.topMargin = marginInPx;
                // 重新设置 RadioButton 的布局参数
                child.setLayoutParams(params);
            }
        }
    }

    private Configuration disableNightMode() {
        Configuration config = getResources().getConfiguration();
        // 清除当前的夜间模式标志
        config.uiMode = config.uiMode & ~Configuration.UI_MODE_NIGHT_MASK;
        // 设置为日间模式
        config.uiMode = config.uiMode | Configuration.UI_MODE_NIGHT_NO;
        getResources().updateConfiguration(config, getResources().getDisplayMetrics());
        return config;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        FastBlurUtils.releaseRenderScript();
        Log.d(TAG, "onDestroy");
    }
}