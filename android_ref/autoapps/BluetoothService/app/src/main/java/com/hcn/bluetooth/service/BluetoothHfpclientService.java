package com.hcn.bluetooth.service;

import static android.bluetooth.BluetoothHeadsetClientCall.CALL_STATE_ACTIVE;
import static android.bluetooth.BluetoothHeadsetClientCall.CALL_STATE_ALERTING;
import static android.bluetooth.BluetoothHeadsetClientCall.CALL_STATE_DIALING;
import static android.bluetooth.BluetoothHeadsetClientCall.CALL_STATE_HELD;
import static android.bluetooth.BluetoothHeadsetClientCall.CALL_STATE_HELD_BY_RESPONSE_AND_HOLD;
import static android.bluetooth.BluetoothHeadsetClientCall.CALL_STATE_INCOMING;
import static android.bluetooth.BluetoothHeadsetClientCall.CALL_STATE_TERMINATED;
import static android.bluetooth.BluetoothHeadsetClientCall.CALL_STATE_WAITING;


import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHeadsetClient;
import android.bluetooth.BluetoothHeadsetClientCall;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.media.AudioManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.text.TextUtils;
import android.util.ArrayMap;
import android.util.Log;
import android.view.KeyEvent;
import android.view.WindowManager;
import android.carsource.McuConstant;

import com.hcn.bluetooth.api.Utils;
import com.hcn.bluetooth.bean.CacheData;
import com.hcn.bluetooth.bean.CallInfo;
import com.hcn.bluetooth.skin.SkinUtils;
import com.hcn.bluetooth.view.PhoneCallView;
import com.hcn.bluetooth.view.PhoneCallViewBase;
import com.hcn.bluetooth.view.PhoneCallViewEx;
import com.hcn.bluetooth.view.PhoneCallViewMini;
import com.hcn.bluetooth.view.PhoneCallViewMiniEx;
import com.hcn.bluetoothservice.R;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class BluetoothHfpclientService extends Service {

    public static final String TAG = "HfpClientService";
    //接电话,挂电话按键广播，来自McuService
    public static final String BTACTION = "com.cardroid.action.START_cardroid.bt";
    //同行者拨号指令
    private static final String ACTION_NOTIFICATION_CALL_CALL_NUMBER =
            "com.autochips.bluetooth.BluetoothReceiver.ACTION.ACTION_NOTIFICATION_CALL_NUMBER";
    private static final String EXTRA_NUMBER = "EXTRA_NUMBER";

    //同行者海外版本接听指令
    private static final String ACTION_NOTIFICATION_CALL_ACCEPT_NONE =
            "com.autochips.bluetooth.BluetoothReceiver.ACTION.ACTION_NOTIFICATION_CALL_ACCEPT_NONE";
    //同行者海外版本拒接指令
    private static final String ACTION_NOTIFICATION_CALL_REJECT =
            "com.autochips.bluetooth.BluetoothReceiver.ACTION.ACTION_NOTIFICATION_CALL_REJECT";

    //HiCar发送,HiCar连接时不需要蓝牙显示通话框
    public static final String ACTION_ENABLE_BT_CALL_VIEW = "ACTION_ENABLE_BT_CALL_VIEW";
    private static final String EXTRA_PACKAGE = "EXTRA_PACKAGE";
    private static final String EXTRA_ENABLE = "EXTRA_ENABLE";
    //同Intent.java中ACTION_GLOBAL_BUTTON定义，car service用来发送全局按键事件
    public static final String ACTION_GLOBAL_BUTTON = "android.intent.action.GLOBAL_BUTTON";

    //T5客户端请求更新通话框广播
    public static final String ACTION_UPDATE_BT_CALL_VIEW = "ACTION_UPDATE_BT_CALL_VIEW";

    private static final String ACTION_CONFIGURATION_CHANGED = "android.intent.action.CONFIGURATION_CHANGED";

    //自动接听延时
    public static final int AUTO_ANSWER_DELAY = 5000;
    //通话时间更新间隔
    private static final int INTERVAL_TIME = 1000;
    //倒车广播
    public static final String ACTION_REVSTATUS = "android.mcu.device.action.REVSTATUS";
    public static final String EXTRA_REVSTATUS = "REVSTATUS";
    //需要显示mini通话框的activity类名
    private static final String NEED_MINI_CALL_VIEW_ACTIVITY =
            "com.auto.hcamera.FCameraVideoInActivity,";

    private Context mContext;
    private AudioManager mAudioManager;
    private WindowManager mWindowManager;
    private BluetoothHeadsetClient mHeadsetClient = null;
    private BluetoothDevice mConnectedDevice = null;

    private boolean isReverseState = false;

    private int mAudioState = BluetoothHeadsetClient.STATE_AUDIO_DISCONNECTED;
    private final ArrayMap<String, CallInfo> mCallTimeMap=new ArrayMap<>(3);
    //通话是否开始计时
    private boolean isCallTimeBegin = false;
    //记忆接听时间，接听和挂断间隔800ms，避免自动接听和语音拒接同时执行时，hfp_enable执行不完导致audio崩溃
    private static final int HANGUP_INTERVAL=800;
    private long mAcceptCallTime=0;

    private String mLastCall = "";

    private static BluetoothHfpclientService sInstance = null;

    private IBluetoothHfpclientService.Stub mBinder = null;

    public static synchronized BluetoothHfpclientService getInstance() {
        return sInstance;
    }

    public BluetoothHeadsetClient getHeadsetClient() {
        return mHeadsetClient;
    }


    private PhoneCallViewBase mCurPhoneCallView;
    private PhoneCallViewBase mPhoneCallView = null;
    private PhoneCallViewBase mPhoneCallViewMini = null;
    public static final int CALL_VIEW_INVALIDATE = -1;
    public static final int CALL_VIEW_NORMAL = 0;
    public static final int CALL_VIEW_MINI = 1;
    private int mSelectCallViewByManual = CALL_VIEW_INVALIDATE;//0 切换到大框，1切换到mini框

    private int mScreenOrientation = Configuration.ORIENTATION_LANDSCAPE;

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        init();
        super.onCreate();
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        deinit();
        if (mBinder != null) {
            mBinder = null;
        }
        super.onDestroy();
    }

    @Override
    public void onStart(Intent intent, int startId) {
        Log.d(TAG, "onStart");
        super.onStart(intent, startId);
    }

    @Override
    public IBinder onBind(Intent arg0) {
        Log.d(TAG, "onBind");
        if (mBinder == null) {
            mBinder = new HfpBinder();
        }
        return mBinder;
    }

    private void init() {
        Log.i(TAG, "init");
        sInstance = this;


        mContext = getApplicationContext();

        mScreenOrientation = mContext.getResources().getConfiguration().orientation;

        mWindowManager = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
        mAudioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);

        renewPhoneCallView();

        // get HFP Client Proxy from BT adapter
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (adapter != null) {
            adapter.getProfileProxy(mContext, mProfileServiceListener,
                    BluetoothProfile.HEADSET_CLIENT);
        }

        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(BluetoothHeadsetClient.ACTION_CALL_CHANGED);
        filter.addAction(BluetoothHeadsetClient.ACTION_AUDIO_STATE_CHANGED);
        filter.addAction(BluetoothHeadsetClient.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(BluetoothHeadsetClient.ACTION_LAST_VTAG);
        filter.addAction(BluetoothHeadsetClient.ACTION_AG_EVENT);
        filter.addAction(BluetoothHeadsetClient.ACTION_RESULT);
        filter.addAction(BTACTION);//接听及挂断按键广播
        filter.addAction(ACTION_NOTIFICATION_CALL_CALL_NUMBER);
        filter.addAction(ACTION_NOTIFICATION_CALL_ACCEPT_NONE);
        filter.addAction(ACTION_NOTIFICATION_CALL_REJECT);
        filter.addAction(ACTION_REVSTATUS);//倒车广播
        filter.addAction(ACTION_ENABLE_BT_CALL_VIEW);
        filter.addAction(ACTION_UPDATE_BT_CALL_VIEW);
        filter.addAction(ACTION_CONFIGURATION_CHANGED);
        filter.addAction(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        filter.addAction(ACTION_GLOBAL_BUTTON);

        mContext.registerReceiver(mReceiver, filter);
    }

    private void renewPhoneCallView(){
        Log.d(TAG, "renewPhoneCallView");
        //mcc 500用全屏通话框
        if(Utils.isT5Platform()){
            mPhoneCallView = new PhoneCallViewEx(mContext, mWindowManager, mAudioManager);
        }else {
            mPhoneCallView = new PhoneCallView(mContext, mWindowManager, mAudioManager);
        }
        //mcc153不使用mini通话框
        if (SkinUtils.getInteger(R.integer.mini_call_view_type) != 0) {
            if(Utils.isT5Platform()) {
                mPhoneCallViewMini = new PhoneCallViewMiniEx(mContext, mWindowManager, mAudioManager);
            }else{
                mPhoneCallViewMini = new PhoneCallViewMini(mContext, mWindowManager, mAudioManager);
            }
        }
        mCurPhoneCallView = mPhoneCallView;
    }

    private void deinit() {
        mContext.unregisterReceiver(mReceiver);
    }

    public String callStateToString(int callState) {
        String state;
        switch (callState) {
            case 0:
                state = "ACTIVE";
                break;
            case 1:
                state = "HELD";
                break;
            case 2:
                state = "DIALING";
                break;
            case 3:
                state = "ALERTING";
                break;
            case 4:
                state = "INCOMING";
                break;
            case 5:
                state = "WAITING";
                break;
            case 6:
                state = "HELD_BY_RESPONSE_AND_HOLD";
                break;
            case 7:
                state = "TERMINATED";
                break;
            default:
                state = String.valueOf(callState);
        }
        return state;
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.i(TAG, "onReceive:" + action);
            if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
                        BluetoothAdapter.STATE_OFF);
                if (state == BluetoothAdapter.STATE_OFF) {
                    onEndCall();
                }
            } else if (action.equals(BluetoothHeadsetClient.ACTION_CALL_CHANGED)) {
                handleCallStateChanged(intent);
            } else if (action.equals(BluetoothHeadsetClient.ACTION_AUDIO_STATE_CHANGED)) {
                mAudioState = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothHeadsetClient.STATE_AUDIO_DISCONNECTED);
                Log.d(TAG, "onReceive: ACTION_AUDIO_STATE_CHANGED " + mAudioState);
                mCurPhoneCallView.updateAudioState(mAudioState);
            } else if (action
                    .equals(BluetoothHeadsetClient.ACTION_CONNECTION_STATE_CHANGED)) {
                mConnectedDevice = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothProfile.STATE_DISCONNECTED);

                if (state == BluetoothProfile.STATE_CONNECTED) {
                    cancelAutoAnswer();
                    mLastCall = "";
                } else if (state == BluetoothProfile.STATE_DISCONNECTED) {
                    onEndCall();
                }
            } else if (action.equals(BluetoothHeadsetClient.ACTION_AG_EVENT)) {
                int batteryLevel = intent.getIntExtra(BluetoothHeadsetClient.EXTRA_BATTERY_LEVEL,
                        -1);
                int signal = intent.getIntExtra(
                        BluetoothHeadsetClient.EXTRA_NETWORK_SIGNAL_STRENGTH, -1);
                int netStatus = intent.getIntExtra(
                        BluetoothHeadsetClient.EXTRA_NETWORK_STATUS, -1);
                int netRoaming = intent.getIntExtra(
                        BluetoothHeadsetClient.EXTRA_NETWORK_ROAMING, -1);
                String operatorName = intent.getStringExtra(
                        BluetoothHeadsetClient.EXTRA_OPERATOR_NAME);
                int voiceState = intent.getIntExtra(
                        BluetoothHeadsetClient.EXTRA_VOICE_RECOGNITION, 0);
                StringBuilder str = new StringBuilder();
                str.append("voiceState=").append(voiceState);
                if (-1 != batteryLevel) {
                    str.append(" battery=").append(batteryLevel);
                }
                if (-1 != signal) {
                    str.append(" signalLevel=").append(signal);
                }
                if (-1 != netStatus) {
                    str.append(" netStatus=").append(netStatus);
                }
                if (-1 != netRoaming) {
                    str.append(" netRoaming=").append(netRoaming);
                }
                if (!TextUtils.isEmpty(operatorName)) {
                    str.append(" operatorName=").append(operatorName);
                }

                Log.d(TAG, "ACTION_AG_EVENT: " + str.toString());
            } else if (action.equals(BluetoothHeadsetClient.ACTION_RESULT)) {
                int result = intent.getIntExtra(
                        BluetoothHeadsetClient.EXTRA_RESULT_CODE,
                        BluetoothHeadsetClient.ACTION_RESULT_ERROR);
                Log.d(TAG, "result =" + result);
                if (result != BluetoothHeadsetClient.ACTION_RESULT_OK) {
                    Log.e(TAG,
                            "Can't call out,please check input number or callhistory");
                }
            } else if (action.equals(BTACTION)) {
                KeyEvent keyEvent = intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
                Log.d(TAG, "onReceive BT KEY: " + keyEvent.getKeyCode() + ", action:"
                        + keyEvent.getAction() + ",long press = " + keyEvent.isLongPress());
                if ((keyEvent.getKeyCode() == KeyEvent.KEYCODE_CALL) && (keyEvent.getAction()
                        == KeyEvent.ACTION_DOWN)) {
                    List<BluetoothHeadsetClientCall> calls = getCurrentCalls();
                    if (null != calls && !calls.isEmpty()) {
                        if (!getCall(calls, CALL_STATE_INCOMING,CALL_STATE_WAITING).isEmpty()) {
                            acceptCall(BluetoothHeadsetClient.CALL_ACCEPT_NONE);
                            return;
                        }
                        //通话状态不响应BT按键
                        return;
                    }
                    //倒车状态不响应按键
                    if (isReverseState) {
                        Log.d(TAG, "isReverse!!");
                        return;
                    }
                    Utils.startBtClient(mContext, "swc_key_bt");
                } else if ((keyEvent.getKeyCode() == KeyEvent.KEYCODE_ENDCALL) && (
                        keyEvent.getAction() == KeyEvent.ACTION_DOWN)) {
                    //接听和挂断间隔800ms，避免自动接听和语音拒接同时执行时，hfp_enable执行不完导致audio崩溃
                    long interval = System.currentTimeMillis() - mAcceptCallTime;
                    if (interval < HANGUP_INTERVAL) {
                        Log.d(TAG, "delay " + (HANGUP_INTERVAL - interval) + "ms to hangup");
                        mHandler.removeMessages(MSG_HANGUP_CALL);
                        mHandler.sendEmptyMessageDelayed(MSG_HANGUP_CALL,
                                HANGUP_INTERVAL - interval);
                    } else {
                        mHandler.removeMessages(MSG_HANGUP_CALL);
                        mHandler.sendEmptyMessage(MSG_HANGUP_CALL);
                    }
                }
            } else if (action.equals(ACTION_NOTIFICATION_CALL_CALL_NUMBER)) {
                String number = intent.getStringExtra(EXTRA_NUMBER);
                if (!TextUtils.isEmpty(number)) {
                    dial(number);
                }
            } else if (action.equals(ACTION_NOTIFICATION_CALL_ACCEPT_NONE)) {
                acceptCall(BluetoothHeadsetClient.CALL_ACCEPT_NONE);
            } else if (action.equals(ACTION_NOTIFICATION_CALL_REJECT)) {
                //接听和挂断间隔800ms，避免自动接听和语音拒接同时执行时，hfp_enable执行不完导致audio崩溃
                long interval = System.currentTimeMillis() - mAcceptCallTime;
                if (interval < HANGUP_INTERVAL) {
                    Log.d(TAG, "delay " + (HANGUP_INTERVAL - interval) + "ms to hangup");
                    mHandler.removeMessages(MSG_HANGUP_CALL);
                    mHandler.sendEmptyMessageDelayed(MSG_HANGUP_CALL,
                            HANGUP_INTERVAL - interval);
                } else {
                    mHandler.removeMessages(MSG_HANGUP_CALL);
                    mHandler.sendEmptyMessage(MSG_HANGUP_CALL);
                }
            } else if (action.equals(ACTION_REVSTATUS)) {
                isReverseState = intent.getBooleanExtra(EXTRA_REVSTATUS, false);
                if (mCurPhoneCallView.isAddView()) {
                    updateCallView();
                }
            } else if (action.equals(ACTION_UPDATE_BT_CALL_VIEW)
                || action.equals(Intent.ACTION_CLOSE_SYSTEM_DIALOGS)) {
                if (mCurPhoneCallView.isAddView() && Utils.isT5Platform()) {
                    mHandler.postDelayed(()->{updateCallView();},300);
                }
            } else if (action.equals(ACTION_ENABLE_BT_CALL_VIEW)) {
                String packageName = intent.getStringExtra(EXTRA_PACKAGE);
                boolean call_view_enable = intent.getBooleanExtra(EXTRA_ENABLE, true);
                Log.d(TAG, "onReceive: ACTION_ENABLE_BT_CALL_VIEW enable=" + call_view_enable
                        + " package=" + packageName);
                if (null != mPhoneCallView) {
                    mPhoneCallView.setEnable(call_view_enable);
                }
                if (null != mPhoneCallViewMini) {
                    mPhoneCallViewMini.setEnable(call_view_enable);
                }
            } else if (action.equals(ACTION_CONFIGURATION_CHANGED)) {
                Log.d(TAG, "onReceive: ACTION_CONFIGURATION_CHANGED");

                if (mScreenOrientation != mContext.getResources().getConfiguration().orientation) {
                    mScreenOrientation = mContext.getResources().getConfiguration().orientation;
                    mPhoneCallView = null;
                    mPhoneCallViewMini = null;
                    renewPhoneCallView();
                }
            } else if (action.equals(ACTION_GLOBAL_BUTTON)) {
                KeyEvent keyEvent = (KeyEvent) intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
                if (null == keyEvent) {
                    Log.e(TAG, "onReceive: ACTION_GLOBAL_BUTTON keyEvent is null");
                    return;
                }
                if (keyEvent.getAction() == KeyEvent.ACTION_DOWN) {
                    switch (keyEvent.getKeyCode()) {
                        case McuConstant.K_HFP_AUDIO:
                            switchAudio();
                            break;
                        case McuConstant.K_MICMUTE:
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    };

    private final BluetoothProfile.ServiceListener mProfileServiceListener =
            new BluetoothProfile.ServiceListener() {
                @Override
                public void onServiceConnected(int profile, BluetoothProfile proxy) {
                    Log.d(TAG, "onServiceConnected :" + proxy);
                    mHeadsetClient = (BluetoothHeadsetClient) proxy;
                }

                @Override
                public void onServiceDisconnected(int profile) {
                    Log.d(TAG, "mHeadsetClient Disconnected");
                }
            };

    private void handleCallStateChanged(Intent intent) {
        BluetoothHeadsetClientCall call = intent.getParcelableExtra(
                BluetoothHeadsetClient.EXTRA_CALL);
        if (call == null) {
            Log.e(TAG, "CallStateChanged get call fall!");
            return;
        }
        int callState = call.getState();
        String number = call.getNumber();
        Log.i(TAG, "number = " + number + " state = " + callStateToString(callState));

        List<BluetoothHeadsetClientCall> callList = getCurrentCalls();
        if (callList == null || callList.isEmpty()) {
            Log.i(TAG, "handleCallStateChanged no calls hideView!!!");
            onEndCall();
            return;
        }

        if (!mCurPhoneCallView.isAddView()) {
            //显示通话框前选择使用哪个通话框
            if (isShowMiniCallView()) {
                mCurPhoneCallView = mPhoneCallViewMini;
                Log.d(TAG, "handleCallStateChanged: select CallViewMini.");
            } else {
                mCurPhoneCallView = mPhoneCallView;
                Log.d(TAG, "handleCallStateChanged: select CallView.");
            }
        }
        mCurPhoneCallView.onActionCallStateChanged(callList);

        switch (callState) {
            case CALL_STATE_ACTIVE:
                //接听后通话时间开始计时
                onStartCall();
                if(mCallTimeMap.containsKey(number)){
                    mCallTimeMap.get(number).setCallState(callState);
                }else{
                    mCallTimeMap.put(number,new CallInfo(number,callState));
                }
                startCallTime();
                cancelAutoAnswer();
                break;
            case CALL_STATE_DIALING:
            case CALL_STATE_ALERTING:
            case CALL_STATE_WAITING:
                onStartCall();
                break;
            case CALL_STATE_INCOMING:
                onStartCall();
                startAutoAnswer();
                break;
            case CALL_STATE_TERMINATED:
                cancelAutoAnswer();
                if(mCallTimeMap.containsKey(number)){
                    mCallTimeMap.remove(number);
                }
                break;
            case CALL_STATE_HELD:
            case CALL_STATE_HELD_BY_RESPONSE_AND_HOLD:
                if(mCallTimeMap.containsKey(number)){
                    mCallTimeMap.get(number).setCallState(callState);
                }
                break;
            default:
                break;
        }
    }

    public static final int MSG_AUTO_ANSWER = 0x01;
    public static final int MSG_UPDATE_TIME = 0x02;
    public static final int MSG_SWITCH_AUDIO = 0x03;
    public static final int MSG_HANGUP_CALL = 0x04;
    public static final int MSG_FILTER_DIAL = 0x05;

    @SuppressLint("HandlerLeak")
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Log.e(TAG, "handleMessage:" + msg.what);
            switch (msg.what) {
                case MSG_AUTO_ANSWER:
                    acceptCall(BluetoothHeadsetClient.CALL_ACCEPT_NONE);
                    break;
                case MSG_UPDATE_TIME:
                    for (Map.Entry<String, CallInfo> entry:mCallTimeMap.entrySet()) {
                        entry.getValue().CallTimeAdd();
                    }
                    if (null != mCurPhoneCallView) {
                        mCurPhoneCallView.updateCallTime(mCallTimeMap);
                    }
                    sendMessageDelayed(obtainMessage(MSG_UPDATE_TIME), INTERVAL_TIME);
                    break;
                case MSG_SWITCH_AUDIO:
                    break;
                case MSG_HANGUP_CALL:
                    hangup();
                    break;
                case MSG_FILTER_DIAL:
                    //不需处理，此消息只用来过滤连续快速拨号
                    break;
                default:
                    break;
            }
            super.handleMessage(msg);
        }
    };

    /**
     * 开始通话计时
     */
    public void startCallTime() {
        if (isCallTimeBegin) {
            return;
        }
        Log.d(TAG, "startCallTime: ");
        isCallTimeBegin = true;
        if (null != mCurPhoneCallView) {
            mCurPhoneCallView.updateCallTime(mCallTimeMap);
        }
        Message msg = Message.obtain(mHandler, MSG_UPDATE_TIME);
        mHandler.sendMessageDelayed(msg, INTERVAL_TIME);
    }

    /**
     * 结束通话计时
     */
    public void stopCallTime() {
        if (!isCallTimeBegin) {
            return;
        }
        Log.d(TAG, "stopCallTime: ");
        mCallTimeMap.clear();
        mHandler.removeMessages(MSG_UPDATE_TIME);
        isCallTimeBegin = false;
    }

    /**
     * 响应通话开始
     */
    private void onStartCall() {
        Log.d(TAG, "onStartCall: ");
        if (!mCurPhoneCallView.isAddView()) {
            Log.d(TAG, "onStartCall: showView");
            mCurPhoneCallView.showView();
            //通话中停止搜索，避免影响通话声音
            BluetoothAdapterManagerService.getInstance().stopDiscovery();
        }
    }

    /**
     * 响应通话结束
     */
    private void onEndCall() {
        Log.d(TAG, "onEndCall: ");
        if (mCurPhoneCallView.isAddView()) {
            Log.d(TAG, "onEndCall: hideView");
            mCurPhoneCallView.hideView();
            stopCallTime();
            if (mAudioManager.isMicrophoneMute()) {
                mAudioManager.setMicrophoneMute(false);
            }
        }
    }

    /**
     * 更新通话框形态
     */
    private void updateCallView() {
        if (isShowMiniCallView()) {
            if (mCurPhoneCallView instanceof PhoneCallViewMini
                || mCurPhoneCallView instanceof PhoneCallViewMiniEx) {
                Log.d(TAG, "updateCallView: already mini.");
            } else {
                Log.d(TAG, "updateCallView: turning to mini.");
                mPhoneCallViewMini.updateCallTime(mCallTimeMap);
                mPhoneCallViewMini.updateAudioState(mAudioState);
                List<BluetoothHeadsetClientCall> calls = getCurrentCalls();
                mPhoneCallViewMini.onActionCallStateChanged(calls);
                mCurPhoneCallView.hideView();
                if (null != calls && !calls.isEmpty()) {
                    mCurPhoneCallView = mPhoneCallViewMini;
                    mCurPhoneCallView.showView();
                }
            }
        } else {
            if (mCurPhoneCallView instanceof PhoneCallView
                || mCurPhoneCallView instanceof PhoneCallViewEx) {
                Log.d(TAG, "updateCallView: already normal.");
            } else {
                Log.d(TAG, "updateCallView: turning to normal.");
                mPhoneCallView.updateCallTime(mCallTimeMap);
                mPhoneCallView.updateAudioState(mAudioState);
                List<BluetoothHeadsetClientCall> calls = getCurrentCalls();
                mPhoneCallView.onActionCallStateChanged(calls);
                mCurPhoneCallView.hideView();
                if (null != calls && !calls.isEmpty()) {
                    mCurPhoneCallView = mPhoneCallView;
                    mCurPhoneCallView.showView();
                }
            }
        }
    }

    private boolean isShowMiniCallView() {
        if (null == mPhoneCallViewMini) {
            return false;
        }
        if (mSelectCallViewByManual == CALL_VIEW_NORMAL) {
            mSelectCallViewByManual = CALL_VIEW_INVALIDATE;
            return false;
        } else if (mSelectCallViewByManual == CALL_VIEW_MINI) {
            mSelectCallViewByManual = CALL_VIEW_INVALIDATE;
            return true;
        }
        if(Utils.isT5Platform()){
            ActivityManager am = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
            try {
                ComponentName cn = am.getRunningTasks(1).get(0).topActivity;
                //Log.d(TAG, "isShowMiniCallView: pkg: " + cn.getPackageName());
                //需求只在主页显示小框(空调界面是弹框，切没有Activity,所以在主页时还要看下空调显示的情况，如空调显示需要显示大框)
                if ("com.android.launcher3".equals(cn.getPackageName())
                        && Utils.getSystemProperty("persist.sys.air.show_state","0").equals("0")) {
                    return true;
                }
            } catch (Exception e) {

            }
            return false;
        }
        if (isReverseState || Utils.isNaviTopRunning(mContext)) {
            return true;
        } else {
            ActivityManager am = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
            try {
                ComponentName cn = am.getRunningTasks(1).get(0).topActivity;
                if (NEED_MINI_CALL_VIEW_ACTIVITY.contains(cn.getClassName())) {
                    return true;
                }
            } catch (Exception e) {

            }
            return false;
        }
    }

    /**
     * 手动切换通话框，仅对本次通话有效
     * @param type
     * CALL_VIEW_NORMAL 切换到正常通话框
     * CALL_VIEW_MINI 切换到小通话框
     */
    public void switchCallViewByManual(int type){
        if (null!=mCurPhoneCallView && mCurPhoneCallView.isAddView()) {
            mSelectCallViewByManual=type;
            updateCallView();
        }
    }

    /**
     * 拨打电话
     *
     * @param number
     */
    public synchronized void dial(String number) {
        Log.i(TAG, "dial:" + number);
        if (null == mHeadsetClient) {
            Log.e(TAG, "dial failed! mHeadsetClient null!!");
            return;
        }
        if (mHeadsetClient.getConnectedDevices().isEmpty()) {
            Log.e(TAG, "dial: getConnectedDevices empty!!");
            return;
        }
        if (TextUtils.isEmpty(number)) {
            Log.e(TAG, "dial failed! number is empty!!");
            return;
        }
        List<BluetoothHeadsetClientCall> callList = getCurrentCalls();
        if (callList != null && callList.size() >= 0x02) {
            Log.e(TAG, "dial failed! too many calls!!");
            return;
        }
        for (BluetoothHeadsetClientCall call : callList) {
            if (call.getState() == CALL_STATE_INCOMING) {
                Log.e(TAG, "dial failed! incoming state!!");
                return;
            }
        }
        //通过MSG_FILTER_DIAL控制600ms拨号一次，避免连续快速拨号
        if (mHandler.hasMessages(MSG_FILTER_DIAL)) {
            Log.e(TAG, "filter fast dial operation！！！");
            return;
        }
        mHandler.sendEmptyMessageDelayed(MSG_FILTER_DIAL, 600);

        mLastCall = number;
        mHeadsetClient.dial(mConnectedDevice, number);
        Log.d(TAG, "dial:" + number + " successful!!!");
    }

    public synchronized void sendDTMF(byte code) {
        Log.i(TAG, "sendDTMF:" + code);
        if (null != mHeadsetClient) {
            mHeadsetClient.sendDTMF(mConnectedDevice, code);
        }
    }

    /**
     * 接听电话
     *
     * @param flag CALL_ACCEPT_NONE CALL_ACCEPT_HOLD CALL_ACCEPT_TERMINATE
     */
    public synchronized void acceptCall(int flag) {
        Log.i(TAG, "acceptCall");
        List<BluetoothHeadsetClientCall> callList = getCurrentCalls();
        if (callList == null || callList.size() == 0) {
            Log.e(TAG, "no call!");
            return;
        }

        if (null != mHeadsetClient) {
            Log.i(TAG, "acceptCall flag=" + flag);
            mHeadsetClient.acceptCall(mConnectedDevice, flag);
            mAcceptCallTime=System.currentTimeMillis();
        }
    }

    /**
     * 挂电话或拒接
     */
    public synchronized void hangup() {
        List<BluetoothHeadsetClientCall> callList = getCurrentCalls();
        if (callList == null || callList.size() == 0) {
            Log.e(TAG, "hangup no call!");
            return;
        }

        List<BluetoothHeadsetClientCall> calls = getCall(callList,
                CALL_STATE_INCOMING,
                CALL_STATE_WAITING);
        if (!calls.isEmpty()) {
            Log.d(TAG, "hangup rejectCall: " + calls.get(0).getNumber());
            mHeadsetClient.rejectCall(mConnectedDevice);
        } else {
            calls = getCall(callList,
                    CALL_STATE_DIALING,
                    CALL_STATE_ALERTING,
                    CALL_STATE_ACTIVE);

            if (!calls.isEmpty()) {
                Log.d(TAG, "hangup terminateCall: " + calls.get(0).getNumber());
                mHeadsetClient.terminateCall(mConnectedDevice, calls.get(0));
            } else {
                Log.d(TAG, "hangup rejectCall");
                mHeadsetClient.rejectCall(mConnectedDevice);
            }
        }
    }

    /**
     * 获取当前连接设备通话列表
     *
     * @return
     */
    public synchronized List<BluetoothHeadsetClientCall> getCurrentCalls() {
        if (null == mHeadsetClient) {
            return null;
        }
        List<BluetoothDevice> deviceList = mHeadsetClient.getConnectedDevices();
        if (deviceList == null || deviceList.size() == 0) {
            Log.e(TAG, "hf client is not connected!");
            return null;
        }

        List<BluetoothHeadsetClientCall> callList = mHeadsetClient.getCurrentCalls(
                deviceList.get(0));
        return callList;
    }

    /**
     * 根据通话状态获取通话列表
     *
     * @param callList
     * @param states
     * @return
     */
    public static synchronized List<BluetoothHeadsetClientCall> getCall(
            List<BluetoothHeadsetClientCall> callList, int... states) {
        List<BluetoothHeadsetClientCall> calls = new ArrayList<>();
        for (BluetoothHeadsetClientCall c : callList) {
            for (int s : states) {
                if (c.getState() == s) {
                    calls.add(c);
                }
            }
        }

        return calls;
    }

    /**
     * 通话音频是否连接
     *
     * @return
     */
    public synchronized boolean isAudioConnected() {
        Log.d(TAG, "isAudioConnected AudioState=" + mAudioState);
        return mAudioState == BluetoothHeadsetClient.STATE_AUDIO_CONNECTED;
    }

    /**
     * 获取通话音频状态
     *
     * @return STATE_AUDIO_CONNECTED STATE_AUDIO_DISCONNECTED STATE_AUDIO_CONNECTING
     */
    public int getAudioState() {
        return mAudioState;
    }

    /**
     * 通话音频在手机和车机端切换
     */
    public synchronized void switchAudio() {
        //通过MSG_SWITCH_AUDIO控制600ms切换一次
        if (mHandler.hasMessages(MSG_SWITCH_AUDIO)) {
            return;
        }
        mHandler.sendEmptyMessageDelayed(MSG_SWITCH_AUDIO, 600);
        Log.i(TAG, "switchAudio mAudioState="+mAudioState);
        if (mAudioState == BluetoothHeadsetClient.STATE_AUDIO_DISCONNECTED) {
            if (null != mHeadsetClient) {
                if (mHeadsetClient.connectAudio(mConnectedDevice)) {
                    Log.i(TAG, "connectAudio Success!");
                } else {
                    Log.i(TAG, "connectAudio Failed!");
                }
            }
        } else if (mAudioState == BluetoothHeadsetClient.STATE_AUDIO_CONNECTED) {
            if (null != mHeadsetClient) {
                if (mHeadsetClient.disconnectAudio(mConnectedDevice)) {
                    Log.i(TAG, "disconnectAudio Success!");
                } else {
                    Log.i(TAG, "disconnectAudio Failed!");
                }
            }
        }
    }

    public synchronized String getLastCall() {
        Log.i(TAG, "getLastCall:" + mLastCall);
        return mLastCall;
    }

    /**
     * 启动自动接听计时
     */
    private void startAutoAnswer() {
        if (BluetoothAdapterManagerService.getInstance().isBluetoothAutoAnswer()
                && !mHandler.hasMessages(MSG_AUTO_ANSWER)) {
            Log.i(TAG, "start auto answer,after 5s will auto answer!");
            mHandler.sendEmptyMessageDelayed(MSG_AUTO_ANSWER, AUTO_ANSWER_DELAY);
        }
    }

    /**
     * 取消自动接听计时
     */
    private void cancelAutoAnswer() {
        Log.i(TAG, "cancelAutoAnswer!");
        mHandler.removeMessages(MSG_AUTO_ANSWER);
    }

    private final class HfpBinder extends IBluetoothHfpclientService.Stub {

        @Override
        public boolean isAudioConnected() {
            return BluetoothHfpclientService.this.isAudioConnected();
        }

        @Override
        public void dial(String number) {
            BluetoothHfpclientService.this.dial(number);
        }

        @Override
        public void sendDTMF(byte code) {
            BluetoothHfpclientService.this.sendDTMF(code);
        }

        @Override
        public void acceptCall(int flag) {
            BluetoothHfpclientService.this.acceptCall(flag);
        }

        @Override
        public void hangup() {
            BluetoothHfpclientService.this.hangup();
        }

        @Override
        public void switchAudio() {
            BluetoothHfpclientService.this.switchAudio();
        }

        @Override
        public String getLastCall() {
            return BluetoothHfpclientService.this.getLastCall();
        }
    }


    public void onAAConenctStateChange(boolean isconnected){
        Log.d(TAG, "onAAConenctStateChange isconnected=" + isconnected);
        if(isconnected){
            Log.d(TAG, "onAAConenctStateChange  connected g");
            if (mCurPhoneCallView!=null && mCurPhoneCallView.isAddView()) {
                Log.d(TAG, "onAAConenctStateChange  connected isAddView hideView");
                mCurPhoneCallView.hideView();
            }
        }else {
            Log.d(TAG, "onAAConenctStateChange  DISCONNECTED");

        }
    }




}
