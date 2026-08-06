package com.hcn.media_base.impl;

import androidx.annotation.NonNull;

import com.hcn.media_base.IMediaEventListener;
import com.hcn.media_base.IMediaEvent;

/**
 * 媒体事件监听扩展
 * <p> 抽象类扩展，需要继承实现；
 *
 * @author 65821
 */
public abstract class MediaEventPostbox implements IMediaEventListener {
    /**
     * 媒体事件
     * <p> {@link IMediaEvent}
     *
     * @param eventId 事件 ID
     * @param wParam  附加参数 1
     * @param lParam  附加阐述 2
     */
    @Override
    public abstract void onMediaEvent(int eventId, Object wParam, Object lParam);

    /**
     * 媒体活动
     * <p> 活动一般比较少，可自己定义名称；
     *
     * @param action 活动名称（一般为函数名称）
     * @param wParam 附加参数 1
     * @param lParam 附加参数 2
     */
    public abstract void onMediaAction(@NonNull final String action, Object wParam, Object lParam);
}
