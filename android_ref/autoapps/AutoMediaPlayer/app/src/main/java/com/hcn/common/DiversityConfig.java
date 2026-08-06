package com.hcn.common;

import com.hcn.common.utils.HUtilsEx;

public class DiversityConfig {
    // 文件夹记忆模式
    public static final String MODE_FOLDER = "1";

    public static String getListMemoryMode() {
        return HUtilsEx.getSystemProperty("ro.media.diversity_config", "0");
    }
}
