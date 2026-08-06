package com.hcn.media_base;

/**
 * 播放器状态
 * @author 65821
 */
public interface IState {
    int IDLE = 0;
    int INITIALIZED = 1;
    int PREPARING = 2;
    int PREPARED = 3;
    int STARTED = 4;
    int PAUSED = 5;
    int STOPPED = 6;
    int PLAYBACK_COMPLETED = 7;
    int END = 8;
    int ERROR = 9;
}
