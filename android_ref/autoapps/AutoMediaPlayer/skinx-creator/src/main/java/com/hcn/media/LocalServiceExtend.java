package com.hcn.media;

import android.content.Intent;
import android.os.IBinder;

import androidx.lifecycle.Lifecycle;

import com.hcn.auto_compat.app.Wallpaper;
import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.media.extend.ServiceExtend;
import com.hcn.skinx.SkinX;
import com.hcn.skinx.base.ISkinExService;

/**
 * 本地服务扩展
 * <pre>
 *    1、类名不可以修改，这个是插件的规则；
 *    2、主要用来对 LocalService 组件操作扩展；
 *    3、最终由 LocalService 实例化并调用；
 * </pre>
 *
 * @author 65821
 */
public class LocalServiceExtend extends ServiceExtend {

    private static final String TAG = LocalServiceExtend.class.getSimpleName();

    /**
     * 媒体服务扩展构造函数
     * @param service 扩展服务所有者
     */
    public LocalServiceExtend(ISkinExService service) {
        super(service);
    }

    @Override
    public void onCreate() {
        super.onCreate();

        LogUtils.vTag(TAG, "onCreate");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return super.onBind(intent);
    }

    /**
     * 服务初始化完成回调
     * @param state 宿主服务状态
     */
    @Override
    public void onInitialized(Lifecycle.State state) {
        super.onInitialized(state);
        LogUtils.vTag(TAG, "onInitialized: " + state);

        // 关联成功，加载壁纸
        postDelayed(() -> {
            // 当前主题只有调试模式下才加载壁纸
            if (!SkinX.getBoolean("support_wallpaper_customized")) {
                return;
            }

            Wallpaper.instance().initialize();
        }, 10);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public String tryCallMethod(String method, Object... args) {
        return super.tryCallMethod(method, args);
    }
}
