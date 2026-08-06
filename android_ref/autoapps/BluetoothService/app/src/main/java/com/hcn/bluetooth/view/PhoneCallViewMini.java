package com.hcn.bluetooth.view;

import android.bluetooth.BluetoothHeadsetClient;
import android.bluetooth.BluetoothHeadsetClientCall;
import android.content.Context;
import android.media.AudioManager;
import android.text.Editable;
import android.text.InputType;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
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

public class PhoneCallViewMini extends PhoneCallViewBase implements OnClickListener {
    private static final String TAG = "PhoneCallViewMini";
    public static final int MAX_CALL_SIZE = 2;
    public static final int OPACITY = 255;
    public static final int HALFOPACITY = 100;

    private BluetoothHfpclientService mHFPService;

    //通话状态显示
    TextView callStatusView;
    TextView callingNameView;
    TextView callingTimeView;
    TextView callingNumView;
    TextView callingStateView;
    //通话保持状态
    private LinearLayout heldLayout;
    private TextView heldNumView;
    private TextView heldStateView;
    //通话音源切换按钮
    private Button voiceSourceCarButton;
    private Button voiceSourcePhoneButton;

    private Button answerButton;
    private Button hangupButton;
    private Button heldButton;
    private Button acceptTerminateButton;
    private Button acceptHeldButton;
    private Button swapButton;
    private Button mergeButton;
    private Button mMicMuteBtn = null;

    private LinearLayout softkeyPadLayout;
    private Button keypadButton;

    private EditText subcallNumber;

    //过掉重复数据库操作
    private Map<String, String> mMapData = new HashMap<>();

    public PhoneCallViewMini(Context context, WindowManager wm, AudioManager am) {
        super(context, wm, am);
        mHFPService = BluetoothHfpclientService.getInstance();
        mLayoutParams.type = SkinUtils.getInteger(R.integer.call_view_mini_window_type);
        mLayoutParams.gravity = SkinUtils.getInteger(R.integer.call_view_mini_window_gravity);
        mLayoutParams.width = SkinUtils.getInteger(R.integer.call_view_mini_window_width);
        mLayoutParams.height = SkinUtils.getInteger(R.integer.call_view_mini_window_height);
        //compatible with 8227 skin apk
        if (SkinUtils.useSkinPackage()) {
            if (SkinUtils.isResourcesExist(R.layout.bt_calling_status_mini) > 0) {
                mViewRoot = SkinUtils.getLayout(R.layout.bt_calling_status_mini, this);
            } else {
                //compatible with 8227 skin apk
                mViewRoot = SkinUtils.getLayout("bt_calling_status_small", this);
            }
        } else {
            mViewRoot = LayoutInflater.from(context).inflate(R.layout.bt_calling_status_mini, this);
        }
        mViewRoot.setOnTouchListener(this);
        initView();
    }

    @Override
    protected int getLayoutId() {
        //compatible with 8227 skin apk
        return 0;
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
        mLayoutParams.x = 0;
        mLayoutParams.y = 0;
        if (null != softkeyPadLayout) {
            softkeyPadLayout.setVisibility(View.GONE);
        }
        if (null != subcallNumber) {
            subcallNumber.setText("");
        }
        if (null != callingTimeView) {
            callingTimeView.setVisibility(GONE);
        }
        if (mMapData != null) {
            mMapData.clear();
        }
    }

    @Override
    public void updateCallTime(Map<String, CallInfo> callTimeMap) {
        if (callTimeMap.isEmpty()) {
            callingTimeView.setVisibility(GONE);
            return;
        }
        if (callingTimeView.getVisibility() != VISIBLE) {
            callingTimeView.setVisibility(VISIBLE);
        }
        //只显示一个总时间时，取最大值
        int callTime = 0;
        for (Map.Entry<String, CallInfo> entry : callTimeMap.entrySet()) {
            callTime = Math.max(callTime, entry.getValue().getCallTime());
        }
        String strText = String.format(Locale.ENGLISH, "%02d:%02d:%02d", callTime / 3600 % 60,
                callTime / 60 % 60, callTime % 60);
        Log.d(TAG, "updateCallTime: " + " time=" + strText);
        if (null != callingTimeView) {
            callingTimeView.setText(strText);
        }
    }

    @Override
    public void onClick(View v) {
        int viewId = SkinUtils.getViewId(v);
        switch (viewId) {
            case R.id.btn_hangup:
                Log.d(TAG, "btn_hangup");
                mHFPService.hangup();
                List<BluetoothHeadsetClientCall> callList = mHFPService.getCurrentCalls();
                if (callList == null || callList.size() == 0) {
                    Log.e(TAG, "btn_hangup no call!");
                    hideView();
                }
                break;
            case R.id.btn_answer:
            case R.id.btn_merge:
                mHFPService.acceptCall(BluetoothHeadsetClient.CALL_ACCEPT_NONE);
                break;
            case R.id.btn_swap:
            case R.id.btn_held:
            case R.id.btn_accept_held:
                mHFPService.acceptCall(BluetoothHeadsetClient.CALL_ACCEPT_HOLD);
                break;
            case R.id.btn_accept_terminate:
                mHFPService.acceptCall(BluetoothHeadsetClient.CALL_ACCEPT_TERMINATE);
                break;
            case R.id.btn_softkeypad:
                onClickSoftkeypad();
                break;
            case R.id.btn_voiceswitch_car:
            case R.id.btn_voiceswitch_phone:
                mHFPService.switchAudio();
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
        callingNumView = findViewById(SkinUtils.getId(R.id.bt_calling_phone_number));
        callingStateView = findViewById(SkinUtils.getId(R.id.bt_calling_state_text));

        heldLayout = findViewById(SkinUtils.getId(R.id.call_held_layout));
        heldNumView = findViewById(SkinUtils.getId(R.id.bt_call_held_number));
        heldStateView = findViewById(SkinUtils.getId(R.id.bt_call_held_state_text));

        voiceSourceCarButton = findViewById(SkinUtils.getId(R.id.btn_voiceswitch_car));
        voiceSourcePhoneButton = findViewById(SkinUtils.getId(R.id.btn_voiceswitch_phone));
        answerButton = findViewById(SkinUtils.getId(R.id.btn_answer));
        hangupButton = findViewById(SkinUtils.getId(R.id.btn_hangup));
        heldButton = findViewById(SkinUtils.getId(R.id.btn_held));
        acceptTerminateButton = findViewById(SkinUtils.getId(R.id.btn_accept_terminate));
        acceptHeldButton = findViewById(SkinUtils.getId(R.id.btn_accept_held));
        swapButton = findViewById(SkinUtils.getId(R.id.btn_swap));
        mergeButton = findViewById(SkinUtils.getId(R.id.btn_merge));

        softkeyPadLayout = findViewById(SkinUtils.getId(R.id.calling_softkeypad));
        keypadButton = findViewById(SkinUtils.getId(R.id.btn_softkeypad));

        subcallNumber = findViewById(SkinUtils.getId(R.id.calling_input_et));

        try {
            answerButton.setOnClickListener(this);
            voiceSourceCarButton.setOnClickListener(this);
            voiceSourcePhoneButton.setOnClickListener(this);
            hangupButton.setOnClickListener(this);
            acceptTerminateButton.setOnClickListener(this);
            acceptHeldButton.setOnClickListener(this);
            heldButton.setOnClickListener(this);
            swapButton.setOnClickListener(this);
            mergeButton.setOnClickListener(this);
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
     *
     * @param callNumber
     * @param address
     * @return
     */
    private String getCallName(String callNumber, String address) {
        String callName = "";
        if (mMapData != null) {
            callName = mMapData.get(callNumber);
            if (TextUtils.isEmpty(callName)) {
                callName = Utils.getContactNameByNumber(mContext, callNumber, address);
                Log.d(TAG, "getCallName: num=" + callNumber + " name=" + callName);
                //查询一次后放弃循环，如一直查导致循环查询数据库ANR
                if (TextUtils.isEmpty(callName)) {
                    callName = SkinUtils.getString(R.string.unkown);
                }
                mMapData.put(callNumber, callName);
            }
            return callName;
        }
        return SkinUtils.getString(R.string.unkown);
    }

    private void onSingleCall(BluetoothHeadsetClientCall call) {
        Log.d(TAG, "onSingleCall");

        if (call == null) {
            return;
        }

        if ((callingStateView != null) && (callingStateView.getVisibility() == View.VISIBLE)) {
            callingStateView.setVisibility(View.GONE);
        }
        if ((heldLayout != null) && (heldLayout.getVisibility() == View.VISIBLE)) {
            heldLayout.setVisibility(View.GONE);
        }

        int callState = call.getState();
        Log.d(TAG, "callState: " + callState);

        //call state
        if (callStatusView != null) {
            callStatusView.setVisibility(View.VISIBLE);
            switch (callState) {
                case BluetoothHeadsetClientCall.CALL_STATE_ACTIVE:
                    callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_active));
                    Log.d(TAG, "callState: CALL_STATE_ACTIVE");
                    break;

                case BluetoothHeadsetClientCall.CALL_STATE_HELD:
                    callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_held));
                    Log.d(TAG, "callState: CALL_STATE_HELD");
                    break;

                case BluetoothHeadsetClientCall.CALL_STATE_DIALING:
                case BluetoothHeadsetClientCall.CALL_STATE_ALERTING:
                    callStatusView.setText(SkinUtils.getString(R.string.str_out_call_status));
                    Log.d(TAG, "callState: CALL_STATE_DIALING");
                    break;
                case BluetoothHeadsetClientCall.CALL_STATE_INCOMING:
                    callStatusView.setText(SkinUtils.getString(R.string.str_income_call_status));
                    Log.d(TAG, "callState: CALL_STATE_INCOMING");
                    break;

                case BluetoothHeadsetClientCall.CALL_STATE_TERMINATED:
                    subcallNumber.setText("");
                    Log.d(TAG, "callState: CALL_STATE_TERMINATED");
                    return;

                default:
                    break;
            }
        }

        //call number or name
        String callNumber = call.getNumber();
        String callName = getCallName(callNumber, call.getDevice().getAddress());
        Log.d(TAG, "callNumber(" + callNumber + "), callName(" + callName + ")");

        if (callingNumView != null) {
            callingNumView.setVisibility(View.VISIBLE);
            callingNumView.setText(callNumber);
        }

        if (callingNameView != null) {
            callingNameView.setVisibility(View.VISIBLE);
            //解决姓名跑马灯跑不全的问题
            if (!callName.equals(callingNameView.getText().toString())) {
                callingNameView.setText(callName);
            }
        }

        //check button
        if (acceptTerminateButton != null
                && acceptTerminateButton.getVisibility() == View.VISIBLE) {
            acceptTerminateButton.setVisibility(View.GONE);
        }
        if (acceptHeldButton != null && acceptHeldButton.getVisibility() == View.VISIBLE) {
            acceptHeldButton.setVisibility(View.GONE);
        }
        if (swapButton != null && swapButton.getVisibility() == View.VISIBLE) {
            swapButton.setVisibility(View.GONE);
        }
        if (mergeButton != null && mergeButton.getVisibility() == View.VISIBLE) {
            mergeButton.setVisibility(View.GONE);
        }
        if (keypadButton != null) {
            keypadButton.setVisibility(View.VISIBLE);
        }

        if (callState == BluetoothHeadsetClientCall.CALL_STATE_HELD) {
            //show call held button
            if (heldButton != null) {
                heldButton.setVisibility(View.VISIBLE);
            }

            if (voiceSourceCarButton != null) {
                voiceSourceCarButton.setVisibility(View.GONE);
            }
            if (voiceSourcePhoneButton != null) {
                voiceSourcePhoneButton.setVisibility(View.GONE);
            }

            if (answerButton != null) {
                answerButton.setVisibility(View.GONE);
            }

        } else if (callState == BluetoothHeadsetClientCall.CALL_STATE_INCOMING) {
            if (voiceSourceCarButton != null) {
                voiceSourceCarButton.setVisibility(View.GONE);
            }
            if (voiceSourcePhoneButton != null) {
                voiceSourcePhoneButton.setVisibility(View.GONE);
            }

            //hide call held button
            if (heldButton != null) {
                heldButton.setVisibility(View.GONE);
            }

            if (answerButton != null) {
                answerButton.setVisibility(View.VISIBLE);
            }
        } else {
            if (answerButton != null && answerButton.getVisibility() == View.VISIBLE) {
                answerButton.setVisibility(View.GONE);
            }

            //hide call held button
            if (heldButton != null) {
                heldButton.setVisibility(View.GONE);
            }

            if (mHFPService != null) {
                updateAudioState(mHFPService.getAudioState());
            }
        }
    }

    public static synchronized String getCallNumber(List<BluetoothHeadsetClientCall> callList) {
        String callNumber = "";
        for (BluetoothHeadsetClientCall c : callList) {
            if (c != null) {
                callNumber = callNumber + "/" + c.getNumber();
                Log.d(TAG, "callNumber(" + callNumber + "), callState(" + c.getState() + ")");
            }
        }

        return callNumber;
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

        List<BluetoothHeadsetClientCall> calls = BluetoothHfpclientService.getCall(callList,
                BluetoothHeadsetClientCall.CALL_STATE_WAITING,
                BluetoothHeadsetClientCall.CALL_STATE_INCOMING);
        if (!calls.isEmpty()) {
            if ((callingStateView != null) && (callingStateView.getVisibility() == View.VISIBLE)) {
                callingStateView.setVisibility(View.GONE);
            }
            if ((heldLayout != null) && (heldLayout.getVisibility() == View.VISIBLE)) {
                heldLayout.setVisibility(View.GONE);
            }

            //call state
            if (callStatusView != null) {
                callStatusView.setVisibility(View.VISIBLE);
                callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_waiting));
            }

            //call number or name
            String callNumber = calls.get(0).getNumber();
            String callName = getCallName(callNumber, calls.get(0).getDevice().getAddress());

            if (callingNumView != null) {
                callingNumView.setVisibility(View.VISIBLE);
                callingNumView.setText(callNumber);
            }

            if (callingNameView != null) {
                callingNameView.setVisibility(View.VISIBLE);
                callingNameView.setText(callName);
            }

            //close keypad in waiting state
            if (softkeyPadLayout != null && softkeyPadLayout.getVisibility() == View.VISIBLE) {
                softkeyPadLayout.setVisibility(View.GONE);
            }

            //check button
            if (keypadButton != null) {
                keypadButton.setVisibility(View.GONE);
            }
            if (voiceSourceCarButton != null) {
                voiceSourceCarButton.setVisibility(View.GONE);
            }
            if (voiceSourcePhoneButton != null) {
                voiceSourcePhoneButton.setVisibility(View.GONE);
            }

            if (acceptTerminateButton != null) {
                acceptTerminateButton.getBackground().setAlpha(OPACITY);
                if (callList.size() > MAX_CALL_SIZE) {
                    acceptTerminateButton.getBackground().setAlpha(HALFOPACITY);
                }
                acceptTerminateButton.setVisibility(View.VISIBLE);
            }
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

            if (mergeButton != null && mergeButton.getVisibility() == View.VISIBLE) {
                mergeButton.setVisibility(View.GONE);
            }

        } else {
            if (keypadButton != null) {
                keypadButton.setVisibility(View.VISIBLE);
            }
            if (acceptTerminateButton != null
                    && acceptTerminateButton.getVisibility() == View.VISIBLE) {
                acceptTerminateButton.setVisibility(View.GONE);
            }
            if (acceptHeldButton != null && acceptHeldButton.getVisibility() == View.VISIBLE) {
                acceptHeldButton.setVisibility(View.GONE);
            }

            if (voiceSourceCarButton != null) {
                voiceSourceCarButton.setVisibility(View.VISIBLE);

                if (mHFPService != null) {
                    updateAudioState(mHFPService.getAudioState());
                }
            }

            List<BluetoothHeadsetClientCall> heldCall = BluetoothHfpclientService.getCall(callList,
                    BluetoothHeadsetClientCall.CALL_STATE_HELD);
            List<BluetoothHeadsetClientCall> outgoingCall = BluetoothHfpclientService.getCall(
                    callList,
                    BluetoothHeadsetClientCall.CALL_STATE_DIALING,
                    BluetoothHeadsetClientCall.CALL_STATE_ALERTING);
            List<BluetoothHeadsetClientCall> activeCall = BluetoothHfpclientService.getCall(
                    callList,
                    BluetoothHeadsetClientCall.CALL_STATE_ACTIVE);
            if (!heldCall.isEmpty() || (!outgoingCall.isEmpty() && !activeCall.isEmpty())) {
                //close call satus view on top
                if (callStatusView != null && (callStatusView.getVisibility() == View.VISIBLE)) {
                    callStatusView.setVisibility(View.GONE);
                }

                if (heldLayout != null) {
                    heldLayout.setVisibility(View.VISIBLE);
                }

                BluetoothHeadsetClientCall c;
                if (!heldCall.isEmpty()) {
                    c = heldCall.get(0);
                    //call held state
                    if (heldStateView != null) {
                        heldStateView.setText(SkinUtils.getString(R.string.str_bt_call_held));
                    }
                } else {
                    c = activeCall.get(0);
                    //call active state
                    if (heldStateView != null) {
                        heldStateView.setText(SkinUtils.getString(R.string.str_bt_call_active));
                    }
                }

                //call number or name
                String callNumber = c.getNumber();
                String callName = getCallName(callNumber, c.getDevice().getAddress());

                if (heldNumView != null) {
                    heldNumView.setText(
                            callName.equals(SkinUtils.getString(R.string.unkown)) ? callNumber
                                    : callName);
                }

                if (!outgoingCall.isEmpty()) {
                    //outgoing call number or name
                    callNumber = outgoingCall.get(0).getNumber();
                    callName = getCallName(callNumber, outgoingCall.get(
                            0).getDevice().getAddress());

                    if (callingNumView != null) {
                        callingNumView.setVisibility(View.VISIBLE);
                        callingNumView.setText(callNumber);
                    }

                    if (callingNameView != null) {
                        callingNameView.setVisibility(View.VISIBLE);
                        callingNameView.setText(callName);
                    }
                    //call state
                    if (callingStateView != null) {
                        callingStateView.setVisibility(View.VISIBLE);
                        callingStateView.setText(SkinUtils.getString(R.string.str_out_call_status));
                    }

                    if (swapButton != null && swapButton.getVisibility() == View.VISIBLE) {
                        swapButton.setVisibility(View.GONE);
                    }

                    if (mergeButton != null && mergeButton.getVisibility() == View.VISIBLE) {
                        mergeButton.setVisibility(View.GONE);
                    }
                    return;
                }

                if (!activeCall.isEmpty()) {
                    //active call number or name
                    callNumber = activeCall.get(0).getNumber();
                    callName = getCallName(callNumber, activeCall.get(0).getDevice().getAddress());

                    if (callingNumView != null) {
                        callingNumView.setVisibility(View.VISIBLE);
                        callingNumView.setText(callNumber);
                    }

                    if (callingNameView != null) {
                        callingNameView.setVisibility(View.VISIBLE);
                        callingNameView.setText(callName);
                    }

                    if (callingStateView != null) {
                        callingStateView.setVisibility(View.VISIBLE);
                        callingStateView.setText(SkinUtils.getString(R.string.str_bt_call_active));
                    }

                    if (swapButton != null) {
                        swapButton.setVisibility(View.VISIBLE);
                    }

                    if (mergeButton != null) {
                        mergeButton.setVisibility(View.VISIBLE);
                    }

                    return;
                }
                //切换通话过程中出现两路通话保持情况
                if (heldCall.size() > 0x01) {
                    //active call number or name
                    callNumber = heldCall.get(0x01).getNumber();
                    callName = getCallName(callNumber, heldCall.get(0x01).getDevice().getAddress());

                    if (callingNumView != null) {
                        callingNumView.setVisibility(View.VISIBLE);
                        callingNumView.setText(callNumber);
                    }

                    if (callingNameView != null) {
                        callingNameView.setVisibility(View.VISIBLE);
                        callingNameView.setText(callName);
                    }

                    //call state
                    if (callingStateView != null) {
                        callingStateView.setVisibility(View.VISIBLE);
                        callingStateView.setText(SkinUtils.getString(R.string.str_bt_call_held));
                    }

                    if (swapButton != null) {
                        swapButton.setVisibility(View.VISIBLE);
                    }
                    if (mergeButton != null) {
                        mergeButton.setVisibility(View.VISIBLE);
                    }
                    return;
                }
            }
            Log.d(TAG, "isMultiParty");
            if ((!activeCall.isEmpty() && activeCall.get(0).isMultiParty()) ||
                    (!heldCall.isEmpty() && heldCall.get(0).isMultiParty())) {
                if (callStatusView != null && (callStatusView.getVisibility() == View.GONE)) {
                    callStatusView.setVisibility(View.VISIBLE);
                    callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_multiparty));
                }
            } else {
                //invalid call status, open call satus view on top
                if (callStatusView != null && (callStatusView.getVisibility() == View.GONE)) {
                    callStatusView.setVisibility(View.VISIBLE);
                    callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_abnormal));
                }
            }
            if ((callingStateView != null) && (callingStateView.getVisibility() == View.VISIBLE)) {
                callingStateView.setVisibility(View.GONE);
            }
            if (heldLayout != null) {
                heldLayout.setVisibility(View.GONE);
            }

            if (swapButton != null && swapButton.getVisibility() == View.VISIBLE) {
                swapButton.setVisibility(View.GONE);
            }

            if (mergeButton != null && mergeButton.getVisibility() == View.VISIBLE) {
                mergeButton.setVisibility(View.GONE);
            }

            String callNumber = getCallNumber(callList);

            if (callingNumView != null) {
                callingNumView.setVisibility(View.VISIBLE);
                callingNumView.setText(callNumber);
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
}
