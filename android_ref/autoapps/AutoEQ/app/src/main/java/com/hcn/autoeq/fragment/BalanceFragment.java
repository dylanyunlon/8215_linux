package com.hcn.autoeq.fragment;

import android.graphics.Point;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.AspBalanceHandler;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.BalanceView;

public class BalanceFragment extends BaseFragment implements View.OnClickListener {

    private static final String ARG_PARAM1 = "save_data";
    private static final String ARG_PARAM2 = "param2";

    private int[] mBalanceData;
    private String mParam2;

    private View mMainBalanceView;
    private BalanceView mAspBalanceView;
    private AspBalanceHandler mAspBalanceHandler;

    public BalanceFragment() {
        // Required empty public constructor
    }

    public static BalanceFragment newInstance(int[] param1, String param2) {
        BalanceFragment fragment = new BalanceFragment();
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
            mBalanceData = getArguments().getIntArray(ARG_PARAM1);
            mParam2 = getArguments().getString(ARG_PARAM2);
        }
    }

    @Override
    public int getLayoutRes() {
        return R.layout.balance_fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mMainBalanceView = super.onCreateView(inflater, container, savedInstanceState);
        return mMainBalanceView;
    }

    @Override
    public void onStart() {
        super.onStart();
        startBalanceView();
    }

    @Override
    public void onStop() {
        super.onStop();
        stopBalanceView();
        Point p = mAspBalanceView.getBalanceLevel(mAspBalanceView.getCurrentX(), mAspBalanceView.getCurrentY());
        mAspBalanceHandler.saveData(p.x, p.y);
    }

    @Override
    public void initView() {
        mAspBalanceView = mMainBalanceView.findViewById(SkinUtils.getId(R.id.id_eq_balance));
        startBalanceView();
        mAspBalanceHandler = new AspBalanceHandler(getActivity().getApplicationContext(), mMainBalanceView);
        mAspBalanceHandler.update();

        mMainBalanceView.findViewById(SkinUtils.getId(R.id.balance_mode_main)).setOnClickListener(this);
        mMainBalanceView.findViewById(SkinUtils.getId(R.id.balance_mode_co)).setOnClickListener(this);
        mMainBalanceView.findViewById(SkinUtils.getId(R.id.balance_mode_rear)).setOnClickListener(this);
        if (!EqUtils.hasAsp()) {
            mMainBalanceView.findViewById(SkinUtils.getId(R.id.balance_mode_rear)).setVisibility(View.GONE);
        }
        mMainBalanceView.findViewById(SkinUtils.getId(R.id.balance_mode_whole)).setOnClickListener(this);
        mMainBalanceView.findViewById(SkinUtils.getId(R.id.balance_front)).setOnClickListener(this);
        mMainBalanceView.findViewById(SkinUtils.getId(R.id.balance_rear)).setOnClickListener(this);
        mMainBalanceView.findViewById(SkinUtils.getId(R.id.balance_left)).setOnClickListener(this);
        mMainBalanceView.findViewById(SkinUtils.getId(R.id.balance_right)).setOnClickListener(this);
    }

    private void startBalanceView() {
        if (mAspBalanceView != null) {
            mAspBalanceView.monitorThreadStart();
        }
    }

    private void stopBalanceView() {
        if (mAspBalanceView != null) {
            mAspBalanceView.monitorThreadStop();
        }
    }

    @Override
    public void onClick(View mButtonView) {
        int id = mButtonView.getId();
        if (id == SkinUtils.getId(R.id.balance_mode_main)) {
            mAspBalanceHandler.setBalance(3, 3);
        } else if (id == SkinUtils.getId(R.id.balance_mode_co)) {
            mAspBalanceHandler.setBalance(11, 3);
        } else if (id == SkinUtils.getId(R.id.balance_mode_rear)) {
            mAspBalanceHandler.setBalance(7, 11);
        } else if (id == SkinUtils.getId(R.id.balance_mode_whole)) {
            mAspBalanceHandler.setBalance(7, 7);
        } else if (id == SkinUtils.getId(R.id.balance_front)) {
            mAspBalanceHandler.onEqBalanceFront();
        } else if (id == SkinUtils.getId(R.id.balance_rear)) {
            mAspBalanceHandler.onEqBalanceRear();
        } else if (id == SkinUtils.getId(R.id.balance_left)) {
            mAspBalanceHandler.onEqBalanceLeft();
        } else if (id == SkinUtils.getId(R.id.balance_right)) {
            mAspBalanceHandler.onEqBalanceRight();
        }
    }
}