package com.hcn.media.base;

import java.util.List;

public interface IExternalControlCallback {

    /** 播放列表回调 **/
    void onPlaylistUpdated(List<String> pathList);

    /** 播放模式回调 **/
    void onPlayModeUpdated(int playMode);

    /** 当前媒体路径回调 **/
    void onCurrentPathUpdated(String path);

    /** 当前媒体播放位置回调 **/
    void onCurrentPositionUpdated(int position);

    /** 当前媒体收藏状态回调 **/
    void onFavoriteStateUpdated(int isFavorite);
}
