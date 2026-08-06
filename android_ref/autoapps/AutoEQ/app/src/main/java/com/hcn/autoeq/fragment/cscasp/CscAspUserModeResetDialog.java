package com.hcn.autoeq.fragment.cscasp;

import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.Band;
import com.hcn.autoeq.util.ConstantCscAsp;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.util.Locale;


public class CscAspUserModeResetDialog extends DialogFragment implements View.OnClickListener {

    private Context context;
    private Band band;

    private ImageButton resetOk;
    private ImageButton resetCancel;
    private TextView tvTextContent;

    private int mode;

    private OnDialogListener listener;

    public static CscAspUserModeResetDialog newInstance(int mode) {
        CscAspUserModeResetDialog fragment = new CscAspUserModeResetDialog(mode);
        return fragment;
    }

    private CscAspUserModeResetDialog(int mode) {
        this.mode = mode;
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
            band = bundle.getParcelable("band");
        }
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        // 设置dialog背景透明，才可以显示圆角背景图
        getDialog().getWindow().setBackgroundDrawableResource(android.R.color.transparent);
        View view = SkinUtils.inflate(R.layout.csc_asp_dialog_user_reset);
        initView(view);
        return view;
    }

    private void initView(View view) {
        resetOk = view.findViewById(SkinUtils.getId(R.id.btn_csc_asp_user_reset_mode_ok));
        resetCancel = view.findViewById(SkinUtils.getId(R.id.btn_csc_asp_user_reset_mode_cancel));
        tvTextContent = view.findViewById(SkinUtils.getId(R.id.tv_text_content));
        resetOk.setOnClickListener(this);
        resetCancel.setOnClickListener(this);

        initTvTextContent();

    }

    private void initTvTextContent() {
        int modeText = 1;
        if (mode == ConstantCscAsp.EXT_CSC_ASP_REVERB_USER0) {
            modeText = 1;
        } else if (mode == ConstantCscAsp.EXT_CSC_ASP_REVERB_USER1) {
            modeText = 2;
        }
        String dialogContent = context.getString(R.string.reset_dialog_content, modeText);
        tvTextContent.setText(String.format(Locale.getDefault(), dialogContent));
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

    }

    @Override
    public void onClick(View view) {
        int id = view.getId();
        if (id == SkinUtils.getId(R.id.btn_csc_asp_user_reset_mode_ok)) {
            listener.isOk(mode);
        } else if (id == SkinUtils.getId(R.id.btn_csc_asp_user_reset_mode_cancel)) {
        }
        dismiss();
    }

    @Override
    public void onPause() {
        super.onPause();
        dismiss();
    }


    public void setOnDialogListener(OnDialogListener listener) {
        this.listener = listener;
    }

    public interface OnDialogListener {
        void isOk(int mode);

    }
}
