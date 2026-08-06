package com.hcn.autoeq.nine;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.fragment.app.DialogFragment;

import com.hcn.autoeq.R;
import com.hcn.skin.support.resources.SkinCompatResources;
import com.hcn_library.util.SkinUtils;

public class NineDspUserModeExitDialog extends DialogFragment implements View.OnClickListener {
    static final String TAG = "NineDspUserModeExitDialog";
    private Context context;
    private TextView exitCancel;
    private TextView exitConfirm;
    public boolean isShow;
    private OnDialogListener listener;
    private int mode;
    private String userName;
    private TextView tvTextContent;


    public interface OnDialogListener {
        void isCancel(int i);

        void isOk(int i);
    }

    private void initTvTextContent() {
    }

    public static NineDspUserModeExitDialog newInstance(int i) {
        return new NineDspUserModeExitDialog(i);
    }

    public NineDspUserModeExitDialog() {
    }

    private NineDspUserModeExitDialog(int i) {
        mode = i;
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        this.context = context;
        Log.d(TAG, "onAttach");
    }

    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        setStyle(DialogFragment.STYLE_NO_TITLE, android.R.style.Theme_Dialog);
        Log.d(TAG, "onCreate");
    }

    @Override
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        super.onCreateView(layoutInflater, viewGroup, bundle);
        getDialog().getWindow().setBackgroundDrawableResource(R.color.color_transparent);
        View view = LayoutInflater.from(SkinCompatResources.getInstance().getSkinResId(R.layout.nine_dsp_dialog_user_exit, "layout") != 0
                        ? SkinUtils.getContext() : context)
                .inflate(SkinUtils.getId(R.layout.nine_dsp_dialog_user_exit), null);

        initView(view);
        return view;
    }

    private void initView(View view) {
        exitConfirm = (TextView) view.findViewById(SkinUtils.getId(R.id.tv_nine_band_ok));
        exitCancel = (TextView) view.findViewById(SkinUtils.getId(R.id.tv_nine_band_cancel));
        tvTextContent = (TextView) view.findViewById(SkinUtils.getId(R.id.tv_nine_text_content));
        exitConfirm.setOnClickListener(this);
        exitCancel.setOnClickListener(this);
        exitConfirm.setSelected(true);
        exitCancel.setSelected(false);
        initTvTextContent();
        tvTextContent.setText(getString(R.string.nine_dialog_description, userName));
    }

    @Override
    public void onViewCreated(View view, Bundle bundle) {
        super.onViewCreated(view, bundle);
    }

    @Override
    public void onClick(View view) {
        int id = view.getId();
        if (id == SkinUtils.getId(R.id.tv_nine_band_ok)) {
            listener.isOk(mode);
            Log.d(TAG, "onClick OK");
        } else if (id == SkinUtils.getId(R.id.tv_nine_band_cancel)) {
            listener.isCancel(mode);
            Log.d(TAG, "onClick CANCEL");
        }
        dismiss();
        isShow = false;
    }

    @Override
    public void onPause() {
        super.onPause();
        dismiss();
        isShow = false;
        Log.d(TAG, "onPause");
    }

    public void setOnDialogListener(OnDialogListener onDialogListener) {
        listener = onDialogListener;
    }

    public void setMode(int i) {
        mode = i;
    }
    public void setUserName(String userName) {
        this.userName = userName;
    }
}