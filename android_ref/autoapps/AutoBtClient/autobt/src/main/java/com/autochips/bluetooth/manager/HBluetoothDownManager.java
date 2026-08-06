package com.autochips.bluetooth.manager;

import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Log;

import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;
import com.hcn.bluetooth.service.IPbapCallback;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

/**
 *     public static final int STATE_DOWNLOADED = 0;
 *     public static final int STATE_DOWNLOADING = 1;
 *     public static final int STATE_FAILED_NO_CONNECT = 2;
 *     public static final int STATE_FAILED_UNKNOWN = 3;
 *
 *     public static final int PARAM_DOWNLOAD_ALL = 1;
 *     public static final int PARAM_DOWNLOAD_PB = 2;
 *     public static final int PARAM_DOWNLOAD_MCH = 3;
 *     public static final int PARAM_DOWNLOAD_OCH = 5;
 *     public static final int PARAM_DOWNLOAD_MIOCH = 6;
 *
 *     public static final int THIRD_PART_NORMAL_MODE = 0;
 */

public class HBluetoothDownManager {
    private IPbapCallback mLocalCallback;
    private HashMap<String, IPbapCallback> mCallbacks = new HashMap<>();
    private LocalBluetoothAdapterManager mBluetoothManager;
    //
    private int mDownType = 0;//没下载
    private int mDownState = 0;

    public HBluetoothDownManager(LocalBluetoothAdapterManager manager) {
        mBluetoothManager = manager;
        initCallback();
    }

    private void initCallback(){
        mLocalCallback = new IPbapCallback.Stub() {
            @Override
            public void onPbapDownloadStateChanged(int state, int type) throws RemoteException {
               logd("onPbapDownloadStateChanged()->state = " + state + "" + type);
                mDownType = type;
                mDownState = state;
                onPbapDownChange(state,type);
            }

            @Override
            public void onPbapConnectStateChanged(int i) throws RemoteException {

                onPbapConnChange(i);
            }
        };
        if(mBluetoothManager != null){
            mBluetoothManager.registerPbapCallback(mLocalCallback);
        }
    }

    private void onPbapDownChange(int state, int type){
        try {
            Set<Map.Entry<String, IPbapCallback>> set = mCallbacks.entrySet();
            Iterator<Map.Entry<String, IPbapCallback>> iterator = set.iterator();
            IPbapCallback callback = null;
            while (iterator.hasNext()) {
                callback = iterator.next().getValue();
                callback.onPbapDownloadStateChanged(state, type);
            }
        }catch (Exception e){
            e.printStackTrace();
        }
    }

    /**
     * 正在下载中()
     * @return
     */
    public boolean isPbDowning(){
        return mDownState == LocalBluetoothAdapterManager.STATE_DOWNLOADING;
    }

    public boolean isPbDownContact(){
        return mDownType == LocalBluetoothAdapterManager.PARAM_DOWNLOAD_PB;
    }

    /**
     * 下载记录，使用PARAM_DOWNLOAD_MIOCH下载
     * 来电去电未接三类都下载，每个50，实际取前100显示
     * @return
     */
    public boolean isPbDownRecord(){
        return mDownType == LocalBluetoothAdapterManager.PARAM_DOWNLOAD_MIOCH;
    }

    public int getDownState(){
        return mDownState;
    }

    private void onPbapConnChange(int state){
        try {
            Set<Map.Entry<String, IPbapCallback>> set = mCallbacks.entrySet();
            Iterator<Map.Entry<String, IPbapCallback>> iterator = set.iterator();
            IPbapCallback callback = null;
            while (iterator.hasNext()) {
                callback = iterator.next().getValue();
                callback.onPbapConnectStateChanged(state);
            }
        }catch (Exception e){
            e.printStackTrace();
        }
    }

    public void registerPbapCallback(){
        if(mBluetoothManager != null){
            mBluetoothManager.registerPbapCallback(mLocalCallback);
        }
    }

    public void addPbapCallback(String tag,IPbapCallback callback){
        if (callback != null) {
            logd("registerPbapCallback:tag =   " + tag);
            if (TextUtils.isEmpty(tag)) {
                tag = callback.getClass().getSimpleName();
            }
            if (mCallbacks.containsKey(tag)) {
                mCallbacks.remove(tag);
            }
            mCallbacks.put(tag, callback);
        }
    }

    private void logd(String msg){
        Log.d("HBluetoothDownManager"," "+msg);
    }
}
