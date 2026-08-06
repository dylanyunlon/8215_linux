package com.hcn.autoeq.fragment.fydsp;

import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;

import com.hcn.autoeq.R;

public class FyDspAlertDialog extends DialogFragment implements View.OnClickListener {
    private static final String TAG = FyDspAlertDialog.class.getSimpleName();

    private TextView tvTitle, tvContent;
    private Button btnOk, btnCancel;

    private String title, content;
    private Callback callback;
    private Context context;

    public static FyDspAlertDialog newInstance() {
        FyDspAlertDialog fragment = new FyDspAlertDialog();
        Bundle args = new Bundle();
        fragment.setArguments(args);
        return fragment;
    }

    private FyDspAlertDialog() {
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);
        this.context = context;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setStyle(DialogFragment.STYLE_NO_TITLE, android.R.style.Theme_Dialog);
        Bundle bundle = getArguments();
        if (bundle != null) {

        }
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        // 设置dialog背景透明，才可以显示圆角背景图
        getDialog().getWindow().setBackgroundDrawableResource(android.R.color.transparent);
        View view = inflater.inflate(R.layout.fydsp_alert_dialog, container, false);
        initView(view);
        return view;
    }

    @Override
    public void onPause() {
        super.onPause();
        dismiss();
    }

    private void initView(View view) {
        tvTitle = view.findViewById(R.id.tv_title);
        tvContent = view.findViewById(R.id.tv_content);
        btnOk = view.findViewById(R.id.btn_ok);
        btnCancel = view.findViewById(R.id.btn_cancel);

        btnOk.setOnClickListener(this);
        btnCancel.setOnClickListener(this);

        tvTitle.setText(title);
        tvContent.setText(content);
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.btn_ok:
                if (callback != null) {
                    callback.onOkClicked();
                }
                dismiss();
                break;
            case R.id.btn_cancel:
                if (callback != null) {
                    callback.onCancelClicked();
                }
                dismiss();
                break;
        }
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public void setContent(String content) {
        this.content = content;
    }

    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    public interface Callback {
        void onOkClicked();

        void onCancelClicked();
    }
}
