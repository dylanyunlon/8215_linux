package com.hcn.bluetooth.api;

import java.util.ArrayList;
import java.util.List;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import com.hcn.bluetooth.service.IBluetoothAdapterManagerService;
import com.hcn.bluetooth.service.IAdapterCallback;
import com.hcn.bluetooth.service.IPbapCallback;

public class LocalBluetoothAdapterManager {

    public static final String TAG = "LocalAdapterManager";
    /**
     * 连接状态广播 include extras：BluetoothProfile.EXTRA_STATE
     */
    public static final String ACTION_CONNECTION_STATE_CHANGED =
            "android.bluetooth.headsetclient.profile.action.CONNECTION_STATE_CHANGED";

    //---------------------PBAP start-----------------
    //must be the same as define in vendor\mediatek\proprietary\packages\apps\Bluetooth\src\com
    // \android\bluetooth\pbapclient\PbapClientStateMachine.java
    //电话本下载状态
    public static final int STATE_DOWNLOADED = 0;
    public static final int STATE_DOWNLOADING = 1;
    public static final int STATE_FAILED_NO_CONNECT = 2;//PBAP未连接
    public static final int STATE_FAILED_UNKNOWN = 3;

    //下载类型
    public static final int PARAM_DOWNLOAD_ALL = 1;
    public static final int PARAM_DOWNLOAD_PB = 2;
    public static final int PARAM_DOWNLOAD_MCH = 3;
    public static final int PARAM_DOWNLOAD_OCH = 5;
    public static final int PARAM_DOWNLOAD_MIOCH = 6;
    //---------------------PBAP end-----------------
    /**
     * 部分第三方app需要与蓝牙互斥,例如Carplay使用中应禁用蓝牙，否则可能导致播放音乐无声音
     */
    public static final int THIRD_PART_NORMAL_MODE = 0;
    public static final int THIRD_PART_ZJ_CARPLAY_MODE = 1;
    public static final int THIRD_PART_LETTER_HICAR_MODE = 2;

    private static LocalBluetoothAdapterManager sInstance;
    private Context mContext;
    private boolean isBound = false;
    private IBluetoothAdapterManagerService mService = null;
    private List<ConnectionListener> mListeners = new ArrayList<>();

    public static synchronized LocalBluetoothAdapterManager getInstance() {
        if (sInstance == null) {
            sInstance = new LocalBluetoothAdapterManager();
        }
        return sInstance;
    }

    private LocalBluetoothAdapterManager() {
        Log.i(TAG, "LocalBluetoothAdapterManager Create");
    }

    public LocalBluetoothAdapterManager init(Context context) {
        Log.i(TAG, "init");
        if (null == mContext) {
            mContext = context.getApplicationContext();
        }
        if (!isBound) {
            Intent intent = new Intent(
                    "com.hcn.bluetooth.service.BluetoothAdapterManagerService");
            intent.setClassName("com.hcn.bluetoothservice",
                    "com.hcn.bluetooth.service.BluetoothAdapterManagerService");
            mContext.bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        }
        return sInstance;
    }

    public void unInit() {
        Log.i(TAG, "unInit");
        if (isBound) {
            mContext.unbindService(mConnection);
        }
        isBound = false;
        mContext = null;
    }

    public boolean isReady() {
        return isBound;
    }

    public void addConnectListener(ConnectionListener listener) {
        if (null != listener) {
            mListeners.remove(listener);
            mListeners.add(listener);
        }
    }

    public void removeConnectListener(ConnectionListener listener) {
        if (null != listener) {
            mListeners.remove(listener);
        }
    }

    public void callConnectListener(boolean connect) {
        for (int i = 0; i < mListeners.size(); i++) {
            if (connect) {
                mListeners.get(i).onServiceConnected();
            } else {
                mListeners.get(i).onServiceDisconnected();
            }
        }
    }

    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "onServiceDisconnected");
            isBound = false;
            mService = null;
            callConnectListener(false);
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "onServiceConnected");
            mService = IBluetoothAdapterManagerService.Stub
                    .asInterface(service);
            isBound = true;
            callConnectListener(true);
        }
    };

    public synchronized List<BluetoothDeviceInfo> getDeviceList() {
        Log.i(TAG, "getDeviceList");
        if (isBound) {
            try {
                return mService.getDeviceList();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return null;
    }

    public synchronized List<BluetoothDeviceInfo> getBondedDevices() {
        if (isBound) {
            try {
                return mService.getBondedDevices();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return null;
    }

    public synchronized BluetoothDeviceInfo getConnectDevice() {
        Log.i(TAG, "getConnectDevice");
        if (isBound) {
            try {
                return mService.getConnectDevice();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return null;
    }

    public synchronized void pairDevice(String address) {
        Log.i(TAG, "pairDevice:" + address);
        if (isBound) {
            try {
                mService.pairDevice(address);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public synchronized void connectDevice(String address) {
        Log.i(TAG, "connectDevice:" + address);
        if (isBound) {
            try {
                mService.connectDevice(address);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public synchronized void disconnectDevice(String address) {
        Log.i(TAG, "disconnectDevice:" + address);
        if (isBound) {
            try {
                mService.disconnectDevice(address);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public synchronized void unpairDevice(String address) {
        Log.i(TAG, "unpairDevice:" + address);
        if (isBound) {
            try {
                mService.unpairDevice(address);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public synchronized boolean isBluetoothEnable() {
        Log.i(TAG, "isBluetoothEnable");
        if (isBound) {
            try {
                return mService.isBluetoothEnable();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public synchronized boolean isBluetoothConnected() {
        Log.i(TAG, "isBluetoothConnected");
        if (isBound) {
            try {
                return mService.isBluetoothConnected();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public synchronized void setBluetoothEnable(boolean enable) {
        Log.i(TAG, "setBluetoothEnable:" + enable);
        if (isBound) {
            try {
                mService.setBluetoothEnable(enable);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public synchronized String getBTName() {
        Log.i(TAG, "getBTName");
        if (isBound) {
            try {
                return mService.getBTName();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return null;
    }

    public synchronized void setBTName(String name) {
        Log.i(TAG, "setBTName");
        if (isBound) {
            try {
                mService.setBTName(name);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public synchronized String getBTPincode() {
        Log.i(TAG, "getBTName");
        if (isBound) {
            try {
                return mService.getBTPincode();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return null;
    }

    public synchronized void setBTPincode(String pincode) {
        if (isBound) {
            try {
                mService.setBTPincode(pincode);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public boolean isBluetoothAutoAnswer() {
        Log.i(TAG, "isBluetoothAutoAnswer");
        if (isBound) {
            try {
                return mService.isBluetoothAutoAnswer();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public void setBluetoothAutoAnswer(boolean enable) {
        Log.i(TAG, "setBluetoothAutoAnswer:" + enable);
        if (isBound) {
            try {
                mService.setBluetoothAutoAnswer(enable);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public boolean isBluetoothAutoConnect() {
        Log.i(TAG, "isBluetoothAutoConnect");
        if (isBound) {
            try {
                return mService.isBluetoothAutoConnect();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public void setBluetoothAutoConnect(boolean enable) {
        Log.i(TAG, "setBluetoothAutoConnect:" + enable);
        if (isBound) {
            try {
                mService.setBluetoothAutoConnect(enable);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public void resetBT() {
        if (isBound) {
            try {
                mService.resetBT();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public void startDiscovery() {
        // TODO Auto-generated method stub
        if (isBound) {
            try {
                mService.startDiscovery();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public void stopDiscovery() {
        // TODO Auto-generated method stub
        if (isBound) {
            try {
                mService.stopDiscovery();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public boolean isDiscovering() {
        // TODO Auto-generated method stub
        if (isBound) {
            try {
                return mService.isDiscovering();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public void setAutoConnectedNetwork(boolean auto) {
        if (isBound) {
            try {
                mService.setAutoConnectedNetwork(auto);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public boolean isAutoConnectedNetwork() {
        if (isBound) {
            try {
                return mService.isAutoConnectedNetwork();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    /**
     * 连接A2dp
     *
     * @param address 设备mac地址
     */
    public void connectA2dp(String address) {
        if (isBound) {
            try {
                mService.connectA2dp(address);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public void disconnectA2dp(String address) {
        if (isBound) {
            try {
                mService.disconnectA2dp(address);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public boolean isA2dpConnected(String address) {
        if (isBound) {
            try {
                return mService.isA2dpConnected(address);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public boolean isPbapConnected() {
        if (isBound) {
            try {
                return mService.isPbapConnected();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public void connectPbap() {
        if (isBound) {
            try {
                mService.connectPbap();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public void disConnectPbap() {
        if (isBound) {
            try {
                mService.disConnectPbap();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public boolean pbapStartDownLoad(int type) {
        if (isBound) {
            try {
                Utils.updateReverseName();
                return mService.pbapStartDownLoad(type);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public boolean getPbapDownLoadState(int type) {
        if (isBound) {
            try {
                return mService.getPbapDownLoadState(type);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public void registerPbapCallback(IPbapCallback callback) {
        if (isBound) {
            try {
                mService.registerPbapCallback(callback);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public void unregisterPbapCallback(IPbapCallback callback) {
        if (isBound) {
            try {
                mService.unregisterPbapCallback(callback);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public void registerCallback(IAdapterCallback callback) {
        if (isBound) {
            try {
                mService.registerCallback(callback);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public void unregisterCallback(IAdapterCallback callback) {
        if (isBound) {
            try {
                mService.unregisterCallback(callback);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public int getThirdPartAPPMode() {
        if (isBound) {
            try {
                return mService.getThirdPartAPPMode();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
        return THIRD_PART_NORMAL_MODE;
    }
}
