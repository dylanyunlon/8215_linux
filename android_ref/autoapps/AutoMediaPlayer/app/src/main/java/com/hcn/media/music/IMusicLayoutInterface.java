package com.hcn.media.music;

import com.hcn.media_base.impl.MediaEventPostbox;

/**
 * 媒体视图交互接口
 * <p> 历史遗留，建议后续全部替换成 ViewModel 来交互;
 *
 * @author 65821
 */
public interface IMusicLayoutInterface {
    /**
     * 初始化布局
     * <pre>
     *    历史接口，兼容使用；
     *    函数名称取的有点问题，初始化布局一般都在构造中搞定了；
     * </pre>
     */
    void initLayout();


    /**
     * 初始化数据对象
     * <p> 初始化布局视图的关键数据对象；
     */
    void initDataObject();

    /**
     * 事件传递接口
     * <pre>
     *    历史接口，兼容使用；
     *    一般用于其父对象 Fragment 传递事件给当前 layout 对象；
     * </pre>
     *
     * @param eventID 事件 ID
     * @deprecated 可以考虑使用观察者模式；
     */
    @Deprecated
    void doCallbackEvent(int eventID);

    /**
     * 设置回调对象接口
     * <pre>
     *    历史接口，兼容使用；
     *    一般用于自定义 layout 对象和其父对象通讯使用；
     * </pre>
     *
     * @param listener 媒体事件监听对象
     * @deprecated 可以考虑使用观察者模式；
     */
    @Deprecated
    void setMediaEventListener(MediaEventPostbox listener);
}
