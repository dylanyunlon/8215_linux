package com.hcn.bluetooth.service;
 import com.hcn.bluetooth.api.MusicPlayState;

interface IMusicCallback {
	oneway void onA2dpConnectStateChanged(int state);
    oneway void onAvrcpConnectStateChanged(int state);
    oneway void onMetadataChanged(String title,String artist,String album);
    void onPlayStatusChanged(in MusicPlayState state);
}