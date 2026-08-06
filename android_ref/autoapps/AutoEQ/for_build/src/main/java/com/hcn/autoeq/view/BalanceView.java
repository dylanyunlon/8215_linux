package com.hcn.autoeq.view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;
import android.widget.ImageView;

@SuppressLint("AppCompatCustomView")
public class BalanceView extends ImageView {

    public BalanceView(Context context) {
        super(context);
    }

    public BalanceView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public BalanceView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

}
