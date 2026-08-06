package com.hcn.media_base.fragment;

/**
 * 可以用于父 Fragment 和子 Fragment 之间传递事件
 * <pre>
 *     采用 ViewMode 对接 Fragment 之间的页面事件；
 *     它不是为 setCallback 设计的，所以不要把它用到 setCallback(listener) 这种方式上；
 * </pre>
 *
 * @author 65821
 */
public interface IPageEventListener {

    /**
     * 处理页面回调事件
     *
     * @param event 事件类型
     * @param obj1 附加参数对象 1
     * @param obj2 附加参数对象 2
     */
    void onPageEvent(int event, Object obj1, Object obj2);
}
