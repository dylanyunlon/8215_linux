package com.hcn.media_base.fragment;

/**
 * 音乐界面 Fragment 定义
 *
 * @author 86158
 */
public interface IMusicPage {
    int E_GROUP_SHOW_NULL = -1;

    int E_GROUP_SHOW_LOADING = 0;

    int E_GROUP_SHOW_MUSIC_INFO = 1;

    /**
     * 未使用到
     * @deprecated
     */
    int E_GROUP_SHOW_MUSIC_LIST = 2;

    int E_GROUP_SHOW_MUSIC_SEARCH = 3;

    int E_GROUP_SHOW_MUSIC_FILE_ITEM = 4;

    /**
     * 未使用到
     * @deprecated
     */
    int E_GROUP_SHOW_FILE_MUSIC_SEARCH = 5;

    int MCC201_E_GROUP_SHOW_MUSIC_INFO = 6;

    int MCC201_E_GROUP_SHOW_MUSIC_LIST = 7;

    int E_GROUP_SHOW_MUSIC_INFO_EX = 8;

    int E_GROUP_SHOW_MUSIC_LIST_EX = 9;

    int E_GROUP_SHOW_MUSIC_SEARCH_EX = 10;

    int MCC204_E_GROUP_SHOW_MUSIC_INFO = 11;

    int MCC204_E_GROUP_SHOW_MUSIC_LIST = 12;
}
