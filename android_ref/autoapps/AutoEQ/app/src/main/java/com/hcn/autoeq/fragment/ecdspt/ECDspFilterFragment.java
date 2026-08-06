package com.hcn.autoeq.fragment.ecdspt;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RadioGroup;

import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;

import com.hcn.autoeq.R;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.SkinUtils;

import java.util.ArrayList;

public class ECDspFilterFragment extends BaseFragment {

    public static final int INDEX_EXTDSP_HLPF = 0;
    public static final int INDEX_EXTDSP_DBB = 1;
    public static final int INDEX_EXTDSP_SURROUND = 2;

    private View mainView;
    private RadioGroup rb_main;
    private BaseFragment mNowFragment;
    private ArrayList<BaseFragment> fragments;

    private int position;

    public ECDspFilterFragment() {
    }

    public static ECDspFilterFragment newInstance() {
        ECDspFilterFragment fragment = new ECDspFilterFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        if (checkLayoutExists(R.layout.ext_c_dsp_fragment_filter)) {
            return R.layout.ext_c_dsp_fragment_filter;
        } else {
            return R.layout.extdsp_fragment_filter;
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        return mainView;
    }

    @Override
    public void initView() {
        rb_main = mainView.findViewById(SkinUtils.getId(R.id.rg_extdsp_hlpf_main));
        initFragment();
        initListener();
        rb_main.check(SkinUtils.getId(R.id.rb_extdsp_hlpf));
    }

    private void initListener() {
        rb_main.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup radioGroup, int mViewId) {
                if (mViewId == SkinUtils.getId(R.id.rb_extdsp_hlpf)) {
                    position = INDEX_EXTDSP_HLPF;
                } else if (mViewId == SkinUtils.getId(R.id.rb_extdsp_dbb)) {
                    position = INDEX_EXTDSP_DBB;
                } else if (mViewId == SkinUtils.getId(R.id.rb_extdsp_surround)) {
                    position = INDEX_EXTDSP_SURROUND;
                } else {
                    position = INDEX_EXTDSP_HLPF;
                }
                //根据位置得到相应的Fragment
                BaseFragment baseFragment = getFragment(position);
                /**
                 * 第一个参数: 上次显示的Fragment
                 * 第二个参数: 当前正要显示的Fragment
                 */
                switchFragment(mNowFragment, baseFragment);
            }
        });
    }

    private void initFragment() {
        fragments = new ArrayList<>();
        fragments.add(ECDspHLPFFragment.newInstance());
        fragments.add(ECDspDbbFragment.newInstance());
        fragments.add(ECDspSurroundFragment.newInstance());
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
        if (mNowFragment != mNextFragment) {
            mNowFragment = mNextFragment;
            if (mNextFragment != null) {
                FragmentTransaction transaction = getParentFragmentManager().beginTransaction();
                if (!mNextFragment.isAdded()) {
                    if (fragment != null) {
                        transaction.hide(fragment);
                    }
                    transaction.add(SkinUtils.getId(R.id.filter_frame), mNextFragment).commit();
                } else {
                    if (fragment != null) {
                        transaction.hide(fragment);
                    }
                    transaction.show(mNextFragment).commit();
                }
            }
        }
    }
}