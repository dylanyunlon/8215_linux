package com.hcn.media;

import android.annotation.SuppressLint;
import android.app.Application;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import androidx.annotation.RequiresApi;

import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.config.Feature;
import com.hcn.media.base.xbus.BusTable;
import com.hcn.media_data.FavoriteManager;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_model.MediaModel;
import com.hcn.media_model.MediaUtils;
import com.hcn.common.utils.HCrashUtils;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.debug.MediaConfigEx;
import com.hcn.media_theme.Argument;
import com.hcn.media_theme.ThemeUtilsEx;
import com.hcn.skinx.SkinX;
import com.hcn.skinx.compat.SkinCompatApplication;
import com.hcn.skinx.config.ConfigX;
import com.hcn.skinx.config.SkinConfig;
import com.orhanobut.logger.Logger;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;

import io.reactivex.rxjava3.plugins.RxJavaPlugins;
import io.vov.vitamio.Vitamio;

/**
 * 多媒体进程入口
 * <pre>
 *     播放器 UI 界面；
 *     多媒体播放器的播放流程、与车载系统的业务交互逻辑等；
 * </pre>
 *
 * @author 86158
 * @telephone +8615889665821
 */
public class HMediaApp extends SkinCompatApplication {
    private static final String TAG = AppGlobalData.TAG;
    private static final String APP_TAG = "MediaApp";

    /**
     * 当前进程相关的全局对象
     * <pre>
     *    1、当前应用程序的 Application 组件对象；
     *    2、全局数据对象/用来进行全局数据共享（这个不是一个很好的设计，会打乱业务逻辑）；
     * </pre>
     */
    private static HMediaApp mInstance = null;
    private static AppGlobalData sAppData = null;

    /**
     * 当前组件消息处理对象
     * <pre>
     *    关联 MainLooper，主线程执行；
     *    仅仅限于 Application 中做延时/定时任务使用；
     * </pre>
     */
    private H mTaskHandler = null;

    /**
     * [Application 实例]
     * <pre>
     *    无法理解最初开放这个接口的原因；
     *    非主流用法，建议尽早关闭这个入口；
     * </pre>
     *
     * @return 当前应用组件实例；
     * @deprecated 这个接口已经关闭
     */
    @Deprecated
    private static HMediaApp instance() {
        return mInstance;
    }

    public HMediaApp() {
        super();

        // 记录实例对象
        mInstance = this;

        // 初始化总线地址表
        BusTable.init();

        // 初始化外部参数
        Argument.initialization();

        // 初始化客制化配置
        Feature.instance();
    }

    @Override
    protected SkinConfig buildSkinPackageConfig() {
        return new SkinConfig.Builder()
                .setModuleProp(ConfigX.MEDIA_SKINX_PROP)
                .setPackageFilePrefix(ConfigX.MEDIA_SKIN_PACKAGE_FILE_PREFIX)
                .setPackageNamePrefix(ConfigX.MEDIA_SKIN_PACKAGE_NAME_PREFIX)
                .setAsyncLoad(true)
                .setLoadTimeOutMs(256)
                .build();
    }

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        LogUtil.e(TAG, "application attachBaseContext");

        // 初始化应用数据
        sAppData = AppGlobalData.getInstance();
        FavoriteManager.getInstance();

        // [工具类初始化]
        MediaUtils.init(this);

        // 调试配置初始化
        MediaConfigEx.init_config(this);
        RxJavaPlugins.setErrorHandler(throwable -> {
            // 异常处理
            Logger.t(APP_TAG).d("RxJava Error: " + throwable.getMessage());
        });

        // 打印当前系统位数
        if (sAppData.is64BitOS()) {
            Logger.t(APP_TAG).d("current system abi: " + Build.SUPPORTED_64_BIT_ABIS[0]);
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.R)
    @Override
    public void onCreate() {
        super.onCreate();
        Logger.t(APP_TAG).d("application onCreate");

        // 初始化皮肤相关
        ThemeUtilsEx.init(this);

        // 是否支持软解码
        if (Argument.isSupportVitamio()) {
            Vitamio.isInitialized(this);
        }

        // 获取当前应用程序的 uid
        try {
            PackageManager pm = getPackageManager();
            // android.uid.system 的 uid 是 1000，正规点直接动态去获取；
            BaseMediaData.UID = pm.getApplicationInfo(getPackageName(), 0).uid;
        } catch (PackageManager.NameNotFoundException ignored) {
            Logger.t(APP_TAG).w("onCreate, PackageManager.NameNotFoundException...");
        }

        // 媒体模型初始化
        MediaModel.init(this);

        // 监听任务处理器
        mTaskHandler = new H(this, Looper.getMainLooper());
        mTaskHandler.sendEmptyMessageDelayed(
                H.MSG_CHECK_SELF_MEM_INFO, 90 * 1000);
        mTaskHandler.sendEmptyMessageDelayed(
                H.MSG_CHECK_LOW_MEM_STATUS, 60 * 1000);

        // [异常信息保存]
        HCrashUtils.init(crashInfo -> {
            // [异常文件保存前被调用]
            Logger.t(APP_TAG).e("onCrash: " + crashInfo.toString());
        });
    }

    /**
     * 方法用于模拟过程环境
     * <p> 它永远不会在 Android 设备上被调用;
     */
    @Override
    public void onTerminate() {
        super.onTerminate();
    }

    /**
     * 系统低内存通知
     * <p> 检查系统内存，如果低于某一个阈值，我们将提出进程；
     */
    @Override
    public void onLowMemory() {
        super.onLowMemory();

        // [低内存时调用, 此时已经没有后台进程]
        Logger.t(APP_TAG).w("onLowMemory...");
        MediaConfigEx.monitorSelfMemInfo(this);

        // 低内存情况下需要尽快退出，避免弹框报错。
        BaseMediaData.updateLowMemory(
                MediaConfigEx.updateLowMemoryInfo(this));
        if (MediaConfigEx.monitorSysMemInfo(this, -1)) {
            MediaModel.call().onLowMemory(-1);
        }
    }

    /**
     * 系统修剪内存通知
     * <p> 检查系统内存，如果低于某一个阈值，我们将提出进程；
     *
     * @param level 内存修剪等级
     */
    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);
        // [内存清理时调用, 按 Home 和 Back 都会触发]
        Logger.t(APP_TAG).w("onTrimMemory: level = " + level);

        switch (level) {
            case TRIM_MEMORY_COMPLETE:
                // 内存不足，并且该进程在后台进程列表最后一个，马上就要被清理
                break;
            case TRIM_MEMORY_MODERATE:
                // 内存不足，并且该进程在后台进程列表的中部
                break;
            case TRIM_MEMORY_BACKGROUND:
                // 内存不足，并且该进程是后台进程
                break;
            case TRIM_MEMORY_UI_HIDDEN:
                // 内存不足，并且该进程的UI已经不可见了。
                break;
            case TRIM_MEMORY_RUNNING_CRITICAL:
                // 内存不足(后台进程不足3个)，并且该进程优先级比较高，需要清理内存
                break;
            case TRIM_MEMORY_RUNNING_LOW:
                // 内存不足(后台进程不足5个)，并且该进程优先级比较高，需要清理内存
                break;
            case TRIM_MEMORY_RUNNING_MODERATE:
                // 内存不足(后台进程超过5个)，并且该进程优先级比较高，需要清理内存
                break;
            default:
                break;
        }

        // 根据实际情况判定是否退出
        BaseMediaData.updateLowMemory(
                MediaConfigEx.updateLowMemoryInfo(this));
        if (MediaConfigEx.monitorSysMemInfo(this, level)) {
            MediaModel.call().onLowMemory(-2);
        }
    }

    /**
     * 任务处理器
     */
    @SuppressLint("HandlerLeak")
    private final class H extends Handler {
        public static final int MSG_NONE = -1;
        public static final int MSG_CHECK_SELF_MEM_INFO = 1;
        public static final int MSG_CHECK_LOW_MEM_STATUS = 2;

        // 上下文弱引用
        private final Reference<Context> mContextRef;

        public H(Context context, Looper looper) {
            super(looper);
            mContextRef = new WeakReference<Context>(context);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            switch (msg.what) {
                case MSG_CHECK_SELF_MEM_INFO: {
                    MediaConfigEx.monitorSelfMemInfo(mContextRef.get());
                    mTaskHandler.sendEmptyMessageDelayed(
                            H.MSG_CHECK_SELF_MEM_INFO, 90 * 1000);
                    break;
                }
                case MSG_CHECK_LOW_MEM_STATUS: {
                    BaseMediaData.updateLowMemory(
                            MediaConfigEx.updateLowMemoryInfo(mContextRef.get()));
                    mTaskHandler.sendEmptyMessageDelayed(
                            H.MSG_CHECK_LOW_MEM_STATUS, 60 * 1000);
                    break;
                }
                case MSG_NONE:
                default:
                    break;
            }
        }
    }
}
