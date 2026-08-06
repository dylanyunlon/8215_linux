package com.hcn.autoeq.nine;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;

import com.hcn.autoeq.R;
import com.hcn_library.BaseFragment;
import com.hcn_library.util.SkinUtils;

import java.util.ArrayList;

public class NineSurroundFilterFragment extends BaseFragment implements View.OnClickListener {
    public static final int INDEX_NINE_GAIN = 0;
    public static final int INDEX_NINE_DELAY = 1;
    private static final String TAG = "NineSurroundFilterFragment";
    private FragmentResetInterface communicationInterfaceDelay;
    private FragmentResetInterface communicationInterfaceGain;
    private ArrayList<BaseFragment> fragments;
    private BaseFragment mNowFragment;
    private View mainView;
    private int position;
    private RadioGroup rb_main;
    private TextView tvReset;

    public interface FragmentResetInterface {
        void onReset();
    }

    @Override
    public int getLayoutRes() {
        return R.layout.nine_surround_fragment_filter;
    }

    public static NineSurroundFilterFragment newInstance() {
        return new NineSurroundFilterFragment();
    }

    @Override
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        View onCreateView = super.onCreateView(layoutInflater, viewGroup, bundle);
        mainView = onCreateView;
        ViewTreeObserver viewTreeObserver = mainView.getViewTreeObserver();
        viewTreeObserver.addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                // 移除监听器，避免重复调用
                if (mainView.getWidth() > 0 && mainView.getHeight() > 0) {
                    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.JELLY_BEAN) {
                        mainView.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                    } else {
                        mainView.getViewTreeObserver().removeGlobalOnLayoutListener(this);
                    }
                }

                int width = mainView.getWidth();
                int height = mainView.getHeight();
                // 处理获取到的宽高
                Log.d("test-surround", "Fragment width: " + width + ", height: " + height);
            }
        });
        return onCreateView;
    }

    @Override
    public void initView() {
        rb_main = (RadioGroup) mainView.findViewById(SkinUtils.getId(R.id.rg_surround));
        tvReset = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_surround_reset));
        initFragment();
        initListener();
        rb_main.check(SkinUtils.getId(R.id.rb_gain));
        refreshCheckedFragment();
        tvReset.setOnClickListener(this);
    }

    private void initListener() {
        tvReset.setOnClickListener(this);
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

    private void initFragment() {
        fragments = new ArrayList<>();
        NineSurroundGainFragment newInstance = NineSurroundGainFragment.newInstance();
        NineSurroundDelayFragment newInstance2 = NineSurroundDelayFragment.newInstance();
        fragments.add(newInstance);
        fragments.add(newInstance2);
        communicationInterfaceGain = newInstance;
        communicationInterfaceDelay = newInstance2;
    }


    public BaseFragment getFragment(int i) {
        ArrayList<BaseFragment> arrayList = fragments;
        if (arrayList == null || arrayList.size() <= 0) {
            return null;
        }
        return fragments.get(i);
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
                    beginTransaction.add(SkinUtils.getId(R.id.surround_filter_frame), baseFragment).commit();
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
        int i = rb_main.getCheckedRadioButtonId();
        if (i == SkinUtils.getId(R.id.rb_gain)) {
            position = INDEX_NINE_GAIN;
        } else if (i == SkinUtils.getId(R.id.rb_delay)) {
            position = INDEX_NINE_DELAY;
        }
        BaseFragment fragment = getFragment(position);
        switchFragment(mNowFragment, fragment);
        Log.d(TAG, "rb_main onTouch position: " + position);
    }


    @Override
    public void onClick(View view) {
        if (view.getId() == SkinUtils.getId(R.id.tv_surround_reset)) {
            BaseFragment baseFragment = mNowFragment;
            if (baseFragment instanceof NineSurroundGainFragment) {
                Log.d(TAG, "onclick--reset--NineSurroundGainFragment");
                communicationInterfaceGain.onReset();
            } else if (baseFragment instanceof NineSurroundDelayFragment) {
                Log.d(TAG, "onclick--reset--NineSurroundDelayFragment");
                communicationInterfaceDelay.onReset();
            }
        }
    }
}