package com.hcn.media.music.common.simple;

/**
 * Simple List 页面类型
 * <p>
 *
 * @author 65821
 */

import androidx.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Simple List 页面类型
 * @author 65821
 */
@Retention(RetentionPolicy.SOURCE)
@IntDef({ISimpleList.PAGE_NONE,
        ISimpleList.PAGE_PLAYLIST,
        ISimpleList.PAGE_FAVORITE,
        ISimpleList.PAGE_SIZE})

public @interface ISimpleList {
    int PAGE_NONE = -1;

    /** 播放列表页 **/
    int PAGE_PLAYLIST = 0;

    /** 收藏列表页 **/
    int PAGE_FAVORITE = 1;

    int PAGE_SIZE = 2;
}
