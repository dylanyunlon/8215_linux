package com.hcn.media_dummy.base.player;

import com.hcn.media_dummy.base.model.FunModel;

import tv.danmaku.ijk.media.player.IMediaPlayer;

/**
 * 播放器初始化监听
 * @author 65821
 */
public interface IPlayerInitListener {
    /**
     * 初始化成功
     *
     * @param player 播放组件
     * @param model 播放器所需的初始化内容
     */
    void onPlayerInitSuccess(IMediaPlayer player, FunModel model);
}
