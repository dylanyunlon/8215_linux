package com.hcn.auto;

import android.carstatus.CarStatus;

/**
 * 车载系统状态类
 *
 * @author 65821
 */
public class AutoStatus {
    private AutoStatus() {
        throw new RuntimeException(
                "The class ‘AutoStatus’ Prohibit instantiation.");
    }

    /**
     * [当前设备状态接口对象]
     * <p> 设备节点 /dev/carstatus 可以查看状态；
     */
    private static final CarStatus S_CAR_STATUS = new CarStatus();

    /**
     * 是否在倒车状态
     * @return {@link boolean}
     */
    public static boolean isReversing() {
        return S_CAR_STATUS.getReversing() > 0;
    }

    /**
     * 当前 ACC 状态，实时的查询接口；
     * <p> 不依赖 ACC 状态广播;
     * @return 当前系统 ACC 状态，是/否；
     */
    public static boolean isRealtimeAccON() {
        return S_CAR_STATUS.getAccStatus() > 0;
    }
}
