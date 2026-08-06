package com.hcn.media.folder;

import android.annotation.SuppressLint;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;

import com.hcn.common.concurrent.NamedThreadFactory;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_data.folder.FilePathScanManager;
import com.hcn.media_data.folder.MediaLoadState;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_common.file.FileSortHelper;

import java.io.File;
import java.util.Comparator;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * 媒体文件路径扫描器
 * <p> 管理执行指定文件节点的扫描任务
 *
 * @author 86158
 */
public class MediaFilePathScan {
    private static final String TAG = MediaFilePathScan.class.getSimpleName();

    /** 需要过滤的文件后缀 **/
    private final static String MUSIC_SUFFIX = ".mp3.wav.aac.flac.ogg.m4a.ape.amr.mid.mp2.";
    private final static String VIDEO_SUFFIX = ".mp4.avi.3gp.m4v.mkv.mov.3gpp.3g2.3gpp2.webm.f4v.mts.m2ts.mpg.";
    public static final String DEFAULT_ROOT = "/storage";
    public StringBuilder MUSIC_SUFFIX_EX;

    /**
     * 扫描承储模式
     * <p> 所有存储设备显示在一个列表中
     */
    public static final int ALL_STORAGE_MODE = 0x00;

    /**
     * 扫描承储模式
     * <p> 列表只显示某一个存储设备里的内容
     */
    public static final int SINGLE_STORAGE_MODE = 0x01;

    /** 扫描承储模式 **/
    private static int mMode = ALL_STORAGE_MODE;

    /** 媒体文件节点扫描线程池 **/
    private final ExecutorService mScanThreadPool;
    private final FileSortHelper mFileSortHelper = new FileSortHelper();

    /** 媒体文件节点扫描完成消息定义 **/
    private final int EVENT_PATH_SCAN_FINISHED = 0;

    /** 当前类的唯一实例对象 **/
    private static MediaFilePathScan mInstance = null;

    public static MediaFilePathScan getInstance() {
        if (null == mInstance) {
            mInstance = getInstance(ALL_STORAGE_MODE);
        }
        return mInstance;
    }

    public static MediaFilePathScan getInstance(int mode) {
        mMode = mode;
        if (null == mInstance) {
            mInstance = new MediaFilePathScan();
        }
        return mInstance;
    }

    private MediaFilePathScan() {
        // 创建一个线程池
        mScanThreadPool = new ThreadPoolExecutor(0, 1,
                60L, TimeUnit.SECONDS, new LinkedBlockingQueue<>(),
                new NamedThreadFactory("file-scan") {

            @Override
            public Thread newThread(Runnable r) {
                Thread thread =  super.newThread(r);
                int priority = thread.getPriority();
                if (priority != Thread.NORM_PRIORITY) {
                    thread.setPriority(Thread.NORM_PRIORITY);
                }
                return thread;
            }
        });

        // 根据不同平台调整格式的过滤
        MUSIC_SUFFIX_EX = new StringBuilder(MUSIC_SUFFIX);
        String FOTA_NAME = HUtilsEx.getSystemProperty("ro.fota.device", "None");
        if (FOTA_NAME.equals("rk3326_pie")) {
            MUSIC_SUFFIX_EX.append("wma.");
        }
    }

    @SuppressLint("HandlerLeak")
    public void loadMediaPathList(final String path,
                                  final MediaLoadState state,
                                  final int index,
                                  final String scanFileType,
                                  final IMediaFilePathScanCallBack callBack) {
        // 路径有效性检查
        if (TextUtils.isEmpty(path)) {
            return;
        }

        // 处理扫描状态（主线程）
        final Handler handler = new Handler(Looper.getMainLooper()) {

            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                if (msg.obj instanceof FilePathScanManager) {
                    if (msg.what == EVENT_PATH_SCAN_FINISHED) {
                        callBack.onPathScanFinishedEx((FilePathScanManager) msg.obj, path);
                    }
                }
            }
        };

        // 拉一个线程池处理
        mScanThreadPool.execute(() -> {
            // 创建一个临时文件扫描管理器
            FilePathScanManager fileManager = new FilePathScanManager(path);

            // 开始扫描指定的路径节点
            scanMediaPathList(path, state, index, fileManager, scanFileType);

            // 按所以对文件夹和文件做排序
            mFileSortHelper.setSortMethod(FileSortHelper.SORT_INDEX);

            if (fileManager.SCAN_MUSIC_FILE_TYPE.equals(scanFileType)) {
                sort(fileManager.mMusicOnlyList);
            } else {
                sort(fileManager.mVideoInfoList);
            }
            sort(fileManager.mMediaFolderList);

            // 全部放到一個存儲管理器中 (把内置承储当一个路径添加进来)
            if (mMode == ALL_STORAGE_MODE && path.equals(DEFAULT_ROOT)) {
                MusicInfo infoFlash = new MusicInfo();
                infoFlash.mFileName = "flash";
                infoFlash.mIndex = -1;
                infoFlash.mFilePath = IConstant.PATH_FLASH;
                fileManager.mMediaFolderList.add(fileManager.mMediaFolderList.size(), infoFlash);
            }

            // 檢查并通知扫描完成
            if (state.mLoadingIndex.get() == index) {
                Message msg = handler.obtainMessage();
                msg.obj = fileManager;
                msg.what = EVENT_PATH_SCAN_FINISHED;
                handler.sendMessage(msg);
            }
        });
    }

    /**
     * 对指定列表进行排序
     * @param infos 列表
     */
    public void sort(List<MusicInfo> infos) {
        if (null == infos) {
            return;
        }

        Comparator<MusicInfo> comparator = mFileSortHelper.getComparator();
        if (null == comparator) {
            return;
        }

        infos.sort(comparator);
    }

    /**
     * 扫描函数体
     * <pre>
     *    1、过滤掉存在 .nomedia 文件的目录；
     *    2、过滤掉隐藏属性的文件；
     *    3、过滤掉指定路径的文件（地图数据等）；
     *    目标路径 DEFAULT_ROOT 有两种情况，一种是U盘，一种是所有盘符根目录;
     * </pre>
     *
     * @param path 指定要扫描的路径节点
     * @param state 扫描状态
     * @param nLoadingIndex 加载任务索引
     * @param fileManager 扫描数据存储对象
     * @param scanFileType 扫描文件类型-主要作用是过滤音乐或者视频
     * @return 任务执行失败/任务执行成功
     */
    private boolean scanMediaPathList(String path,
                                     MediaLoadState state,
                                     int nLoadingIndex,
                                     FilePathScanManager fileManager,
                                     String scanFileType) {
        LogUtil.v(TAG, "scanMediaPathList.");

        File file = new File(path);
        if (!file.exists() || !file.canRead() || !file.canExecute()) {
            return false;
        }

        File[] listFiles = null;
        if (mMode == SINGLE_STORAGE_MODE && path.equals(DEFAULT_ROOT)) {
            listFiles = file.listFiles(file1 -> {
                // 只返回 U 盘路径
                return file1.getAbsolutePath().startsWith(IConstant.PATH_USB_PREFIX);
            });
        } else if (mMode == ALL_STORAGE_MODE && path.equals(DEFAULT_ROOT)) {
            listFiles = file.listFiles(file12 -> {
                // 返回 U 盘和 SD 卡，Flash 后续环节加入
                return file12.getAbsolutePath().startsWith(IConstant.PATH_USB_PREFIX)
                        || file12.getAbsolutePath().startsWith(IConstant.PATH_SD);
            });
        } else {
            listFiles = file.listFiles();
        }

        if (Objects.isNull(listFiles)) {
            return false;
        }

        assert listFiles != null;
        for (File child : listFiles) {
            // do not show selected file if in move state
            if (state.mLoadingIndex.get() != nLoadingIndex) {
                return false;
            }

            // 如果是文件夹
            if (child.isDirectory()) {
                File noMediaFile = new File(child.getAbsolutePath() + "/.nomedia");
                if (noMediaFile.exists()
                        || child.isHidden()) {
                    continue;
                }

                // [绝对路径]
                String absolutePath = child.getAbsolutePath();
                if (isNonScanPath(absolutePath)
                        || !hasMediaFile(absolutePath, scanFileType, fileManager)) {
                    continue;
                }

                MusicInfo info = new MusicInfo();
                info.mIndex = -1;
                info.mFileName = child.getName();
                info.mFilePath = absolutePath;
                fileManager.mMediaFolderList.add(info);
                continue;
            }

            String strPath = child.getPath();
            int pos = strPath.lastIndexOf('.');
            if (-1 == pos) {
                continue;
            }

            String strSuffix = strPath.substring(pos) + ".";
            if (fileManager.SCAN_MUSIC_FILE_TYPE.equals(scanFileType)) {
                if (MUSIC_SUFFIX_EX.toString().contains(strSuffix.toLowerCase(Locale.getDefault()))) {
                    MusicInfo info = new MusicInfo();
                    info.mIndex = fileManager.mMusicOnlyList.size();
                    info.mFileName = child.getName();
                    info.mFilePath = child.getAbsolutePath();
                    fileManager.mMusicOnlyList.add(info);
                }
            } else {
                if (VIDEO_SUFFIX.contains(strSuffix.toLowerCase(Locale.getDefault()))) {
                    MusicInfo info = new MusicInfo();
                    info.mIndex = fileManager.mVideoOnlyList.size();
                    info.mFileName = child.getName();
                    info.mFilePath = child.getAbsolutePath();
                    fileManager.mVideoOnlyList.add(info);
                }
            }
        }

        return true;
    }

    /**
     * 是非扫描路径
     * <p> 要过滤的路径统一在此添加；
     *
     * @param path 路径
     * @return 是/否
     */
    private boolean isNonScanPath(String path) {
        return path.contains("/Android")
                || path.contains("/LOST.DIR")
                || path.contains("/DCIM")
                || path.contains("/DATA")
                || path.contains("dvr")
                || path.contains("/ivicar")
                || path.contains("/txz/webchat")
                || path.contains("/storage/sdcard0")
                || path.contains("/autonavidata")
                || path.contains("/System Volume Information");
    }

    /**
     * 存在音乐文件与否
     * <p> 如果文件夹下有音乐文件，那么该文件夹也需要被命中；
     *
     * @param path         目标路径
     * @param scanFileType 扫描文件类型-主要作用是过滤音乐或者视频
     * @param fileManager  扫描数据存储对象
     * @return 存在/不存在
     */
    private boolean hasMediaFile(String path, String scanFileType, FilePathScanManager fileManager) {
        File file = new File(path);

        if (file.isDirectory()) {
            File[] listFiles = file.listFiles();
            if (null == listFiles) {
                return false;
            }

            for (File child : listFiles) {
                if (hasMediaFile(child.getAbsolutePath(), scanFileType, fileManager)) {
                    return true;
                }
            }

            return false;
        }

        String strPath = file.getPath();
        int pos = strPath.lastIndexOf('.');
        if (pos == -1) {
            return false;
        }

        String strSuffix = strPath.substring(pos);
        if (fileManager.SCAN_MUSIC_FILE_TYPE.equals(scanFileType)) {
            return MUSIC_SUFFIX.contains(
                    strSuffix.toLowerCase(Locale.getDefault()));
        } else {
            return VIDEO_SUFFIX.contains(
                    strSuffix.toLowerCase(Locale.getDefault()));
        }
    }

    /**
     * 媒体路径扫描回调接口
     * <p> 文件扫描状态回调接口（扫描完成）；
     */
    public interface IMediaFilePathScanCallBack {
        /**
         * 路径扫描完成
         *
         * @param fileManager 文件扫描管理器
         * @param path 路径
         */
        void onPathScanFinishedEx(FilePathScanManager fileManager, String path);
    }
}
