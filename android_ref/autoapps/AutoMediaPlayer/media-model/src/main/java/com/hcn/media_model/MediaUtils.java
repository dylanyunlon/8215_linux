package com.hcn.media_model;

import android.content.Context;
import android.graphics.Point;
import android.util.DisplayMetrics;

import com.hcn.config.Customer;
import com.hcn.config.Feature;
import com.hcn.media_base.HMediaConfig;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_theme.ThemeEx;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.Objects;

/**
 * 多媒体工具类
 * <pre>
 *    和多媒体强关联的方法，其它模块用不上的，都整合到此工具中；
 *    非强相关的方法，可以整和到 {@link MiscUtils} 类中；
 *    每个修改者请一定要遵守规则，否则这些设计就没有意义了；
 * </pre>
 *
 * @author 65821
 */
public class MediaUtils {
    /** 静态成员变量 **/
    private static AppGlobalData sAppData;
    private static Reference<Context> sContextRef;

    private MediaUtils() {
        throw new RuntimeException(
                "The class ‘MediaUtils’ Prohibit instantiation.");
    }

    /** 初始化多媒体工具类 **/
    public static void init(Context context) {
        sAppData = AppGlobalData.getInstance();
        sContextRef = new WeakReference<>(context);
    }

    /**
     * 窗口是物理全屏
     * <p> 改接口只能参考使用，不一定准确；
     */
    public static boolean isVideoWindowFullScreen() {
        Context ctx = sContextRef.get();
        if (Objects.isNull(ctx)) {
            return false;
        }

        // 该接口会扣除虚拟导航高度
        DisplayMetrics dm = ctx.getApplicationContext().getResources().getDisplayMetrics();
        return (dm.widthPixels == sAppData.mVideoUiWidth)
                && (dm.heightPixels == sAppData.mVideoUiHeight);
    }

    /**
     * 根据当前视频尺寸模式计算视频尺寸
     *
     * @param videoWidth 视频原始宽度
     * @param videoHeight 视频原始高度
     * @return 目标视频显示尺寸
     */
    public static Point computeAndUpdateVideoSize(int videoWidth, int videoHeight) {
        Context context = sContextRef.get();
        if (Objects.isNull(context)) {
            throw new RuntimeException(
                    "The context of class 'MediaUtils' has been recycled, Why?");
        }

        boolean isWndFullScreen = isVideoWindowFullScreen();
        boolean isOut0fBounds = videoWidth > sAppData.mVideoUiWidth
                || videoHeight > sAppData.mVideoUiHeight;

        // 用户期望显示横纵比
        float displayAspectRatio = Float.NaN;
        // 显示区域规格横纵比
        float specAspectRatio = (float) sAppData.mVideoUiWidth / (float) sAppData.mVideoUiHeight;

        // 视频缩放, 现阶段支持自适应尺寸
        switch (sAppData.mVideoScaleType) {
            case HMediaConfig.VIDEO_SCALE_AUTO_ZOOM: {
                boolean need2Zoom = true;

                // 横屏设备如果是全屏显示，且视频尺寸不越界, 就不需要缩放；
                if (ThemeEx.isHorizontalScreenDeviceCompat(context)) {
                    if (isWndFullScreen && !isOut0fBounds) {
                        need2Zoom = false;
                    }
                }

                if (need2Zoom) {
                    // 等比例填充显示
                    float wRatio = videoWidth / (float) sAppData.mVideoUiWidth;
                    float hRatio = 0;

                    if (Feature.instance().hasFeature(Feature.BIT.DOUBLE_KNOB_SCREEN)) {
                        // 双旋钮屏幕导航栏常显，需要手动减去它的高度
                        hRatio = videoHeight / ((float) sAppData.mVideoUiHeight - Customer.NAVIGATION_BAR_HEIGHT);
                    } else {
                        hRatio = videoHeight / ((float) sAppData.mVideoUiHeight);
                    }

                    // 选择大的一个进行缩放
                    float ratio = Math.max(wRatio, hRatio);
                    videoWidth = (int) Math.ceil((float) videoWidth / ratio);
                    videoHeight = (int) Math.ceil((float) videoHeight / ratio);
                }
                break;
            }
            case HMediaConfig.VIDEO_SCALE_AUTO_FULL: {
                videoWidth = sAppData.mVideoUiWidth;
                videoHeight = sAppData.mVideoUiHeight;
                break;
            }
            case HMediaConfig.VIDEO_SCALE_0403_SIZE: {
                displayAspectRatio = 4.0f / 3.0f;
                break;
            }
            case HMediaConfig.VIDEO_SCALE_1609_SIZE: {
                displayAspectRatio = 16.0f / 9.0f;
                break;
            }
            case HMediaConfig.VIDEO_SCALE_0101_ZOOM: {
                if (isOut0fBounds) {
                    // 等比例填充显示
                    float wRatio = videoWidth / (float) sAppData.mVideoUiWidth;
                    float hRatio = videoHeight / (float) sAppData.mVideoUiHeight;

                    // 选择大的一个进行缩放
                    float ratio = Math.max(wRatio, hRatio);
                    videoWidth = (int) Math.ceil((float) videoWidth / ratio);
                    videoHeight = (int) Math.ceil((float) videoHeight / ratio);
                } else {
                    // 没有越界就原始大小显示
                }
                break;
            }
            default:
                break;
        }

        if (!Float.isNaN(displayAspectRatio)) {
            boolean shouldBeWider = displayAspectRatio > specAspectRatio;
            if (shouldBeWider) {
                // too wide, fix width
                videoWidth = sAppData.mVideoUiWidth;
                videoHeight = (int) (videoWidth / displayAspectRatio);
            } else {
                // too high, fix height
                videoHeight = sAppData.mVideoUiHeight;
                videoWidth = (int) (videoHeight * displayAspectRatio);
            }
        }

        return new Point(videoWidth, videoHeight);
    }
}
