package com.hcn.media_dummy.listener;

/**
 * 播放进度监听
 * @author 65821
 */
public interface FunMediaProgressListener {
    /**
     * 播放进度回调
     * @param progress 当前播放进度（暂停后再播放可能会有跳动）
     * @param secProgress 当前内存缓冲进度（可能会有 0 值）
     * @param currentPosition 当前播放位置（暂停后再播放可能会有跳动）
     * @param duration 总时长
     */
    void onProgress(long progress, long secProgress, long currentPosition, long duration);
}
