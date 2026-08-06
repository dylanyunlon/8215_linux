package com.autochips.bluetooth.manager;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import com.autochips.bluetooth.util.Constants;
import com.hcn.bluetooth.api.ConnectionListener;
import com.hcn.bluetooth.api.LocalBTMusicManager;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;
import com.hcn.bluetooth.api.Utils;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

public class HStateBroadReceiver {
    private final String TAG = "_BT_HStateBroadReceiver";
    private final int STATE_CONNECT = 1;
    private final int STATE_DISCONNECT = 2;
    private final int STATE_POWER = 3;
    /**
     * 数据变化(通话记录)
     */
    private final int STATE_DATA_CHANGE= 4;
    /**
     * 通话状态改变
     * BluetoothHeadsetClient.ACTION_CALL_CHANGED
     */
    private final String ACTION_CALL_CHANGED = "android.bluetooth.headsetclient.profile.action.AG_CALL_CHANGED";
    /**
     * BluetoothHeadsetClient.EXTRA_CALL
     */
    public static final String EXTRA_CALL = "android.bluetooth.headsetclient.extra.CALL";

    /**
     * acl
     */
    public static final String ACTION_ACL_DISCONNECTED = "android.bluetooth.device.action.ACL_DISCONNECTED";
    /**
     * 屏保显示状态通知（KEY_DISPLAY: show/hide）
     */
    public static final String ACTION_SCREENSAVER_SHOW_HIDE = "action.screensaver.show_hide";
    public static final String KEY_DISPLAY = "KEY_DISPLAY";



    private Context mContext;
    private HBluetoothManager mBluetoothManager = null;
    private LocalBTMusicManager mMusicManager = null;
    private HashMap<String, BtStateCallback> mCallbacks = new HashMap<>();

    private boolean isCalling = false;
    /**
     * TODO 屏保 start
     */
    //屏保前状态
    private boolean mLastPlayState = false;
    //是否屏保
    private boolean bHasScreenSaver = false;
    /**
     * TODO 屏保 end
     */

    /**
     * connect check
     */
    private boolean bConnectCheck = false;
    private String sDeviceAddr = "";
    private StateHandler mStateHandler;

    public interface BtStateCallback {
        void callbackConnect();

        void callbackDisconnect();

        void callbackPower();

        void callbackChange();

        String callbackTag();
    }

    private void logd(String msg) {
        Log.d(TAG, "" + msg);
    }

    private void logw(String msg) {
        Log.w(TAG, "" + msg);
    }

    public HStateBroadReceiver(Context context, HBluetoothManager manager) {
        mContext = context;
        mBluetoothManager = manager;
        mMusicManager = mBluetoothManager.getMusicService();
        mStateHandler = new StateHandler();
        registerReceivers();
    }

    public void unInit() {
        mContext.unregisterReceiver(mReceiver);
        mCallbacks.clear();
        mCallbacks = null;
        mContext = null;
    }

    public void addCallback(BtStateCallback callback) {
        if (callback != null) {
            String tag = callback.callbackTag();
            logd("addCallback:tag =   " + tag);
            if (TextUtils.isEmpty(tag)) {
                tag = callback.getClass().getSimpleName();
            }
            if (mCallbacks.containsKey(tag)) {
                mCallbacks.remove(tag);
            }
            mCallbacks.put(tag, callback);
        }
    }

    public boolean isCalling(){
        logd("isCalling : " + isCalling);
        return isCalling;
    }

    public void notifyCallStateToService(){
        if(mContext != null) {
            mContext.sendBroadcast(new Intent(Constants.ACTION_BT_UI_CHANGE));
        }
    }

    public void removeCallback(BtStateCallback callback) {
        if (callback != null) {
            String tag = callback.callbackTag();
            if (TextUtils.isEmpty(tag)) {
                tag = callback.getClass().getSimpleName();
            }
            if (mCallbacks.containsKey(tag)) {
                mCallbacks.remove(tag);
            }
        }
    }

    public void notifyDataChange(){
        callListener(STATE_DATA_CHANGE);
    }

    private void callListener(int id) {
        callListener(id, -1);
    }

    private void callListener(int id, int value) {
        try {
            logd("callListener " + id + " , " + mCallbacks.size());
            Set<Map.Entry<String, BtStateCallback>> set = mCallbacks.entrySet();
            Iterator<Map.Entry<String, BtStateCallback>> iterator = set.iterator();
            BtStateCallback callback = null;
            while (iterator.hasNext()) {
                callback = iterator.next().getValue();
                if (callback != null) {
                    switch (id) {
                        case STATE_CONNECT:
                            callback.callbackConnect();
                            break;
                        case STATE_DISCONNECT:
                            callback.callbackDisconnect();
                            break;
                        case STATE_POWER:
                            callback.callbackPower();
                            break;
                        case STATE_DATA_CHANGE:
                            callback.callbackChange();
                            break;
                        default:
                            break;
                    }
                }
            }
        }catch (Exception e){
            e.printStackTrace();
        }
    }
    /**
     * show : 屏保显示状态
     */
    private void updatePlayState(boolean show){
        if(mBluetoothManager.isA2dpConnected()) {
            if(show){
                mLastPlayState = mBluetoothManager.isPlay();
                if(mLastPlayState){
                    mBluetoothManager.musicPause();
                }
            }else{
                if(mLastPlayState){
                    mBluetoothManager.musicPlay();
                }
            }
            bHasScreenSaver = show;
            logd("updatePlayState : show:" + show + " , " + mLastPlayState);
        }
    }

    public boolean isScreenSaverState(){
        return bHasScreenSaver;
    }

    /**
     * 通话结束后更新通讯录
     */
    private void updateRecord() {
        mBluetoothManager.startRecordDownLoad();
    }

    /**
     * 更新蓝牙名称
     * T5Chery的名字是CHERY_XXXX
     */
    private void setName() {
        //String name = Utils.getSystemProperty(Constants.BT_NAME_PROP, null);
        //if (!TextUtils.isEmpty(name) && name.startsWith("CHERY_")) {
        //    return;
        //}
        String code = Utils.getSystemProperty(Constants.DEVICE_MACHINECODE,"");
        if(TextUtils.isEmpty(code)){
            code = Utils.getSystemProperty(Constants.DEVICE_SERIAL,"");
            if(TextUtils.isEmpty(code)){
                return;
            }
        }
        if(code.length() > 4){
            code = code.trim();
            String subname = code.substring(code.length() -4,code.length());
            String rename = "CHERY_"+subname;
            String name = Utils.getSystemProperty(Constants.BT_NAME_PROP, null);
            logd("lastname = " + name + ",  rename = " + rename);
            if(!TextUtils.isEmpty(name) && rename.equals(name)){
                logd("name is same ,so return!");
                return;
            }
            mBluetoothManager.setBluetoothName(rename);
        }
    }

    private void registerReceivers() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(ACTION_CALL_CHANGED);
        filter.addAction(ACTION_ACL_DISCONNECTED);
        filter.addAction(ACTION_SCREENSAVER_SHOW_HIDE);
        filter.addAction(LocalBluetoothAdapterManager.ACTION_CONNECTION_STATE_CHANGED);
        mContext.registerReceiver(mReceiver, filter);
    }

    /**
     * 针对当前已连接一个设备，然后去连另外一个时失败的情况。
     * 检测连接是否OK，连接前需要先断开上一个
     * 有连接时出现proxy繁忙的情况，导致连接失败又回连上一个设备。
     * 增加一次复联操作。
     *
     * 实际第一次连接之前会开启自动连接检测 20秒的时间计时。
     * 所以在第一次连接发起的20s内如果失败就在连一次。
     *
     *
     * 复连流程：
     * 1.先等5秒，看是否已经断开，5s内没断开就取消
     * 2.5秒内断开了，那就再等8s看当前的设备是不是能连上。
     * 3.如8秒后设备还没连上，则主动再去连一次
     *
     * @param address
     */
    public void checkDeviceConnect(String address){
        //反复操作连接过掉，取消计时逻辑。
        if(bConnectCheck
                && !TextUtils.isEmpty(sDeviceAddr)){
            if(!sDeviceAddr.equals(address)){
                logd("[checkDeviceConnect] remove check, use connect more than once by user!");
                if(mStateHandler.hasMessages(StateHandler.MSG_CHECK_CONNECT)) {
                    mStateHandler.removeMessages(StateHandler.MSG_CHECK_CONNECT);
                }
                if(mStateHandler.hasMessages(StateHandler.MSG_REQUEST_CONNECT)) {
                    mStateHandler.removeMessages(StateHandler.MSG_REQUEST_CONNECT);
                }
            return;
            }
        }

        bConnectCheck = true;
        sDeviceAddr = address;
        if(mStateHandler != null){
            if(mStateHandler.hasMessages(StateHandler.MSG_CHECK_CONNECT)) {
                mStateHandler.removeMessages(StateHandler.MSG_CHECK_CONNECT);
            }
            mStateHandler.sendEmptyMessageDelayed(StateHandler.MSG_CHECK_CONNECT,5000);
        }

    }

    private class StateHandler extends Handler{
        public static final int MSG_CHECK_CONNECT = 1;
        public static final int MSG_REQUEST_CONNECT = 2;
        @Override
        public void handleMessage(@NonNull Message msg) {
            logd("handleMessage what: "+ msg.what);
            switch(msg.what){
                case MSG_CHECK_CONNECT:
                    bConnectCheck = false;
                    break;
                case MSG_REQUEST_CONNECT:
                    if(!mBluetoothManager.isBluetoothConnected()){
                        logd("handleMessage reconnect : "+ sDeviceAddr);
                        mBluetoothManager.connectDevice(sDeviceAddr,false);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logd("onReceive: action :" + action);
            if (action.equals(LocalBluetoothAdapterManager.ACTION_CONNECTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothProfile.STATE_DISCONNECTED);
                logd("state1: " + state);
                if (state == BluetoothProfile.STATE_CONNECTED) {
                    callListener(STATE_CONNECT);
                    if(mStateHandler.hasMessages(StateHandler.MSG_REQUEST_CONNECT)){
                        mStateHandler.removeMessages(StateHandler.MSG_REQUEST_CONNECT);
                    }
                } else if (state == BluetoothProfile.STATE_DISCONNECTED) {
                    callListener(STATE_DISCONNECT);
                    if(mStateHandler.hasMessages(StateHandler.MSG_REQUEST_CONNECT)){
                        mStateHandler.removeMessages(StateHandler.MSG_REQUEST_CONNECT);
                    }
                }
            } else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
                        BluetoothAdapter.STATE_OFF);
                logd("state2: " + state);
                if (state == BluetoothAdapter.STATE_OFF) {
                    callListener(STATE_DISCONNECT);
                    callListener(STATE_POWER);
                } else if (state == BluetoothAdapter.STATE_ON) {
                    callListener(STATE_POWER);
                }
            } else if (action.equals(ACTION_CALL_CHANGED)) {
                Object obj = intent.getParcelableExtra(EXTRA_CALL);
                String info = obj.toString();
                logd("(onReceive:)->ACTION_CALL_CHANGED " + info);
                //通话结束
                if (info.contains("mState: TERMINATED")) {
                    updateRecord();
                    isCalling = false;
                }else{
                    isCalling = true;
                }
            } else if(ACTION_ACL_DISCONNECTED.equals(action)){
                if(bConnectCheck){
                    mStateHandler.sendEmptyMessageDelayed(StateHandler.MSG_REQUEST_CONNECT,8000);
                }
            } else if(ACTION_SCREENSAVER_SHOW_HIDE.equals(action)){
                String key = intent.getStringExtra(KEY_DISPLAY);
                updatePlayState(key.equals("show"));
            }
        }
    };

    //蓝牙服务绑定状态
    public ConnectionListener mBluetoothConnectListener = new ConnectionListener() {
        @Override
        public void onServiceConnected() {
            logd("onServiceConnected");

            setName();
            /**
             * 服务绑定要后于界面显示，所以一般在显示UI后蓝牙句柄是还没拿到的
             * 需要等拿到后在做一次更新。
             * 主要是settings中
             */
            callListener(STATE_POWER);
            if (mBluetoothManager != null) {
                mBluetoothManager.registerCallback();
            }
        }

        @Override
        public void onServiceDisconnected() {
            logd("onServiceDisconnected");
        }
    };

    /**
     * 蓝牙音乐服务状态。
     */
    public ConnectionListener mMusicConnectListener = new ConnectionListener() {

        @Override
        public void onServiceConnected() {
            if (mMusicManager != null) {
                //ui挂掉会导致服务解绑焦点，然后卡死
                //mMusicManager.regMusicClientBinder(new Binder());
                //不能立即做，需要播放的时候请求，不然同步操作会把service卡死
                //mMusicManager.requestA2dp();
                mBluetoothManager.registerMusicCallback();
            }
        }

        @Override
        public void onServiceDisconnected() {

        }
    };
}
