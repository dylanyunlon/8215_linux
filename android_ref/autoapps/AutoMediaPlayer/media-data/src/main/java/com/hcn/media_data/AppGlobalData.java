package com.hcn.media_data;

import android.content.res.Configuration;
import android.text.TextUtils;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;

import com.hcn.media_base.constant.IConstant;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_data.storage.StorageDeviceEx;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.HMediaConfig;

import java.util.List;
import java.util.Objects;

/**
 * 全局数据对象
 * <pre>
 *    请不要随便在这里乱加东西（只可减少，不可增加）；
 *    全局变量会导致代码耦合性增强，会增加代码的熵值；
 *    在次警告，全局数据变量是毒药，别他妈啥都往这里添加，多动脑筋；
 * </pre>
 *
 * @author 65821
 */
public class AppGlobalData extends BaseMediaData {
    public static final String TAG = "HMediaPlayer";

    /** 唯一实例对象 **/
    private static final AppGlobalData S_INSTANCE = new AppGlobalData();
    public static AppGlobalData getInstance() {
        return S_INSTANCE;
    }

    /** 没使用到，好像没什么用处，后续可以删除它 **/
    public List<MusicInfo> mMusicListFiles = null;
    public boolean mIsControlPage = true;
    public boolean mIsMediaPlayerLocked = true;

    /** [文件不存在计数，管理是否需要自动扫描] **/
    public int mFileNotExistCount = 0;

    /**
     * 音乐列表页面类型
     * <pre>
     *    用来标记当前音乐列表页面显示的 Tab 标签；
     *    e.g. FLASH、SDCARD、USB、FOLDER...
     * </pre>
     */
    public volatile int mMusicListPageType = IMusicState.PAGE_INDEX_PLAY;

    /**
     * 外部跳转的播放路径与状态
     * <p> e.g. 文件管理器跳转过来的播放任务；
     */
    public String mSingleMusicFilePath = "";
    public boolean mSingleMusicPlay = false;
    public MusicRegInfo mMusicRegInfo = new MusicRegInfo();

    /**
     * 以下变量是视频播放相关全局变量
     * <p> 后续可以慢慢消灭掉这里面的部分垃圾对象；
     */
    public SurfaceHolder mFrontSurfaceHolder = null;
    public SurfaceHolder mRearSurfaceHolder = null;

    /** 视频在前台播放 **/
    public boolean isFrontVideo = true;

    /** 视频软解码标记 **/
    public boolean mSoftCodeFlag = false;
    public SurfaceHolder mFrontSurfaceHolderEx = null;

    public boolean mFullScreen = false;
    public boolean mDrivingWatchVideoEnable = true;
    public int mScreenBrightness = 0;

    /**
     * 是否在视频播放界面
     * <p> 视频播放界面和视频列表界面我们都认为在视频播放界面；
     */
    public boolean mInVideoPlayUi = false;
    public boolean mVideoUiShow = false;

    /**
     * 外部跳转的播放路径与状态
     * <p> e.g. 文件管理器跳转过来的播放任务；
     */
    public String mSingleVideoFilePath = "";
    public boolean mSingleVideoPlay = false;

    /**
     * [音乐界面]
     * <p> 音乐页面窗口显示大小、方向等；
     */
    public int mMusicUiWidth = 0;
    public int mMusicUiHeight = 0;
    public int mMusicUiOrientation = Configuration.ORIENTATION_UNDEFINED;

    /**
     * [视频界面]
     * <p> 视频页面窗口显示大小、方向、缩放类型等；
     */
    public int mVideoUiWidth = 0;
    public int mVideoUiHeight = 0;
    public int mVideoUiOrientation = Configuration.ORIENTATION_UNDEFINED;
    public int mVideoScaleType = HMediaConfig.VIDEO_SCALE_AUTO_ZOOM;

    /**
     * 是否可以显示 Toast 提示信息
     * <pre>
     *     现有设计是只要媒体页面未销毁就可以显示；
     *     那做只启动后台服务播放的情况怎么办？
     * </pre>
     */
    public boolean mShowToast = false;

    private AppGlobalData() {
        super();
    }

    /**
     * 判断是否是目标播放状态
     *
     * @param state 播放状态
     * @return 是/否
     */
    public boolean isPlayState(int state) {
        return mMediaPlayState == state;
    }

    /**
     * [由存储设备路径映射存储设备管理对象]
     *
     * @param path 存储设备路径/或者播放文件路径
     * @return 存储设备 {@link StorageDeviceEx}
     */
    public StorageDeviceEx getStorageDeviceFromPath(String path) {
        if (TextUtils.isEmpty(path)) {
            return null;
        }

        StorageDeviceEx mManager = null;
        if (path.contains(IConstant.PATH_FLASH)) {
            mManager = mFlashStorage;
        } else if (path.contains(IConstant.PATH_SD)) {
            mManager = mSdStorage;
        } else if (path.startsWith(IConstant.PATH_USB_PREFIX)) {
            mManager = mUsbStorage;
        }
        
        return mManager;
    }

    /**
     * 获取目标所在的存储设备对象
     * <p> 返回全局存储设备对象 StorageDeviceEx（内存数据对象）；
     *
     * @param musicInfo 媒体文件信息对象
     * @return {@link StorageDeviceEx}
     */
    public static StorageDeviceEx getStorageDeviceEx(MusicInfo musicInfo) {
        if (Objects.isNull(musicInfo)) {
            return null;
        }

        return getInstance().getStorageDeviceFromPath(musicInfo.mFilePath);
    }
}
