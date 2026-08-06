package com.hcn.autoeq.nine;

import android.os.Build;
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
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.SkinUtils;

import java.util.ArrayList;

public class NineFilterFilterFragment extends BaseFragment implements View.OnClickListener {
    public static final int INDEX_NINE_BASS_BOOST = 1;
    public static final int INDEX_NINE_HLPT = 0;
    private FragmentResetInterface communicationInterfaceDBB;
    private FragmentResetInterface communicationInterfaceHLPF;
    private ArrayList<BaseFragment> fragments;
    private BaseFragment mNowFragment;
    private View mainView;
    private int position;
    private RadioGroup rb_main;
    private TextView tvReset;
    private RadioButton rbHlpf;
    private RadioButton rbDbb;


    public interface FragmentResetInterface {
        void onReset();
    }

    @Override
    public int getLayoutRes() {
        return R.layout.nine_filter_fragment_filter;
    }

    public static NineFilterFilterFragment newInstance() {
        return new NineFilterFilterFragment();
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
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                        mainView.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                    } else {
                        mainView.getViewTreeObserver().removeGlobalOnLayoutListener(this);
                    }
                }

                int width = mainView.getWidth();
                int height = mainView.getHeight();
                // 处理获取到的宽高
               Log.d("test--filter","Fragment width: " + width + ", height: " + height);
            }
        });
        return onCreateView;
    }

    @Override
    public void initView() {
        rbHlpf = mainView.findViewById(SkinUtils.getId(R.id.rb_hlpf));
        rbDbb = mainView.findViewById(SkinUtils.getId(R.id.rb_filter_bass_boost));
        rb_main = (RadioGroup) mainView.findViewById(SkinUtils.getId(R.id.rg_filter));
        tvReset = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_filter_reset));
        initFragment();
        initListener();
        initCheckedFragment();
        if (EqUtils.isChip7739()) {
            rb_main.setVisibility(View.INVISIBLE);
            rb_main.setEnabled(false);
            mainView.findViewById(SkinUtils.getId(R.id.tv_hlpf)).setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        if (!hidden && position == INDEX_NINE_HLPT) {
            final ViewTreeObserver vto = rbHlpf.getViewTreeObserver();
            vto.addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
                @Override
                public void onGlobalLayout() {
                    // 当控件有高度的时候才请求焦点
                    if (rbHlpf.getHeight() != 0){
                        rbHlpf.requestFocus();
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                            rbHlpf.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                        } else {
                            rbHlpf.getViewTreeObserver().removeGlobalOnLayoutListener(this);
                        }
                    }
                }
            });
        }
        super.onHiddenChanged(hidden);
    }

    private void initListener() {
        rbHlpf.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction();
                if (action == MotionEvent.ACTION_DOWN) {
                    rbHlpf.requestFocus();
                    rbDbb.clearFocus();
                    rbHlpf.setChecked(true);
                    position = INDEX_NINE_HLPT;
                    BaseFragment fragment = getFragment(position);
                    switchFragment(mNowFragment, fragment);
                }
                return false;
            }
        });

        rbDbb.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction();
                if (action == MotionEvent.ACTION_DOWN) {
                    rbHlpf.clearFocus();
                    rbDbb.requestFocus();
                    rbDbb.setChecked(true);
                    position = INDEX_NINE_BASS_BOOST;
                    BaseFragment fragment = getFragment(position);
                    switchFragment(mNowFragment, fragment);
                }
                return false;
            }
        });
        tvReset.setOnClickListener(this);
    }

    private void initFragment() {
        fragments = new ArrayList<>();
        NineDspHLPFFragment newInstance = NineDspHLPFFragment.newInstance();
        NineDspDbbFragment newInstance2 = NineDspDbbFragment.newInstance();
        fragments.add(newInstance);
        if (!EqUtils.isChip7739()) {
            fragments.add(newInstance2);
        }
        communicationInterfaceHLPF = newInstance;
        communicationInterfaceDBB = newInstance2;
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
                    beginTransaction.add(SkinUtils.getId(R.id.filter_filter_frame), baseFragment).commit();
                } else {
                    if (fragment != null) {
                        beginTransaction.hide(fragment);
                    }
                    beginTransaction.show(baseFragment).commit();
                }
            }
        }
    }

    private void initCheckedFragment() {
        rb_main.check(SkinUtils.getId(R.id.rb_hlpf));
        position = 0;
        BaseFragment fragment = getFragment(position);
        switchFragment(mNowFragment, fragment);
    }

    @Override
    public void onClick(View view) {
        if (rb_main.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_hlpf)) {
            communicationInterfaceHLPF.onReset();
        } else if (rb_main.getCheckedRadioButtonId() == SkinUtils.getId(R.id.rb_filter_bass_boost)) {
            communicationInterfaceDBB.onReset();
        }
    }
}