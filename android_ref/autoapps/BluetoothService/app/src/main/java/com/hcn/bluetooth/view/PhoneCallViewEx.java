package com.hcn.bluetooth.view;

import android.bluetooth.BluetoothHeadsetClient;
import android.bluetooth.BluetoothHeadsetClientCall;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.text.Editable;
import android.text.InputType;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.hcn.bluetooth.api.Utils;
import com.hcn.bluetooth.bean.CallInfo;
import com.hcn.bluetooth.service.BluetoothHfpclientService;
import com.hcn.bluetooth.skin.SkinUtils;
import com.hcn.bluetoothservice.R;

import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

/**
 * mcc500全屏弹框
 */
public class PhoneCallViewEx extends PhoneCallViewBase implements OnClickListener {
    private static final String TAG = "PhoneCallViewFull";
    public static final int MAX_CALL_SIZE = 2;
    public static final int OPACITY = 255;
    public static final int HALFOPACITY = 100;

    private BluetoothHfpclientService mHFPService;

    //通话状态显示
    TextView callStatusView;
    TextView callingNameView;
    TextView callingTimeView;
    //通话保持状态
    private LinearLayout heldLayout;
    private TextView heldNumView;
    private TextView heldPhoneView;
    private TextView heldStateView;
    //通话音源切换按钮
    private Button voiceSourceCarButton;
    private Button voiceSourcePhoneButton;

    private Button answerButton;
    private Button hangupButton;
    private Button heldButton;
    private Button acceptHeldButton;
    private Button swapButton;
    //通话mic静音和添加通话
    private Button mMicMuteBtn;
    private Button mCallAddBtn;
    //键盘上带的挂断和隐藏按键
    private Button mKeyBroadHangup;
    private Button mKeyBroadHide;

    private LinearLayout softkeyPadLayout;
    private Button keypadButton;

    private EditText subcallNumber;
    //当前通话的号码
    private String mPhone;
    //过掉重复数据库操作
    private Map<String,String> mMapData = new HashMap<>();


    public PhoneCallViewEx(Context context, WindowManager wm, AudioManager am) {
        super(context, wm, am);
        mLayoutParams.type = SkinUtils.getInteger(R.integer.call_view_window_type);
        mLayoutParams.gravity = SkinUtils.getInteger(R.integer.call_view_window_gravity);
        mLayoutParams.width = SkinUtils.getInteger(R.integer.call_view_window_width);
        mLayoutParams.height = SkinUtils.getInteger(R.integer.call_view_window_height);
        mLayoutParams.flags |=  WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN;

        mHFPService = BluetoothHfpclientService.getInstance();
    }

    @Override
    protected int getLayoutId() {
        return R.layout.bt_calling_status;
    }

    @Override
    protected void onAttachedToWindow() {
        Log.d(TAG, "onAttachedToWindow");
        super.onAttachedToWindow();
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        Log.d(TAG, "onDetachedFromWindow");
        if (null != softkeyPadLayout) {
            softkeyPadLayout.setVisibility(View.GONE);
        }
        if (null != subcallNumber) {
            subcallNumber.setText("");
        }
        if(mMapData != null){
            mMapData.clear();
        }
    }

    @Override
    public void updateCallTime(Map<String, CallInfo> callTimeMap) {
        if (callTimeMap.isEmpty() || TextUtils.isEmpty(mPhone)) {
            return;
        }
        //依据号码显示
        int callTime = 0;
        if (!TextUtils.isEmpty(mPhone) && callTimeMap.containsKey(mPhone)) {
            callTime = callTimeMap.get(mPhone).getCallTime();
        }
        Log.d(TAG, " callTime = " + callTime);
        String strText = "";
        if (callTime > 3600) {
            strText = String.format(Locale.ENGLISH, "%02d:%02d:%02d", callTime / 3600 % 60,
                    callTime / 60 % 60, callTime % 60);
        } else {
            strText = String.format(Locale.ENGLISH, "%02d:%02d", callTime / 60 % 60, callTime % 60);
        }
        Log.d(TAG, "updateCallTime:" + " time=" + strText + " , mPhone = " + mPhone);
        if (null != callingTimeView) {
            callingTimeView.setText(strText);
        }
    }

    @Override
    public void onClick(View v) {
        int viewId = SkinUtils.getViewId(v);
        Log.d(TAG, "(onClick)-》" + viewId);
        switch (viewId) {
            case R.id.btn_hangup:
            case R.id.btn_calling_hungup:
                mHFPService.hangup();

                List<BluetoothHeadsetClientCall> callList = mHFPService.getCurrentCalls();
                if (callList == null || callList.size() == 0) {
                    Log.e(TAG, "btn_hangup no call!");
                    hideView();
                }
                if (viewId == R.id.btn_calling_hungup) {
                    onClickSoftkeypad();
                }
                break;

            case R.id.btn_answer:
                mHFPService.acceptCall(BluetoothHeadsetClient.CALL_ACCEPT_NONE);
                break;

            case R.id.btn_swap:
            case R.id.btn_held:
            case R.id.btn_accept_held:
            case R.id.call_held_layout:
                mHFPService.acceptCall(BluetoothHeadsetClient.CALL_ACCEPT_HOLD);
                break;

            case R.id.btn_softkeypad:
            case R.id.btn_calling_hidkeybroad:
                onClickSoftkeypad();
                break;

            case R.id.btn_voiceswitch_car:
            case R.id.btn_voiceswitch_phone:
                mHFPService.switchAudio();
                break;

            case R.id.btn_disable_mic:
                onClickMicSwitch();
                break;

            case R.id.btn_add:
                addCall();
                if(mHFPService != null){
                    mHFPService.switchCallViewByManual(BluetoothHfpclientService.CALL_VIEW_MINI);
                }
                break;

            case R.id.btn_calling_zero:
                addSubPhoneCallInputString("0");
                mHFPService.sendDTMF((byte) '0');
                break;

            case R.id.btn_calling_one:
                addSubPhoneCallInputString("1");
                mHFPService.sendDTMF((byte) '1');
                break;

            case R.id.btn_calling_two:
                addSubPhoneCallInputString("2");
                mHFPService.sendDTMF((byte) '2');
                break;

            case R.id.btn_calling_three:
                addSubPhoneCallInputString("3");
                mHFPService.sendDTMF((byte) '3');
                break;

            case R.id.btn_calling_four:
                addSubPhoneCallInputString("4");
                mHFPService.sendDTMF((byte) '4');
                break;

            case R.id.btn_calling_five:
                addSubPhoneCallInputString("5");
                mHFPService.sendDTMF((byte) '5');
                break;

            case R.id.btn_calling_six:
                addSubPhoneCallInputString("6");
                mHFPService.sendDTMF((byte) '6');
                break;

            case R.id.btn_calling_seven:
                addSubPhoneCallInputString("7");
                mHFPService.sendDTMF((byte) '7');
                break;

            case R.id.btn_calling_eight:
                addSubPhoneCallInputString("8");
                mHFPService.sendDTMF((byte) '8');
                break;

            case R.id.btn_calling_nine:
                addSubPhoneCallInputString("9");
                mHFPService.sendDTMF((byte) '9');
                break;

            case R.id.btn_calling_asterisk:
                addSubPhoneCallInputString("*");
                mHFPService.sendDTMF((byte) '*');
                break;

            case R.id.btn_calling_pound:
                addSubPhoneCallInputString("#");
                mHFPService.sendDTMF((byte) '#');
                break;

            default:
                break;
        }
    }

    @Override
    protected void initView() {
        callStatusView = findViewById(SkinUtils.getId(R.id.bt_calling_status_tv));
        callingNameView = findViewById(SkinUtils.getId(R.id.bt_calling_phone_name));
        callingTimeView = findViewById(SkinUtils.getId(R.id.bt_calling_time_text));

        heldLayout = findViewById(SkinUtils.getId(R.id.call_held_layout));
        heldNumView = findViewById(SkinUtils.getId(R.id.bt_call_held_number));
        heldPhoneView = findViewById(SkinUtils.getId(R.id.bt_call_held_phone));
        heldStateView = findViewById(SkinUtils.getId(R.id.bt_call_held_state_text));

        voiceSourceCarButton = findViewById(SkinUtils.getId(R.id.btn_voiceswitch_car));
        voiceSourcePhoneButton = findViewById(SkinUtils.getId(R.id.btn_voiceswitch_phone));
        answerButton = findViewById(SkinUtils.getId(R.id.btn_answer));
        hangupButton = findViewById(SkinUtils.getId(R.id.btn_hangup));
        heldButton = findViewById(SkinUtils.getId(R.id.btn_held));
        acceptHeldButton = findViewById(SkinUtils.getId(R.id.btn_accept_held));
        swapButton = findViewById(SkinUtils.getId(R.id.btn_swap));
        mMicMuteBtn = findViewById(SkinUtils.getId(R.id.btn_disable_mic));
        mCallAddBtn = findViewById(SkinUtils.getId(R.id.btn_add));
        mKeyBroadHangup = findViewById(SkinUtils.getId(R.id.btn_calling_hungup));
        mKeyBroadHide = findViewById(SkinUtils.getId(R.id.btn_calling_hidkeybroad));

        softkeyPadLayout = findViewById(SkinUtils.getId(R.id.calling_softkeypad));
        keypadButton = findViewById(SkinUtils.getId(R.id.btn_softkeypad));

        subcallNumber = findViewById(SkinUtils.getId(R.id.calling_input_et));

        try {
            answerButton.setOnClickListener(this);
            voiceSourceCarButton.setOnClickListener(this);
            voiceSourcePhoneButton.setOnClickListener(this);
            hangupButton.setOnClickListener(this);
            acceptHeldButton.setOnClickListener(this);
            heldButton.setOnClickListener(this);
            swapButton.setOnClickListener(this);
            mMicMuteBtn.setOnClickListener(this);
            mCallAddBtn.setOnClickListener(this);
            mKeyBroadHangup.setOnClickListener(this);
            mKeyBroadHide.setOnClickListener(this);
            heldLayout.setOnClickListener(this);
            keypadButton.setOnClickListener(this);

            findViewById(SkinUtils.getId(R.id.btn_calling_zero)).setOnClickListener(this);
            findViewById(SkinUtils.getId(R.id.btn_calling_one)).setOnClickListener(this);
            findViewById(SkinUtils.getId(R.id.btn_calling_two)).setOnClickListener(this);
            findViewById(SkinUtils.getId(R.id.btn_calling_three)).setOnClickListener(this);
            findViewById(SkinUtils.getId(R.id.btn_calling_four)).setOnClickListener(this);
            findViewById(SkinUtils.getId(R.id.btn_calling_five)).setOnClickListener(this);
            findViewById(SkinUtils.getId(R.id.btn_calling_six)).setOnClickListener(this);
            findViewById(SkinUtils.getId(R.id.btn_calling_seven)).setOnClickListener(this);
            findViewById(SkinUtils.getId(R.id.btn_calling_eight)).setOnClickListener(this);
            findViewById(SkinUtils.getId(R.id.btn_calling_nine)).setOnClickListener(this);
            findViewById(SkinUtils.getId(R.id.btn_calling_asterisk)).setOnClickListener(this);
            findViewById(SkinUtils.getId(R.id.btn_calling_pound)).setOnClickListener(this);
            subcallNumber.setInputType(InputType.TYPE_NULL);

        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onActionCallStateChanged(List<BluetoothHeadsetClientCall> callList) {
        if (callList == null || callList.size() == 0) {
            Log.e(TAG, "onActionCallStateChanged no call!");
            return;
        }
        if (callList.size() == 0x01) {
            onSingleCall(callList.get(0));
        } else {
            onMultiCall(callList);
        }
    }

    /**
     * 过掉重复的读取导致anr的
     * @param callNumber
     * @param addr
     * @return
     */
    private String getCallName(String callNumber,String addr){
        String callName = "";
        if(mMapData != null){
            callName = mMapData.get(callNumber);
            if(TextUtils.isEmpty(callName)){
                Log.d(TAG, "getCallName: getContactNameByNumber" + callNumber);
                callName = Utils.getContactNameByNumber(mContext, callNumber,addr);
                //查询一次后放弃循环，如一直查导致循环查询数据库ANR
                if (TextUtils.isEmpty(callName)) {
                    callName = callNumber;
                }
                mMapData.put(callNumber, callName);
            }else {
                return callName;
            }
        }
        return callNumber;
    }

    private void onSingleCall(BluetoothHeadsetClientCall call) {
        Log.d(TAG, "onSingleCall");
        if (call == null) {
            return;
        }

        int callState = call.getState();
        Log.d(TAG, "callState: " + callState);
        //通话时间只在接听后显示
        if (callState == BluetoothHeadsetClientCall.CALL_STATE_INCOMING
                || callState == BluetoothHeadsetClientCall.CALL_STATE_DIALING
                || callState == BluetoothHeadsetClientCall.CALL_STATE_ALERTING) {
            if (callingTimeView != null && callingTimeView.getVisibility() == VISIBLE) {
                callingTimeView.setVisibility(View.GONE);
            }
        }

        //call state
        if (callStatusView != null) {
            switch (callState) {
                case BluetoothHeadsetClientCall.CALL_STATE_ACTIVE:
                    callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_active));
                    if (callingTimeView != null && callingTimeView.getVisibility() != VISIBLE) {
                        callingTimeView.setVisibility(View.VISIBLE);
                    }
                    break;
                case BluetoothHeadsetClientCall.CALL_STATE_HELD:
                    callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_held));
                    break;
                case BluetoothHeadsetClientCall.CALL_STATE_DIALING:
                case BluetoothHeadsetClientCall.CALL_STATE_ALERTING:
                    callStatusView.setText(SkinUtils.getString(R.string.str_out_call_status));
                    break;
                case BluetoothHeadsetClientCall.CALL_STATE_INCOMING:
                    callStatusView.setText(SkinUtils.getString(R.string.str_income_call_status));
                    if (answerButton != null) {
                        answerButton.setVisibility(View.VISIBLE);
                    }
                    break;
                case BluetoothHeadsetClientCall.CALL_STATE_TERMINATED:
                    mPhone = "";
                    subcallNumber.setText("");
                    return;
                default:
                    break;
            }
        }

        //非来电下隐藏接听按钮
        if (callState != BluetoothHeadsetClientCall.CALL_STATE_INCOMING) {
            if (answerButton != null && answerButton.getVisibility() == View.VISIBLE) {
                answerButton.setVisibility(View.GONE);
            }
        }

        //call number or name
        String callNumber = call.getNumber();
        mPhone = callNumber;
        String callName = getCallName(callNumber,call.getDevice().getAddress());
        Log.d(TAG, "callNumber(" + callNumber + "), callName(" + callName + ")");
        //主卡片名称显示，没有显示号码
        if (callingNameView != null) {
            if(callingNameView.getVisibility() != View.VISIBLE) {
                callingNameView.setVisibility(View.VISIBLE);
            }
            callingNameView.setText(callName);
        }

        //隐藏三方通话框和三方通话按钮
        if ((heldLayout != null) && (heldLayout.getVisibility() == View.VISIBLE)) {
            heldLayout.setVisibility(View.GONE);
        }
        //通话接听并保持按钮
        if (acceptHeldButton != null && acceptHeldButton.getVisibility() == View.VISIBLE) {
            acceptHeldButton.setVisibility(View.GONE);
        }
        if (swapButton != null && swapButton.getVisibility() == View.VISIBLE) {
            swapButton.setVisibility(View.GONE);
        }

        //未进通话前隐藏键盘和不用的按钮
        if (callState != BluetoothHeadsetClientCall.CALL_STATE_ACTIVE
                && callState != BluetoothHeadsetClientCall.CALL_STATE_HELD) {
            if (keypadButton != null) {
                keypadButton.setVisibility(View.GONE);
            }
            if (mMicMuteBtn != null) {
                mMicMuteBtn.setVisibility(View.GONE);
            }
            if (mCallAddBtn != null) {
                mCallAddBtn.setVisibility(View.GONE);
            }
            if (heldButton != null) {
                heldButton.setVisibility(View.GONE);
            }
            if (voiceSourcePhoneButton != null) {
                voiceSourcePhoneButton.setVisibility(View.GONE);
            }
            if (voiceSourceCarButton != null) {
                voiceSourceCarButton.setVisibility(View.GONE);
            }
        } else {
            if (keypadButton != null) {
                keypadButton.setVisibility(View.VISIBLE);
            }
            if (mMicMuteBtn != null) {
                mMicMuteBtn.setVisibility(View.VISIBLE);
            }
            if (mCallAddBtn != null) {
                mCallAddBtn.setVisibility(View.VISIBLE);
                mCallAddBtn.setEnabled(true);
            }
            if (heldButton != null) {
                heldButton.setVisibility(View.VISIBLE);
            }
        }

        //刷新MIC模式,是否静音输入
        updateMicSource(mAudioManager.isMicrophoneMute());
        //刷新音频输出是在蓝牙还是手机。(同时会刷新两个切换按钮)
        if (mHFPService != null) {
            updateAudioState(mHFPService.getAudioState());
        }
    }

    private void onMultiCall(List<BluetoothHeadsetClientCall> callList) {
        if (callList == null) {
            return;
        }
        Log.d(TAG, "onMultiCall");

        //no show held and answer button on multi call
        if ((heldButton != null) && (heldButton.getVisibility() == View.VISIBLE)) {
            heldButton.setVisibility(View.GONE);
        }
        if ((answerButton != null) && (answerButton.getVisibility() == View.VISIBLE)) {
            answerButton.setVisibility(View.GONE);
        }

        //显示三方通话框
        if ((heldLayout != null) && (heldLayout.getVisibility() != View.VISIBLE)) {
            heldLayout.setVisibility(View.VISIBLE);
        }

        List<BluetoothHeadsetClientCall> incomingCalls = BluetoothHfpclientService.getCall(callList,
                BluetoothHeadsetClientCall.CALL_STATE_WAITING,
                BluetoothHeadsetClientCall.CALL_STATE_INCOMING);
        List<BluetoothHeadsetClientCall> heldCall = BluetoothHfpclientService.getCall(callList,
                BluetoothHeadsetClientCall.CALL_STATE_HELD);
        List<BluetoothHeadsetClientCall> outgoingCall = BluetoothHfpclientService.getCall(
                callList,
                BluetoothHeadsetClientCall.CALL_STATE_DIALING,
                BluetoothHeadsetClientCall.CALL_STATE_ALERTING);
        List<BluetoothHeadsetClientCall> activeCall = BluetoothHfpclientService.getCall(
                callList,
                BluetoothHeadsetClientCall.CALL_STATE_ACTIVE);

        String callNumber = "";
        String callName = "";
        if (!incomingCalls.isEmpty()) {
            callNumber = incomingCalls.get(0).getNumber();
            callName = incomingCalls.get(0).getDevice().getAddress();
        } else if (!outgoingCall.isEmpty()) {
            callNumber = outgoingCall.get(0).getNumber();
            callName = outgoingCall.get(0).getDevice().getAddress();
        } else if (!activeCall.isEmpty()) {
            callNumber = activeCall.get(0).getNumber();
            callName = activeCall.get(0).getDevice().getAddress();
            mPhone = callNumber;
        } else {
            if (heldCall.size() > 0x01) {
                callNumber = heldCall.get(0).getNumber();
                callName = heldCall.get(0).getDevice().getAddress();
            }
        }

        callName = getCallName(callNumber,callName);

        if (callingNameView != null) {
            callingNameView.setVisibility(View.VISIBLE);
            callingNameView.setText(callName);
        }

        //来电
        if (!incomingCalls.isEmpty()) {
            //来去电时不让切换,无效
            if(heldLayout != null) {
                heldLayout.setEnabled(false);
            }
            //隐藏通话时间，显示状态
            if (callStatusView != null) {
                if (callingTimeView != null) {
                    callingTimeView.setVisibility(View.GONE);
                }
                callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_waiting));
            }

            //close keypad in waiting state
            if (softkeyPadLayout != null && softkeyPadLayout.getVisibility() == View.VISIBLE) {
                softkeyPadLayout.setVisibility(View.GONE);
            }
            //check button
            if (keypadButton != null) {
                keypadButton.setVisibility(View.GONE);
            }

            if (mMicMuteBtn != null) {
                mMicMuteBtn.setVisibility(View.GONE);
            }
            if (mCallAddBtn != null) {
                mCallAddBtn.setVisibility(View.GONE);
            }
            if (voiceSourceCarButton != null) {
                voiceSourceCarButton.setVisibility(View.GONE);
            }
            if (voiceSourcePhoneButton != null) {
                voiceSourcePhoneButton.setVisibility(View.GONE);
            }

            //接听并保持
            if (acceptHeldButton != null) {
                acceptHeldButton.getBackground().setAlpha(OPACITY);
                if (callList.size() > MAX_CALL_SIZE) {
                    acceptHeldButton.getBackground().setAlpha(HALFOPACITY);
                }
                acceptHeldButton.setVisibility(View.VISIBLE);
            }

            if (swapButton != null && swapButton.getVisibility() == View.VISIBLE) {
                swapButton.setVisibility(View.GONE);
            }
        } else {//非来电状态
            //隐藏接听按钮
            if (acceptHeldButton != null && acceptHeldButton.getVisibility() == View.VISIBLE) {
                acceptHeldButton.setVisibility(View.GONE);
            }
            //通话显示mic和添加通话按钮
            if (!outgoingCall.isEmpty()) {//去电中
                heldLayout.setEnabled(false);
                if (mMicMuteBtn != null) {
                    mMicMuteBtn.setVisibility(View.GONE);
                }
                if (mCallAddBtn != null) {
                    mCallAddBtn.setVisibility(View.GONE);
                }
                if (callingTimeView != null) {
                    callingTimeView.setVisibility(View.GONE);
                }
                if (keypadButton != null) {
                    keypadButton.setVisibility(View.GONE);
                }
                if (voiceSourceCarButton != null) {
                    voiceSourceCarButton.setVisibility(View.GONE);
                }
                if (callStatusView != null) {
                    callStatusView.setText(SkinUtils.getString(R.string.str_out_call_status));
                }

                if (swapButton != null && swapButton.getVisibility() == View.VISIBLE) {
                    swapButton.setVisibility(View.GONE);
                }
            } else {//不是来电和去电
                heldLayout.setEnabled(true);
                if (mMicMuteBtn != null) {
                    mMicMuteBtn.setVisibility(View.VISIBLE);
                }
                if (mCallAddBtn != null) {
                    mCallAddBtn.setVisibility(View.VISIBLE);
                    mCallAddBtn.setEnabled(false);
                }
                if (callingTimeView != null) {
                    callingTimeView.setVisibility(View.VISIBLE);
                }
                if (keypadButton != null) {
                    keypadButton.setVisibility(View.VISIBLE);
                }
                if (voiceSourceCarButton != null) {
                    voiceSourceCarButton.setVisibility(View.VISIBLE);
                }
                if (swapButton != null) {
                    swapButton.setVisibility(View.VISIBLE);
                }
            }

            if (mHFPService != null) {
                updateAudioState(mHFPService.getAudioState());
            }

            //显示三方通话内容
            if (!heldCall.isEmpty() || (
                    (!outgoingCall.isEmpty() || !incomingCalls.isEmpty()) && !activeCall.isEmpty())
            ) {
                BluetoothHeadsetClientCall c;
                if (!heldCall.isEmpty()) {
                    c = heldCall.get(0);
                } else {
                    c = activeCall.get(0);
                }
                if (heldStateView != null) {
                    heldStateView.setText(SkinUtils.getString(R.string.str_bt_call_held));
                }
                //call number or name
                callNumber = c.getNumber();
                callName = getCallName(callNumber,c.getDevice().getAddress());

                if (heldNumView != null) {
                    //三方通话显示号码
                    if (heldPhoneView != null) {
                        heldPhoneView.setText(callNumber);
                    }
                    heldNumView.setText(callName);
                }
                return;
            }

            Log.d(TAG, "isMultiParty");
            if ((!activeCall.isEmpty() && activeCall.get(0).isMultiParty()) ||
                    (!heldCall.isEmpty() && heldCall.get(0).isMultiParty())) {
                if (callStatusView != null && (callStatusView.getVisibility() == View.GONE)) {
                    callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_multiparty));
                }
            } else {
                //invalid call status, open call satus view on top
                if (callStatusView != null && (callStatusView.getVisibility() == View.GONE)) {
                    callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_abnormal));
                }
            }

            if (heldLayout != null) {
                heldLayout.setVisibility(View.GONE);
            }

            if (swapButton != null && swapButton.getVisibility() == View.VISIBLE) {
                swapButton.setVisibility(View.GONE);
            }
        }
    }

    private void onClickSoftkeypad() {
        if (softkeyPadLayout == null) {
            Log.d(TAG, "get calling_softkeypad fail!");
            return;
        }

        if (softkeyPadLayout.getVisibility() == View.VISIBLE) {
            softkeyPadLayout.setVisibility(View.GONE);
        } else {
            softkeyPadLayout.setVisibility(View.VISIBLE);
        }
    }

    private void onClickMicSwitch() {
        Log.i(TAG, "on click mic switch!");
        if (mAudioManager.isMicrophoneMute()) {
            mAudioManager.setMicrophoneMute(false);
            updateMicSource(false);
        } else {
            mAudioManager.setMicrophoneMute(true);
            updateMicSource(true);
        }
    }

    private void addSubPhoneCallInputString(CharSequence str) {
        if (subcallNumber == null) {
            Log.d(TAG, "get calling_input_et fail!");
            return;
        }

        int index = subcallNumber.getSelectionStart();
        Editable subcallNumstr = subcallNumber.getEditableText();

        if (subcallNumstr == null) {
            Log.d(TAG, "get subcallNumstr_Edt error!");
            return;
        }

        if (index < 0 || index > subcallNumstr.length()) {
            subcallNumstr.append(str);
        } else {
            subcallNumstr.insert(index, str);
        }
        subcallNumber.setText(subcallNumstr);
        subcallNumber.setSelection(index + 1);
    }

    private void updateMicSource(boolean mute) {
        if (null == mMicMuteBtn) {
            return;
        }
        if (mute) {
            mMicMuteBtn.setBackgroundResource(SkinUtils.getResId(R.drawable.skpad_mic_disable));
        } else {
            mMicMuteBtn.setBackgroundResource(SkinUtils.getResId(R.drawable.skpad_mic_enable));
        }
    }

    @Override
    public void updateAudioState(int state) {
        Log.d(TAG, "updateAudioState state=" + state);
        List<BluetoothHeadsetClientCall> callList = mHFPService.getCurrentCalls();
        if (callList == null || callList.size() == 0) {
            Log.e(TAG, "updateAudioState no call!");
            return;
        }
        List<BluetoothHeadsetClientCall> calls = BluetoothHfpclientService.getCall(callList,
                BluetoothHeadsetClientCall.CALL_STATE_INCOMING,
                BluetoothHeadsetClientCall.CALL_STATE_WAITING);

        if (!calls.isEmpty()) {
            Log.d(TAG, "incoming or waiting not show audio state!!!");
            return;
        }

        //只在通话或者通话保持时显示
        if (callList.get(0).getState() != BluetoothHeadsetClientCall.CALL_STATE_ACTIVE
                && callList.get(0).getState() != BluetoothHeadsetClientCall.CALL_STATE_HELD) {
            Log.d(TAG, "507 only in active or held state can enter!!!");
            voiceSourceCarButton.setVisibility(View.GONE);
            voiceSourcePhoneButton.setVisibility(View.GONE);
            return;
        }

        if (callList.size() == 1 && callList.get(0).getState()
                == BluetoothHeadsetClientCall.CALL_STATE_HELD) {
            Log.d(TAG, "only one call and state is held not show audio state!!!");
            return;
        }

        //should set Gone
        if (answerButton != null && answerButton.getVisibility() == View.VISIBLE) {
            answerButton.setVisibility(View.GONE);
        }

        if (voiceSourceCarButton == null || voiceSourcePhoneButton == null) {
            Log.e(TAG, "updateAudioState:audio buttion is null!!!");
            return;
        }

        if (state == BluetoothHeadsetClient.STATE_AUDIO_CONNECTED) {
            voiceSourcePhoneButton.setVisibility(View.GONE);
            voiceSourceCarButton.setVisibility(View.VISIBLE);
        } else if (state == BluetoothHeadsetClient.STATE_AUDIO_DISCONNECTED) {
            voiceSourceCarButton.setVisibility(View.GONE);
            voiceSourcePhoneButton.setVisibility(View.VISIBLE);
        }
    }

    /**
     * 添加三方通话时，需要跳转指定页面。
     * 就T5用到，常量定义暂无通用性，先放此处。
     * public final static String EXTRA_UI_PAGE = "extra_page_index";
     * 目前定义卡片页ID
     * //CONTACT = 0;
     * //RECORD = 1;
     * //DIAL = 2;
     * //SETTING = 3;
     */
    private void addCall() {
        try {
            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.setComponent(new ComponentName(Utils.BT_PACKAGE_NAME, Utils.BT_ACTIVITY_NAME));
            intent.putExtra("extra_page_index",2);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mContext.startActivity(intent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
