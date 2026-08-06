package com.hcn.media;

import java.util.List;
import com.hcn.mediaservice.data.MusicInfo;

interface IMediaPlayerService {
	boolean getSDState();
	boolean getUSBState();
	boolean getSD2State();

    boolean targetStorageMounted(String path);

	boolean isRemoteConnected();
	void requestExitApp(int reason);
	boolean inAccON();

	void onRequestAudioFocus();
	void doShouldPlayEvent();
	void doShouldPauseEvent(boolean stop, int reason);

	boolean isCanWatchVideo();
	boolean existsHighPriorityEvent();
	boolean isCanPlayVideo();

	void requestSwitchMediaType();

	void requestSeekToTime(int time);
	void requestPlayDataSource(in MusicInfo info);

	void requestPlayMusiclist(int playlistType, in List<MusicInfo> infoList, int position);
	void requestPlayVideolist(int playlistType, in List<MusicInfo> infoList, int position);
    boolean requestUpdateMusicPlaylist(int playlistType, in List<MusicInfo> infoList);

	void requestPlayControl(int nCommand);

	void switchMusicRepeatMode();
	void switchVideoRepeatMode();

	void requestScanTargetPath(String filePath);

	int readMediaTime(int type, String path);
	void writeMediaTime(int type, String path, int nTime, int reason);

	void dispatchMusicEvent(int eventId, String strParam, int nParam);

	void registerMediaButton();
	int readVideoScaleType();
	void writeVideoScaleType(int type);
}
