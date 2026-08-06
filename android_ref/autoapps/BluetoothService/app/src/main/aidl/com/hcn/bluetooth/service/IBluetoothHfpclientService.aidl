package com.hcn.bluetooth.service;

interface IBluetoothHfpclientService {
	boolean isAudioConnected();
	void dial(String number);
	void sendDTMF(byte code);
	void acceptCall(int flag);
	void hangup();
	void switchAudio();
	String getLastCall();
}