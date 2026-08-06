package com.hcn.media_base.activity;

/**
 * Resume 相关定义
 * <p> 主要是常量定义设计
 *
 * @author 65821
 */
public interface IResume {

    /**
     * 当前在 onResume 执行前的状态
     * <pre>
     *  onResume 时窗口还未显示出来；
     *  callActivityOnResume
     *      onResume {
     *          onPreResume();
     *          super.onResume();
     *      }
     *  函数 onPreResume 可以参考 BaseMediaActivity 类设计。
     * </pre>
     */
    String PRE_RESUME_STATE = "preResume";

    /**
     * 当前在 onPostResume 执行状态
     * <pre>
     *  由于 onTopResumedActivityChanged 方法在 Q 版本开始才有；
     *  onPostResume 更具有阶段代表性；
     * </pre>
     */
    String POST_RESUME_STATE = "postResume";

    /**
     * 当前在 onTopResumedActivityChanged(true) 状态
     * <pre>
     *  函数执行调用时序
     *  performResumeActivity
     *      performResume
     *          performRestart
     *          callActivityOnResume
     *              onResume
     *          onPostResume
     *      reportTopResumedActivityChanged
     *          performTopResumedActivityChanged
     *              onTopResumedActivityChanged
     * </pre>
     */
    String TOP_RESUMED_STATE = "topResumed";
}
