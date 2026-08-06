package com.hcn.media.external.debug;

import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.UserHandle;

import com.hcn.auto_compat.app.CompatUtils;
import com.hcn.common.HConfig;
import com.hcn.common.app.HServiceUtils;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.media.base.IMedia;
import com.hcn.media.external.ReceptionService;
import com.hcn.media.external.debug.BroadcastApi.ITypeValue;
import com.hcn.media_common.debug.MediaDebug;
import com.hcn.media_data.base.BaseMediaData;

import java.util.Objects;

/**
 * 媒体调试接口广播
 * @see BroadcastApi#AUTO_MEDIA_STANDARD_API
 *
 * @author 65821
 */
public class MediaApiReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (Objects.isNull(action)) {
            return;
        }

        // 只处理标准调试广播
        if (action.equals(BroadcastApi.AUTO_MEDIA_STANDARD_API)) {
            int typeValue = intent.getIntExtra(BroadcastApi.KEY_API_TYPE, -1);
            if (typeValue == -1) {
                return;
            }

            onMediaStandardApiAction(typeValue);
        }
    }

    /**
     * 处理多媒体标准接口广播事件
     *
     * @param typeValue {@link BroadcastApi.ITypeValue} 类型值
     * @see BroadcastApi#AUTO_MEDIA_STANDARD_API
     */
    private void onMediaStandardApiAction(int typeValue) {
        switch (typeValue) {
            case ITypeValue.START_BACKGROUND_PLAYBACK:
                onStartBackgroundPlayback();
                break;
            case ITypeValue.TRIGGER_PLAY_PAUSE_TRACK:
            case ITypeValue.TRIGGER_PLAY_TRACK:
            case ITypeValue.TRIGGER_PAUSE_TRACK:
            case ITypeValue.TRIGGER_SWITCH_PREV_TRACK:
            case ITypeValue.TRIGGER_SWITCH_NEXT_TRACK:
                BroadcastApi.postDebugEvent(typeValue, IMedia.TriggerReason.BROADCAST_API);
            default:
                break;
        }
    }

    /**
     * 开始后台播放任务
     * <pre>
     *    如果当前已经在播放，不处理；
     *    如果当前没有在播放：
     *       1、播放前将检查 ReceptionService 的状态（启动）；
     *       2、如果没有媒体播放数据，我们将等待媒体扫描完成；
     *       3、如果有媒体播放数据，我们将触发音乐进入后台播放；
     * </pre>
     *
     * @see ITypeValue#START_BACKGROUND_PLAYBACK
     */
    @SuppressLint("ObsoleteSdkInt")
    private void onStartBackgroundPlayback() {
        // 这里可以使用本地广播与 ReceptionService 交互
        BroadcastApi.postDebugEvent(
                ITypeValue.START_BACKGROUND_PLAYBACK, IMedia.TriggerReason.BROADCAST_API);

        // 接待服务是运行状态
        if (HServiceUtils.isServiceRunning(ReceptionService.class)) {
            return;
        }

        // 服务如果没有启动，则需要启动服务；
        Context context = HUtilsEx.getApp().getApplicationContext();
        Intent serviceIntent = new Intent(context, ReceptionService.class);
        serviceIntent.putExtra(HConfig.START_REASON_EXTRA_KEY, "media-debug");

        // 支持启动前台服务（system.uid 需要）
        if (!MediaDebug.START_FOREGROUND_SERVICE) {
            context.startService(serviceIntent);
            return;
        }

        // Android 8.0 开始息屏情况下，不容许创建后台服务。
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
            context.startService(serviceIntent);
        } else {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                // 非多用户情况下 UserHandle 基本都是 {@link UserHandle.SYSTEM}
                UserHandle userHandler = UserHandle.getUserHandleForUid(BaseMediaData.UID);
                CompatUtils.startForegroundServiceAsUser(context, serviceIntent, userHandler);
            } else {
                HUtilsEx.getApp().startForegroundService(serviceIntent);
            }
        }
    }
}
