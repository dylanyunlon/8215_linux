package com.hcn.media.music;

/**
 * 触摸事件监听器
 * <p> 主要用来监听指定列表是否发生了触摸动作，以此来判定列表活跃状态；
 *
 * @Author youwj
 * @Create 2021/7/28 19:48
 */
public interface ITouchEventListener {
    /**
     * 触摸事件分发拦截
     * <p> 一般用来做全屏和非全屏显示切换，已经超时全屏处理；
     */
    void onTouchTrigger();
}
