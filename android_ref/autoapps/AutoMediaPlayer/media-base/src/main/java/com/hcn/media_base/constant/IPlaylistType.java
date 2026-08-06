package com.hcn.media_base.constant;

import androidx.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * 定义播放列表类型
 * @author 65821
 */
@Retention(RetentionPolicy.SOURCE)
@IntDef({IPlaylistType.DEVICE_LIST, IPlaylistType.FOLDER_LIST, IPlaylistType.FAVORITE_LIST})

public @interface IPlaylistType {
    /** 存储设备播放列表 **/
    int DEVICE_LIST = 0;

    /** 文件夹播放列表 **/
    int FOLDER_LIST = 1;

    /** 收藏夹播放列表 **/
    int FAVORITE_LIST = 2;
}
