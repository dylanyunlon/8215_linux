package com.hcn.media_dummy.view.base;

/**
 * 播放器状态
 *
 * @author 65821
 */
public interface FunMediaState {
    /** 正常 */
    public static final int CURRENT_STATE_NORMAL = 0;

    /** 准备中 */
    public static final int CURRENT_STATE_PREPAREING = 1;

    /** 播放中 */
    public static final int CURRENT_STATE_PLAYING = 2;

    /** 开始缓冲 */
    public static final int CURRENT_STATE_PLAYING_BUFFERING_START = 3;

    /** 暂停 */
    public static final int CURRENT_STATE_PAUSE = 5;

    /** 自动播放结束 */
    public static final int CURRENT_STATE_AUTO_COMPLETE = 6;

    /** 错误状态 */
    public static final int CURRENT_STATE_ERROR = 7;
}
