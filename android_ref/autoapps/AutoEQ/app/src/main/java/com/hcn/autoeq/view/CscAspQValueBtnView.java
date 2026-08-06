package com.hcn.autoeq.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.skin.support.resources.SkinCompatResources;

public class CscAspQValueBtnView extends LinearLayout {
    private TextView tvCscQValue;

    private TextView tvCscCenterFre;
    private Context context;


    public CscAspQValueBtnView(Context context) {
        super(context);
        this.context = context;
        initView();
    }

    public CscAspQValueBtnView(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        initView();
    }

    private void initView() {
        LayoutInflater.from(SkinCompatResources.getInstance().getSkinResId(R.layout.csc_asp_q_value_item, "layout") != 0
                        ? SkinUtils.getContext() : context)
                .inflate(SkinUtils.getId(R.layout.csc_asp_q_value_item), this);
        tvCscQValue = findViewById(SkinUtils.getId(R.id.tv_csc_q_value));
        tvCscCenterFre = findViewById(SkinUtils.getId(R.id.tv_csc_center_fre));
    }


    public void setCscCenterFre(String value) {
        if (tvCscCenterFre != null) {
            tvCscCenterFre.setText(value);
        }
    }

    public void setQValue(int qValue) {
        if (tvCscQValue != null) {
            tvCscQValue.setText(String.valueOf(qValue));
        }
    }


}
