package com.hcn.media.adapter.base;

public interface IRvDecoration {
    /**
     * 测试装饰类型
     * <p> 测试使用，无实际意义；
     */
    int TEST_RV_ITEM_LIST_TYPE = -1;

    /**
     * 壁纸列表装饰类型
     * <p> 用来显示并管理壁纸列表序列的项类对象；
     */
    int RV_WALLPAPER_ITEM_TYPE = 2024;

    /**
     * 最多回收的视图数量
     * <p> 设置合理的值可以提高内存回收和显示效率；
     */
    int LIST_MAX_RECYCLED_VIEWS = 12;
}
