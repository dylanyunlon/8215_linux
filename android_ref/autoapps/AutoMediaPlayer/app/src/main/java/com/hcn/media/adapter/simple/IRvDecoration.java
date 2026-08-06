package com.hcn.media.adapter.simple;

/**
 * RecyclerView 装饰类型定义
 * <p> 当存在复杂的装饰列表时，它就显得很重要了；
 *
 * @author 65821
 */
public interface IRvDecoration {
    /**
     * 测试装饰类型
     * <p> 测试使用，无实际意义；
     */
    int TEST_RV_ITEM_LIST_TYPE = -1;

    /**
     * 通用的媒体装饰类型
     * <pre>
     *    一般格式：
     *      icon | media file name | state
     *    最大 Item 缓存个数至少需要为一屏的 3 倍；
     * </pre>
     */
    int SIMPLE_RV_ITEM_LIST_TYPE = 2023;
    int SIMPLE_LIST_MAX_RECYCLED_VIEWS = 20;
}
