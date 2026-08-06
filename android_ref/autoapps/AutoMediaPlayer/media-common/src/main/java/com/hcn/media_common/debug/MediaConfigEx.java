package com.hcn.media_common.debug;

import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.os.Debug;

import androidx.annotation.NonNull;

import com.hcn.common.HConfig;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.media_common.utils.MiscUtils;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * 调试工具类
 * <p> 为兼容历史代码，这里调用了 {@link HConfig} 工具;
 *
 * @author 86158
 * @deprecated 过时了，不再维护;
 */
public class MediaConfigEx implements MediaDebug {
    public static final String TAG = MediaConfigEx.class.getSimpleName();

    /**
     * 监控当前进程内存使用
     * PSS(实际使用的物理内存（包含按比例分配共享库占用的内存）)
     */
    public static boolean MEMORY_MONITORING = false;

    interface IProperty {
        public static final String MEMORY_MONITORING_CFG = "persist.sys.mm.mem.m.cfg";
    }

    /**
     * 配置初始化
     * @param context 上下文环境
     */
    public static void init_config(@NonNull Context context) {
        mem_monitor_config();
        config_monitor_init(context);
        HConfig.initConfig(context, MODULE_KEY, MediaDebug.TAG);
    }

    /**
     * 内存监控配置
     */
    public static void mem_monitor_config() {
        MEMORY_MONITORING = "1".equals(
                HUtilsEx.getSystemProperty(IProperty.MEMORY_MONITORING_CFG, "0"));
    }

    /** [应用配置广播接收者] **/
    private static BroadcastReceiver sConfigBroadcastReceiver = null;

    /**
     * Monitor 初始化
     *
     * @param context
     */
    @SuppressLint("UnspecifiedRegisterReceiverFlag")
    private static void config_monitor_init(Context context) {
        if (context != null) {
            if (null == sConfigBroadcastReceiver) {
                sConfigBroadcastReceiver = new ConfigBroadcastReceiver();
            }

            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(IBroadcast.HMEDIA_CONFIG_ACTION);
            context.registerReceiver(sConfigBroadcastReceiver, intentFilter);
        }
    }

    /**
     * 取消 Monitor 初始化
     *
     * @param context
     */
    private static void config_monitor_uninit(Context context) {
        if (null == context) {
            throw new IllegalArgumentException("mem_monitor_uninit <context> can not be null!");
        }

        if (sConfigBroadcastReceiver != null) {
            context.unregisterReceiver(sConfigBroadcastReceiver);
            sConfigBroadcastReceiver = null;
        }
    }

    /**
     * 更下低内存信息
     * <p> 如果在低内存状态了，自动不再支持软解吗；
     *
     * @param context 上下文环境
     * @return 低内存状态/正常状态
     */
    public static boolean updateLowMemoryInfo(Context context) {
        if (null == context) {
            throw new IllegalArgumentException("updateLowMemoryInfo <context> can not be null!");
        }

        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        if (null == am) {
            LogUtil.d(TAG, "updateLowMemoryInfo <Context.ACTIVITY_SERVICE> fails!");
            return false;
        }

        ActivityManager.MemoryInfo memInfo = new ActivityManager.MemoryInfo();
        am.getMemoryInfo(memInfo);
        if (memInfo.totalMem != 0) {
            long threshold = 128;
            long totalMem = MiscUtils.getTotalMemory(memInfo.totalMem);
            long availMem = memInfo.availMem / 1024 / 1024;
            totalMem = totalMem / 1024 / 1024;

            // [onLowMemory]
            if (totalMem > 1024 + 768) {
                threshold = 256;
            }

            LogUtil.d(TAG, "totalMem: " + totalMem + "MB, "
                    + "availMem: " + availMem + "MB, threshold: " + threshold + "MB");
            return availMem < threshold;
        }

        return false;
    }

    /**
     * 打印当前系统可用内存
     *
     * @param context 上下文环境
     * @param reason 监听原因
     * @return 是否需要退出释放进程
     */
    public static boolean monitorSysMemInfo(Context context, int reason) {
        if (null == context) {
            throw new IllegalArgumentException("monitorSysMemInfo <context> can not be null!");
        }

        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        if (null == am) {
            LogUtil.d(TAG, "getSystemService <Context.ACTIVITY_SERVICE> fails!");
            return false;
        }

        ActivityManager.MemoryInfo memInfo = new ActivityManager.MemoryInfo();
        am.getMemoryInfo(memInfo);
        if (memInfo.totalMem != 0) {
            long totalMem = MiscUtils.getTotalMemory(memInfo.totalMem);
            long availMem = memInfo.availMem / 1024 / 1024;
            long threshold = memInfo.threshold / 1024 / 1024;
            totalMem = totalMem / 1024 / 1024;

            LogUtil.d(TAG, "totalMem: " + totalMem + "MB, "
                    + "availMem: " + availMem + "MB, "
                    + "threshold: " + threshold + "MB, "
                    + "lowMemory: " + memInfo.lowMemory);

            boolean exitProcess = false;
            int lowThreshold = HUtilsEx.getSystemProperty(
                    "ro.media.low_mem_threshold", -1);
            // [memInfo.availMem] 只是一个大概的值, 局限于应用层面。

            switch (reason) {
                case -1: {
                    // [onLowMemory]
                    if (totalMem > 1024 + 768) {
                        // 2G 内存
                        lowThreshold = lowThreshold < 0? 204: lowThreshold;
                    } else if (totalMem <= 1024) {
                        // 1G 内存
                        lowThreshold = lowThreshold < 0? 102: lowThreshold;
                    }
                    exitProcess = (availMem < lowThreshold);
                    break;
                }

                case Application.TRIM_MEMORY_UI_HIDDEN: {
                    // [onTrimMemory]
                    if (totalMem > 1024 + 768) {
                        // 2G 内存
                        lowThreshold = lowThreshold < 0? 256: lowThreshold;
                    } else if (totalMem <= 1024) {
                        // 1G 内存
                        lowThreshold = lowThreshold < 0? 128: lowThreshold;
                    }
                    exitProcess = (availMem < lowThreshold);
                    break;
                }

                default:
                    break;
            }

            return memInfo.lowMemory || exitProcess;
        }

        return false;
    }

    /**
     * 打印当前进程内存使用情况
     *
     * @param context 上下文环境
     */
    public static void monitorSelfMemInfo(Context context) {
        if (!MediaConfigEx.MEMORY_MONITORING) {
            return; // 没有配置监听，不做处理
        }

        if (null == context) {
            throw new IllegalArgumentException("monitorSelfMemInfo <context> can not be null!");
        }

        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        if (null == am) {
            LogUtil.d(TAG, "getSystemService <Context.ACTIVITY_SERVICE> fails!");
            return;
        }

        int[] pids = new int[1];
        pids[0] = android.os.Process.myPid();
        Debug.MemoryInfo[] memInfos = am.getProcessMemoryInfo(pids);
        if (null == memInfos || memInfos.length <= 0) {
            LogUtil.d(TAG, "getProcessMemoryInfo <myPid> fails!");
            return;
        }

        // PSS + USS
        int pss = memInfos[0].getTotalPss() / 1024;
        LogUtil.d(TAG, "PSS: " + pss + "MB");

        try {
            Method method = null;
            method = Debug.MemoryInfo.class.getMethod("getTotalUss");
            int uss = (int) method.invoke(memInfos[0], (Object[]) null);
            LogUtil.d(TAG, "USS: " + uss / 1024 + "MB");
        } catch (NoSuchMethodException
                | InvocationTargetException
                | IllegalAccessException e) {
            e.printStackTrace();
        }
    }
}
