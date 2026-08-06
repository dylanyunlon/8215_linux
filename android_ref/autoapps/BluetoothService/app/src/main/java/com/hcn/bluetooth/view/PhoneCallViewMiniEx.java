package com.hcn.bluetooth.view;

import android.app.ActivityManager;
import android.bluetooth.BluetoothHeadsetClient;
import android.bluetooth.BluetoothHeadsetClientCall;
import android.content.ComponentName;
import android.content.Context;
import android.media.AudioManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
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

public class PhoneCallViewMiniEx extends PhoneCallViewBase implements OnClickListener {
    private static final String TAG = "PhoneCallViewMiniEx";
    public static final int MAX_CALL_SIZE = 2;
    public static final int OPACITY = 255;
    public static final int HALFOPACITY = 100;

    private BluetoothHfpclientService mHFPService;
    //
    private View mCallBg;
    //通话状态显示
    TextView callStatusView;
    TextView callingTimeView;
    TextView callingNameView;
    //通话音源切换按钮
    private Button voiceSourceCarButton;
    private Button voiceSourcePhoneButton;

    private Button answerButton;
    private Button hangupButton;
    private Button acceptHeldButton;
    //当前通话的号码
    private String mPhone;
    //过掉重复数据库操作
    private Map<String,String> mMapData = new HashMap<>();

    public PhoneCallViewMiniEx(Context context, WindowManager wm, AudioManager am) {
        super(context, wm, am);
        mLayoutParams.type = SkinUtils.getInteger(R.integer.call_view_mini_window_type);
        mLayoutParams.gravity = SkinUtils.getInteger(R.integer.call_view_mini_window_gravity);
        mLayoutParams.width = SkinUtils.getInteger(R.integer.call_view_mini_window_width);
        mLayoutParams.height = SkinUtils.getInteger(R.integer.call_view_mini_window_height);
        mHFPService = BluetoothHfpclientService.getInstance();
    }

    @Override
    protected int getLayoutId() {
        return R.layout.bt_calling_status_mini;
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

        if (null != callingTimeView) {
            callingTimeView.setVisibility(GONE);
        }
        if(mMapData != null){
            mMapData.clear();
        }
    }

    @Override
    public void showView() {
        ActivityManager am = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        try {
            ComponentName cn = am.getRunningTasks(1).get(0).topActivity;
            if (Utils.BT_ACTIVITY_NAME.equals(cn.getClassName())) {
                mLayoutParams.height = -2;
            }else{
                mLayoutParams.height = SkinUtils.getInteger(R.integer.call_view_mini_window_height);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        super.showView();
    }

    @Override
    public void updateCallTime(Map<String, CallInfo> callTimeMap) {
        if (callTimeMap.isEmpty()) {
            callingTimeView.setVisibility(GONE);
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
            case R.id.btn_voiceswitch_car:
            case R.id.btn_voiceswitch_phone:
                mHFPService.switchAudio();
                break;

            case R.id.bt_calling_root:
                Utils.startApp(mContext, Utils.BT_PACKAGE_NAME, Utils.BT_ACTIVITY_NAME);
                if(mHFPService != null) {
                    mHFPService.switchCallViewByManual(BluetoothHfpclientService.CALL_VIEW_NORMAL);
                }
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

        voiceSourceCarButton = findViewById(SkinUtils.getId(R.id.btn_voiceswitch_car));
        voiceSourcePhoneButton = findViewById(SkinUtils.getId(R.id.btn_voiceswitch_phone));
        answerButton = findViewById(SkinUtils.getId(R.id.btn_answer));
        hangupButton = findViewById(SkinUtils.getId(R.id.btn_hangup));
        acceptHeldButton = findViewById(SkinUtils.getId(R.id.btn_accept_held));
        mCallBg = findViewById(SkinUtils.getId(R.id.bt_calling_mini_bg));
        try {
            answerButton.setOnClickListener(this);
            voiceSourceCarButton.setOnClickListener(this);
            voiceSourcePhoneButton.setOnClickListener(this);
            hangupButton.setOnClickListener(this);
            acceptHeldButton.setOnClickListener(this);
            if (findViewById(SkinUtils.getId(R.id.bt_calling_root)) != null) {
                findViewById(SkinUtils.getId(R.id.bt_calling_root)).setOnClickListener(this);
            }
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

        //call number or name
        String callNumber = call.getNumber();
        mPhone = callNumber;
        String callName = getCallName(callNumber,call.getDevice().getAddress());

        Log.d(TAG, "callNumber(" + callNumber + "), callName(" + callName + ")");

        if (callingNameView != null) {
            callingNameView.setVisibility(View.VISIBLE);
            callingNameView.setText(callName);
        }

        if (acceptHeldButton != null && acceptHeldButton.getVisibility() == View.VISIBLE) {
            acceptHeldButton.setVisibility(View.GONE);
        }

        if(mCallBg != null) {
            if (callState == BluetoothHeadsetClientCall.CALL_STATE_INCOMING) {
                mCallBg.setBackgroundResource(SkinUtils.getResId(R.drawable.mini_dialog_bg2));
            } else {
                mCallBg.setBackgroundResource(SkinUtils.getResId(R.drawable.mini_dialog_bg));
            }
        }

        if (callState == BluetoothHeadsetClientCall.CALL_STATE_ACTIVE) {
//            if (voiceSourceCarButton != null) {
//                voiceSourceCarButton.setVisibility(View.VISIBLE);
//            }
//            if (voiceSourcePhoneButton != null) {
//                voiceSourcePhoneButton.setVisibility(View.VISIBLE);
//            }
            if (callingTimeView != null) {
                callingTimeView.setVisibility(View.VISIBLE);
            }
            if (callStatusView != null) {
                callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_active));
            }
            if (answerButton != null) {
                answerButton.setVisibility(View.GONE);
            }
        } else {
            if (voiceSourceCarButton != null) {
                voiceSourceCarButton.setVisibility(View.GONE);
            }
            if (voiceSourcePhoneButton != null) {
                voiceSourcePhoneButton.setVisibility(View.GONE);
            }
            if (callState == BluetoothHeadsetClientCall.CALL_STATE_HELD) {
                if (answerButton != null) {
                    answerButton.setVisibility(View.GONE);
                }
                if (callStatusView != null) {
                    callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_held));
                }
            } else if (callState == BluetoothHeadsetClientCall.CALL_STATE_INCOMING) {
                if (answerButton != null) {
                    answerButton.setVisibility(View.VISIBLE);
                }
                if (callStatusView != null) {
                    callStatusView.setText(SkinUtils.getString(R.string.str_income_call_status));
                }
            } else if (callState == BluetoothHeadsetClientCall.CALL_STATE_DIALING
                    || callState == BluetoothHeadsetClientCall.CALL_STATE_ALERTING) {
                if (callStatusView != null) {
                    callStatusView.setText(SkinUtils.getString(R.string.str_out_call_status));
                }
                if (answerButton != null && answerButton.getVisibility() == View.VISIBLE) {
                    answerButton.setVisibility(View.GONE);
                }
            } else {
                if (answerButton != null && answerButton.getVisibility() == View.VISIBLE) {
                    answerButton.setVisibility(View.GONE);
                }
            }
        }
        if (mHFPService != null) {
            updateAudioState(mHFPService.getAudioState());
        }
    }

    private void onMultiCall(List<BluetoothHeadsetClientCall> callList) {
        if (callList == null) {
            return;
        }
        Log.d(TAG, "onMultiCall");
        for (int i = 0; i < callList.size(); i++) {
            BluetoothHeadsetClientCall call = callList.get(i);
            Log.d(TAG, "onMultiCall: num=" + call.getNumber() + " state="
                    + mHFPService.callStateToString(call.getState()));
        }
        Log.d(TAG, "*************************************************************");

        if ((answerButton != null) && (answerButton.getVisibility() == View.VISIBLE)) {
            answerButton.setVisibility(View.GONE);
        }

        List<BluetoothHeadsetClientCall> inComingCalls = BluetoothHfpclientService.getCall(callList,
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
        if (!inComingCalls.isEmpty()) {
            callNumber = inComingCalls.get(0).getNumber();
            callName = inComingCalls.get(0).getDevice().getAddress();
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

        //来电时的状态UI显示
        if (!inComingCalls.isEmpty()) {
            //隐藏通话时间，显示状态
            if (callStatusView != null) {
                callStatusView.setVisibility(View.VISIBLE);
                if (callingTimeView != null) {
                    callingTimeView.setVisibility(View.GONE);
                }
                callStatusView.setText(SkinUtils.getString(R.string.str_bt_call_waiting));
            }

            if (voiceSourceCarButton != null) {
                voiceSourceCarButton.setVisibility(View.GONE);
            }
            if (voiceSourcePhoneButton != null) {
                voiceSourcePhoneButton.setVisibility(View.GONE);
            }

            if (acceptHeldButton != null) {
                acceptHeldButton.getBackground().setAlpha(OPACITY);
                if (callList.size() > MAX_CALL_SIZE) {
                    acceptHeldButton.getBackground().setAlpha(HALFOPACITY);
                }
                acceptHeldButton.setVisibility(View.VISIBLE);
            }
            if(mCallBg != null) {
                mCallBg.setBackgroundResource(SkinUtils.getResId(R.drawable.mini_dialog_bg2));
            }
        } else {
            if (acceptHeldButton != null && acceptHeldButton.getVisibility() == View.VISIBLE) {
                acceptHeldButton.setVisibility(View.GONE);
            }

            if(mCallBg != null) {
                mCallBg.setBackgroundResource(SkinUtils.getResId(R.drawable.mini_dialog_bg));
            }

            if (!outgoingCall.isEmpty()) {
                if (callStatusView != null) {
                    callStatusView.setVisibility(View.VISIBLE);
                    callStatusView.setText(SkinUtils.getString(R.string.str_out_call_status));
                }
                if (callingTimeView != null) {
                    callingTimeView.setVisibility(View.GONE);
                }

                if (voiceSourceCarButton != null) {
                    voiceSourceCarButton.setVisibility(View.GONE);
                }
                if (voiceSourcePhoneButton != null) {
                    voiceSourcePhoneButton.setVisibility(View.GONE);
                }
            } else {
                if (callingTimeView != null) {
                    callingTimeView.setVisibility(View.VISIBLE);
                }
                //if (voiceSourceCarButton != null) {
                //    voiceSourceCarButton.setVisibility(View.VISIBLE);
                //}
                //if (voiceSourcePhoneButton != null) {
                //    voiceSourcePhoneButton.setVisibility(View.VISIBLE);
                //}
            }
        }

        if (mHFPService != null) {
            updateAudioState(mHFPService.getAudioState());
        }
        if (
                !heldCall.isEmpty() || ((!outgoingCall.isEmpty() || !inComingCalls.isEmpty()) && !activeCall.isEmpty())
        ) {
            return ;
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
}
