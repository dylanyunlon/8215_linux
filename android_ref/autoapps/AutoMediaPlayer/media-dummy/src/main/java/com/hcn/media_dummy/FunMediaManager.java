package com.hcn.media_dummy;

import static com.hcn.media_dummy.utils.CommonUtil.hideNavKey;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;

import com.hcn.media_dummy.base.FunMediaBaseManager;
import com.hcn.media_dummy.listener.FunMediaPlayerListener;
import com.hcn.media_dummy.utils.CommonUtil;
import com.hcn.media_dummy.view.video.FunVideoPlayer;

/**
 * 媒体播放管理器
 * <p> 唯一实例对象，现阶段只处理 ijk 播放器;
 * @author 65821
 */
public class FunMediaManager extends FunMediaBaseManager {
    public static String TAG = "FunMediaManager";

    /** 大小屏切换的按钮资源 ID */
    public static final int SMALL_ID = R.id.small_id;
    public static final int FULLSCREEN_ID = R.id.full_id;

    @SuppressLint("StaticFieldLeak")
    private static FunMediaManager sMediaManager;

    private FunMediaManager() {
        init();
    }

    /**
     * 单例管理器
     * @return {@link FunMediaManager}
     */
    public static synchronized FunMediaManager instance() {
        if (sMediaManager == null) {
            sMediaManager = new FunMediaManager();
        }
        return sMediaManager;
    }

    /**
     * 同步创建一个临时管理器
     * @return {@link FunMediaManager}
     */
    public static synchronized FunMediaManager tmpInstance(FunMediaPlayerListener listener) {
        FunMediaManager funMediaManager = new FunMediaManager();
        funMediaManager.setListener(listener);
        if (sMediaManager != null) {
            funMediaManager.bufferPoint = sMediaManager.bufferPoint;
            funMediaManager.optionModelList = sMediaManager.optionModelList;
            funMediaManager.playTag = sMediaManager.playTag;
            funMediaManager.currentVideoWidth = sMediaManager.currentVideoWidth;
            funMediaManager.currentVideoHeight = sMediaManager.currentVideoHeight;
            funMediaManager.context = sMediaManager.context;
            funMediaManager.lastState = sMediaManager.lastState;
            funMediaManager.playPosition = sMediaManager.playPosition;
            funMediaManager.timeOut = sMediaManager.timeOut;
            funMediaManager.needMute = sMediaManager.needMute;
            funMediaManager.needTimeOutOther = sMediaManager.needTimeOutOther;
        }
        return funMediaManager;
    }

    /**
     * 替换管理器
     * @param mediaManager {@link FunMediaManager}
     */
    public static synchronized void changeManager(FunMediaManager mediaManager) {
        sMediaManager = mediaManager;
    }

    /**
     * 释放所有 MediaPlayer
     * <p> 页面销毁了记得调用是否所有的资源
     */
    public static void releaseAllMedia() {
        if (FunMediaManager.instance().listener() != null) {
            FunMediaManager.instance().listener().onCompletion();
        }

        FunMediaManager.instance().releaseMediaPlayer();
    }

    /**
     * 暂停播放
     * <p> 与 UI 绑定时调用；
     */
    public static void onPause() {
        if (FunMediaManager.instance().listener() != null) {
            FunMediaManager.instance().listener().onMediaPause();
        }
    }

    /**
     * 恢复播放
     * <p> 与 UI 绑定时调用；
     */
    public static void onResume() {
        if (FunMediaManager.instance().listener() != null) {
            FunMediaManager.instance().listener().onMediaResume();
        }
    }

    /**
     * 恢复暂停状态
     * <p> 与 UI 绑定时调用；
     * @param seek 是否产生 seek 动作,直播设置为 false
     */
    public static void onResume(boolean seek) {
        if (FunMediaManager.instance().listener() != null) {
            FunMediaManager.instance().listener().onMediaResume(seek);
        }
    }

    /**
     * 当前是否全屏状态
     *
     * @param activity 上下文对象
     * @return 当前是否全屏状态
     */
    @SuppressWarnings("ResourceType")
    public static boolean isFullState(Activity activity) {
        ViewGroup vp = (ViewGroup)
                (CommonUtil.scanForActivity(activity))
                        .findViewById(Window.ID_ANDROID_CONTENT);
        final View full = vp.findViewById(FULLSCREEN_ID);
        FunVideoPlayer funVideoPlayer = null;
        if (full != null) {
            funVideoPlayer = (FunVideoPlayer) full;
        }
        return funVideoPlayer != null;
    }

    /**
     * 退出全屏，主要用于返回键
     *
     * @param context 上下文
     * @return 返回是否全屏
     */
    @SuppressWarnings("ResourceType")
    public static boolean backFromWindowFull(Context context) {
        boolean backFrom = false;
        ViewGroup vp = (ViewGroup)
                (CommonUtil.scanForActivity(context))
                        .findViewById(Window.ID_ANDROID_CONTENT);
        View oldF = vp.findViewById(FULLSCREEN_ID);
        if (oldF != null) {
            backFrom = true;
            hideNavKey(context);

            // 退出全屏通知前一个监听对象
            if (FunMediaManager.instance().lastListener() != null) {
                FunMediaManager.instance().lastListener().onBackFullscreen();
            }
        }
        return backFrom;
    }
}
