package com.hcn.media.adapter.simple;

import androidx.annotation.NonNull;

import com.hcn.mediaservice.data.MusicInfo;

/**
 * Simple List 事件监听接口
 * <p> 用于 {@link  SimpleRvAdapter 反馈事件到其所有者}
 *
 * @author 65821
 */
public interface ISimpleRvListener {

    /**
     * 列表选项点击事件
     *
     * @param info 被点击的文件选项信息
     * @param position 选项在列表中的位置信息
     */
    void onItemClick(MusicInfo info, int position);

    /**
     * 列表选项长按事件
     *
     * @param info 被长按的文件选项信息
     * @param position 选项在列表中的位置信息
     */
    void onItemLongClick(MusicInfo info, int position);

    /**
     * 列表选项删除事件
     *
     * @param info 被删除的文件选项信息
     * @param position 选项在列表中的位置信息
     */
    void onItemDelete(MusicInfo info, int position);

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
