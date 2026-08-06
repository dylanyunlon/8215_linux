package com.hcn.media_model.base;

import com.hcn.media_data.FavoriteManager;
import com.hcn.mediaservice.data.MusicInfo;

/**
 * 数据模型接口
 * <pre>
 *    用来处理需要保存的数据信息；
 *    e.g. 存储恢复收藏列表、播放列表、播放状态等；
 * </pre>
 * @author 65821
 */
public interface IDataModel {
    /**
     * 增加收藏信息
     * @param info 媒体信息对象
     * @param type 最喜欢的媒体类型
     */
    void insertFavoriteInfo(@FavoriteManager.Type int type, MusicInfo info);

    /**
     * 删除收藏信息
     * @param info 媒体信息对象
     * @param type 最喜欢的媒体类型
     */
    void removeFavoriteInfo(@FavoriteManager.Type int type, MusicInfo info);

    /**
     * 关闭数据模型
     * <p> 数据模型涉及到异步任务，需要对外提供关闭接口；
     */
    void closeDataModel();
}
