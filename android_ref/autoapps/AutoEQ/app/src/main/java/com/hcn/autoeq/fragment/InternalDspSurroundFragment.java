package com.hcn.autoeq.fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.DspInternalSettings;
import com.hcn.autoeq.util.SkinUtils;

public class InternalDspSurroundFragment extends BaseFragment {

    private static final String ARG_PARAM1 = "save_data";
    private static final String ARG_PARAM2 = "param2";

    private int[] mSurroundData;
    private String mParam2;

    private View mMainSurroundView;
    private CheckBox mSurroundCheck, mBassCheck;
    private DspInternalSettings mDspInternalSettings;

    public InternalDspSurroundFragment() {
        // Required empty public constructor
    }

    public static InternalDspSurroundFragment newInstance(int[] param1, String param2) {
        InternalDspSurroundFragment fragment = new InternalDspSurroundFragment();
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
            mSurroundData = getArguments().getIntArray(ARG_PARAM1);
            mParam2 = getArguments().getString(ARG_PARAM2);
        }
    }

    @Override
    public int getLayoutRes() {
        return R.layout.internal_dsp_surround_fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mMainSurroundView = super.onCreateView(inflater, container, savedInstanceState);
        mDspInternalSettings = DspInternalSettings.getInstance(mContext);
        return mMainSurroundView;
    }

    @Override
    public void initView() {
        mSurroundCheck = mMainSurroundView.findViewById(SkinUtils.getId(R.id.surround_ck_surround));
        mSurroundCheck.setChecked(mDspInternalSettings.getDspSurround());
        mSurroundCheck.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean mCheck) {
                mDspInternalSettings.setDspSurround(mCheck ? 10 : 0);
            }
        });
        mBassCheck = mMainSurroundView.findViewById(SkinUtils.getId(R.id.surround_ck_bassboot));
        mBassCheck.setChecked(mDspInternalSettings.getDspBassBoost() > 0);
        mBassCheck.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean mCheck) {
                mDspInternalSettings.setDspBassBoost(mCheck ? 10 : 0, true);
            }
        });
    }
}