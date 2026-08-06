package com.hcn.autoeq.fragment.fydsp;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RadioGroup;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.EventMessage;
import com.hcn.autoeq.bean.FyDspOutputMode;
import com.hcn.autoeq.data.FyDspBalanceSettings;
import com.hcn.autoeq.data.FyDspDelaySettings;
import com.hcn.autoeq.data.FyDspHLPFSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.view.FyDspDelayView;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.util.ArrayList;
import java.util.List;

public class FyDspDelayFragment extends BaseFragment implements RadioGroup.OnCheckedChangeListener, View.OnClickListener, FyDspAlertDialog.Callback, FyDspDelayView.Callback {

    private static final String TAG = FyDspDelayFragment.class.getSimpleName();
    private View mainView;
    private FyDspDelayView fddvLF, fddvLR_WAY2, fddvLR_CHANNEL51, fddvRF, fddvRR_WAY2, fddvRR_CHANNEL51, fddvCenter, fddvSubwoofer;
    private RadioGroup rgDelayUnit;
    private Button btnSample, btnReset;
    private ImageView ivDelayCar;

    private List<FyDspDelayView> fyDspDelayViewList = new ArrayList<>();
    private FyDspBalanceSettings fyDspBalanceSettings;
    private FyDspDelaySettings fyDspDelaySettings;
    private FyDspHLPFSettings fyDspHLPFSettings;
    private FyDspOutputMode fyDspOutputMode;

    public FyDspDelayFragment() {
    }

    public static FyDspDelayFragment newInstance() {
        FyDspDelayFragment fragment = new FyDspDelayFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fydsp_fragment_delay;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        fyDspBalanceSettings = FyDspBalanceSettings.getInstance(mContext);
        fyDspDelaySettings = FyDspDelaySettings.getInstance(mContext);
        fyDspHLPFSettings = FyDspHLPFSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        fddvLF = mainView.findViewById(R.id.fddv_lf);
        fddvLR_WAY2 = mainView.findViewById(R.id.fddv_way2_lr);
        fddvLR_CHANNEL51 = mainView.findViewById(R.id.fddv_channel51_lr);
        fddvRF = mainView.findViewById(R.id.fddv_rf);
        fddvRR_WAY2 = mainView.findViewById(R.id.fddv_way2_rr);
        fddvRR_CHANNEL51 = mainView.findViewById(R.id.fddv_channel51_rr);
        fddvCenter = mainView.findViewById(R.id.fddv_center);
        fddvSubwoofer = mainView.findViewById(R.id.fddv_subwoofer);

        fyDspDelayViewList.add(fddvLF);
        fyDspDelayViewList.add(fddvLR_WAY2);
        fyDspDelayViewList.add(fddvLR_CHANNEL51);
        fyDspDelayViewList.add(fddvRF);
        fyDspDelayViewList.add(fddvRR_WAY2);
        fyDspDelayViewList.add(fddvRR_CHANNEL51);
        fyDspDelayViewList.add(fddvCenter);
        fyDspDelayViewList.add(fddvSubwoofer);

        for (FyDspDelayView fyDspDelayView : fyDspDelayViewList) {
            fyDspDelayView.setCallback(this);
        }

        rgDelayUnit = mainView.findViewById(R.id.rg_delay_unit);
        rgDelayUnit.setOnCheckedChangeListener(this);
        rgDelayUnit.check(R.id.rb_delay_unit_ms);

        btnSample = mainView.findViewById(R.id.btn_sample);
        btnReset = mainView.findViewById(R.id.btn_reset_delay);
        btnSample.setOnClickListener(this);
        btnReset.setOnClickListener(this);

        ivDelayCar = mainView.findViewById(R.id.iv_delay_car);
    }

    @Override
    public void initData() {
        super.initData();
        fyDspOutputMode = fyDspHLPFSettings.getOutputMode();
        refreshViews();
    }

    private void refreshViews() {
        fddvCenter.setVisibility(fyDspOutputMode == FyDspOutputMode.CHANNEL51 ? View.VISIBLE : View.GONE);
        fddvSubwoofer.setVisibility(fyDspOutputMode == FyDspOutputMode.CHANNEL51 ? View.VISIBLE : View.GONE);

        // 5.1 声道时，不需要显示第二排
        fddvLR_WAY2.setVisibility(fyDspOutputMode == FyDspOutputMode.CHANNEL51 ? View.GONE : View.VISIBLE);
        fddvRR_WAY2.setVisibility(fyDspOutputMode == FyDspOutputMode.CHANNEL51 ? View.GONE : View.VISIBLE);

        // 第三排，在5.1声道时是左后右后，在WAY2/WAY3/WAY6时是重左重右（中置和重低音）
        fddvLR_CHANNEL51.setTag(fyDspOutputMode == FyDspOutputMode.CHANNEL51 ? "LR" : "SUBWOOFER");
        fddvRR_CHANNEL51.setTag(fyDspOutputMode == FyDspOutputMode.CHANNEL51 ? "RR" : "CENTER");

        // 注意：只操作可见控件
        fyDspDelayViewList.stream()
                .filter(fyDspDelayView -> fyDspDelayView.getVisibility() == View.VISIBLE)
                .forEach(fyDspDelayView -> {
                    String tag = (String) fyDspDelayView.getTag();
                    fyDspDelayView.setTimeValue(fyDspDelaySettings.getDelay(tag));
                    if ("LF".equals(tag)) {
                        fyDspDelayView.setTitle(getString(R.string.fydsp_attenuate_way6_lf));
                    } else if ("LR".equals(tag)) {
                        fyDspDelayView.setTitle(getString(R.string.fydsp_attenuate_way6_lr));
                    } else if ("SUBWOOFER".equals(tag)) {
                        fyDspDelayView.setTitle(getString(R.string.fydsp_attenuate_way6_subwoofer));
                    } else if ("RF".equals(tag)) {
                        fyDspDelayView.setTitle(getString(R.string.fydsp_attenuate_way6_rf));
                    } else if ("RR".equals(tag)) {
                        fyDspDelayView.setTitle(getString(R.string.fydsp_attenuate_way6_rr));
                    } else if ("CENTER".equals(tag)) {
                        fyDspDelayView.setTitle(getString(R.string.fydsp_attenuate_way6_center));
                    }
                });

        switch (fyDspOutputMode) {
            case WAY2:
            case WAY6:
                ivDelayCar.setBackgroundResource(R.drawable.fydsp_delay_car_way2);
                break;
            case WAY3:
                ivDelayCar.setBackgroundResource(R.drawable.fydsp_delay_car_way3);
                break;
            case CHANNEL51:
                ivDelayCar.setBackgroundResource(R.drawable.fydsp_delay_car_channel51);
                break;
        }
    }

    @Override
    public void onCheckedChanged(RadioGroup radioGroup, int checkedId) {
        boolean hasChildPressed = false;
        for (int i = 0; i < radioGroup.getChildCount(); i++) {
            if (radioGroup.getChildAt(i).isPressed()) {
                hasChildPressed = true;
            }
        }
        // 界面初始化时也会调用 onCheckedChanged 事件，这个时候不需要改变数据和实际音效
        // 手动点击时才往下执行
        if (!hasChildPressed) {
            return;
        }

        switch (checkedId) {
            case R.id.rb_delay_unit_ms:
                // 注意：只操作可见控件
                fyDspDelayViewList.stream()
                        .filter(fyDspDelayView -> fyDspDelayView.getVisibility() == View.VISIBLE)
                        .forEach(fyDspDelayView -> fyDspDelayView.setUnit(0));
                break;
            case R.id.rb_delay_unit_cm:
                // 注意：只操作可见控件
                fyDspDelayViewList.stream()
                        .filter(fyDspDelayView -> fyDspDelayView.getVisibility() == View.VISIBLE)
                        .forEach(fyDspDelayView -> fyDspDelayView.setUnit(1));
                break;
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_sample:
                FyDspAlertDialog fyDspAlertDialog = FyDspAlertDialog.newInstance();
                fyDspAlertDialog.setTitle(getString(R.string.fydsp_delay_to_balance_title));
                fyDspAlertDialog.setContent(getString(R.string.fydsp_delay_to_balance_content));
                fyDspAlertDialog.setCallback(this);
                fyDspAlertDialog.show(getParentFragmentManager(), "");
                break;
            case R.id.btn_reset_delay:
                // 注意：只操作可见控件
                fyDspDelayViewList.stream()
                        .filter(fyDspDelayView -> fyDspDelayView.getVisibility() == View.VISIBLE)
                        .forEach(fyDspDelayView -> fyDspDelayView.reset(false));

                fyDspDelaySettings.nativeAll();
                break;
        }
    }

    @Override
    public void onOkClicked() {
        btnReset.performClick();
        fyDspBalanceSettings.saveBalanceOrDelayUIMode(FyDspBalanceSettings.UI_MODE.BALANCE);
        EventBus.getDefault().post(new EventMessage(EventMessage.MSG_DELAY_CHANGE_TO_BALANCE));
    }

    @Override
    public void onCancelClicked() {

    }

    @Override
    @Subscribe(sticky = true)
    public void onEvent(EventMessage eventMessage) {
        super.onEvent(eventMessage);
        if (EventMessage.MSG_STICKY_OUTPUT_MODE_CHANGED.equals(eventMessage.getMessage())) {
            fyDspOutputMode = (FyDspOutputMode) eventMessage.getData();
            Log.d(TAG, "MSG_STICKY_OUTPUT_MODE_CHANGED change to " + fyDspOutputMode);
            // 切换输出模式时，清除所有控件的值
            fyDspDelayViewList.forEach(fyDspDelayView -> fyDspDelayView.reset(false));
            refreshViews();

            fyDspDelaySettings.nativeAll();
        } else if (EventMessage.MSG_STICKY_USER_MODE_CHANGED.equals(eventMessage.getMessage())) {
            Log.d(TAG, "MSG_STICKY_USER_MODE_CHANGED");
            initData();
        }
    }

    @Override
    public void onValueChanged(FyDspDelayView fyDspDelayView, int value, boolean needNativeData) {
        EventMessage.anyChanged(mContext, TAG + "_" + "onValueChanged");

        fyDspDelaySettings.saveDelay((String) fyDspDelayView.getTag(), value);

        if (needNativeData) {
            fyDspDelaySettings.nativeAll();
        }
    }
}
