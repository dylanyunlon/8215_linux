package com.hcn.media_base.fragment;

import com.hcn.media_base.IMediaEvent;

/**
 * Fragment/Activity 相关的页面事件
 * <p> 这里只能定义页面事件，禁止添加其它不相干的常量；
 *
 * @author 65821
 */
public class PageEvent implements IMediaEvent {

    /**
     * 测试事件
     * <p> 无实际意义
     */
    public static final int TEST_EVENT = IMediaEvent.EVENT_DEFINE_LIMIT_THRESHOLD;

    /**
     * 准备创建 Fragment 页面；
     * <p> 某个页面对象构造前，可以触发该事件给订阅者（测试使用）；
     */
    public static final int PREPARE_CREATE_FRAGMENT = TEST_EVENT + 1;

    /**
     * 返回指定目标事件的名字
     * <p> 用错将抛出异常，直接中断程序运行；
     *
     * @param event {@link  PageEvent}
     * @return 页面的字符串描述
     */
    public static String name(final int event) {
        // 先拦截媒体事件
        if (event < IMediaEvent.EVENT_DEFINE_LIMIT_THRESHOLD) {
            return "[MediaEvent = " + event + "]";
        }

        switch (event) {
            case TEST_EVENT:
                return "test_event";
            case PREPARE_CREATE_FRAGMENT:
                return "prepare_create_fragment";
            default:
                break;
        }

        throw new RuntimeException("Invalid page event parameter");
    }
}
