package com.hcn.media.main.observer;

import android.Configures.HConfig;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;

import com.hcn.media.local.event.VehicleConfigEx;

import java.util.Objects;

/**
 * 设置值观察者
 * <pre>
 *    UI 部分统一监听，不做逻辑处理，回到到调用的地方去处理逻辑;
 *    同一个值理论上不应该在一个进程中有多处监听，但是这里暂时懒得改；
 * </pre>
 *
 * @author 86158
 */
public class SettingsKeyObserver extends ContentObserver {
    private ICallback mCallback;

    public SettingsKeyObserver(ICallback callback) {
        super(new Handler(Looper.getMainLooper()));
        this.mCallback = callback;
    }

    @Override
    public void onChange(boolean selfChange, Uri uri) {
        super.onChange(selfChange, uri);
        final String path = uri.getPath();
        if (TextUtils.isEmpty(path) || Objects.isNull(mCallback)) {
            return;
        }

        String systemKey = path.replace("/system/", "");
        switch (systemKey) {
            // 行车中是否可以观看视频状态改变
            case HConfig.driving_disable_video:
                mCallback.onDrivingWatchVideoStateChanged();
                break;
            case VehicleConfigEx.VEHICLE_SPEED_VIDEO_STATE:
                mCallback.onVehicleSpeedVideoStateChanged();
                break;
            default:
                break;
        }
    }

    /** 系统 Settings/System 键值改变回调接口 **/
    public interface ICallback {
        /** 行车中可以看视频改变 **/
        default void onDrivingWatchVideoStateChanged(){
        }

        /** 车辆速度视频状态改变 **/
        default void onVehicleSpeedVideoStateChanged() {
        }
    }
}