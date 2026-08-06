package com.hcn.autoeq.nine;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.ToggleButton;

import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;

import com.hcn.autoeq.R;
import com.hcn_library.BaseFragment;
import com.hcn_library.data.NineDspDtsFilterSettings;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;
import com.hcn_library.util.SkinUtils;

import java.util.ArrayList;

public class NineDtsFilterFragment extends BaseFragment implements CompoundButton.OnCheckedChangeListener, NineConstantExtDsp {
    public static final int INDEX_NINE_MODEL = 0;
    public static final int INDEX_NINE_SOUND_FOCUS = 1;
    public static final int INDEX_NINE_SURROUND = 2;
    public static final int INDEX_NINE_BASS_BOOST = 3;
    public static final int INDEX_NINE_VIRTUAL_CENTER = -1;
    private static final String TAG = NineDtsFilterFragment.class.getSimpleName();
    private ToggleButton bt_dts;
    private ToggleButton bt_dts_center;
    private TextView tvCenter;
    private boolean centerSwitchFromUser;
    private boolean dtsSwitchFromUser;
    private EnableViewInterface enableBassBoost;
    private EnableViewInterface enableModel;
    private EnableViewInterface enableSoundFocus;
    private EnableViewInterface enableSurround;
    private EnableViewInterface enableVirtualCenter;
    private ArrayList<BaseFragment> fragments;
    private BaseFragment mNowFragment;
    private View mainView;
    private NineDspDtsFilterSettings nineDspDtsFilterSettings;
    private int position;
    private RadioGroup rb_main;

    public interface EnableViewInterface {
        void updateView(boolean enable);
    }

    @Override
    public int getLayoutRes() {
        return R.layout.nine_dts_fragment_filter;
    }

    public NineDtsFilterFragment() {
    }

    public static NineDtsFilterFragment newInstance() {
        return new NineDtsFilterFragment();
    }

    @Override
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        mainView = super.onCreateView(layoutInflater, viewGroup, bundle);
        nineDspDtsFilterSettings = NineDspDtsFilterSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        rb_main = (RadioGroup) mainView.findViewById(SkinUtils.getId(R.id.rg_dts));
        bt_dts = (ToggleButton) mainView.findViewById(SkinUtils.getId(R.id.bt_dts));
        bt_dts_center = (ToggleButton) mainView.findViewById(SkinUtils.getId(R.id.bt_dts_center));
        tvCenter = mainView.findViewById(SkinUtils.getId(R.id.tv_center));
        initFragment();
        initListener();
        rb_main.check(EqUtils.isChip7739() ? SkinUtils.getId(R.id.rb_virtual_center) : SkinUtils.getId(R.id.rb_situation));
        refreshCheckedFragment();
        bt_dts.setChecked(nineDspDtsFilterSettings.getDtsSwitch() == 0);
        bt_dts_center.setChecked(nineDspDtsFilterSettings.getCenterSwitch() == 1);
        if (EqUtils.isChip7739()) {
            ViewGroup.LayoutParams params = rb_main.getLayoutParams();
            params.width = 1200;
            rb_main.setLayoutParams(params);
            bt_dts.setVisibility(View.GONE);
            mainView.findViewById(SkinUtils.getId(R.id.tv_dts)).setVisibility(View.GONE);
            mainView.findViewById(SkinUtils.getId(R.id.rb_virtual_center)).setVisibility(View.VISIBLE);
        }
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
        bt_dts.setOnCheckedChangeListener(this);
        bt_dts_center.setOnCheckedChangeListener(this);
        bt_dts.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                Log.d(TAG, "bt_dts onTouch");
                dtsSwitchFromUser = true;
                return false;
            }
        });
        bt_dts_center.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                Log.d(TAG, "bt_dts_center onTouch");
                centerSwitchFromUser = true;
                return false;
            }
        });
    }

    private void initFragment() {
        fragments = new ArrayList<>();
        NineDTSFragment newInstance = NineDTSFragment.newInstance();
        NineDtsSoundFocusFragment newInstance2 = NineDtsSoundFocusFragment.newInstance();
        NineDtsSurroundFragment newInstance3 = NineDtsSurroundFragment.newInstance();
        NineDtsBassBoostFragment newInstance4 = NineDtsBassBoostFragment.newInstance();
        if (EqUtils.isChip7739()) {
            NineDtsVirtualCenterFragment virtualCenterFragment = NineDtsVirtualCenterFragment.newInstance();
            fragments.add(virtualCenterFragment);
            enableVirtualCenter = virtualCenterFragment;
        }
        fragments.add(newInstance);
        fragments.add(newInstance2);
        fragments.add(newInstance3);
        fragments.add(newInstance4);
        enableModel = newInstance;
        enableSoundFocus = newInstance2;
        enableSurround = newInstance3;
        enableBassBoost = newInstance4;
    }


    public BaseFragment getFragment(int position) {
        ArrayList<BaseFragment> arrayList = fragments;
        if (arrayList == null || arrayList.size() <= 0) {
            return null;
        }
        return fragments.get(position);
    }

    public void switchFragment(Fragment fragment, BaseFragment baseFragment) {
        if (mNowFragment != baseFragment) {
            mNowFragment = baseFragment;
            if (baseFragment != null) {
                FragmentTransaction beginTransaction = getParentFragmentManager().beginTransaction();
                if (!baseFragment.isAdded()) {
                    if (fragment != null) {
                        beginTransaction.hide(fragment);
                    }
                    beginTransaction.add(SkinUtils.getId(R.id.bts_filter_frame), baseFragment).commit();
                } else {
                    if (fragment != null) {
                        beginTransaction.hide(fragment);
                    }
                    beginTransaction.show(baseFragment).commit();
                }
            }
        }
    }

    private void refreshCheckedFragment() {
        int id = rb_main.getCheckedRadioButtonId();
        if (id == SkinUtils.getId(R.id.rb_virtual_center)) {
            position = INDEX_NINE_VIRTUAL_CENTER;
        } else if (id == SkinUtils.getId(R.id.rb_situation)) {
            position = INDEX_NINE_MODEL;
        } else if (id == SkinUtils.getId(R.id.rb_sound_focus)) {
            position = INDEX_NINE_SOUND_FOCUS;
        } else if (id == SkinUtils.getId(R.id.rb_surround)) {
            position = INDEX_NINE_SURROUND;
        } else if (id == SkinUtils.getId(R.id.rb_bass_boost)) {
            position = INDEX_NINE_BASS_BOOST;
        } else {
            position = EqUtils.isChip7739() ? INDEX_NINE_VIRTUAL_CENTER : INDEX_NINE_MODEL;
        }
        if (EqUtils.isChip7739()) {
            if (position == INDEX_NINE_VIRTUAL_CENTER) {
                tvCenter.setText(R.string.nine_tv_channel_center);
                bt_dts_center.setChecked(nineDspDtsFilterSettings.getCenterSwitchEnable());
                enableVirtualCenter.updateView(nineDspDtsFilterSettings.getCenterSwitchEnable());
            } else if (position == INDEX_NINE_MODEL) {
                tvCenter.setText(R.string.nint_btn_dts_bar_model);
                bt_dts_center.setChecked(nineDspDtsFilterSettings.getDtsSwitchEnable());
                enableModel.updateView(nineDspDtsFilterSettings.getDtsSwitchEnable());
            } else if (position == INDEX_NINE_SOUND_FOCUS) {
                tvCenter.setText(R.string.nine_btn_dts_bar_sound_focus);
                bt_dts_center.setChecked(nineDspDtsFilterSettings.getSoundFocusSwitchEnable());
                enableSoundFocus.updateView(nineDspDtsFilterSettings.getSoundFocusSwitchEnable());
            } else if (position == INDEX_NINE_SURROUND) {
                tvCenter.setText(R.string.surround_check_surround);
                bt_dts_center.setChecked(nineDspDtsFilterSettings.getSurroundSwitchEnable());
                enableSurround.updateView(nineDspDtsFilterSettings.getSurroundSwitchEnable());
            } else if (position == INDEX_NINE_BASS_BOOST) {
                tvCenter.setText(R.string.nine_btn_dts_bar_bass_boost);
                bt_dts_center.setChecked(nineDspDtsFilterSettings.getBassBoostSwitchEnable());
                enableBassBoost.updateView(nineDspDtsFilterSettings.getBassBoostSwitchEnable());
            }
        }
        BaseFragment fragment = getFragment(EqUtils.isChip7739() ? (position + 1) : position);
        switchFragment(mNowFragment, fragment);
        Log.d(TAG, "refreshCheckedFragment position: " + position);
    }

    @Override
    public void onCheckedChanged(CompoundButton compoundButton, boolean checked) {
        if (compoundButton.getId() == SkinUtils.getId(R.id.bt_dts)) {
            if (dtsSwitchFromUser) {
                dtsSwitchFromUser = false;
                nineDspDtsFilterSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_BYPASS, !checked ? 1 : 0);
                nineDspDtsFilterSettings.saveDtsSwitch(!checked ? 1 : 0);
                refreshEnableStatus();
            }
            bt_dts_center.setEnabled(checked);
            bt_dts_center.setAlpha(checked ? 1.0f : 0.4f);
        } else if (compoundButton.getId() == SkinUtils.getId(R.id.bt_dts_center) && centerSwitchFromUser) {
            centerSwitchFromUser = false;
            if (EqUtils.isChip7739()) {
                switch (position) {
                    case INDEX_NINE_VIRTUAL_CENTER:
                        nineDspDtsFilterSettings.saveCenterSwitch(checked ? 1 : 0);
                        enableVirtualCenter.updateView(checked);
                        break;
                    case INDEX_NINE_MODEL:
                        nineDspDtsFilterSettings.saveDtsSwitch(checked ? 0 : 1);
                        enableModel.updateView(checked);
                        break;
                    case INDEX_NINE_SOUND_FOCUS:
                        nineDspDtsFilterSettings.saveSoundFocusSwitch(checked ? 0 : 1);
                        enableSoundFocus.updateView(checked);
                        break;
                    case INDEX_NINE_SURROUND:
                        nineDspDtsFilterSettings.saveSurroundSwitch(checked ? 0 : 1);
                        enableSurround.updateView(checked);
                        break;
                    case INDEX_NINE_BASS_BOOST:
                        nineDspDtsFilterSettings.saveBassBoostSwitch(checked ? 0 : 1);
                        enableBassBoost.updateView(checked);
                        break;
                    default:
                        break;
                }
            } else {
                nineDspDtsFilterSettings.saveCenterSwitch(checked ? 1 : 0);
                nineDspDtsFilterSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_PHANTOM_CENTER_ENABLE, checked ? 1 : 0);
            }
            refreshEnableStatus();
        }
    }

    private void refreshEnableStatus() {
        if (EqUtils.isChip7739()) return;
        boolean enable = nineDspDtsFilterSettings.getDtsSwitch() == 0;
        if (rb_main.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_situation) && enableModel != null) {
            enableModel.updateView(enable);
            return;
        }
        if (rb_main.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_sound_focus) && enableSoundFocus != null) {
            enableSoundFocus.updateView(enable);
            return;
        }
        if (rb_main.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_surround) && enableSurround != null) {
            enableSurround.updateView(enable);
        }
        if (rb_main.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_bass_boost) && enableBassBoost != null) {
            enableBassBoost.updateView(enable);
        }

        if (rb_main.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_virtual_center) && enableVirtualCenter != null) {
            enableVirtualCenter.updateView(nineDspDtsFilterSettings.getCenterSwitch() == 0);
        }

    }
}