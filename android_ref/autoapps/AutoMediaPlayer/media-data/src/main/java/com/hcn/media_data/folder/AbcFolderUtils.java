package com.hcn.media_data.folder;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.common.lang.HThreadUtils;
import com.hcn.common.lang.RunnableEx;
import com.hcn.media_common.thread.HPublicExecutor;
import com.hcn.media_common.thread.HTaskRunnable;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.storage.StorageDeviceEx;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.Callable;

/**
 * 文件夹数据工具
 * <p> 工具接口，用来更新数据信息；
 */
public class AbcFolderUtils {

    /**
     * 更新文件夹列表 ID3 信息
     * <pre>
     *    传输进来的文件夹列表中的 MusicInfo 肯定都属于一个存储设备；
     *    考虑到效率问题:
     *    1、注意接口只支持更新音乐列表信息（视频本身也没有 ID3 一说）；
     *    2、理论上这个函数只能运行在主线程中，所以需要对其做线程检查；
     *    3、我们先确定存储设备类型，再去存储设备中比对信息，更新 ID3;
     * </pre>
     *
     * @param taskTag 任务标签（执行结果匹配用）
     * @param listInfo 音乐列表信息
     * @param isMusicFolderType 判断是否是音乐（防止视频扫描浪费资源）
     * @param listener 回调信息（主线程）；
     */
    @Nullable
    public static void updateFolderListId3Info(long taskTag,
                                               List<MusicInfo> listInfo,
                                               boolean isMusicFolderType,
                                               @NonNull HTaskRunnable.OnCompletionListener listener) {
        // 接口必须运行在主线程
        if (!HThreadUtils.isMainThread()) {
            throw new RuntimeException(
                    "updateFolderListId3Info/not running on the main thread!");
        }

        // 不能是空数据列表和对象
        if (Objects.isNull(listInfo) || listInfo.isEmpty()) {
            listener.onCompletion(null);
            return;
        }

        MusicInfo info = listInfo.get(0);
        StorageDeviceEx storage = AppGlobalData.getStorageDeviceEx(info);
        if (Objects.isNull(storage)
                || Objects.requireNonNull(storage).isLoading()
                || !storage.isMounted()) {
            listener.onCompletion(null);
            return;
        }

        // 线程安全考虑（线程操作前，先拷贝一份值）
        final ArrayList<MusicInfo> sourceList = new ArrayList<>(storage.mMusicInfoList);

        // 请求异步线程任务（返回结果直接到主线程）
        HPublicExecutor.instance().submitTask2(() -> {
            // 返回值需要单独一个列表存储
            final ArrayList<MusicInfo> targetList = new ArrayList<>();

            // 双重循环效率还是差了点（字符串比较）
            for (MusicInfo info1 : listInfo) {
                if (isMusicFolderType) {
                    if (info1.mID3Type != MusicInfo.ID3_TYPE_NONE) {
                        targetList.add(info1);
                        continue;
                    }

                    for (MusicInfo info2 : sourceList) {
                        if (info1.compareTo(info2) == 0
                                && info2.mID3Type != MusicInfo.ID3_TYPE_NONE) {
                            info1.mID3Type = info2.mID3Type;
                            info1.mTitle = info2.mTitle;
                            info1.mArtist = info2.mArtist;
                            info1.mAlbum = info2.mAlbum;
                            break;
                        }
                    }
                }
                targetList.add(info1);
            }

            return targetList;
        }, o -> listener.onCompletion(taskTag, o));
    }

    /**
     * 更新文件夹列表 ID3 信息
     * <p> 旧的皮肤包还在使用三参函数(xt470)，重载函数防止影响旧的皮肤包的功能
     *
     * @param taskTag 任务标签（执行结果匹配用）
     * @param listInfo 音乐列表信息
     * @param listener 回调信息（主线程）；
     */
    @Nullable
    public static void updateFolderListId3Info(long taskTag,
                                               List<MusicInfo> listInfo,
                                               @NonNull HTaskRunnable.OnCompletionListener listener) {
        updateFolderListId3Info(taskTag, listInfo, true, listener);
    }
}
