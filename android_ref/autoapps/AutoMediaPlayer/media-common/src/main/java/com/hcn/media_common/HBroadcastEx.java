package com.hcn.media_common;

import android.content.Context;
import android.content.Intent;

import com.hcn.common.misc.HBroadcastUtils;

/**
 * 广播相关定义
 * <p> 主要定义与当前应用相关联的广播；
 *
 * @author 65821
 */
public class HBroadcastEx {

    /**
     * 画中画相关
     * <p> 定义画中画相关的广播；
     */
    public interface PIP {
        /**
         * 画中画发送的广播事件
         * [SystemUI/PipMotionHelper.java] 发送广播;
         */
        String ACTION_PIP = "action.pip";

        /**
         * 画中画播放控制广播事件
         * Intent action for media controls from Picture-in-Picture mode.
         */
        String ACTION_PIP_MEDIA_CONTROL = "media_control";

        /**
         * 播放控制广播携带的控制类型
         * Intent extra for media controls from Picture-in-Picture mode.
         */
        String EXTRA_CONTROL_TYPE = "control_type";

        /**
         * 播放请求码，发件人的私人请求代码（当前未使用）
         * The request code for play action PendingIntent.
         */
        int REQUEST_PLAY = 1;

        /**
         * 暂停请求码，发件人的私人请求代码（当前未使用）
         * The request code for pause action PendingIntent.
         */
        int REQUEST_PAUSE = 2;

        /**
         * 上一曲请求码，发件人的私人请求代码（当前未使用）
         * The request code for prev action PendingIntent.
         */
        int REQUEST_PREV = 3;

        /**
         * 下一曲请求码，发件人的私人请求代码（当前未使用）
         * The request code for next action PendingIntent.
         */
        int REQUEST_NEXT = 4;

        /**
         * 播放控制类型
         * The intent extra value for play action.
         */
        int CONTROL_TYPE_PLAY = 1;

        /**
         * 暂停控制类型
         * The intent extra value for pause action.
         */
        int CONTROL_TYPE_PAUSE = 2;

        /**
         * 上一曲控制类型
         * The intent extra value for prev action.
         */
        int CONTROL_TYPE_PREV = 3;

        /**
         * 下一曲控制类型
         * The intent extra value for pause action.
         */
        int CONTROL_TYPE_NEXT = 4;
    }

    /**
     * 历史代码相关的广播
     * <p> 这些定义是最早代码设计的，不可以废除；
     */
    public interface SpecialChain {
        /**
         * 跨进程广播事件
         * <p> 由 HMediaService 下发给 HMediaPlayer 进程；
         */
        String ACTION_MESSAGE_CALLBACK =
                "com.hcn.automediaplayer.action.MESSAGE_CALLBACK";

        /**
         * 本地广播（不可以跨进程传输）
         * <p> 由 HMediaPlayer/LocalService 下发给 UI 模块；
         */
        String ACTION_LOCAL_CALLBACK =
                "com.hcn.automediaplayer.action.LOCAL_CALLBACK";

        /**
         * 广播当前曲目号，总曲目号
         * <p> 由 HMediaPlayer 进程广播给外部模块（需要的模块主动监听）；
         */
        String ACTION_OTHER_CALLBACK =
                "com.hcn.automediaplayer.action.ACTION_OTHER_CALLBACK";

        /**
         * 上述广播的附加参数 KEY
         * <pre>
         *    EXTRA_CALLBACK_TYPE：回调事件类型
         *    EXTRA_CALLBACK_DATA：回调事件数据
         * </pre>
         */
        String EXTRA_CALLBACK_TYPE = "CALLBACK_TYPE";
        String EXTRA_CALLBACK_DATA = "CALLBACK_DATA";
    }

    /**
     * 发送本地广播
     * <p> 无 binder 交互消耗；
     * @param context 上下文环境
     * @param eventId 事件
     */
    public static void sendLocalBroadcast(Context context, int eventId) {
        Intent intent = new Intent(SpecialChain.ACTION_LOCAL_CALLBACK);
        intent.putExtra(SpecialChain.EXTRA_CALLBACK_TYPE, eventId);
        HBroadcastUtils.getInstance(context).sendBroadcast(intent);
    }

    /**
     * 发送本地广播
     * <p> 无 binder 交互消耗；
     * @param context 上下文环境
     * @param eventId 事件
     * @param data 附加字符串数据
     */
    public static void sendLocalBroadcast(Context context, int eventId, String data) {
        Intent intent = new Intent(SpecialChain.ACTION_LOCAL_CALLBACK);
        intent.putExtra(SpecialChain.EXTRA_CALLBACK_TYPE, eventId);
        intent.putExtra(SpecialChain.EXTRA_CALLBACK_DATA, data);
        HBroadcastUtils.getInstance(context).sendBroadcast(intent);
    }
}
