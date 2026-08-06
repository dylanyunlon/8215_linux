package com.hcn.bluetooth.service;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;

interface IAdapterCallback {
	oneway void onBluetoothStateChanged(int state);
	oneway void onDiscoveryStateChanged(int state);
	oneway void onDiscoveryDeviceFound(in BluetoothDeviceInfo device);
	oneway void onDiscoveryDeviceNameChanged(in BluetoothDeviceInfo device);
	oneway void onDeviceBondStateChanged(in BluetoothDeviceInfo device, int bondState);
	oneway void onConnectionStateChanged(in BluetoothDeviceInfo device, int state);
}