package com.hcn.media_model.base;

/**
 * 类型定义
 * @author 65821
 */
public interface MediaModule {
    /**
     * Ui 模型接口
     * @return {@link IUiModel}
     */
    IUiModel uiModel();

    /**
     * 本地交互模型接口
     * @return {@link ILocalzModel}
     */
    ILocalzModel localzModel();

    /**
     * 播放模型接口
     * @return {@link IPlayerModel}
     */
    IPlayerModel playerModel();

    /**
     * 数据模型接口
     * @return {@link IDataModel}
     */
    IDataModel dataModel();
}
