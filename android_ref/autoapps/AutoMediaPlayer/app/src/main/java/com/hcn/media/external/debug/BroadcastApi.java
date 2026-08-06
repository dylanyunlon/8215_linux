package com.hcn.media.external.debug;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import androidx.annotation.NonNull;

import com.hcn.common.misc.HBroadcastUtils;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.media_common.debug.LogUtil;

import java.util.Objects;

/**
 * 广播 API 定义
 * <pre>
 *    定义触发播放的外部广播；
 *    这里定义的是静态广播，可以拉起媒体进程；
 * </pre>
 *
 * @author 65821
 */
public final class BroadcastApi {
    /**
     * 多媒体 API 标准广播接口
     * <pre>
     *    外部程序可以通过该广播拉起或者唤醒多媒体进程；
     *    这个广播是为调试多媒体后台播放 api 接口而设计；
     * </pre>
     */
    public static final String AUTO_MEDIA_STANDARD_API = "com.hcn.media.api.action";

    /**
     * 多媒体标准 api 广播的类型键值
     * @see BroadcastApi#AUTO_MEDIA_STANDARD_API
     */
    public static final String KEY_API_TYPE = "api.type.key";

    /**
     * 多媒体标准 api 广播的数据键值
     * @see BroadcastApi#AUTO_MEDIA_STANDARD_API
     */
    public static final String KEY_API_DATA = "api.data.key";
    
    /**
     * 媒体 API 类型值定义
     * @see BroadcastApi#KEY_API_TYPE
     */
    public interface ITypeValue {
        /**
         * 开始后台播放
         * <pre>
         *    启动进程并开始后台播放；
         *    后台只能播放音乐模式（继续上次记忆播放歌曲流程）；
         *    注意：当前如果已经在播放状态，广播将无效；
         *    发送隐式广播:
         *    am broadcast -a com.hcn.media.api.action --ei api.type.key 10001
         *    发送显示广播（Android O 以后不支持隐式广播）:
         *    am broadcast -n com.hcn.AutoMediaPlayer/com.hcn.media.external.debug.MediaApiReceiver
         *                 -a com.hcn.media.api.action --ei api.type.key 10001
         * </pre>
         */
        int START_BACKGROUND_PLAYBACK = 10001;

        /** 触发播放暂停事件 **/
        int TRIGGER_PLAY_PAUSE_TRACK = 10002;

        /** 触发切换播放事件 **/
        int TRIGGER_PLAY_TRACK = 10003;

        /** 触发切换暂停事件 **/
        int TRIGGER_PAUSE_TRACK = 10004;

        /** 触发切换上一曲事件 **/
        int TRIGGER_SWITCH_PREV_TRACK = 10005;

        /** 触发切换下一曲事件 **/
        int TRIGGER_SWITCH_NEXT_TRACK = 10006;
    }
    
    /**
     * 调试接口广播事件接受者
     * <p> 这里只是一个封装，中装的作用；
     * @see BroadcastApi#AUTO_MEDIA_STANDARD_API
     */
    private static DebugBroadcastReceiver sDebugReceiver = null;

    /** 调试接口事件监听者 **/
    private static IDebugEvent sDebugListener = null;

    /** 禁止实例化（工具类） **/
    private BroadcastApi() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    /**
     * 判断目标类型值是否是播放类型值
     * <p> 不是每个类型值都支持播放动作；
     *
     * @param typeValue {@link ITypeValue}
     * @return 是播放类型值/否
     */
    public static boolean isPlayTypeValue(int typeValue) {
        switch (typeValue) {
            case ITypeValue.START_BACKGROUND_PLAYBACK:
            case ITypeValue.TRIGGER_PLAY_PAUSE_TRACK:
            case ITypeValue.TRIGGER_PLAY_TRACK:
            case ITypeValue.TRIGGER_PAUSE_TRACK:
            case ITypeValue.TRIGGER_SWITCH_PREV_TRACK:
            case ITypeValue.TRIGGER_SWITCH_NEXT_TRACK:
                return true;
            default:
                break;
        }
        return false;
    }

    /**
     * 注册调试回调事件
     * <p> 同时间段只容许被注册一次（重复注册需要先取消前一个注册）；
     *
     * @param callback 回调监听者
     */
    public static void registerDebugReceiver(@NonNull IDebugEvent callback) {
        if (sDebugReceiver == null) {
            sDebugReceiver = new DebugBroadcastReceiver();
        } else {
            LogUtil.w("BroadcastApi",
                    "Function registerDebugReceiver called repeatedly!");
            return;
        }

        sDebugListener = callback;
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(AUTO_MEDIA_STANDARD_API);
        HBroadcastUtils.getInstance(HUtilsEx.getApp())
                .registerReceiver(sDebugReceiver, intentFilter);
    }

    /**
     * 反注册调试回调事件
     * @see #registerDebugReceiver(IDebugEvent) 
     */
    public static void unregisterDebugReceiver() {
        if (sDebugReceiver != null) {
            HBroadcastUtils.getInstance(HUtilsEx.getApp())
                    .unregisterReceiver(sDebugReceiver);
            sDebugReceiver = null;
        }

        sDebugListener = null;
    }

    /**
     * 发送本地调试事件
     * <p> 无 binder 交互消耗；

     * @param eventId 事件
     */
    public static void postDebugEvent(int eventId) {
        Intent intent = new Intent(AUTO_MEDIA_STANDARD_API);
        intent.putExtra(KEY_API_TYPE, eventId);
        HBroadcastUtils.getInstance(HUtilsEx.getApp()).sendBroadcast(intent);
    }

    /**
     * 发送本地调试事件
     * <p> 无 binder 交互消耗；
     * 
     * @param eventId 事件
     * @param data 附加字符串数据
     */
    public static void postDebugEvent(int eventId, String data) {
        Intent intent = new Intent(AUTO_MEDIA_STANDARD_API);
        intent.putExtra(KEY_API_TYPE, eventId);
        intent.putExtra(KEY_API_DATA, data);
        HBroadcastUtils.getInstance(HUtilsEx.getApp()).sendBroadcast(intent);
    }

    /**
     * 调试广播接收者
     * <p> 就是个本地广播中转器，解耦使用；
     */
    private static final class DebugBroadcastReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Objects.isNull(action)) {
                return;
            }

            // 获取调试接口事件类型
            int typeValue = intent.getIntExtra(BroadcastApi.KEY_API_TYPE, -1);
            String extraData = intent.getStringExtra(BroadcastApi.KEY_API_DATA);
            if (sDebugListener != null) {
                sDebugListener.onDebugEvent(typeValue, extraData);
            }
        }
    }
}
