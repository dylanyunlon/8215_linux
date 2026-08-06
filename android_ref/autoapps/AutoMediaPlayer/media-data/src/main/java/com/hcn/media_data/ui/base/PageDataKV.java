package com.hcn.media_data.ui.base;

import android.content.Context;

import androidx.annotation.NonNull;

import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HMessageUtils;
import com.tencent.mmkv.MMKV;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;

/**
 * 页面基础数据类
 * <pre>
 *    主要用来保存多媒体页面的相关显示状态；
 *    e.g. 进入页面是先显示歌词还是频谱等；
 * <pre>
 * @author 65821
 */
abstract public class PageDataKV {
    /** 页面数据打印标签 */
    protected static final String TAG = "PageDataKV";

    /** 数据存储文件加密秘钥 */
    private static final String MEDIA_KV_CRYPT_KEY = "media-page-data";

    /**
     * 数据存储对象（写文件）
     * <p> [key - value] 存储方式
     */
    protected final MMKV mKV;

    /** 数据存储的键值规则 - 键 */
    public interface Key {
        /**
         * 歌词与频谱的显示标签
         * <pre>
         *    大部分 UI 设计的时候，歌词与频谱同一时间只会显示一个；
         *    这个 key 就是用来标记上一次退出时当前页面歌词频谱的显示状态；
         * </pre>
         */
        String LYRICS_SPECTRUM_FLAG = "lyrics_spectrum_flag";

        /**
         * 音乐壁纸路径
         * <pre>
         *    {@link com.hcn.auto_compat.app.Wallpaper}
         *    支持自定义壁纸，这个 key 就是用来标记上一次退出时当前页面的壁纸路径；
         *    具体参见 apd/appWallpaper 中的壁纸文件；
         * </pre>
         */
        String MUSIC_WALLPAPER_PATH = "music_wallpaper_path";

        /**
         * 用户列表操作行为
         * <pre>
         *    用户的列表操作行为：普通模式/文件夹模式
         * </pre>
         */
        String CURRENT_LIST_ACTION_SCENE = "current_list_action_scene";

        /**
         * 上一次记忆的文件夹播放路径
         * <pre>
         *    如果用户在音乐播放器的文件夹中播放歌曲，会记录上一次退出时播放的文件夹目录
         * </pre>
         */
        String MUSIC_FOLDER_PATH = "music_folder_path";
    }

    /** 数据存储的键值规则 - 值 */
    public interface Value {
        /** 显示歌词 */
        int LYRICS = 0;

        /** 显示频谱 */
        int SPECTRUM = 1;

        /** 显示歌词和频谱 */
        int LYRICS_SPECTRUM = 2;
    }

    /** 用户列表操作行为 - 值 */
    public interface ActionSceneValue {
        /** 普通模式 */
        int NORMAL = 0;

        /** 文件夹模式 */
        int FOLDER = 1;
    }

    /**
     * 上下文引用
     * <p> 安全第一，避免不必要的强引用导致内存泄露；
     */
    protected final Reference<Context> mContextRef;

    /**
     * 数据 KV 对象是打开状态
     * <p> 主要是用来避免重复关闭存储和消息对象；
     */
    protected boolean mIsOpen;

    /**
     * 主线程消息处理器封装
     * <p> 仅在当前类有效，禁止外溢使用；
     */
    protected final HMessageUtils MSG;

    /** 内部消息定义 */
    protected interface H {
        int MSG_NONE = -1;

        // 同步数据到磁盘
        int MSG_SYNC_MMKV_DATA2DISK = 1;
    }

    /**
     * 构造函数
     * @param context 上下文
     */
    protected PageDataKV(@NonNull Context context) {
        mContextRef = new WeakReference<>(context);
        mIsOpen = true;

        String rootDir = MMKV.initialize(context);
        LogUtils.dTag(TAG, "Page data storage path: " + rootDir);
        mKV = MMKV.defaultMMKV(MMKV.SINGLE_PROCESS_MODE, MEDIA_KV_CRYPT_KEY);

        // 构造一个消息处理工具对象
        MSG = new HMessageUtils.Builder()
                .setName(TAG)
                .setSupportGlobalUsage(false)
                .build();

        // 初始化消息
        onInitializeMessage();
    }

    /**
     * 初始化消息处理
     * <p> 对于主要关联的消息，我们可以在此体检注册监听；
     */
    protected void onInitializeMessage() {
        // 存储到磁盘
        MSG.addListener(H.MSG_SYNC_MMKV_DATA2DISK, uiMessage -> {
            syncToDisk();
        });
    }

    /**
     * 关闭当前对象
     * <p>关闭数据库句柄、清除消息队列；
     */
    protected void close() {
        if (!mIsOpen) {
            return;
        }

        mIsOpen = false;
        mKV.close();

        MSG.clearMessageQueue();
        MSG.release();
    }

    /**
     * 存储信息
     *
     * @param key 键值 {@link Key}
     * @param value 需要存储的值
     * @param syncNow 立即存储到文件
     */
    public void write(String key, String value, boolean syncNow) {
        if (!mIsOpen) {
            return;
        }

        mKV.encode(key, value);
        trySync(syncNow, 5000);
    }

    /**
     * 存储信息
     * <p> 默认延时存储到磁盘；
     *
     * @param key 键值 {@link Key}
     * @param value 需要存储的值
     */
    public void write(String key, String value) {
        if (!mIsOpen) {
            return;
        }

        write(key, value, false);
    }

    /**
     * 存储信息
     *
     * @param key 键值 {@link Key}
     * @param value 需要存储的值
     * @param syncNow 立即存储到文件
     */
    public void write(String key, int value, boolean syncNow) {
        if (!mIsOpen) {
            return;
        }

        mKV.encode(key, value);
        trySync(syncNow, 5000);
    }

    /**
     * 存储信息
     * <p> 默认延时存储到磁盘；
     *
     * @param key 键值 {@link Key}
     * @param value 需要存储的值
     */
    public void write(String key, int value) {
        if (!mIsOpen) {
            return;
        }

        write(key, value, false);
    }

    /**
     * 尝试同步
     * @param syncNow 立即同步
     * @param delayMillis 延时同步
     */
    private void trySync(boolean syncNow, long delayMillis) {
        if (!mIsOpen) {
            return;
        }

        if (syncNow) {
            mKV.sync();
        } else {
            MSG.sendUnique(
                    H.MSG_SYNC_MMKV_DATA2DISK, delayMillis);
        }
    }

    /**
     * 同步到磁盘
     * <p> 强制同步到磁盘文件，避免数据状态丢失；
     */
    public void syncToDisk() {
        if (!mIsOpen) {
            return;
        }

        if (mKV != null) {
            mKV.sync();
        }
    }

    /**
     * 从存储中读取
     *
     * @param key 键值 {@link Key}
     * @return 存储的值
     */
    public String readString(String key) {
        if (!mIsOpen) {
            return null;
        }

        return mKV.decodeString(key);
    }

    /**
     * 从存储中读取
     *
     * @param key 键值 {@link Key}
     * @param defaultValue 默认值
     * @return 存储的值
     */
    public int readInt(String key, int defaultValue) {
        if (!mIsOpen) {
            return defaultValue;
        }

        return mKV.decodeInt(key, defaultValue);
    }
}
