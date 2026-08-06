/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.hcn.bluetooth.protocol;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothPan;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.os.SystemProperties;
import android.util.Log;


import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;

/**
 * PanProfile handles Bluetooth PAN profile (NAP and PANU).
 */
public final class PanProfile implements LocalBluetoothProfile {
    private static final String TAG = "PanProfile";
    private static boolean V = true;

    private BluetoothPan mService;
    private boolean mIsProfileReady;

    // Tethering direction for each device
    private final HashMap<BluetoothDevice, Integer> mDeviceRoleMap =
            new HashMap<BluetoothDevice, Integer>();

    public static final String NAME = "PAN";

    // Order of this profile in device profiles list
    private static final int ORDINAL = 4;

    /**
     * 通知systemUI显示或隐藏蓝牙网络图标,包含一个参数{@link EXTRA_STATE},类型boolean
     */
    private static final String PAN_STATE_CHANGED_ACTION = "com.hcn.bluetooth.pan.state.changed";
    private static final String EXTRA_STATE = "extra_state";
    private static final String KEY_AUTO_CONNECT_NET = "persist.sys.AutoConnectBtNet";
    private Context mContext = null;
    private boolean mAutoConnectable = SystemProperties.getBoolean(KEY_AUTO_CONNECT_NET, false);
    private ConnectivityManager cm = null;

    // These callbacks run on the main thread.
    private final class PanServiceListener
            implements BluetoothProfile.ServiceListener {

        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            if (V) Log.d(TAG,"Bluetooth service connected");
            mService = (BluetoothPan) proxy;
            mIsProfileReady=true;
        }

        public void onServiceDisconnected(int profile) {
            if (V) Log.d(TAG,"Bluetooth service disconnected");
            mIsProfileReady=false;
        }
    }

    public boolean isProfileReady() {
        return mIsProfileReady;
    }

    PanProfile(Context context) {
        mContext = context;
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        adapter.getProfileProxy(context, new PanServiceListener(),
                BluetoothProfile.PAN);
        cm = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
    }

    public boolean isConnectable() {
        return true;
    }

    public boolean isAutoConnectable() {
        return mAutoConnectable;
    }

    public boolean connect(BluetoothDevice device) {
        if (mService == null) return false;
        disconnect();
        return mService.connect(device);
    }

    public boolean disconnect(BluetoothDevice device) {
        if (mService == null) return false;
        return mService.disconnect(device);
    }

    public boolean disconnect() {
        if (mService == null) return false;

        List<BluetoothDevice> devices = mService.getConnectedDevices();
        if (devices != null) {
            for (BluetoothDevice device : devices) {
                disconnect(device);
            }
        }
        return true;
    }

    @Override
    public int getProfileId() {
        return BluetoothProfile.PAN;
    }

    public int getConnectionStatus(BluetoothDevice device) {
        if (mService == null) {
            return BluetoothProfile.STATE_DISCONNECTED;
        }
        return mService.getConnectionState(device);
    }

    public List<BluetoothDevice> getConnectedDevices() {
        if (mService == null) {
            return new ArrayList<BluetoothDevice>(0);
        }
        return mService.getConnectedDevices();
    }

    public boolean isPreferred(BluetoothDevice device) {
        return true;
    }

    public int getPreferred(BluetoothDevice device) {
        return -1;
    }

    public void setPreferred(BluetoothDevice device, boolean preferred) {
        // ignore: isPreferred is always true for PAN
    }

    public String toString() {
        return NAME;
    }

    public int getOrdinal() {
        return ORDINAL;
    }

    // Tethering direction determines UI strings.
    void setLocalRole(BluetoothDevice device, int role) {
        mDeviceRoleMap.put(device, role);
    }

    boolean isLocalRoleNap(BluetoothDevice device) {
        if (mDeviceRoleMap.containsKey(device)) {
            Log.d(TAG, "isLocalRoleMap is " + mDeviceRoleMap.get(device));
            return mDeviceRoleMap.get(device) == BluetoothPan.LOCAL_NAP_ROLE;
        } else {
            return false;
        }
    }

    protected void finalize() {
        if (V) Log.d(TAG, "finalize()");
        if (mService != null) {
            try {
                BluetoothAdapter.getDefaultAdapter().closeProfileProxy(BluetoothProfile.PAN, mService);
                mService = null;
            }catch (Throwable t) {
                Log.w(TAG, "Error cleaning up PAN proxy", t);
            }
        }
    }

    /**
     * 通知状态栏更新蓝牙网络图标显示及隐藏状态
     */
    public void notifySystemUINetworkState(boolean state) {
        Intent it = new Intent(PAN_STATE_CHANGED_ACTION);
        it.putExtra(EXTRA_STATE, state);
        if (null != mContext) {
            mContext.sendBroadcast(it);
        }
    }

    public void saveAutoConnectState(boolean state) {
        if(mAutoConnectable!=state){
            mAutoConnectable=state;
            SystemProperties.set(KEY_AUTO_CONNECT_NET, String.valueOf(mAutoConnectable));
        }
    }

    public List<BluetoothDevice> getDevicesMatchingConnectionStates(int[] states) {
        if (mService == null) return new ArrayList<BluetoothDevice>(0);
        return mService.getDevicesMatchingConnectionStates(states);
    }

    public boolean isBluetoothNetworkError() {
        if (mContext == null) {
            return false;
        }
        if (null == cm) {
            Log.d(TAG, "connectivityManager is null!");
            return false;
        }

        Network network = cm.getActiveNetwork();
        if (null != network) {
            NetworkCapabilities capabilities = cm.getNetworkCapabilities(network);
            if (capabilities.hasTransport(NetworkCapabilities.TRANSPORT_BLUETOOTH)) {
                boolean bErrorAddress = isValidateHostAddress();
                boolean isValidated = capabilities.hasCapability(
                        NetworkCapabilities.NET_CAPABILITY_VALIDATED);
                if (!isValidated || !bErrorAddress) {
                    Log.e(TAG, "bt_pan Address=" + bErrorAddress + " Capability=" + isValidated);
                    return true;
                }
            }
        } else {
            Log.e(TAG, "bt_pan network null!!");
            return false;
        }
        return false;
    }

    private static boolean isValidateHostAddress() {
        try {
            Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces();
            // scan all network interface
            while (en.hasMoreElements()) {
                // 得到指定名字的网络接口绑定的所有ip-one
                NetworkInterface nif = en.nextElement();

                if (nif.getName().equals("bt-pan")) {
                    Enumeration<InetAddress> inet = nif.getInetAddresses();
                    // scan all ip of one network interface
                    while (inet.hasMoreElements()) {
                        InetAddress inetAddress = inet.nextElement();
                        if (!inetAddress.isLoopbackAddress() && (inetAddress instanceof Inet4Address
                                || inetAddress instanceof Inet6Address)) {
                            String hostAddress = inetAddress.getHostAddress();

                            if (inetAddress instanceof Inet4Address
                                    && hostAddress != null
                                    && !"".equals(hostAddress)
                                    && !hostAddress.equals("00.00.00.00")) {
                                return true;
                            }

                            if (inetAddress instanceof Inet6Address
                                    && hostAddress != null
                                    && !"".equals(hostAddress)
                                    && !hostAddress.toLowerCase().startsWith("fe80")) {
                                return true;
                            }
                        }
                    }
                }
            }
        } catch (SocketException e) {
            Log.e(TAG, "SocketException");
        }
        Log.e(TAG, "Host Address error!");
        return false;
    }
}
