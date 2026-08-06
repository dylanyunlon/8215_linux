package com.hcn.bluetooth.service;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;

interface IPbapCallback {
	oneway void onPbapDownloadStateChanged(int state,int type);
	oneway void onPbapConnectStateChanged(int state);
}