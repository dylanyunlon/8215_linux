package com.hcn.autoeq.fragment.fydsp;

import android.content.Context;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;

import com.blankj.utilcode.util.StringUtils;
import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.Band;
import com.hcn.autoeq.bean.EventMessage;
import com.hcn.autoeq.util.ConstantFyDsp;

import org.greenrobot.eventbus.EventBus;

public class FyDspFrequencyDialog extends DialogFragment implements View.OnClickListener, ConstantFyDsp {
    private static final String TAG = FyDspFrequencyDialog.class.getSimpleName();

    private Button btnKeyboard1;
    private Button btnKeyboard2;
    private Button btnKeyboard3;
    private Button btnKeyboard4;
    private Button btnKeyboard5;
    private Button btnKeyboard6;
    private Button btnKeyboard7;
    private Button btnKeyboard8;
    private Button btnKeyboard9;
    private Button btnKeyboard0;
    private Button btnKeyboardBackspace;
    private Button btnKeyboardOk;
    private EditText etValue;

    private Context context;
    private Band band;
    private int originalFreq; // 传进来的原始数据

    public static FyDspFrequencyDialog newInstance(Band band) {
        FyDspFrequencyDialog fragment = new FyDspFrequencyDialog();
        Bundle args = new Bundle();
        args.putParcelable("band", band);
        fragment.setArguments(args);
        return fragment;
    }

    private FyDspFrequencyDialog() {
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
        View view = inflater.inflate(R.layout.fydsp_dialog_frequency, container, false);
        initView(view);
        return view;
    }

    private void initView(View view) {
        btnKeyboard1 = view.findViewById(R.id.btn_keyboard_1);
        btnKeyboard2 = view.findViewById(R.id.btn_keyboard_2);
        btnKeyboard3 = view.findViewById(R.id.btn_keyboard_3);
        btnKeyboard4 = view.findViewById(R.id.btn_keyboard_4);
        btnKeyboard5 = view.findViewById(R.id.btn_keyboard_5);
        btnKeyboard6 = view.findViewById(R.id.btn_keyboard_6);
        btnKeyboard7 = view.findViewById(R.id.btn_keyboard_7);
        btnKeyboard8 = view.findViewById(R.id.btn_keyboard_8);
        btnKeyboard9 = view.findViewById(R.id.btn_keyboard_9);
        btnKeyboard0 = view.findViewById(R.id.btn_keyboard_0);
        btnKeyboardBackspace = view.findViewById(R.id.btn_keyboard_backspace);
        btnKeyboardOk = view.findViewById(R.id.btn_keyboard_ok);
        etValue = view.findViewById(R.id.tv_value);

        btnKeyboard1.setOnClickListener(this);
        btnKeyboard2.setOnClickListener(this);
        btnKeyboard3.setOnClickListener(this);
        btnKeyboard4.setOnClickListener(this);
        btnKeyboard5.setOnClickListener(this);
        btnKeyboard6.setOnClickListener(this);
        btnKeyboard7.setOnClickListener(this);
        btnKeyboard8.setOnClickListener(this);
        btnKeyboard9.setOnClickListener(this);
        btnKeyboard0.setOnClickListener(this);
        btnKeyboardBackspace.setOnClickListener(this);
        btnKeyboardOk.setOnClickListener(this);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        originalFreq = band.getFreq();
        etValue.setText(String.valueOf(band.getFreq()));
        etValue.setSelection(etValue.getText().length()); // 光标移动到最后
    }

    @Override
    public void onClick(View view) {
        KeyEvent keyEvent = null;
        switch (view.getId()) {
            case R.id.btn_keyboard_1:
                keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_1);
                break;
            case R.id.btn_keyboard_2:
                keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_2);
                break;
            case R.id.btn_keyboard_3:
                keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_3);
                break;
            case R.id.btn_keyboard_4:
                keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_4);
                break;
            case R.id.btn_keyboard_5:
                keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_5);
                break;
            case R.id.btn_keyboard_6:
                keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_6);
                break;
            case R.id.btn_keyboard_7:
                keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_7);
                break;
            case R.id.btn_keyboard_8:
                keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_8);
                break;
            case R.id.btn_keyboard_9:
                keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_9);
                break;
            case R.id.btn_keyboard_0:
                keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_0);
                break;
            case R.id.btn_keyboard_backspace:
                keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL);
                break;
            case R.id.btn_keyboard_ok:
                boolean valueInvalidate;
                int freq;
                String text = etValue.getText().toString();
                // 没输入，或者输入的值不在范围内，则不关闭 dialog
                if (StringUtils.isEmpty(text)) {
                    freq = 0;
                } else {
                    freq = Integer.parseInt(text);
                }
                if (freq < DEF_FREQ_MIN) {
                    freq = DEF_FREQ_MIN;
                    valueInvalidate = true;
                } else if (freq > DEF_FREQ_MAX) {
                    freq = DEF_FREQ_MAX;
                    valueInvalidate = true;
                } else {
                    valueInvalidate = false;
                }

                band.setFreq(freq);

                EventBus.getDefault().post(new EventMessage(EventMessage.MSG_BAND_FREQ_CHANGED, band));
                if (valueInvalidate) {
                    etValue.setText(String.valueOf(freq));
                    etValue.setSelection(etValue.getText().length()); // 光标移动到最后
                } else {
                    dismiss();
                }
                return;
        }

        if (keyEvent != null) {
            etValue.dispatchKeyEvent(keyEvent);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        dismiss();
    }
}
