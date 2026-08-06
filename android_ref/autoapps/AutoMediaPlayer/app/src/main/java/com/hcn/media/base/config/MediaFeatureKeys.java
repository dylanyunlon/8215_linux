package com.hcn.media.base.config;

import com.hcn.skinx_config.HFeatureKeys;

/**
 * 多媒体功能开关管理字符串
 *
 * @author : zj970
 * @since : 2025/4/11
 */
public interface MediaFeatureKeys extends HFeatureKeys {

    /**
     * 是否支持延迟加载 侧边栏， 默认为 false
     * <p> true 支持，false 不支持 </p>
     * <p> 通过皮肤包进行配置 </p>
     * <p> 用于优化可视化启动速度、按需加载 </p>
     */
    String SUPPORT_DELAY_DRAWER_LAYOUT = "support_delay_drawer_layout";

}
