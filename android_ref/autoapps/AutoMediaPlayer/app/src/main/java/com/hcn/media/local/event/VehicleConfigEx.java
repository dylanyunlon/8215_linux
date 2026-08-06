package com.hcn.media.local.event;

import android.content.ContentResolver;
import android.content.Context;
import android.provider.Settings;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.Objects;

/**
 * 车辆配置扩展
 * <p> 更多是收集前装的特定需求，然后扩展到后装；
 *
 * @author 65821
 */
public class VehicleConfigEx {
    /**
     * 车速视频状态 Key
     * <pre>
     *    在系统设置 "行车中允许观看视频" 开关 Disable 的情况下；
     *    如果车速状态比较低，我们将容许观看视频，反之不容许；
     *    当然后续可以根据情况扩展（车厂可能会添加：可观看但有警告显示的状态需求）
     * </pre>
     */
    public static final String VEHICLE_SPEED_VIDEO_STATE = "vehicle_speed_video_state";
    public static final int LOW_SPEED_WATCH_STATE = 0;
    public static final int HIGH_SPEED_PROHIBIT_STATE = 1;

    /** 上下文环境引用 **/
    private final Reference<Context> mContextRef;

    public VehicleConfigEx(Context context) {
        mContextRef = new WeakReference<>(context);
    }

    /**
     * 当前车辆是否可以观看视频
     * <pre>
     *     当前车辆处于低速行驶状态；
     *     当设置 “行车中允许观看视频” 开关为 Disable 的时候，并不是 100% 不能观看视频；
     *     有些车厂会要求低速行驶的时候也可以显示视频画面（车辆定制需求）；
     * </pre>
     *
     * @return 可以观看/不可以观看
     */
    public boolean vehicleCanWatchVideo() {
        Context context = mContextRef.get();
        if (Objects.isNull(context)) {
            return false;
        }

        // 兼容国内后装（未配置），默认值需要是 HIGH_SPEED_PROHIBIT_STATE;
        ContentResolver contentResolver = context.getContentResolver();
        int canWatchVideo = Settings.System.getInt(
                contentResolver, VEHICLE_SPEED_VIDEO_STATE, HIGH_SPEED_PROHIBIT_STATE);
        return LOW_SPEED_WATCH_STATE == canWatchVideo;
    }
}

