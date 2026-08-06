package com.hcn.autoeq.fragment.fydsp;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.EventMessage;
import com.hcn.autoeq.data.FyDspBalanceSettings;
import com.hcn.autoeq.fragment.BaseFragment;

public class FyDspBalanceOrDelayFragment extends BaseFragment {

    private static final String TAG = FyDspBalanceOrDelayFragment.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(FyDspBalanceOrDelayFragment.class.getSimpleName(), Log.DEBUG);

    private View mainView;
    private FyDspBalanceSettings fyDspBalanceSettings;

    public FyDspBalanceOrDelayFragment() {
    }

    public static FyDspBalanceOrDelayFragment newInstance() {
        FyDspBalanceOrDelayFragment fragment = new FyDspBalanceOrDelayFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fydsp_fragment_balance_or_delay;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        fyDspBalanceSettings = FyDspBalanceSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        // 根据记忆来显示界面（balance or delay）
        if (fyDspBalanceSettings.getBalanceOrDelayUIMode() == FyDspBalanceSettings.UI_MODE.BALANCE.ordinal()) {
            onEvent(new EventMessage(EventMessage.MSG_DELAY_CHANGE_TO_BALANCE));
        } else {
            onEvent(new EventMessage(EventMessage.MSG_BALANCE_CHANGE_TO_DELAY));
        }
    }

    @Override
    public void onEvent(EventMessage eventMessage) {
        if (EventMessage.MSG_BALANCE_CHANGE_TO_DELAY.equals(eventMessage.getMessage())) {
            mainView.findViewById(R.id.fragment_fy_dsp_balance).setVisibility(View.GONE);
            mainView.findViewById(R.id.fragment_fy_dsp_delay).setVisibility(View.VISIBLE);
        } else if (EventMessage.MSG_DELAY_CHANGE_TO_BALANCE.equals(eventMessage.getMessage())) {
            mainView.findViewById(R.id.fragment_fy_dsp_balance).setVisibility(View.VISIBLE);
            mainView.findViewById(R.id.fragment_fy_dsp_delay).setVisibility(View.GONE);
        }
    }

}