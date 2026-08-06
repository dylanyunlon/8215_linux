package com.hcn.media_data;

import android.annotation.SuppressLint;
import android.text.TextUtils;

import androidx.annotation.IntDef;
import androidx.annotation.NonNull;

import com.hcn.media_common.utils.MiscUtils;
import com.hcn.mediaservice.data.MusicInfo;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * 收藏列表管理器
 * <pre>
 *    1、收藏列表的存储、读取、清除；
 *    2、定时检查收藏列表内容的有效性；
 *    3、提供收藏列表相关接口；
 * </pre>
 *
 * @warn 考虑效率和读写安全，收藏列表最多容许收藏 256 首歌曲；
 * @author 65821
 */
public class FavoriteManager {

    /** 收藏列表类型 **/
    @Retention(RetentionPolicy.SOURCE)
    @IntDef({
            Type.NONE,
            Type.MUSIC,
            Type.VIDEO,
            Type.MAIN
    })

    public @interface Type {
        int NONE = -1;
        int MUSIC = 0;
        int VIDEO = 1;
        int MAIN = 2;
    }

    /** 当前唯一实例 **/
    private static FavoriteManager sInstance;

    /** 获取唯一实例 **/
    public static FavoriteManager getInstance() {
        if (sInstance == null) {
            sInstance = new FavoriteManager();
        }
        return sInstance;
    }

    /** 收藏列表的操作类型 **/
    public static final String OPERATE_INITED = "inited";
    public static final String OPERATE_ADD = "add";
    public static final String OPERATE_REMOVE = "remove";
    public static final String OPERATE_UPDATE = "update";
    public static final String OPERATE_MAX_LIMIT = "max-limit";

    /** 最大可收藏媒体个数 **/
    private static final int MAX_FAVORITE_INFO_THRESHOLD = 128;

    /** 列表初始化完成标记 **/
    private boolean[] mInitCompleted = new boolean[Type.MAIN];

    /**
     * 当前媒体收藏列表
     * <pre>
     *    收藏列表是针对全局存储设备的，所有歌曲点击收藏都归到此表中；
     *    收藏列表需要保存到数据库中，且需要做校准检查；
     *    收藏列表和播放列表都授予播放列表（都可以作为当前播放列表）
     * </pre>
     */
    private final List<MusicInfo> mFavoriteMusicList = new ArrayList<>();
    private final List<MusicInfo> mFavoriteVideoList = new ArrayList<>();

    /**
     * 外部监听对象
     * <p> 音频和视频单独监听，也可以设置 TypeMain 独立监听；
     */
    private List<IOperateListener> mMusicOperateListeners = null;
    private List<IOperateListener> mVideoOperateListeners = null;

    /**
     * 监听收藏列表相关事件
     * <pre>
     *    反馈列表操作动作；
     *    e.g. addList、removeList、saveList、initList...
     * </pre>
     */
    public interface IOperateListener {
        /**
         * 收藏列表事件反馈函数
         *
         * @param listType 列表类型 {@link Type}
         * @param operate “add” | "remove" | “save” | ...
         * @param obj0 额外附加数据对象（e.g. 移除列表所在索引）
         */
        void onFavoriteEvent(@Type int listType, String operate, Object obj0);
    }

    /**
     * 临时信息包
     * <p> 仅供内部业务逻辑使用；
     */
    public static final class InfoPackage {
        public MusicInfo info;
        public int index;

        InfoPackage(MusicInfo info, int index ) {
            this.info = info;
            this.index = index;
        }
    }

    /** 私有的构造函数 **/
    private FavoriteManager() {
    }

    /**
     * 数据列表是否初始化完成
     * <pre>
     *    收藏列表，先要去数据库中读取数据；
     *    只有读取过数据库了，我们才认为收藏数据列表已经初始化完成；
     * </pre>
     *
     * @param type 最喜欢的列表类型
     * @return 初始化完成与否
     */
    public boolean isInitCompleted(@Type int type) {
        if (type <= Type.NONE|| type >= Type.MAIN) {
            return false;
        }

        return mInitCompleted[type];
    }

    /**
     * 设置初始化完成状态
     *
     * @param type 最喜欢的列表类型
     * @param completed
     */
    public void setInitCompleted(@Type int type, boolean completed) {
        if (type <= Type.NONE|| type >= Type.MAIN) {
            return;
        }

        mInitCompleted[type] = completed;
        if (completed) {
            if (Objects.isNull(mMusicOperateListeners)) {
                return;
            }

            // 通知收藏列表初始化完成
            for (int i = mMusicOperateListeners.size() - 1; i >= 0; i--) {
                mMusicOperateListeners.get(i)
                        .onFavoriteEvent(type, OPERATE_INITED, null);
            }
        }
    }

    /**
     * 增加操作监听对象
     * <p> 监听收藏管理器中的操作动作与状态；
     *
     * @param type 最喜欢的列表类型
     * @param listener 监听对象
     */
    @SuppressLint("SwitchIntDef")
    public void addOperateListener(@Type int type,
                                   @NonNull IOperateListener listener) {
        switch (type) {
            case Type.MUSIC:
                if (Objects.isNull(mMusicOperateListeners)) {
                    mMusicOperateListeners = new ArrayList<>();
                    mMusicOperateListeners.add(listener);
                } else {
                    // 避免重复添加监听对象
                    if (!mMusicOperateListeners.contains(listener)) {
                        mMusicOperateListeners.add(listener);
                    }
                }
                break;
            case Type.VIDEO:
                if (Objects.isNull(mVideoOperateListeners)) {
                    mVideoOperateListeners = new ArrayList<>();
                    mVideoOperateListeners.add(listener);
                } else {
                    // 避免重复添加监听对象
                    if (!mVideoOperateListeners.contains(listener)) {
                        mVideoOperateListeners.add(listener);
                    }
                }
                break;
            default:
                break;
        }
    }

    /**
     * 移除监听对象
     *
     * @param type 最喜欢的列表类型
     * @param listener 监听对象
     */
    @SuppressLint("SwitchIntDef")
    public void removeOperateListener(@Type int type,
                                      @NonNull IOperateListener listener) {
        switch (type) {
            case Type.MUSIC:
                if (!Objects.isNull(mMusicOperateListeners)) {
                    mMusicOperateListeners.remove(listener);
                }
                break;
            case Type.VIDEO:
                if (!Objects.isNull(mVideoOperateListeners)) {
                    mVideoOperateListeners.remove(listener);
                }
                break;
            default:
                break;
        }
    }

    /** 最喜欢的音乐播放列表 **/
    public @NonNull List<MusicInfo> favoriteMusicList() {
        return mFavoriteMusicList;
    }

    /** 最喜欢的视频播放列表 **/
    public @NonNull List<MusicInfo> favoriteVideoList() {
        return mFavoriteVideoList;
    }

    /**
     * 初始化最喜欢的音乐列表
     * @param list 数据列表
     */
    public void initFavoriteMusicList(@NonNull List<MusicInfo> list) {
        mFavoriteMusicList.clear();
        mFavoriteMusicList.addAll(list);
    }

    /**
     * 初始化最喜欢的视频列表
     * @param list 数据列表
     */
    public void initFavoriteVideoList(@NonNull List<MusicInfo> list) {
        mFavoriteVideoList.clear();
        mFavoriteVideoList.addAll(list);
    }

    /**
     * 更新最喜欢的音乐列表
     * @param list 数据列表
     */
    public void updateFavoriteMusicList(@NonNull List<MusicInfo> list) {
        mFavoriteMusicList.clear();
        mFavoriteMusicList.addAll(list);

        // 通知更新成功（外部可刷新显示列表）
        if (mMusicOperateListeners != null) {
            for (int i = mMusicOperateListeners.size() - 1; i >= 0; i--) {
                mMusicOperateListeners.get(i)
                        .onFavoriteEvent(Type.MUSIC, OPERATE_UPDATE, null);
            }
        }
    }

    /**
     * 更新最喜欢的视频列表
     * @param list 数据列表
     */
    public void updateFavoriteVideoList(@NonNull List<MusicInfo> list) {
        mFavoriteVideoList.clear();
        mFavoriteVideoList.addAll(list);

        // 通知更新成功（外部可刷新显示列表）
        if (mVideoOperateListeners != null) {
            for (int i = mVideoOperateListeners.size() - 1; i >= 0; i--) {
                mVideoOperateListeners.get(i)
                        .onFavoriteEvent(Type.VIDEO, OPERATE_UPDATE, null);
            }
        }
    }

    /**
     * 增加目标歌曲到最喜欢的列表
     * <p> 不要随意强制添加，除非你确定它不在列表中；
     *
     * @param info 目标歌曲
     * @param force 强制添加
     */
    public boolean addFavoriteMusic(MusicInfo info, boolean force) {
        if (Objects.isNull(info)
                || TextUtils.isEmpty(info.mFilePath)) {
            return false;
        }

        if (force || !inFavoriteMusicList(info)) {
            // 超过最大收藏个数，不再添加（可以做提示信息）
            if (!force && mFavoriteMusicList.size() > MAX_FAVORITE_INFO_THRESHOLD) {
                // 通知已到最大收藏阈值
                if (mMusicOperateListeners != null) {
                    for (int i = mMusicOperateListeners.size() - 1; i >= 0; i--) {
                        mMusicOperateListeners.get(i)
                                .onFavoriteEvent(Type.MUSIC,
                                        OPERATE_MAX_LIMIT, null);
                    }
                }
                return false;
            }

            mFavoriteMusicList.add(info);

            // 通知增加成功（外部可刷新显示列表）
            if (mMusicOperateListeners != null) {
                for (int i = mMusicOperateListeners.size() - 1; i >= 0; i--) {
                    int index = mFavoriteMusicList.size() - 1;
                    mMusicOperateListeners.get(i)
                            .onFavoriteEvent(Type.MUSIC,
                                    OPERATE_ADD, new InfoPackage(info, index));
                }
            }
        }

        return true;
    }

    /**
     * 增加目标视频到最喜欢的列表
     * <p> 不要随意强制添加，除非你确定它不在列表中；
     *
     * @param info 目标视频
     * @param force 强制添加
     */
    public void addFavoriteVideo(MusicInfo info, boolean force) {
        if (Objects.isNull(info)
                || TextUtils.isEmpty(info.mFilePath)) {
            return;
        }

        if (force || !inFavoriteVideoList(info)) {
            // 超过最大收藏个数，不再添加（可以做提示信息）
            if (!force && mFavoriteVideoList.size() > MAX_FAVORITE_INFO_THRESHOLD) {
                // 通知已到最大收藏阈值
                if (mVideoOperateListeners != null) {
                    for (int i = mVideoOperateListeners.size() - 1; i >= 0; i--) {
                        mVideoOperateListeners.get(i)
                                .onFavoriteEvent(Type.VIDEO,
                                        OPERATE_MAX_LIMIT, null);
                    }
                }
                return;
            }

            mFavoriteVideoList.add(info);

            if (mVideoOperateListeners != null) {
                for (int i = mVideoOperateListeners.size() - 1; i >= 0; i--) {
                    int index = mFavoriteVideoList.size() - 1;
                    mVideoOperateListeners.get(i)
                            .onFavoriteEvent(Type.VIDEO,
                                    OPERATE_ADD, new InfoPackage(info, index));
                }
            }
        }
    }

    /** 是否在最喜欢的音乐列表中 **/
    public boolean inFavoriteMusicList(MusicInfo target) {
        if (Objects.isNull(target)) {
            return false;
        }

        return inFavoriteList(target, mFavoriteMusicList) != null;
    }

    /** 是否在最喜欢的视频列表中 **/
    public boolean inFavoriteVideoList(MusicInfo target) {
        if (Objects.isNull(target)) {
            return false;
        }

        return inFavoriteList(target, mFavoriteVideoList) != null;
    }

    /**
     * 是否在最喜欢的媒体列表中
     *
     * @param target 目标对象
     * @param favoriteList 参考列表
     * @return 不为空在/为空不在
     */
    private InfoPackage inFavoriteList(
            @NonNull MusicInfo target, List<MusicInfo> favoriteList) {
        // 文件名有效性检查
        if (TextUtils.isEmpty(target.mFilePath)) {
            return null;
        }

        // 根据用户喜欢反向查找更加科学
        MusicInfo tempInfo;
        int size = favoriteList.size();
        for (int index = size-1; index > -1; index--) {
            tempInfo = favoriteList.get(index);

            // 反向比较效率高（99% 的情况只需要比较一个字符）
            if (MiscUtils.reverseEquals(
                    target.mFilePath,  tempInfo.mFilePath)) {
                return new InfoPackage(tempInfo, index);
            }
        }
        return null;
    }

    /**
     * 从音乐收藏列表中移除当前目标对象
     *
     * @param info 媒体信息对象
     */
    public void removeFavoriteMusic(MusicInfo info) {
        if (Objects.isNull(info)) {
            return;
        }

        InfoPackage tempInfo = inFavoriteList(info, mFavoriteMusicList);
        if (tempInfo != null) {
            mFavoriteMusicList.remove(tempInfo.info);

            if (mMusicOperateListeners != null) {
                for (int i = mMusicOperateListeners.size() - 1; i >= 0; i--) {
                    mMusicOperateListeners.get(i)
                            .onFavoriteEvent(Type.MUSIC, OPERATE_REMOVE, tempInfo);
                }
            }
        }
    }

    /**
     * 从视频收藏列表中移除当前目标对象
     *
     * @param info 媒体信息对象
     */
    public void removeFavoriteVideo(MusicInfo info) {
        if (Objects.isNull(info)) {
            return;
        }

        InfoPackage tempInfo = inFavoriteList(info, mFavoriteVideoList);
        if (tempInfo != null) {
            mFavoriteVideoList.remove(tempInfo.info);

            if (mVideoOperateListeners != null) {
                for (int i = mVideoOperateListeners.size() - 1; i >= 0; i--) {
                    mVideoOperateListeners.get(i)
                            .onFavoriteEvent(Type.VIDEO, OPERATE_REMOVE, tempInfo);
                }
            }
        }
    }
}
