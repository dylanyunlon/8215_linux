package com.hcn.media_model;

import android.app.Application;

import androidx.annotation.NonNull;

import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_model.base.IDataModel;
import com.hcn.media_model.base.ILocalzModel;
import com.hcn.media_model.base.IMediaModel;
import com.hcn.media_model.base.IPlayerModel;
import com.hcn.media_model.base.IUiModel;
import com.hcn.media_model.base.MediaModule;
import com.hcn.media_model.impl.ManifestParser;

import java.util.List;
import java.util.Objects;

/**
 * 媒体模型
 * <p> 分层隔离用
 *
 * @author 65821
 */
public class MediaModel implements IMediaModel {
    private static final String TAG = MediaModel.class.getSimpleName();

    /** Model 必须是唯一实例设计 **/
    private static MediaModel sInstance = null;

    /** LocalzModel 对外接口实例 **/
    public static MediaModel call() {
        if (Objects.isNull(sInstance)) {
            throw new RuntimeException(
                    "Please initialize [MediaModel] Object!");
        }

        return sInstance;
    }

    /**
     * 初始化 MediaModel 实例
     * @param application 应用上下文
     */
    public static void init(@NonNull Application application) {
        if (Objects.isNull(sInstance)) {
            LogUtil.v(TAG, "init...");
            sInstance = new MediaModel(application);
        } else {
            throw new RuntimeException(
                    "[MediaLocalzModel] already initialized!");
        }
    }

    /** 多媒体模块对象 **/
    private final MediaModule mMediaModule;

    /** 禁止构造无参对象 **/
    private MediaModel() {
        throw new RuntimeException(
                "Prohibit the construction of parameterless objects");
    }

    /**
     * 构造函数
     * <p> 必须使用 Application 的上下文；
     * @param application 应用上下文
     */
    private MediaModel(@NonNull Application application) {
        List<MediaModule> manifestModules;
        manifestModules = new ManifestParser(application).parse();
        if (manifestModules.size() != 1) {
            throw new RuntimeException("AndroidManifest.xml file configuration exception!");
        }

        mMediaModule = manifestModules.get(0);
    }

    @Override
    public IUiModel uiModel() {
        return mMediaModule.uiModel();
    }

    @Override
    public ILocalzModel localzModel() {
        return mMediaModule.localzModel();
    }

    @Override
    public IPlayerModel playerModel() {
        return mMediaModule.playerModel();
    }

    @Override
    public IDataModel dataModel() {
        return mMediaModule.dataModel();
    }

    /**
     * 低内存處理
     * <p> 触发退出进程最低内存阈值时调用；
     */
    @Override
    public void onLowMemory(int reason) {
        uiModel().onLowMemory(reason);
        localzModel().onLowMemory(reason);
    }
}
