package com.hcn.media_data.folder;

import android.os.SystemClock;

import com.hcn.mediaservice.data.MusicInfo;

import java.util.ArrayList;
import java.util.List;

/**
 * 文件路径扫描管理器
 * <p> 为文件夹访问方式提供数据结构支持；
 *
 * @author 65821
 */
public class FilePathScanManager {

    /** 当前所在文件路径 */
    public String mFilePath = "";

    /** 当前数据对象标记 */
    public long mObjectTag = -1;

    public boolean mIsLoading = false;

    public List<MusicInfo> mMusicInfoList;

    /** [Folder]存当前路径下的音乐媒体文件 **/
    public List<MusicInfo> mMusicOnlyList;

    /** [Folder]存当前路径下的文件夹 **/
    public List<MusicInfo> mMediaFolderList;

    public List<MusicInfo> mVideoInfoList;

    /** [Folder]存当前路径下的媒体文件 **/
    public List<MusicInfo> mVideoOnlyList;

    public MediaLoadState mMediaPathState;

    public final String SCAN_MUSIC_FILE_TYPE = "music";
    public final String SCAN_VIDEO_FILE_TYPE = "video";

    public FilePathScanManager(String filePath) {
        mFilePath = filePath;
        mObjectTag = SystemClock.elapsedRealtime();

        mMediaPathState = new MediaLoadState();
        mMusicInfoList = new ArrayList<>();
        mVideoInfoList = new ArrayList<>();
        mMusicOnlyList = new ArrayList<>();
        mVideoOnlyList = new ArrayList<>();
        mMediaFolderList = new ArrayList<>();
    }

    public long getObjectTag() {
        return mObjectTag;
    }

    public boolean isLoading() {
        return mIsLoading;
    }

    public String getFilePath() {
        return mFilePath;
    }

    public List<MusicInfo> getMusicInfoList() {
        return mMusicInfoList;
    }
}
