package com.hcn.media_data.storage;

import com.hcn.media_data.folder.MediaLoadState;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * 存储设备信息
 * <pre>
 *    这是一个存储设备数据结构体；
 *    保存一些与当前存储设备相关的数据和状态；
 *    并提供一些简单的判定方法；
 * </pre>
 *
 * @author 65821
 */
public class StorageDeviceEx implements IStorageDevice {
    private int mStorageType;
    public String mFilePath;

    private boolean mIsLoading = false;
    private boolean mIsMounted = false;

    public MediaLoadState mFileScanState;
    public MediaLoadState mID3ParseState;

    public List<MusicInfo> mMusicInfoList;
    public List<MusicInfo> mVideoInfoList;
    public List<MusicInfo> mMusicFavoriteList;

    public HashMap<String, List<MusicInfo>> mAlbumListMap;
    public HashMap<String, List<MusicInfo>> mArtistListMap;
    public HashMap<String, List<MusicInfo>> mPathListMap;

    public StorageDeviceEx(String filePath, int storageType) {
        mFilePath = filePath;
        mStorageType = storageType;

        mFileScanState = new MediaLoadState();
        mID3ParseState = new MediaLoadState();

        mMusicInfoList = new ArrayList<>();
        mVideoInfoList = new ArrayList<>();
        mMusicFavoriteList = new ArrayList<>();

        mAlbumListMap = new HashMap<>();
        mArtistListMap = new HashMap<>();
        mPathListMap = new HashMap<>();
    }

    /** 当前在扫描中 **/
    public boolean isLoading() {
        return mIsLoading;
    }

    /** 当前是挂载的 **/
    public boolean isMounted() {
        return mIsMounted;
    }

    /**
     * 更新加载扫描状态
     * @param loading 加载
     */
    public void updateLoading(boolean loading) {
        mIsLoading = loading;
    }

    /**
     * 更新设备挂载状态
     * @param mounted 挂载
     */
    public void updateMounted(boolean mounted) {
        mIsMounted = mounted;
    }

    /**
     * 当前存储设备类型
     * @return {@link IStorageDevice#STORAGE_TYPE_USB ...}
     */
    public int storageType() {
        return mStorageType;
    }

    public boolean isFlash() {
        return (IStorageDevice.STORAGE_TYPE_FLASH == mStorageType);
    }

    public boolean isUsb() {
        return (IStorageDevice.STORAGE_TYPE_USB == mStorageType);
    }

    public boolean isSdcard() {
        return (IStorageDevice.STORAGE_TYPE_SDCARD == mStorageType);
    }

    public String getFilePath() {
        return mFilePath;
    }

    public List<MusicInfo> getMusicInfoList() {
        return mMusicInfoList;
    }

    public List<MusicInfo> getVideoInfoList() {
        return mVideoInfoList;
    }

    /**
     * [当前存储设备是否存在有效媒体数据]
     * <p> 当音乐和视频都没有的时候，不存在有效媒体源；
     *
     * @return 有/无
     */
    public boolean existValidMediaInfo() {
        if (isMounted()) {
            boolean noMusic = mMusicInfoList.isEmpty();
            boolean noVideo = mVideoInfoList.isEmpty();

            return (!noMusic || !noVideo);
        }

        return false;
    }

    public void clear() {
        mMusicInfoList.clear();
        mVideoInfoList.clear();
        mMusicFavoriteList.clear();

        mAlbumListMap.clear();
        mArtistListMap.clear();
        mPathListMap.clear();
    }
}
