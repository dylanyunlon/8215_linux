package com.hcn.autoeq.fragment.cscasp;


import static com.hcn.autoeq.data.CscAspSubwooferSettings.CSC_ASP_LP_FFR_EQ;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.TextView;
import com.hcn.autoeq.R;
import com.hcn.autoeq.data.CscAspSubwooferSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.SkinUtils;

/**
 * hcn_asp低音炮
 */
public class CscAspSubwooferFragment extends BaseFragment
        implements View.OnClickListener {

    public final String[] cscAspLpfFreqValueGroup = {"off", "55", "85", "120", "160"};
    private View mainView;

    private RadioButton rbCscAspFirstLpfFreq;
    private RadioButton rbCscAspSecondLpfFreq;
    private RadioButton rbCscAspThirdLpfFreq;
    private RadioButton rbCscAspFourthLpfFreq;

    private LinearLayout llCscAspLpfSwitch;
    private ImageView ivCscAspLpfFreqLine;

    private TextView tvCscAspLpfSlope;
    private TextView tvCscAspLpfFreq;
    private TextView tvCscAspLpfFreqUnit;

    //Freq数值
    private int cscAspFreqValue;

    private TextView tvCscAspTextLpfOn;

    private CscAspSubwooferSettings cscAspSubwooferSettings;


    public static CscAspSubwooferFragment newInstance() {
        CscAspSubwooferFragment fragment = new CscAspSubwooferFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.csc_asp_subwoofer_fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        cscAspSubwooferSettings = CscAspSubwooferSettings.getInstance(mContext);
        return mainView;
    }

    /**
     * 获取数据
     */
    @Override
    public void initView() {
        //按钮
        rbCscAspFirstLpfFreq = mainView.findViewById(SkinUtils.getId(R.id.rb_csc_asp_first_lpf_freq));
        rbCscAspSecondLpfFreq = mainView.findViewById(SkinUtils.getId(R.id.rb_csc_asp_second_lpf_freq));
        rbCscAspThirdLpfFreq = mainView.findViewById(SkinUtils.getId(R.id.rb_csc_asp_third_lpf_freq));
        rbCscAspFourthLpfFreq = mainView.findViewById(SkinUtils.getId(R.id.rb_csc_asp_fourth_lpf_freq));
        llCscAspLpfSwitch = mainView.findViewById(SkinUtils.getId(R.id.ll_csc_asp_lpf_switch));
        rbCscAspFirstLpfFreq.setOnClickListener(this);
        rbCscAspSecondLpfFreq.setOnClickListener(this);
        rbCscAspThirdLpfFreq.setOnClickListener(this);
        rbCscAspFourthLpfFreq.setOnClickListener(this);
        llCscAspLpfSwitch.setOnClickListener(this);

        //文本
        tvCscAspLpfSlope = mainView.findViewById(SkinUtils.getId(R.id.tv_csc_asp_lpf_slope));
        tvCscAspLpfFreq = mainView.findViewById(SkinUtils.getId(R.id.tv_csc_asp_lpf_freq));
        tvCscAspLpfFreqUnit = mainView.findViewById(SkinUtils.getId(R.id.tv_csc_asp_lpf_freq_unit));
        tvCscAspLpfSlope.setText(CSC_ASP_LP_FFR_EQ);
        ivCscAspLpfFreqLine = mainView.findViewById(SkinUtils.getId(R.id.iv_csc_asp_lpf_freq_line));
        tvCscAspTextLpfOn = mainView.findViewById(SkinUtils.getId(R.id.tv_csc_asp_text_lpf_on));

        if (cscAspSubwooferSettings != null) {
            cscAspFreqValue = cscAspSubwooferSettings.getSurroundFre();
        }

        updateView(cscAspFreqValue);
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        if (id == SkinUtils.getId(R.id.ll_csc_asp_lpf_switch)) {
            cscAspFreqValue = 0;
        }
        if (id == SkinUtils.getId(R.id.rb_csc_asp_first_lpf_freq)) {
            cscAspFreqValue = 1;
        } else if (id == SkinUtils.getId(R.id.rb_csc_asp_second_lpf_freq)) {
            cscAspFreqValue = 2;
        } else if (id == SkinUtils.getId(R.id.rb_csc_asp_third_lpf_freq)) {
            cscAspFreqValue = 3;
        } else if (id == SkinUtils.getId(R.id.rb_csc_asp_fourth_lpf_freq)) {
            cscAspFreqValue = 4;
        }

        updateView(cscAspFreqValue);
    }


    /**
     * 更新view
     */
    public void updateView(int index) {
        tvCscAspLpfFreq.setText(String.valueOf(cscAspLpfFreqValueGroup[index]));
        if (index == 0) {
            tvCscAspTextLpfOn.setSelected(true);
            llCscAspLpfSwitch.setSelected(true);
            tvCscAspLpfFreqUnit.setVisibility(View.INVISIBLE);
        } else {
            tvCscAspTextLpfOn.setSelected(false);
            llCscAspLpfSwitch.setSelected(false);
            tvCscAspLpfFreqUnit.setVisibility(View.VISIBLE);
        }

        if (index == 0) {
            ivCscAspLpfFreqLine.setBackground(SkinUtils.getDrawable(R.drawable.csc_asp_seekbar_thumb_0));
        } else if (index == 1) {
            ivCscAspLpfFreqLine.setBackground(SkinUtils.getDrawable(R.drawable.csc_asp_seekbar_thumb_55));
        } else if (index == 2) {
            ivCscAspLpfFreqLine.setBackground(SkinUtils.getDrawable(R.drawable.csc_asp_seekbar_thumb_85));
        } else if (index == 3) {
            ivCscAspLpfFreqLine.setBackground(SkinUtils.getDrawable(R.drawable.csc_asp_seekbar_thumb_120));
        } else if (index == 4) {
            ivCscAspLpfFreqLine.setBackground(SkinUtils.getDrawable(R.drawable.csc_asp_seekbar_thumb_160));
        }
        cscAspSubwooferSettings.saveSurround(cscAspFreqValue);
    }
}
