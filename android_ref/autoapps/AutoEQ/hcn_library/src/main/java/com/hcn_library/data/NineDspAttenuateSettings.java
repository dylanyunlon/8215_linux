package com.hcn_library.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn_library.nativeextdsp.NativeHelper;
import com.hcn_library.util.ConstantExtDsp;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;

import java.util.Arrays;

public class NineDspAttenuateSettings implements NineConstantExtDsp {

    // 日志标签，用于在日志中识别此类
    private static final String TAG = NineDspAttenuateSettings.class.getSimpleName();
    // 调试标志，用于判断是否开启调试日志
    private static final boolean DEBUG = Log.isLoggable(NineDspAttenuateSettings.class.getSimpleName(), Log.DEBUG);

    // 存储衰减及反相信息的 SharedPreferences 文件名称
    private static final String NINE_DSP_ATTENUATE_FILE = "nine_dsp_attenuate";
    // 存储衰减值的 SharedPreferences 键的前缀
    private static final String KEY_ATTENUATE_PREFIX = "nine_dsp_attenuate_";
    // 存储静音状态的 SharedPreferences 键的前缀
    private static final String KEY_MUTE_PREFIX = "nine_dsp_mute_";
    // 存储反转状态的 SharedPreferences 键的前缀
    private static final String KEY_INVERT_PREFIX = "nine_dsp_invert_";
    // 存储左前和右前声道链接状态的 SharedPreferences 键
    private static final String KEY_LINK_LF_RF = "nine_dsp_link_lf_rf";
    // 存储左后和右后声道链接状态的 SharedPreferences 键
    private static final String KEY_LINK_LR_RR = "nine_dsp_link_lr_rr";

    // 上下文对象，用于访问系统资源和服务
    private Context context;
    // 单例实例，用于实现单例模式
    private static NineDspAttenuateSettings instance = null;
    // SharedPreferences 工具类实例，用于存储和获取设置数据
    private SPUtils sharedPreferencesUtils;

    /**
     * 获取 NineDspAttenuateSettings 的单例实例
     *
     * @param mContext 上下文对象
     * @return NineDspAttenuateSettings 的单例实例
     */
    public static NineDspAttenuateSettings getInstance(Context mContext) {
        if (instance == null) {
            instance = new NineDspAttenuateSettings(mContext);
        }
        return instance;
    }

    /**
     * 构造函数，初始化上下文和 SharedPreferences 工具类
     *
     * @param context 上下文对象
     */
    private NineDspAttenuateSettings(Context context) {
        this.context = context;
        sharedPreferencesUtils = SPUtils.getInstance(NINE_DSP_ATTENUATE_FILE);
    }

    /**
     * 调用 Native 方法设置衰减、静音和反转状态
     *
     * @param channel   声道名称
     * @param attenuate 衰减值
     * @param mute      是否静音
     * @param isInvert  是否反转 7604c指令实际用到，si47925只是占位
     */
    public void nativeAttenuate(String channel, int attenuate, boolean mute, boolean isInvert) {
        Log.d(TAG, "nativeAttenuate: channel = " + channel + " attenuate = " + attenuate + " mute = " + mute + " isInvert = " + isInvert);
        int[] data = new int[]{convertChannel(channel), attenuate, mute ? 1 : 0, isInvert ? 1 : 0};
        NativeHelper.getEq().setEqAttSpeaker(data);
        Log.d(TAG, String.format("nativeAttenuate data : %s", Arrays.toString(data)));
    }

    /**
     * 调用 Native 方法设置衰减、静音、反转状态，根据 change 标志决定是否使用延迟指令
     *
     * @param channel   声道名称
     * @param attenuate 衰减值
     * @param mute      是否静音
     * @param isInvert  是否反转
     * @param change    是否使用延迟指令
     */
    public void nativeAttenuate(String channel, int attenuate, boolean mute, boolean isInvert, boolean change) {
        Log.d(TAG, "nativeAttenuate: channel = " + channel + " attenuate = " + attenuate + " mute = " + mute + " isInvert = " + isInvert + ",change = " + change);
        int[] data = new int[]{convertChannel(channel), attenuate, mute ? 1 : 0, isInvert ? 1 : 0};
        if (change) {
            NineDspDelaySettings delaySettings = NineDspDelaySettings.getInstance(context);
            int delay = delaySettings.getDelay(channel);
            int polarity = isInvert ? 1 : 0;
            data = new int[]{convertChannel(channel), delay == 0 && polarity == 0 ? 1 : 0, delay, polarity};
            NativeHelper.getEq().setEqSpeakerDelay(data);
        } else {
            NativeHelper.getEq().setEqAttSpeaker(data);
        }
        Log.d(TAG, String.format("nativeAttenuate data : %s", Arrays.toString(data)));
    }

    /**
     * 保存衰减、静音和反转状态到 SharedPreferences
     *
     * @param channel   声道名称
     * @param attenuate 衰减值
     * @param mute      是否静音
     * @param revert    是否反转
     */
    public void saveAttenuate(String channel, int attenuate, boolean mute, boolean revert) {
        Log.d(TAG, String.format("saveAttenuate channel : %s, attenuate : %d, mute : %b, revert : %b", channel, attenuate, mute, revert));
        sharedPreferencesUtils.put(KEY_ATTENUATE_PREFIX + channel, attenuate);
        sharedPreferencesUtils.put(KEY_MUTE_PREFIX + channel, mute);
        sharedPreferencesUtils.put(KEY_INVERT_PREFIX + channel, revert);
    }


    /**
     * 保存声道链接状态到 SharedPreferences
     *
     * @param lfRf 左前和右前声道是否链接
     * @param lrRr 左后和右后声道是否链接
     */
    public void saveLink(boolean lfRf, boolean lrRr) {
        Log.d(TAG, String.format("saveLink lfRf : %b, lrRr : %b", lfRf, lrRr));
        sharedPreferencesUtils.put(KEY_LINK_LF_RF, lfRf);
        sharedPreferencesUtils.put(KEY_LINK_LR_RR, lrRr, true);
    }

    /**
     * 获取左前和右前声道的链接状态
     *
     * @return 左前和右前声道是否链接
     */
    public boolean getLinkLfRf() {
        return sharedPreferencesUtils.getBoolean(KEY_LINK_LF_RF, false);
    }

    /**
     * 获取左后和右后声道的链接状态
     *
     * @return 左后和右后声道是否链接
     */
    public boolean getLinkLrRr() {
        return sharedPreferencesUtils.getBoolean(KEY_LINK_LR_RR, false);
    }

    /**
     * 从 SharedPreferences 获取衰减值
     *
     * @param channel 声道名称
     * @return 衰减值
     */
    public int getAttenuate(String channel) {
        return sharedPreferencesUtils.getInt(KEY_ATTENUATE_PREFIX + channel, ConstantExtDsp.DEF_ATTENUATE);
    }

    /**
     * 从 SharedPreferences 获取静音状态
     *
     * @param channel 声道名称
     * @return 是否静音
     */
    public boolean getMute(String channel) {
        return sharedPreferencesUtils.getBoolean(KEY_MUTE_PREFIX + channel, false);
    }

    /**
     * 从 SharedPreferences 获取反转状态
     *
     * @param channel 声道名称
     * @return 是否反转
     */
    public boolean getInvert(String channel) {
        return sharedPreferencesUtils.getBoolean(KEY_INVERT_PREFIX + channel, false);
    }

    /**
     * 将声道名称转换为对应的声道编号
     *
     * @param channel 声道名称
     * @return 声道编号，未匹配时返回 -1
     */
    private int convertChannel(String channel) {
        if ("gb05".equals(EqUtils.getSkinName())) {
            if ("LF".equals(channel)) return 1;
            if ("RF".equals(channel)) return 2;
            if ("LR".equals(channel)) return 3;
            if ("RR".equals(channel)) return 4;
            if ("CENTER".equals(channel)) return 6;
            if ("SUBWOOFER".equals(channel)) return 5;
        } else {
            if ("LF".equals(channel)) return NineConstantExtDsp.NINE_SURROUND_CHANEL_FL;
            if ("RF".equals(channel)) return NineConstantExtDsp.NINE_SURROUND_CHANEL_FR;
            if ("LR".equals(channel)) return NineConstantExtDsp.NINE_SURROUND_CHANEL_RL;
            if ("RR".equals(channel)) return NineConstantExtDsp.NINE_SURROUND_CHANEL_RR;
            if ("CENTER".equals(channel)) return NineConstantExtDsp.NINE_SURROUND_CHANEL_CEN;
            if ("SUBWOOFER".equals(channel)) return NineConstantExtDsp.NINE_SURROUND_CHANEL_SUB;
        }
        return -1;
    }

    /**
     * 重置增益，清除 SharedPreferences 并调用 nativeAll 方法处理多个声道
     */
    public void resetGain() {
        Log.d(TAG, "resetGain");
        sharedPreferencesUtils.clear(true);
        nativeAll("LF", "RF", "LR", "RR", "CENTER", "SUBWOOFER");
    }

    /**
     * 对多个声道调用 nativeAttenuate 方法
     *
     * @param channels 声道名称数组
     */
    public void nativeAll(String... channels) {
        for (String channel : channels) {
            int attenuate = getAttenuate(channel);
            boolean mute = getMute(channel);
            boolean isInvert = getInvert(channel);
            if ("gb05".equals(EqUtils.getSkinName())) {
                nativeAttenuate(channel, attenuate, mute, isInvert);
            } else {
                nativeAttenuate(channel, attenuate, mute, isInvert, true);
                nativeAttenuate(channel, attenuate, mute, isInvert);
            }
        }
    }
}