package com.hcn.media_data.base;

import com.hcn.mediaservice.data.MusicInfo;

import java.util.List;

/**
 * 媒体信息打包数据结构
 * <p> 应用到专辑和艺术家列表（Album、Artist）;
 *
 * @author 65821
 */
public class MusicKeyInfo {
    public String mKey;
    public List<MusicInfo> mInfoList;

    public MusicKeyInfo(String key, List<MusicInfo> list) {
        mKey = key;
        mInfoList = list;
    }
}
