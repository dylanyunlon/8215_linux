package com.autochips.bluetooth;

/**
 * 用于Fragment与Activity通讯
 *
 * @Author Simon
 * @Create 2022/12/14
 */
public interface IFragmentCallback {
    /**
     * 更新Activity虚化背景
     * @param update
     * @return
     */
    boolean updateBackground(boolean update);

    /**
     * 清除通话记录列表
     * @return
     */
    boolean clearRecordList();
}
