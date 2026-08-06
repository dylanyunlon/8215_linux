package com.hcn.media.local.utils;

import static android.carsource.McuConstant.K_EQ;

import android.carsource.McuManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;

import androidx.annotation.NonNull;

import com.hcn.auto_compat.PlatformUtils;
import com.hcn.eq.EQMainUI;
import com.hcn.media_data.debug.DebugUiData;

/**
 * 主要用来提取常调用的接口
 * <p> 为了简洁代码使用，避免过多重复代码调用；
 * <p> 原则：不要授予它保存外部上下文的功能，需要什么都由外部传输进来。
 *
 * @Author youwj
 * @Create 2021/7/15 11:32
 */
public class HFuncUtils {
    /** 唯一实例 **/
    private static HFuncUtils sInstance = null;

    public static HFuncUtils instance() {
        if (sInstance == null) {
            sInstance = new HFuncUtils();
        }
        return sInstance;
    }

    private HFuncUtils() {
    }

    /**
     * 检查并尝试跳转到内置 EQ
     *
     * @param context 上下文环境
     * @param cls EQ 页面类，类对象必须存在。
     * @return 是否跳转成功，返回 <code>true</code> 跳转成功，反之失败；
     */
    public boolean tryGotoInternalEQ(Context context,  @NonNull Class<?> cls) {
        if (context == null) {
            throw new NullPointerException("");
        }

        // 使用内置 DSP（也可以强制使能使用内置 EQ 程序）
        if (DebugUiData.FORCE_ENABLE_MEDIA_EQ || PlatformUtils.usedInternalDSP(context)) {
            Intent i = new Intent();
            i.setComponent(new ComponentName(context, cls));
            context.startActivity(i);
            return true;
        }

        return false;
    }

    /**
     * 跳转到 EQ 界面
     * <p> 先尝试跳转到内置 EQ 界面，如果跳转失败再通知 CarServices 让其决策。
     *
     * @param context 上下文环境
     */
    public void gotoEQ(@NonNull Context context) {
        if (!tryGotoInternalEQ(context, EQMainUI.class)) {
            McuManager.getsInstance().injectKeyEventTimeout(K_EQ, 50);
        }
    }
}
