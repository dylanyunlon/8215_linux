package com.hcn.media_data.base;

import android.os.Build;

import com.hcn.common.utils.HUtilsEx;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.folder.FilePathScanManager;
import com.hcn.media_data.storage.IStorageDevice;
import com.hcn.media_data.storage.StorageDeviceEx;
import com.hcn.mediaservice.data.MediaTimeInfo;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * 当前应用基础数据
 * <pre>
 *    这些成员变量是多媒体进程必须常驻内存的关键数据信息与状态；
 *    AppGlobalData 有些混乱了，我们使用继承关系剥离下重要的数据，方便管理；
 * </pre>
 *
 * @author 65821
 */
public abstract class BaseMediaData implements IConstant {
    /**
     * [是 64 位系统]
     * <p> 通过读取 "ro.product.cpu.abi" 判定；
     */
    private boolean mIsArm64Bit = false;

    /**
     * 当前进程的 UID
     * <pre>
     *    由于高版本发送广播、绑定服务等都需要 UserHandle 对象；
     *    但是 UserHandle 对象又只能通过当前进程的 UID 获取到；
     * <pre>
     */
    public static int UID = -1;

    /**
     * 是否在媒体低内存状态
     * <p> 低内存状态，我们将释放一些资源，限制一些功能；
     */
    protected static boolean sLowMemory = false;

    /**
     * 是否在低内存状态
     * <p> 低内存状态需要做一些功能限制；
     *
     * @return 是低内存/正常状态
     */
    public static boolean isLowMemory() {
        return sLowMemory;
    }

    /**
     * 更新低内存状态
     * @param lowMemory 内存状态
     */
    public static void updateLowMemory(boolean lowMemory) {
        sLowMemory = lowMemory;
    }

    /**
     * 存储类型变量
     * <p> 记录上一次播放存储类型和当前播放存储类型；
     */
    public int mLastMediaType = -1;
    public int mMediaType = IMusicState.MEDIA_TYPE_MUSIC;

    /**
     * 当前支持的存储设备
     * <p> 车载只支持：内置存储、SDCard、USB三种存储设备；
     */
    public StorageDeviceEx mFlashStorage;
    public StorageDeviceEx mSdStorage;
    public StorageDeviceEx mSd2Storage;
    public StorageDeviceEx mUsbStorage;
    public List<StorageDeviceEx> mStorageDeviceList;

    /**
     * 当前播放存储设备
     * <pre>
     *    这个当前播放存储设备与收藏列表没有关系；
     *    注意：收藏列表是全局的，它可以包含任意存储设备的文件信息；
     * </pre>
     */
    public StorageDeviceEx mCurrentDevice;

    /**
     * 列表选中的那一个播放存储设备
     * <p> 选择的不一定是正在播放的，选择或许只是为了查看信息；
     */
    public StorageDeviceEx mSelectedDevice;
    public StorageDeviceEx mSearchClickDevice = null;

    /**
     * 搜索界面列表数据
     * <p> 暂时只看到音乐模块使用了，视频未使用它（莫名其妙的设计）；
     */
    public List<MusicInfo> mSearchList = new ArrayList<>();

    /**
     * 当前音乐播放列表管理对象
     * <pre>
     *     用来管理日益复杂的播放列表需求（多播放列表切换）;
     *     e.g. 主播放列表、收藏播放列表等等；
     * </pre>
     */
    protected MusicPlaylistEx mMusicPlaylistEx = new MusicPlaylistEx();

    /**
     * 当前视频播放列表管理对象
     * <pre>
     *     用来管理日益复杂的播放列表需求（多播放列表切换）;
     *     e.g. 主播放列表、收藏播放列表等等；
     * </pre>
     */
    protected VideoPlaylistEx mVideoPlaylistEx = new VideoPlaylistEx();

    /**
     * 当前播放媒体相关信息（音视频共用）
     * <pre>
     *    注意这些变量只能在主线程使用；
     *    当前媒体播放信息对象、播放状态、播放进度；
     * </pre>
     */
    public MusicInfo mCurrentMediaInfo = null;
    public int mMediaPlayState = IMusicState.E_PLAY_STATE_STOP;
    public final MediaTimeInfo mPlayTimeInfo = new MediaTimeInfo();

    /**
     * 当前音乐播放对象的标签索引（播放对象的标签）
     * <p> 这是一个设计非常不合理且没有必要的成员变量，后续建议替换掉；
     * @deprecated 过时的接口变量，少用；
     */
    public int mMusicPlayIndex = -1;

    /**
     * 当前视频播放对象的标签索引（播放对象的标签）
     * <p> 这是一个设计非常不合理且没有必要的成员变量，后续建议替换掉；
     * @deprecated 过时的接口变量，少用；
     */
    public int mVideoPlayIndex = -1;

    /**
     * 文件夹列表浏览相关变量
     * <p> 记录当前文件夹路径、以及路径扫描管理对象；
     */
    public String mCurrentFilePath = "";
    public final FilePathScanManager mFilePathScanManager;

    /**
     * 容许恢复播放状态标记
     * <pre>
     *    很多情况下暂停/停止播放后不容许恢复播放；
     *    也有部分情况容许恢复播放，例如：
     *      1、音频焦点丢失场景；
     *      2、刹车状态监测场景；
     *      3、藍牙通話状态场景；
     *      4、ACC-OFF/ON 状态场景等；
     * </pre>
     */
    public boolean mAllowResumePlay = false;

    /**
     * 判断索引在指定的列表中是否有效
     * <p> 这是一个列表数据的越界判断函数；
     *
     * @param targetList 列表源
     * @param index 索引
     * @return 有效/无效
     */
    public static boolean isValidIndex(List<MusicInfo> targetList, int index) {
        if (Objects.isNull(targetList) || index < 0) {
            return false;
        }

        return index < targetList.size();
    }

    // 当前实例（唯一实例）
    private static BaseMediaData S_INSTANCE = null;

    /**
     * 单纯方便访问使用
     * @return {@link BaseMediaData}
     */
    public static BaseMediaData call() {
        if (Objects.isNull(S_INSTANCE)) {
            throw new RuntimeException("u can't instantiate me...");
        }

        return S_INSTANCE;
    }

    /** 默认无参构造函数 **/
    public BaseMediaData() {
        super();

        S_INSTANCE = this;

        // 系统 abi 接口检查
        if (Build.SUPPORTED_64_BIT_ABIS.length != 0) {
            mIsArm64Bit = Build.SUPPORTED_64_BIT_ABIS[0].equals(
                    HUtilsEx.getSystemProperty("ro.product.cpu.abi", "armeabi"));
        }

        // 创建存储类型设备对象
        mSd2Storage = null;
        mFlashStorage = new StorageDeviceEx(PATH_FLASH, IStorageDevice.STORAGE_TYPE_FLASH);
        mSdStorage = new StorageDeviceEx(PATH_SD, IStorageDevice.STORAGE_TYPE_SDCARD);
        mUsbStorage = new StorageDeviceEx(PATH_USB, IStorageDevice.STORAGE_TYPE_USB);
        mStorageDeviceList = new ArrayList<>();

        // 创建文件扫描管理对象
        mFilePathScanManager = new FilePathScanManager(mCurrentFilePath);

        // 列表操作记忆相关对象
        mCurrentDevice = mFlashStorage;
        mSelectedDevice = mFlashStorage;
        mCurrentFilePath = mSelectedDevice.mFilePath;
    }

    /**
     * 当前是否是 64 位系统
     * <pre>
     *    64 位系统就需要加载 64 位的播放库；
     *    VitamioPlayer 64 位的播放解码库应该是有些问题；
     * </pre>
     *
     * @return 是/否
     */
    public boolean is64BitOS() {
        return mIsArm64Bit;
    }

    /**
     * 判断当前媒体类型是否是指定目标类型
     * <pre>
     *     注意音乐和视频不可能同时在播放状态；
     *     所以当前媒体类型要么在 MUSIC，要么在 VIDEO，或者是 IDLE 状态；
     * </pre>
     *
     * @param type {@link IMusicState#MEDIA_TYPE_MUSIC/VIDEO/IDLE}
     * @return 是/否
     */
    public boolean isMediaType(int type) {
        return mMediaType == type;
    }

    /**
     * 媒体类型字符串
     * @return 'music' / 'video' / 'none'
     */
    public String mediaType() {
        switch (mMediaType) {
            case IMusicState.MEDIA_TYPE_MUSIC:
                return "music";
            case IMusicState.MEDIA_TYPE_VIDEO:
                return "video";
            default:
                return "none";
        }
    }

    /**
     * 当前正在播放的媒体信息对象
     * <pre>
     *    尽量不要直接使用成员变量访问；
     *    音视频不可能同时播放，所以他们共用同一个播放媒体信息对象；
     *    非播放状态，它将返回 null 对象（上下曲切换间隙也是非播放状态）；
     * </pre>
     *
     * @return {@link MusicInfo}
     */
    public MusicInfo currentMediaInfo() {
        return mCurrentMediaInfo;
    }

    /** [暂停状态不进入 PIP] **/
    public boolean isPauseStatus() {
        return (IMusicState.E_PLAY_STATE_PAUSE == mMediaPlayState);
    }

    /**
     * 播放状态字符串
     * @return 'play' / 'pause' / 'stop'
     */
    public String mediaPlayState() {
        switch (mMediaPlayState) {
            case IMusicState.E_PLAY_STATE_PLAY:
                return "play";
            case IMusicState.E_PLAY_STATE_PAUSE:
                return "pause";
            case IMusicState.E_PLAY_STATE_STOP:
            default:
                return "stop";
        }
    }

    /* ------------------------------------ 音乐播放列表相关接口 ------------------------------------ */

    /** 获取当前音乐播放列表类型 **/
    public int musicPlayListType() {
        return mMusicPlaylistEx.playListType();
    }

    /** 获取音乐第一播放列表类型 **/
    public int musicFirstPlayListType() {
        return mMusicPlaylistEx.firstPlayListType();
    }

    /** 获取音乐第一播放列表 **/
    public List<MusicInfo> musicFirstPlaylist() {
        return mMusicPlaylistEx.firstPlaylist();
    }

    /** 获取当前音乐循环播放模式 **/
    public int musicRepeatMode() {
        return mMusicPlaylistEx.repeatMode();
    }

    /** 是否是期望的音乐循环模式 **/
    public boolean isMusicRepeatMode(int mode) {
        return mMusicPlaylistEx.repeatMode() == mode;
    }

    /** 设置当前音乐循环播放模式 **/
    public void setMusicRepeatMode(int mode) {
        mMusicPlaylistEx.setRepeatMode(mode);
    }

    /** 获取当前音乐播放列表 **/
    public List<MusicInfo> musicPlaylist() {
        return mMusicPlaylistEx.playList();
    }

    /** 更新当前音乐播放列表 **/
    public void updateMusicPlaylist(@IPlaylistType int type, List<MusicInfo> list) {
        mMusicPlaylistEx.updatePlaylist(type, list);
    }

    /** 获取当前音乐播放位置 **/
    public int musicPlayPosition() {
        return mMusicPlaylistEx.playPosition();
    }

    /** 更新当前音乐播放位置 **/
    public void updateMusicPlayPosition(int position) {
        mMusicPlaylistEx.updatePlayPosition(position, true);
    }

    /** 更新当前音乐播放位置（非安全的） **/
    public void updateMusicPlayPosition(int position, boolean check) {
        mMusicPlaylistEx.updatePlayPosition(position, check);
    }

    /** 获取当前音乐播放列表播放位置信息 **/
    public MusicInfo musicPlayPositionInfo() {
        return mMusicPlaylistEx.playPositionInfo();
    }

    /** 获取当前音乐播放列表随机位置列表 **/
    public List<Integer> musicRandomPositionList() {
        return mMusicPlaylistEx.randomPositionList();
    }

    /** 获取当前音乐列表选择位置 **/
    public int musicSelectPosition() {
        return mMusicPlaylistEx.selectPosition();
    }

    /** 设置当前音乐列表选择位置 **/
    public void updateMusicSelectPosition(int position) {
        mMusicPlaylistEx.updateSelectPosition(position);
    }

    /** 更新音乐播放列表对应的随机位置列表 **/
    public void updateMusicRandomPositionList() {
        mMusicPlaylistEx.updateRandomPositionList();
    }

    /** 从音乐随机位置列表中移除目标索引（随机播放模式才会被移除成功） **/
    public void removeFromMusicRandomPositionList(int position) {
        mMusicPlaylistEx.removeFromRandomPositionList(position);
    }

    /** 获取音乐下一个随机播放位置（相对当前音乐播放列表） **/
    public int getMusicNextRandomPlayPosition() {
        return mMusicPlaylistEx.getNextRandomPosition();
    }

    /** 根据上下切曲动作更新音乐播放位置（相对音乐播放列表） **/
    public void adjustMusicPlayPosition(boolean isNextSong) {
        mMusicPlaylistEx.adjustPlayPosition(isNextSong);
    }

    /* ------------------------------------ 视频播放列表相关接口 ------------------------------------ */

    /** 获取当前视频循环播放模式 **/
    public int videoRepeatMode() {
        return mVideoPlaylistEx.repeatMode();
    }

    /** 是否是期望的视频循环模式 **/
    public boolean isVideoRepeatMode(int mode) {
        return mVideoPlaylistEx.repeatMode() == mode;
    }

    /** 设置当前视频循环播放模式 **/
    public void setVideoRepeatMode(int mode) {
        mVideoPlaylistEx.setRepeatMode(mode);
    }

    /** 获取当前视频播放列表 **/
    public List<MusicInfo> videoPlaylist() {
        return mVideoPlaylistEx.playList();
    }

    /** 更新当前视频播放列表 **/
    public void updateVideoPlaylist(List<MusicInfo> list) {
        mVideoPlaylistEx.updatePlaylist(list);
    }

    /** 获取当前视频播放位置 **/
    public int videoPlayPosition() {
        return mVideoPlaylistEx.playPosition();
    }

    /** 更新当前视频播放位置 **/
    public void updateVideoPlayPosition(int position) {
        mVideoPlaylistEx.updatePlayPosition(position);
    }

    /** 更新当前视频播放位置（非安全的） **/
    public void updateVideoPlayPosition(int position, boolean check) {
        mVideoPlaylistEx.updatePlayPosition(position, check);
    }

    /** 获取当前视频播放列表播放位置信息 **/
    public MusicInfo videoPlayPositionInfo() {
        return mVideoPlaylistEx.playPositionInfo();
    }

    /** 获取当前视频播放列表随机位置列表 **/
    public List<Integer> videoRandomPositionList() {
        return mVideoPlaylistEx.randomPositionList();
    }

    /** 更新视频播放列表对应的随机位置列表 **/
    public void updateVideoRandomPositionList() {
        mVideoPlaylistEx.updateRandomPositionList();
    }

    /** 从视频随机位置列表中移除目标索引（随机播放模式才会被移除成功） **/
    public void removeFromVideoRandomPositionList(int position) {
        mVideoPlaylistEx.removeFromRandomPositionList(position);
    }

    /** 获取视频下一个随机播放位置（相对当前视频播放列表） **/
    public int getVideoNextRandomPlayPosition() {
        return mVideoPlaylistEx.getNextRandomPosition();
    }

    /** 根据上下切曲动作更新视频播放位置（相对视频播放列表） **/
    public void adjustVideoPlayPosition(boolean isNextSong) {
        mVideoPlaylistEx.adjustPlayPosition(isNextSong);
    }

    /* end -------------------------------- 视频播放列表相关接口 -------------------------------- end */
}
