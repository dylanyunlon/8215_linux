package com.hcn.auto.app;

import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import com.hcn.auto.app.base.Listenable;
import com.hcn.auto.app.base.RunnableEx;
import com.hcn.auto.concurrent.HPublicExecutor;
import com.hcn.auto.concurrent.HTaskRunnable;
import com.hcn.auto.utils.HFileUtils;
import com.hcn.auto.utils.HUtilsEx;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.Callable;

/**
 * 模块自定义壁纸管理器
 * <pre>
 *    模块壁纸通过 apd/appWallpaper 目录下的图片来管理；
 *    当前类通过 HMedia 服务解析图片目录下的文件信息，然后对外提供数据；
 * </pre>
 */
public class Wallpaper {

    /** 状态: 完成检索 */
    public static final String ST_COMPLETED = "Completed";

    /** 事件: 保存壁纸 */
    public static final String ET_SAVE_PATH = "Save-Path";

    /** 壁纸管理器单例 */
    private static final class WallpaperHolder {
        static final Wallpaper sInstance = new Wallpaper();
    }

    /**
     * 壁纸管理器单例获取函数
     *
     * @return 壁纸管理器单例
     */
    public static Wallpaper instance() {
        return WallpaperHolder.sInstance;
    }

    private static final String TAG = "Wallpaper";

    /** 壁纸存放目录 */
    public static final String APP_WALLPAPER_PATH = "/apd/appWallpaper";

    /** 壁纸管理器初始化状态 */
    private boolean mInitialized = false;

    /** 壁纸信息状态 */
    private final ArrayList<Info> mList = new ArrayList<>();

    /** 壁纸信息状态监听者 */
    private final List<Listenable<String>> mListeners;

    /** 壁纸管理器构造函数 */
    private Wallpaper() {
        mListeners = new ArrayList<>();
    }

    /**
     * 判定目标文件后缀
     *
     * @param fileName 文件名
     * @param suffix 后缀
     * @return 是/否
     */
    private boolean isFileSuffix(String fileName, String suffix) {
        if (TextUtils.isEmpty(fileName) || TextUtils.isEmpty(suffix)) {
            return false;
        }

        return fileName.endsWith(suffix);
    }

    /**
     * 注册壁纸信息状态监听者
     * @param listener 监听者
     */
    public void register(@NonNull Listenable<String> listener) {
        // 不要重复添加同一个对象
        if (mListeners.contains(listener)) {
            return;
        }

        // 添加到连接状态监听管理
        mListeners.add(listener);

        // 如果已经初始化完成，直接通知监听者
        if (mInitialized) {
            listener.listen(ST_COMPLETED, null);
        }
    }

    /**
     * 注销壁纸信息状态监听者
     * @param listener 监听者
     */
    public void unregister(Listenable<String> listener) {
        if (Objects.isNull(listener)) {
            return;
        }

        mListeners.remove(listener);
    }

    /**
     * 通知监听者
     * @param event 信息事件
     */
    private void notifyListener(String event, Object arg) {
        for (Listenable<String> listener : mListeners) {
            if (listener == null) {
                continue;
            }

            listener.listen(event, arg);
        }
    }

    /**
     * 壁纸管理器初始化函数
     * <p> 读取壁纸目录下的图片信息，然后对外提供数据；
     */
    public void initialize() {
        mList.clear();

        // 壁纸文件如果不存在，直接返回；
        if (!HFileUtils.isFileExists(APP_WALLPAPER_PATH)) {
            return;
        }

        int taskId = 20140109;
        HPublicExecutor.instance().submitTask(
                taskId,
                new loadWallpaperTask(),
                new HTaskRunnable.OnCompletionListener() {

                    @Override
                    public void onCompletion(long taskUniqueId, Object result) {
                        if (taskUniqueId != taskId) {
                            return;
                        }

                        // 通知壁纸管理器初始化完成
                        mInitialized = true;
                        if (HUtilsEx.isAppDebug()) {
                            for (Info info: mList) {
                                Log.v(TAG, ": " + info.toString());
                            }
                        }

                        // 通知壁纸信息状态监听者
                        notifyListener(ST_COMPLETED, null);
                    }
                });
    }

    /**
     * 加载壁纸信息任务
     * <p> 执行完后，可直接检查是否有新的壁纸；
     */
    private class loadWallpaperTask extends RunnableEx {
        @RequiresApi(api = Build.VERSION_CODES.N)
        @Override
        public void run() {
            // 壁纸文件如果不存在，直接返回；
            if (!HFileUtils.isFileExists(APP_WALLPAPER_PATH)) {
                Log.v(TAG, "loadWallpaperTask: " + APP_WALLPAPER_PATH + " not exists");
                return;
            }

            // 起一个子线程去查找（容错处理，避免文件内东西过多）
            List<File> files = HFileUtils.listFilesInDir(APP_WALLPAPER_PATH);

            for (File file : files) {
                if (file.isDirectory()) {
                    continue;
                }

                String fileName = file.getName();
                if (fileName.startsWith("thumbnail_")
                        && (isFileSuffix(fileName, ".jpg") || isFileSuffix(fileName, ".png"))) {
                    Info info = new Info();
                    info.thumbnailPath = file.getAbsolutePath();
                    int suffixIndex = fileName.lastIndexOf(".");
                    String wallpaperPath = APP_WALLPAPER_PATH
                            + File.separator + "wallpaper_" + fileName.substring(10, suffixIndex);

                    String jpgWallpaperPath = wallpaperPath + ".jpg";
                    if (HFileUtils.isFileExists(jpgWallpaperPath)) {
                        info.wallpaperPath = jpgWallpaperPath;
                    } else {
                        String pngWallpaperPath = wallpaperPath + ".png";
                        if (HFileUtils.isFileExists(pngWallpaperPath)) {
                            info.wallpaperPath = pngWallpaperPath;
                        } else {
                            continue;
                        }
                    }

                    mList.add(info);
                }
            }

            // 不为空，直接排序
            if (!mList.isEmpty()) {
                mList.sort((o1, o2) -> {
                    if (o1 == null || o2 == null) {
                        return 0;
                    }

                    // 按 thumbnailPath 字符串大小排序
                    if (o1.thumbnailPath != null && o2.thumbnailPath != null) {
                        return o1.thumbnailPath.compareTo(o2.thumbnailPath);
                    }

                    return 0;
                });
            }
        }
    }

    /**
     * 当前壁纸管理器是否初始化完成
     * @return 是/否
     */
    public boolean isInitialized() {
        return mInitialized;
    }

    /**
     * 获取壁纸信息
     * <p> 该函数只有在初始化完成后才有效；
     * @return 壁纸信息
     */
    public List<Info> getInfo() {
        if (!mInitialized) {
            return null;
        }

        return mList;
    }

    /**
     * 保存壁纸路径
     * <p> Wallpaper 只提供保存机制，不提供具体保存实体；
     *
     * @param path 壁纸原图路径
     */
    public void saveWallpaperPath(String path) {
        if (!mInitialized) {
            return;
        }

        notifyListener(ET_SAVE_PATH, path);
    }

    /**
     * 壁纸管理器销毁函数
     * <p> 释放资源，避免内存泄漏；
     */
    public void destroy() {
        mList.clear();
    }

    /**
     * 壁纸信息
     * <pre>
     *    /apd/appWallpaper 目录下的图片信息；
     *    壁纸缩略图文件名字格式: thumbnail_${Num}.jpg/png
     *    壁纸原始图文件名字格式: wallpaper_${Num}.jpg/png
     * </pre>
     */
    public static class Info {
        /** 壁纸缩略图路径 */
        public String thumbnailPath;

        /** 壁纸原始图路径 */
        public String wallpaperPath;

        public String toString() {
            return "Info{thumbnailPath=" + thumbnailPath + ", wallpaperPath=" + wallpaperPath + "}";
        }
    }
}
