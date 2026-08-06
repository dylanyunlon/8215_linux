package com.hcn.media_data.storage;

/**
 * 承储设备相关定义
 * @author 65821
 */
public interface IStorageDevice {

    /**
     * 文件路径定义
     * <p> FLASH/USB/SDCARD
     */
    int STORAGE_TYPE_NONE = -1;
    int STORAGE_TYPE_FLASH = 1;
    int STORAGE_TYPE_USB = 2;
    int STORAGE_TYPE_SDCARD = 3;
}
