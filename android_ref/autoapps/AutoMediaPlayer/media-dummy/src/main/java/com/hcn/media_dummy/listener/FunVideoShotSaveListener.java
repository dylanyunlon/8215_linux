package com.hcn.media_dummy.listener;

import java.io.File;

/**
 * 截屏保存结果
 * @author 65821
 */
public interface FunVideoShotSaveListener {
    /**
     * 截屏结果
     *
     * @param success 结果
     * @param file 保存文件
     */
    void result(boolean success, File file);
}