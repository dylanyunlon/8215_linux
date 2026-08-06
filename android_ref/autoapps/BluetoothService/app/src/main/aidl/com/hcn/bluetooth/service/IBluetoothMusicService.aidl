package com.hcn.bluetooth.service;
import com.hcn.bluetooth.service.IMusicCallback;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;

interface IBluetoothMusicService {
	boolean isA2dpConnected();
	boolean isAvrcpConnected();
	boolean isA2dpPlaying();
	BluetoothDeviceInfo getConnectDevice();
	void send_Avrcp_Cmd(int avrcp_cmd);
	void requestA2dp();
	void releaseA2dp();
	void regMusicClientBinder(IBinder cBinder);
	void unRegMusicClientBinder();
	void registerBTMusicCallback(IMusicCallback callback);
	void unregisterBTMusicCallback(IMusicCallback callback);
	String[] getID3Info();
}
