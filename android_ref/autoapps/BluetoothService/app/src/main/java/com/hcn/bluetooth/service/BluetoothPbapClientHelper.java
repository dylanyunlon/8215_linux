package com.hcn.bluetooth.service;

import static com.hcn.bluetooth.api.LocalBluetoothAdapterManager.PARAM_DOWNLOAD_MIOCH;
import static com.hcn.bluetooth.api.LocalBluetoothAdapterManager.STATE_DOWNLOADED;
import static com.hcn.bluetooth.api.LocalBluetoothAdapterManager.STATE_DOWNLOADING;
import static com.hcn.bluetooth.api.LocalBluetoothAdapterManager.STATE_FAILED_NO_CONNECT;
import static com.hcn.bluetooth.api.LocalBluetoothAdapterManager.STATE_FAILED_UNKNOWN;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;
import com.hcn.bluetooth.api.Utils;
import com.hcn.bluetooth.protocol.HeadsetClientProfile;
import com.hcn.bluetooth.protocol.LocalBluetoothProfileManager;
import com.hcn.bluetooth.protocol.PbapClientProfile;
import com.hcn.bluetooth.service.IPbapCallback;

import java.util.List;

public class BluetoothPbapClientHelper {
    private static final String TAG = "BluetoothPbapClientHelper";
    //通知Bluetooth.apk去下载
    public static final String ACTION_HCN_BT_DATA_DW = "ACTION_HCN_BT_DATA_DW";
    //Bluetooth.apk发送广播通知下载状态
    public static final String ACTION_SYNC_FINISH = "ACTION_MIOCH_SYNC_FINISH";
    public static final String EXTRA_DW_TYPE = "EXTRA_DW_TYPE";
    //Bluetooth.apk发送广播通知下载失败
    public static final String ACTION_HCN_BT_ERROR = "ACTION_HCN_BT_ERROR";
    public static final String ERROR_CODE = "ERROR_CODE";//错误码
    //ERROR_CODE定义如下
    public static final int ERROR_CODE_DW_PB = 0; //联系人下载失败
    public static final int ERROR_CODE_DW_MIOCH = 1; //通话记录下载失败
    public static final int ERROR_CODE_OBEX_FAILED = 2;//OBEX连接失败

    //通知同行者电话本下载完成
    public static final String ACTION_BLUETOOTH_PB_DOWNLOAD_FINISH =
            "com.autochips.bluetooth.BtUtils.action.ACTION_BLUE_PB_DOWNLOAD_FINISH";

    private Context mContext;
    private LocalBluetoothProfileManager mProfileManager;

    private static final int MSG_START_DOWNLOAD = 0x01;
    private Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_START_DOWNLOAD:
                    Log.d(TAG, "handleMessage: MSG_START_DOWNLOAD type=" + msg.arg1);
                    if (isPbapConnected()) {
                        Intent intent = new Intent(ACTION_HCN_BT_DATA_DW);
                        intent.putExtra(EXTRA_DW_TYPE, msg.arg1);
                        mContext.sendBroadcast(intent);
                    } else {
                        Log.e(TAG, "handleMessage: download failed pbap not connect!!!");
                        //pbap未连接成功，回调下载结束状态
                        mDownloadState = STATE_FAILED_NO_CONNECT;
                        callPbapListener(CALL_PBAP_DOWNLOAD_STATE_CHANGE);
                        mDownloadType = 0;
                    }
                    break;
                default:
                    break;
            }
        }
    };

    private int mPbapConnectState = BluetoothProfile.STATE_DISCONNECTED;
    private int mDownloadType = 0;
    private int mDownloadState = STATE_DOWNLOADED;

    //回调列表
    private RemoteCallbackList<IPbapCallback> mCallBackList;
    //对应IPbapCallback的回调方法
    public static final int CALL_PBAP_DOWNLOAD_STATE_CHANGE = 0;
    public static final int CALL_PBAP_CONNECT_STATE_CHANGE = 1;

    public BluetoothPbapClientHelper(Context context,
            LocalBluetoothProfileManager profileManager) {
        mContext = context;
        mProfileManager = profileManager;
        mCallBackList = new RemoteCallbackList<IPbapCallback>();
    }

    public int getPbapConnectState() {
        return mPbapConnectState;
    }

    public int getDownloadType() {
        return mDownloadType;
    }

    public void setDownloadType(int downloadType) {
        this.mDownloadType = downloadType;
    }

    public int getDownloadState() {
        return mDownloadState;
    }

    public void setDownloadState(int downloadState) {
        this.mDownloadState = downloadState;
    }

    public synchronized boolean pbapStartDownLoad(int dwType) {
        Log.d(TAG, "pbapStartDownLoad dwType : " + dwType);
        if (mDownloadState == STATE_DOWNLOADING) {
            return false;
        }
        mDownloadType = dwType;
        mDownloadState = STATE_DOWNLOADING;
        callPbapListener(CALL_PBAP_DOWNLOAD_STATE_CHANGE);
        int delay;
        if (isPbapConnected()) {
            delay = 0;
        } else {
            //尝试连接，延时1.5s开始下载
            connectPbap();
            delay = 1500;
        }
        Utils.updateReverseName();
        mHandler.removeMessages(MSG_START_DOWNLOAD);
        Message msg = mHandler.obtainMessage(MSG_START_DOWNLOAD);
        msg.arg1 = dwType;
        mHandler.sendMessageDelayed(msg, delay);
        return true;
    }

    public synchronized boolean getPbapDownLoadState(int type) {
        if (mDownloadType == type) {
            return true;
        } else {
            return false;
        }
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(android.content.Context context,
                android.content.Intent intent) {
            String action = intent.getAction();
            if (action.equals(ACTION_SYNC_FINISH)) {
                mDownloadState = STATE_DOWNLOADED;
                mDownloadType = intent.getIntExtra(EXTRA_DW_TYPE, PARAM_DOWNLOAD_MIOCH);
                Log.d(TAG, "ACTION_SYNC_FINISH dwType : " + mDownloadType);
                callPbapListener(CALL_PBAP_DOWNLOAD_STATE_CHANGE);
                if (mDownloadType == LocalBluetoothAdapterManager.PARAM_DOWNLOAD_PB) {
                    //通知同行者语音，电话本下载完成
                    mContext.sendBroadcast(new Intent(ACTION_BLUETOOTH_PB_DOWNLOAD_FINISH));
                }
                //置为0,表示下载结束
                mDownloadType = 0;
            } else if (action.equals(
                    android.bluetooth.BluetoothPbapClient.ACTION_CONNECTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothProfile.STATE_DISCONNECTED);
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                handlePbapConnectStateChange(device, state);
            } else if (action.equals(ACTION_HCN_BT_ERROR)) {
                int error = intent.getIntExtra(ERROR_CODE, ERROR_CODE_DW_PB);
                if (error == ERROR_CODE_DW_PB) {
                    mDownloadType = LocalBluetoothAdapterManager.PARAM_DOWNLOAD_PB;
                    mDownloadState = STATE_FAILED_UNKNOWN;
                } else if (error == ERROR_CODE_DW_MIOCH) {
                    mDownloadType = LocalBluetoothAdapterManager.PARAM_DOWNLOAD_MIOCH;
                    mDownloadState = STATE_FAILED_UNKNOWN;
                }else if(error == ERROR_CODE_OBEX_FAILED){
                    mDownloadState = STATE_FAILED_UNKNOWN;
                }
                callPbapListener(CALL_PBAP_DOWNLOAD_STATE_CHANGE);
                //置为0,表示下载结束
                mDownloadType = 0;
            }
        }
    };

    private void handlePbapConnectStateChange(BluetoothDevice device, int state) {
        mPbapConnectState = state;
        if (state == BluetoothProfile.STATE_DISCONNECTED) {
            Log.d(TAG, "handlePbapConnectStateChange pbap disconnect!");
            if (mDownloadState == STATE_DOWNLOADING) {
                Log.e(TAG, "download failed pbap disconnect!!!");
                //pbap断开连接，回调下载结束状态
                mDownloadState = STATE_DOWNLOADED;
                callPbapListener(CALL_PBAP_DOWNLOAD_STATE_CHANGE);
                mDownloadType = 0;
            }
        } else if (state == BluetoothProfile.STATE_CONNECTING) {

        } else if (state == BluetoothProfile.STATE_CONNECTED) {
            Log.d(TAG, "handlePbapConnectStateChange pbap connect!");
        } else if (state == BluetoothProfile.STATE_DISCONNECTING) {

        }
        callPbapListener(CALL_PBAP_CONNECT_STATE_CHANGE);
    }

    public void handleAdapterStateChange(int state) {
        if (state == BluetoothAdapter.STATE_OFF) {
            if (mDownloadState == STATE_DOWNLOADING) {
                Log.e(TAG, "download finish caused by adapter off!!!");
                //下载中关闭蓝牙，可能无法收到pbap断开连接广播，在此更正客户端状态
                mDownloadState = STATE_DOWNLOADED;
                callPbapListener(CALL_PBAP_DOWNLOAD_STATE_CHANGE);
                mDownloadType = 0;
            }
        }
    }

    public void registerReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_SYNC_FINISH);
        filter.addAction(ACTION_HCN_BT_ERROR);
        filter.addAction(android.bluetooth.BluetoothPbapClient.ACTION_CONNECTION_STATE_CHANGED);
        mContext.registerReceiver(mReceiver, filter);
    }

    public void unRegisterReceiver() {
        try {
            mContext.unregisterReceiver(mReceiver);
        } catch (Exception e) {

        }
    }

    public synchronized boolean isPbapConnected() {
        boolean ret = false;
        PbapClientProfile pbapProfile = mProfileManager.getPbapClientProfile();
        HeadsetClientProfile headsetClientProfile = mProfileManager.getHeadsetClientProfile();
        if (null == headsetClientProfile || null == pbapProfile) {
            Log.e(TAG, "isPbapConnected failed,profile is null!");
            return ret;
        }
        List<BluetoothDevice> devices = headsetClientProfile.getConnectedDevices();
        if (!devices.isEmpty()) {
            ret = pbapProfile.getConnectionStatus(devices.get(0))
                    == BluetoothProfile.STATE_CONNECTED;
        }
        return ret;
    }

    public synchronized void connectPbap() {
        PbapClientProfile pbapProfile = mProfileManager.getPbapClientProfile();
        HeadsetClientProfile headsetClientProfile = mProfileManager.getHeadsetClientProfile();
        if (null == headsetClientProfile || null == pbapProfile) {
            Log.e(TAG, "connectPbap failed,profile null");
            return;
        }
        List<BluetoothDevice> devices = headsetClientProfile.getConnectedDevices();
        if (!devices.isEmpty()) {
            pbapProfile.connect(devices.get(0));
        }
    }

    public synchronized void disConnectPbap() {
        PbapClientProfile pbapProfile = mProfileManager.getPbapClientProfile();
        HeadsetClientProfile headsetClientProfile = mProfileManager.getHeadsetClientProfile();
        if (null == headsetClientProfile || null == pbapProfile) {
            Log.e(TAG, "disConnectPbap failed,profile null");
            return;
        }
        List<BluetoothDevice> devices = headsetClientProfile.getConnectedDevices();
        if (!devices.isEmpty()) {
            pbapProfile.disconnect(devices.get(0));
        }
    }

    public synchronized void registerPbapCallback(IPbapCallback callback) {
        if (null == callback) {
            return;
        }
        mCallBackList.unregister(callback);
        mCallBackList.register(callback);
    }

    public synchronized void unregisterPbapCallback(IPbapCallback callback) {
        if (null == callback) {
            return;
        }
        mCallBackList.unregister(callback);
    }

    public synchronized void callPbapListener(final int method) {
        int count = mCallBackList.beginBroadcast();
        Log.d(TAG, "callPbapListener: count=" + count);
        try {
            for (int i = 0; i < count; i++) {
                IPbapCallback c = mCallBackList.getBroadcastItem(i);
                switch (method) {
                    case CALL_PBAP_DOWNLOAD_STATE_CHANGE:
                        c.onPbapDownloadStateChanged(getDownloadState(), getDownloadType());
                        break;
                    case CALL_PBAP_CONNECT_STATE_CHANGE:
                        c.onPbapConnectStateChanged(mPbapConnectState);
                        break;
                    default:
                        break;
                }
            }
        } catch (RemoteException e) {

        } finally {
            mCallBackList.finishBroadcast();
        }
    }
}
