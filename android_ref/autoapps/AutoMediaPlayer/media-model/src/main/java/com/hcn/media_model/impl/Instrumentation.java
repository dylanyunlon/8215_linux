package com.hcn.media_model.impl;

import android.app.Application;
import android.content.Context;

import androidx.annotation.NonNull;

import com.hcn.media_model.base.IDataModel;
import com.hcn.media_model.eq.EQMediaController;
import com.hcn.media_model.base.ILocalzModel;
import com.hcn.media_model.base.MediaModule;
import com.hcn.media_model.base.IPlayerModel;
import com.hcn.media_model.base.IUiModel;

/**
 * 模型初始化工具
 * <p> 禁止外部直接调用；
 *
 * @author 65821
 */
public class Instrumentation implements MediaModule {
    /** 模型接口 **/
    private IUiModel mUiModel;
    private ILocalzModel mLocalzModel;
    private IPlayerModel mPlayerModel;
    private IDataModel mDataModel;

    /** 构造函数 **/
    public Instrumentation(@NonNull Application application) {
        init(application);
    }

    /**
     * 初始化进程相关 Model
     * @param context 上下文环境
     */
    private void init(@NonNull Context context) {
        // 初始化所有 Model
        MediaUiModel.init(context);
        MediaLocalzModel.init(context, MediaUiModel.instance());
        MediaPlayerModel.init(context, MediaLocalzModel.instance());
        MediaDataModel.init(context);

        mUiModel = MediaUiModel.instance();
        mLocalzModel = MediaLocalzModel.instance();
        mPlayerModel = MediaPlayerModel.instance();
        mDataModel = MediaDataModel.instance();

        // 设置 EQ 改变监听器
        EQMediaController.init(context, mLocalzModel);
        MediaLocalzModel.instance()
                .setEqChangeListener(EQMediaController.instance());
    }

    @Override
    public IUiModel uiModel() {
        return mUiModel;
    }

    @Override
    public ILocalzModel localzModel() {
        return mLocalzModel;
    }

    @Override
    public IPlayerModel playerModel() {
        return mPlayerModel;
    }

    @Override
    public IDataModel dataModel() {
        return mDataModel;
    }
}
