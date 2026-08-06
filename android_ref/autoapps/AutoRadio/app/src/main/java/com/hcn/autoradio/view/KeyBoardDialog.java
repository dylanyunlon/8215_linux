package com.hcn.autoradio.view;


import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.StyleRes;

import com.hcn.autoradio.R;
import com.hcn.autoradio.ScreenSpec;
import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.skin.SkinUtils;

public class KeyBoardDialog extends Dialog implements View.OnTouchListener, View.OnClickListener {
    private static final String TAG = "Radio_KeyBoard";
    private Context context;//上下文
    private int layoutResID;//布局文件id
    FMDataControl mFMDCC = null;

    private int mAnimResId = -1;
    private int mGravity = Gravity.CENTER;
    private int mXpos = -1;
    private int mYpos = -1;

    public void setAnimResId(@StyleRes int resId) {
        mAnimResId = resId;
    }

    public void setGravity(int gravity) {
        mGravity = gravity;
    }

    public void setXpos(int xpos) {
        mXpos = xpos;
    }

    public void setYpos(int ypos) {
        mYpos = ypos;
    }

    private TextView mTextInput = null;
    private View mBtnKey1 = null;
    private View mBtnKey2 = null;
    private View mBtnKey3 = null;
    private View mBtnKey4 = null;
    private View mBtnKey5 = null;
    private View mBtnKey6 = null;
    private View mBtnKey7 = null;
    private View mBtnKey8 = null;
    private View mBtnKey9 = null;
    private View mBtnKey0 = null;
    private View mBtnKeyDel = null;
    private View mBtnKeyDot = null;
    private View mBtnKeyEnter = null;

    public KeyBoardDialog(@NonNull Context context, int layoutResID, @NonNull FMDataControl data) {
        super(context, R.style.CustomDialog);
        this.context = context;
        this.layoutResID = layoutResID;
        mFMDCC = data;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Window dialogWindow = getWindow();
        if (mAnimResId != -1 && dialogWindow != null) {
            dialogWindow.setWindowAnimations(R.style.KeyboardAnimation);
        }
        View view = LayoutInflater.from(context).inflate(layoutResID, null);
        setContentView(view);
        int width = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
        int height = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
        view.measure(width, height);

        WindowManager.LayoutParams params = dialogWindow.getAttributes();

        if (view.getMeasuredWidth() < ScreenSpec.mScreenWidth) {
            params.width = view.getMeasuredWidth();
            params.height = view.getMeasuredHeight();
        } else {
            params.width = Math.min(ScreenSpec.mScreenWidth, ScreenSpec.mScreenHeight);
            params.height = (int) ((params.width * view.getMeasuredHeight() * 1.0f)
                    / view.getMeasuredWidth());
        }
        params.gravity = mGravity;
        if (mXpos != -1) {
            params.x = mXpos;
        }
        if (mYpos != -1) {
            params.y = mYpos;
        }
        dialogWindow.setAttributes(params);
        setCanceledOnTouchOutside(true);

        mTextInput = findViewById(SkinUtils.getId(R.id.input));

        mBtnKey1 = findViewById(SkinUtils.getId(R.id.key_1));
        mBtnKey2 = findViewById(SkinUtils.getId(R.id.key_2));
        mBtnKey3 = findViewById(SkinUtils.getId(R.id.key_3));
        mBtnKey4 = findViewById(SkinUtils.getId(R.id.key_4));
        mBtnKey5 = findViewById(SkinUtils.getId(R.id.key_5));
        mBtnKey6 = findViewById(SkinUtils.getId(R.id.key_6));
        mBtnKey7 = findViewById(SkinUtils.getId(R.id.key_7));
        mBtnKey8 = findViewById(SkinUtils.getId(R.id.key_8));
        mBtnKey9 = findViewById(SkinUtils.getId(R.id.key_9));
        mBtnKey0 = findViewById(SkinUtils.getId(R.id.key_0));
        mBtnKeyDel = findViewById(SkinUtils.getId(R.id.key_del));
        mBtnKeyDot = findViewById(SkinUtils.getId(R.id.key_dot));
        mBtnKeyEnter = findViewById(SkinUtils.getId(R.id.key_enter));

        mBtnKey1.setOnClickListener(this);
        mBtnKey1.setOnTouchListener(this);
        mBtnKey2.setOnClickListener(this);
        mBtnKey2.setOnTouchListener(this);
        mBtnKey3.setOnClickListener(this);
        mBtnKey3.setOnTouchListener(this);
        mBtnKey4.setOnClickListener(this);
        mBtnKey4.setOnTouchListener(this);
        mBtnKey5.setOnClickListener(this);
        mBtnKey5.setOnTouchListener(this);
        mBtnKey6.setOnClickListener(this);
        mBtnKey6.setOnTouchListener(this);
        mBtnKey7.setOnClickListener(this);
        mBtnKey7.setOnTouchListener(this);
        mBtnKey8.setOnClickListener(this);
        mBtnKey8.setOnTouchListener(this);
        mBtnKey9.setOnClickListener(this);
        mBtnKey9.setOnTouchListener(this);
        mBtnKey0.setOnClickListener(this);
        mBtnKey0.setOnTouchListener(this);
        mBtnKeyDel.setOnClickListener(this);
        mBtnKeyDel.setOnTouchListener(this);
        mBtnKeyDot.setOnClickListener(this);
        mBtnKeyDot.setOnTouchListener(this);
        mBtnKeyEnter.setOnClickListener(this);
        mBtnKeyEnter.setOnTouchListener(this);
    }

    @Override
    protected void onStart() {
        super.onStart();
        mTextInput.setText("");
        updateKeyboardState(0);
    }

    @Override
    public void onClick(View v) {
        // int btnId = v.getId();
        int btnId = SkinUtils.getViewId(v);
        switch (btnId) {
            case R.id.key_1:
                onButtKeyEvent("1");
                break;
            case R.id.key_2:
                onButtKeyEvent("2");
                break;
            case R.id.key_3:
                onButtKeyEvent("3");
                break;
            case R.id.key_4:
                onButtKeyEvent("4");
                break;
            case R.id.key_5:
                onButtKeyEvent("5");
                break;
            case R.id.key_6:
                onButtKeyEvent("6");
                break;
            case R.id.key_7:
                onButtKeyEvent("7");
                break;
            case R.id.key_8:
                onButtKeyEvent("8");
                break;
            case R.id.key_9:
                onButtKeyEvent("9");
                break;
            case R.id.key_0:
                onButtKeyEvent("0");
                break;
            case R.id.key_del:
                onButtKeyEvent("del");
                break;
            case R.id.key_dot:
                onButtKeyEvent("dot");
                break;
            case R.id.key_enter:
                onButtKeyEvent("enter");
                break;
            default:
                break;
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        return false;
    }

    private void onButtKeyEvent(String key) {

        if (mTextInput != null) {
            String strInput = mTextInput.getText().toString();

            if (key.equals("1")) {
                strInput += "1";
            } else if (key.equals("2")) {
                strInput += "2";
            } else if (key.equals("3")) {
                strInput += "3";
            } else if (key.equals("4")) {
                strInput += "4";
            } else if (key.equals("5")) {
                strInput += "5";
            } else if (key.equals("6")) {
                strInput += "6";
            } else if (key.equals("7")) {
                strInput += "7";
            } else if (key.equals("8")) {
                strInput += "8";
            } else if (key.equals("9")) {
                strInput += "9";
            } else if (key.equals("0")) {
                strInput += "0";
            } else if (key.equals("dot")) {
                strInput += ".";
            } else if (key.equals("del")) {
                if (strInput.length() > 0) {
                    Log.i(TAG, "input length:" + strInput.length());
                    strInput = (strInput.length() - 1) > 0 ? strInput.substring(0,
                            strInput.length() - 1) : "";
                }
            } else if (key.equals("enter")) {
                Log.i(TAG, "enter input:" + strInput);
                if (strInput != null && !strInput.equals("")) {
                    if (mFMDCC.isFMBand()) {
                        float item = Float.parseFloat(strInput);
                        int nFreq = (int) (item * 1000);
                        mFMDCC.setFreq(nFreq);
                    } else {
                        int nFreq = Integer.parseInt(strInput);
                        mFMDCC.setFreq(nFreq);
                    }
                }
                dismiss();
            }

            Log.i(TAG, "update input:" + strInput + " length:" + strInput.length());
            mTextInput.setText(strInput);

            updateKeyboardState(strInput != null ? strInput.length() : 0);
        }
    }

    private void updateKeyboardState(int pos) {
        int nMinFreq, nMaxFreq;

        if (mFMDCC.isFMBand()) {
            nMinFreq = FMDataControl.mRadioParameters.FmMin / 10;
            nMaxFreq = FMDataControl.mRadioParameters.FmMax / 10;
        } else {
            nMinFreq = FMDataControl.mRadioParameters.AmMin;
            nMaxFreq = FMDataControl.mRadioParameters.AmMax;
        }
        Log.i(TAG, "updateKeyboardState:(" + nMinFreq + "," + nMaxFreq + ")" + pos);

        mBtnKey1.setEnabled(false);
        mBtnKey2.setEnabled(false);
        mBtnKey3.setEnabled(false);
        mBtnKey4.setEnabled(false);
        mBtnKey5.setEnabled(false);
        mBtnKey6.setEnabled(false);
        mBtnKey7.setEnabled(false);
        mBtnKey8.setEnabled(false);
        mBtnKey9.setEnabled(false);
        mBtnKey0.setEnabled(false);
        mBtnKeyDot.setEnabled(false);

        String strMin = String.valueOf(nMinFreq);
        String strMax = String.valueOf(nMaxFreq);

        String strInput = mTextInput.getText().toString();

        if (strInput != null) {
            strInput = strInput.replace(".", "");
        }

        if (pos == 0) {
            String strMinNum = String.valueOf(nMinFreq).substring(pos, pos + 1);
            int numMin = Integer.parseInt(strMinNum);
            String strMaxNum = String.valueOf(nMaxFreq).substring(pos, pos + 1);
            int numMax = Integer.parseInt(strMaxNum);
            for (int i = numMin; i <= 9; i++) {
                setKeyEnable(i);
            }
            setKeyEnable(numMax);
        } else {
            String strFirstNum = String.valueOf(strInput).substring(0, 1);
            int numFirst = Integer.parseInt(strFirstNum);

            int nStepPos = 0;
            int nMaxInputLength = 0;
            int nStepValue = 0;

            if (mFMDCC.isFMBand()) {
                if (FMDataControl.mRadioParameters.FmStep / 10 == 5) {
                    nStepPos = (numFirst == 1) ? 4 : 3;
                } else {
                    nStepPos = (numFirst == 1) ? 3 : 2;
                }
                nMaxInputLength = (numFirst == 1) ? 5 : 4;
                nStepValue = FMDataControl.mRadioParameters.FmStep / 10;
            } else {
                if (FMDataControl.mRadioParameters.AmStep == 9) {
                    nStepPos = (numFirst == 1) ? 3 : 2;
                } else {
                    nStepPos = (numFirst == 1) ? 2 : 1;
                }
                nMaxInputLength = (numFirst == 1) ? 4 : 3;
                nStepValue = FMDataControl.mRadioParameters.AmStep;
            }

            if (mFMDCC.isFMBand() && ((pos == 2 && numFirst != 1) || (pos == 3 && numFirst == 1))) {
                mBtnKeyDot.setEnabled(true);
            } else {
                if (mFMDCC.isFMBand()) {
                    if ((numFirst == 1 && pos >= 4) || (numFirst != 1 && pos >= 3)) {
                        pos = pos - 1;
                    }
                }
                int nInputLength = strInput.length();

                if (numFirst == 1 && strMax.startsWith(strInput) && pos < nStepPos) {
                    if (nInputLength >= nMaxInputLength) {
                        Log.e(TAG, "Max Input");
                        return;
                    }
                    String strMaxNum = String.valueOf(nMaxFreq).substring(nInputLength,
                            nInputLength + 1);
                    int numMax = Integer.parseInt(strMaxNum);
                    Log.i(TAG, "can set Max to:" + numMax);
                    for (int i = 0; i <= numMax; i++) {
                        setKeyEnable(i);
                    }
                } else if (strMin.startsWith(strInput) && pos < nStepPos) {
                    if (nInputLength >= nMaxInputLength) {
                        Log.e(TAG, "Max Input");
                        return;
                    }
                    String strMaxNum = String.valueOf(nMinFreq).substring(nInputLength,
                            nInputLength + 1);
                    int numMax = Integer.parseInt(strMaxNum);
                    Log.i(TAG, "can set Min from:" + numMax);
                    for (int i = numMax; i <= 9; i++) {
                        setKeyEnable(i);
                    }
                } else {
                    Log.i(TAG, "pos:" + pos + " nStepValue:" + nStepValue + " nMaxInputLength:"
                            + nMaxInputLength);
                    if (pos > nStepPos) {
                        if (nStepValue >= 10) {
                            if (pos < nMaxInputLength) {
                                setKeyEnable(0);
                            }
                        }
                    } else if (pos == nStepPos) {
                        for (int i = 0; i <= 9; i++) {
                            String num = strInput + i;
                            if (nStepValue >= 10) {
                                num += "0";
                            }
                            int value = Integer.parseInt(num);
                            if (value < nMinFreq || value > nMaxFreq) {
                                continue;
                            }
                            if ((value - nMinFreq) % nStepValue == 0) {
                                setKeyEnable(i);
                            }
                        }

                    } else {
                        for (int i = 0; i <= 9; i++) {
                            setKeyEnable(i);
                        }
                    }
                }
            }
        }
    }

    private void setKeyEnable(int key) {
        switch (key) {
            case 0:
                mBtnKey0.setEnabled(true);
                break;
            case 1:
                mBtnKey1.setEnabled(true);
                break;
            case 2:
                mBtnKey2.setEnabled(true);
                break;
            case 3:
                mBtnKey3.setEnabled(true);
                break;
            case 4:
                mBtnKey4.setEnabled(true);
                break;
            case 5:
                mBtnKey5.setEnabled(true);
                break;
            case 6:
                mBtnKey6.setEnabled(true);
                break;
            case 7:
                mBtnKey7.setEnabled(true);
                break;
            case 8:
                mBtnKey8.setEnabled(true);
                break;
            case 9:
                mBtnKey9.setEnabled(true);
                break;
            default:
                break;
        }
    }
}
