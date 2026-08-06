package com.hcn.media_model.base;

import android.media.audiofx.Visualizer;

/**
 * 对 Ui 部分开放的音频接口
 * @author 65821
 */
public interface IMediaAudio {
    /**
     * 获取音频视觉均衡器
     * @return {@link Visualizer}
     */
    Visualizer getVisualizer();

    /**
     * 获取当前媒体音频会话 ID
     * @return audio session ID.
     */
    int getAudioSessionId();
}
