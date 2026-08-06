package com.hcn.media.adapter.base;

import androidx.annotation.NonNull;

import com.hcn.auto_compat.app.Wallpaper;

/**
 *
 */
public interface IRvListener {
    /**
     * 列表选项点击事件
     *
     * @param info 被点击的壁纸选项信息
     * @param position 选项在列表中的位置信息
     */
    void onItemClick(Wallpaper.Info info, int position);

    /**
     * 通用事件接口
     * <pre>
     *    因为场景事件不多，所以事件直接采用 String 做描述标签；
     *    所以对于 Adapter 传递出来的事件名字定义，可以随意发挥；
     * </pre>
     *
     * @param event 事件名称
     * @param obj1 附加参数 1
     * @param obj2 附加参数 2
     */
    void onRvAdapterEvent(@NonNull final String event, Object obj1, Object obj2);
}
