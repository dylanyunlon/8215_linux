package com.hcn.bluetooth.service;

import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.service.IAdapterCallback;
import com.hcn.bluetooth.service.IPbapCallback;

interface IBluetoothAdapterManagerService {
	void setBluetoothEnable(boolean enable);
	boolean isBluetoothEnable();
	void resetBT();
	String getBTName();
    void setBTName(String name);
    String getBTPincode();
    void setBTPincode(String pincode);
	List<BluetoothDeviceInfo> getDeviceList();
	List<BluetoothDeviceInfo> getBondedDevices();
	BluetoothDeviceInfo getConnectDevice();
	void connectDevice(String address);
	void disconnectDevice(String address);
	boolean isBluetoothConnected();
	void pairDevice(String address);
	void unpairDevice(String address);
    void connectA2dp(String address);
    void disconnectA2dp(String address);
    boolean isA2dpConnected(String address);
	void setBluetoothAutoAnswer(boolean enable);
	boolean isBluetoothAutoAnswer();
	void setBluetoothAutoConnect(boolean enable);
	boolean isBluetoothAutoConnect();
	void setAutoConnectedNetwork(boolean auto);
    boolean isAutoConnectedNetwork();
	void startDiscovery();
	void stopDiscovery();
	boolean isDiscovering();
	boolean isPbapConnected();
	void connectPbap();
	void disConnectPbap();
	boolean pbapStartDownLoad(int type);
	boolean getPbapDownLoadState(int type);
	void registerPbapCallback(IPbapCallback callback);
    void unregisterPbapCallback(IPbapCallback callback);
	void registerCallback(IAdapterCallback callback);
    void unregisterCallback(IAdapterCallback callback);
    int getThirdPartAPPMode();
}
