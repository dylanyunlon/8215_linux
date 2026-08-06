package com.hcn.media_model.impl.data;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Build;
import android.os.SystemClock;
import android.text.TextUtils;

import androidx.annotation.NonNull;

import com.hcn.common.utils.HFileUtils;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.file.EnvironmentUtils;
import com.hcn.media_common.file.StorageUtilsEx;
import com.hcn.media_data.FavoriteManager;
import com.hcn.media_data.room.FavoriteMusic;
import com.hcn.media_data.room.FavoriteMusicDao;
import com.hcn.media_data.room.MediaDatabase;
import com.hcn.media_data.room.MediaInfoDao;
import com.hcn.media_data.storage.StorageDeviceEx;
import com.hcn.media_model.base.IDataModel;
import com.hcn.media_theme.ThemeEx;
import com.hcn.mediaservice.data.MusicInfo;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

/**
 * 数据任务模式
 * <pre>
 *    这里只处理和数据操作相关的实现；
 *    主要是数据库的异步操作，以及数据比较和检查任务；
 * </pre>
 *
 * @author 65821
 */
public abstract class DataTaskModel
        extends DataBaseModel implements IDataModel {
    /**
     * 关系型数据库对象
     * <p> 存储播放列表、收藏列表等数据；
     */
    private MediaDatabase mMediaDatabase;
    private FavoriteMusicDao mFavoriteMusicDao;
    private MediaInfoDao mMediaInfoDao;

    /**
     * BaseModel 构造函数
     *
     * @param context 上下文环境
     * @param name    线程池的名字（如果为空，不创建线程池）
     */
    public DataTaskModel(Context context, String name) {
        super(context, name);

        // 支持数据库存储功能
        if (ThemeEx.supportDatabaseStorage()) {
            openDataModel();
        }
    }

    /**
     * 打开数据模式
     * <p> 与 {@link #closeDataModel()} 配对；
     */
    private void openDataModel() {
        // 已经关闭不许打开
        if (isClosed()) {
            return;
        }

        // 上下文有效性检查
        Context context = mContextRef.get();
        if (Objects.isNull(context)) {
            return;
        }

        // 初始化媒体数据库
        mMediaDatabase = MediaDatabase.instance(context);
        mFavoriteMusicDao = mMediaDatabase.favoriteMusicDao();
        mMediaInfoDao = mMediaDatabase.mediaInfoDao();

        // 添加音乐收藏列表操作监听器
        FavoriteManager.getInstance().addOperateListener(
                FavoriteManager.Type.MUSIC, mFavoriteListOperateListener);
    }

    /**
     * 是否支持数据库操作
     * <p> 只有实例化了数据库对象，才支持数据库操作；
     *
     * @return 支持/不支持
     */
    protected boolean supportDatabaseOperate() {
        return mMediaDatabase != null;
    }

    /**
     * 最喜欢的列表操作监听器
     * <p> 删除、插入都会触发操作事件；
     **/
    private final FavoriteManager.IOperateListener
            mFavoriteListOperateListener = (listType, operate, obj0) -> {
        if (TextUtils.isEmpty(operate)) {
            return;
        }

        FavoriteManager.InfoPackage packet = null;
        if (obj0 instanceof FavoriteManager.InfoPackage) {
            packet = (FavoriteManager.InfoPackage) obj0;
        }

        if (Objects.isNull(packet)) {
            return;
        }

        switch (operate) {
            case FavoriteManager.OPERATE_ADD:
                insertFavoriteInfo(listType, packet.info);
                break;
            case FavoriteManager.OPERATE_REMOVE:
                removeFavoriteInfo(listType, packet.info);
                break;
            default:
                break;
        }
    };

    /**
     * 增加最喜欢的信息
     *
     * @param info 目标对象
     * @param type 媒体类型
     */
    @Override
    public abstract void insertFavoriteInfo(@FavoriteManager.Type int type, MusicInfo info);

    /**
     * 移除最喜欢的信息
     * @param info 目标对象
     * @param type 媒体类型
     */
    @Override
    public abstract void removeFavoriteInfo(@FavoriteManager.Type int type, MusicInfo info);

    /**
     * 关闭当前模型活动
     * <pre>
     *    1、退出异步任务，关闭数据库操作；
     *    2、从主消息队列移除所有延时任务；
     * </pre>
     */
    @Override
    public void closeDataModel() {
        LogUtil.d(TAG, "closeModel");
        super.close();

        // 取消音乐收藏列表操作监听
        FavoriteManager.getInstance().removeOperateListener(
                FavoriteManager.Type.MUSIC, mFavoriteListOperateListener);

        // 关闭媒体数据库（如果打开）
        if (mMediaDatabase != null) {
            mMediaDatabase.close();
        }
    }

    /**
     * 任务 | 初始化收藏信息到收藏表
     * <p> {@link com.hcn.media_data.room.MediaDatabase}
     */
    protected static final class InitMusicFavoriteTask extends BaseTask<List<FavoriteMusic>> {
        /**
         * 当前数据库中存储的数据列表
         * <pre>
         *    当数据库中数据超过一定数量，我们就需要做删除处理;
         *    删除最近最久未使用的数据（FavoriteMusic.getValue()）；
         * </pre>
         */
        private final List<FavoriteMusic> mListInfo = new ArrayList<>();

        /**
         * 构造函数
         * <p> 如果参数 info 不为空，则表示当前只删除一个对象；
         *
         * @param model 所有者
         * @param mainThreadResult 执行结果
         */
        public InitMusicFavoriteTask(DataTaskModel model,
                                     @NonNull Runnable mainThreadResult) {
            super(model, mainThreadResult);
        }

        @Override
        protected List<FavoriteMusic> doInBackground() {
            DataTaskModel model = (DataTaskModel) mOwnerRef.get();
            if (Objects.isNull(model)) {
                return new ArrayList<>();
            }

            // 获取所有信息 (测试：256 条数据耗时 35ms)
            long time = SystemClock.elapsedRealtime();
            List<FavoriteMusic> list = model.mFavoriteMusicDao.getAll();
            long diff = SystemClock.elapsedRealtime() - time;
            LogUtil.d(TAG, "InitMusicFavoriteTask(Query):" +
                    " size=" + list.size() + " | time=" + diff + "ms");

            // 接近收藏列表存储最大阈值（测试时候可以取少一点）
            int dataLimitSize = (int) (MediaDatabase.FAVORITE_MUSIC_MAX_STORE_SIZE * 0.75f);
            if (list.size() > dataLimitSize) {
                // 按当前收藏对象的价值排序
                list.sort(FavoriteMusic.mValueComparator);

                // 取有效值与需要删除的值
                mListInfo.clear();
                List<FavoriteMusic> listInfo = new ArrayList<>();
                for (int i = 0; i< list.size(); i++) {
                    if (i < dataLimitSize) {
                        // 取高价值数据到收藏列表
                        listInfo.add(list.get(i));
                    } else {
                        // 低价值的数据需要删除掉
                        mListInfo.add(list.get(i));
                    }
                }

                return listInfo;
            }

            return list;
        }

        @Override
        protected void doInBackgroundLast() {
            if (mListInfo.isEmpty()) {
                return;
            }

            // 删除过时的收藏信息（低价值数据）
            long time = SystemClock.elapsedRealtime();
            for (FavoriteMusic info : mListInfo) {
                // 当前模式有效性检查
                DataTaskModel model = (DataTaskModel) mOwnerRef.get();
                if (Objects.isNull(model)) {
                    continue;
                }

                model.mFavoriteMusicDao.delete(info);
            }

            long diff = SystemClock.elapsedRealtime() - time;
            LogUtil.d(TAG, "InitMusicFavoriteTask(Delete):" +
                    " size=" + mListInfo.size() + " | time=" + diff + "ms");
        }
    }

    /**
     * 任务 | 检查更新收藏信息
     * <p> 检测数据是否有效（数据文件当前是否存在）；
     */
    protected static final class CheckUpdateFavoriteTask extends BaseTask<List<MusicInfo>> {
        /** 任务触发原因 **/
        public interface Reason {
            String INITED = "database-inited";
            String MOUNTED = "mounted";
            String UNMOUNTED = "unmounted";
            String SCAN_COMPLETED = "scan-completed";
        }

        /**
         * 检查更新任务触发的原因
         * <pre>
         *    参见 {@link Reason}
         *    数据库初始化完成触发会特殊一点（具体参见代码）；
         * </pre>
         */
        private String mTaskTriggerReason = "none";

        /**
         * 要 Check 的列表信息
         * <p> 列表只需要显示当前有效的（文件存在）收藏信息；
         */
        private final List<FavoriteMusic> mListInfo = new ArrayList<>();

        /**
         * 要 Update 的列表信息
         * <p> 收藏信息对象权重发生改变后，需要更新到数据库；
         */
        private final List<FavoriteMusic> mUpdateListInfo = new ArrayList<>();

        /** 检查更新任务默认构造函数 **/
        public CheckUpdateFavoriteTask(DataTaskModel model,
                                 @NonNull Runnable mainThreadResult) {
            super(model, mainThreadResult);
        }

        /**
         * 设置数据对象
         * <p> 只有在构造时没有指定媒体信息才可以调用；
         * @param listInfo 数据集
         * @return 当前类对象
         */
        public CheckUpdateFavoriteTask setListInfo(List<FavoriteMusic> listInfo) {
            if (!mListInfo.isEmpty()) {
                throw new RuntimeException("The current list is not empty!");
            }

            mListInfo.addAll(listInfo);
            return this;
        }

        /**
         * 设置触发扫描的原因
         * @param reason 原因 {@link Reason}
         * @return  当前类对象
         */
        public CheckUpdateFavoriteTask setTriggerReason(@NonNull String reason) {
            mTaskTriggerReason = reason;
            return this;
        }

        @Override
        protected List<MusicInfo> doInBackground() {
            List<MusicInfo> list = new ArrayList<>();

            // 检查当前记忆的收藏文件
            long time = SystemClock.elapsedRealtime();
            String[] mountedStoragePath = EnvironmentUtils.instance().getAllMountedPaths();
            // 这个返回值 mountedStoragePath 是 100% 不为空的（至少会有内置存储）
            for (String storagePath : mountedStoragePath) {
                LogUtil.d(TAG, "CheckFavoriteTask/mounted: " + storagePath);
            }

            // 是数据库初始化完成后第一次检查更新
            boolean isDatabaseInited = Reason.INITED.equals(mTaskTriggerReason);

            // 轮训所有收藏信息，检查其是否是有效状态
            for (FavoriteMusic info : mListInfo) {
                if (Objects.isNull(info)) {
                    continue;
                }

                // 收藏文件如果有效，添加到列表
                String filePath = info.getFilePath();

                // 文件所在存储设备是否在 mounted 状态
                boolean isStorageMounted = false;
                String storageDevicePath = StorageUtilsEx.storageDevicePath(filePath);
                for (String storagePath : mountedStoragePath) {
                    if (storagePath.equals(storageDevicePath)) {
                        isStorageMounted = true;
                        break;
                    }
                }

                // 标记文件存在状态（如变化则需要更新权重）
                boolean fileExist = info.isFileExist();

                // 文件是否是可以读状态（可读就肯定存在）
                File file = HFileUtils.getFileByPath(filePath);
                boolean isFileCanRead = file.canRead();
                if (isStorageMounted && isFileCanRead) {
                    info.setFileExist(true);

                    MusicInfo musicInfo = new MusicInfo();
                    musicInfo.mFavorite = true;
                    musicInfo.mFileName = info.getTitle();
                    musicInfo.mFilePath = filePath;
                    list.add(musicInfo);
                } else {
                    info.setFileExist(false);
                }

                // 更新收藏媒体对象的权重（从不存在到存在）
                if (!fileExist && info.isFileExist()) {
                    int weight = info.getWeight();
                    info.setWeight(weight + 1);
                    // 如果文件存在，则重置其价值；
                    info.setValue(0);
                    info.setAllowUpdate(true);

                    // 不要在此更新（影响初始化收藏列表显示的时间）
                    // model.mFavoriteMusicDao.update(info);
                    mUpdateListInfo.add(info);
                } else {
                    // 首次初始化文件如果不存在，则需要降低其价值；
                    if (isDatabaseInited
                            && !info.isFileExist()) {
                        int value = info.getValue();
                        info.setValue(value - 1);
                        info.setAllowUpdate(true);
                        mUpdateListInfo.add(info);
                    }
                }
            }

            // 打印文件有效性检查耗时
            long diff = SystemClock.elapsedRealtime() - time;
            LogUtil.d(TAG, "CheckFavoriteTask:" +
                    " size=" + list.size() + " | time=" + diff + "ms");

            return list;
        }

        @Override
        protected void doInBackgroundLast() {
            // 无更新信息
            if (mUpdateListInfo.isEmpty()) {
                return;
            }

            // 一个一个更新到数据库中
            long time = SystemClock.elapsedRealtime();
            for (FavoriteMusic info : mUpdateListInfo) {
                // 当前模式有效性检查
                DataTaskModel model = (DataTaskModel) mOwnerRef.get();
                if (Objects.isNull(model)) {
                    continue;
                }

                if (info != null && info.isAllowUpdate()) {
                    info.setAllowUpdate(false);
                    model.mFavoriteMusicDao.update(info);
                }
            }

            // 打印更新数据耗时
            long diff = SystemClock.elapsedRealtime() - time;
            LogUtil.d(TAG, "UpdateFavoriteTask:" +
                    " size=" + mUpdateListInfo.size() + " | time=" + diff + "ms");
        }
    }

    /**
     * 任务 | 插入收藏信息到收藏表
     * <p> {@link com.hcn.media_data.room.MediaDatabase}
     */
    protected static final class InsertFavoriteTask extends BaseTask<List<FavoriteMusic>> {
        private final List<MusicInfo> mListInfo = new ArrayList<>();

        /**
         * 构造函数
         * <p> 如果参数 info 不为空，则表示当前只插入一个对象；
         *
         * @param model 所有者
         * @param info 插入数据对象
         * @param mainThreadResult 返回结果
         */
        public InsertFavoriteTask(DataTaskModel model,
                                  @NonNull MusicInfo info,
                                  @NonNull Runnable mainThreadResult) {
            super(model, mainThreadResult);
            if (!Objects.isNull(info)) {
                mListInfo.add(info);
            }
        }

        /**
         * 设置数据对象
         * @param listInfo 数据集
         */
        public InsertFavoriteTask setListInfo(List<MusicInfo> listInfo) {
            if (!mListInfo.isEmpty()) {
                throw new RuntimeException("The current list is not empty!");
            }

            mListInfo.addAll(listInfo);
            return this;
        }

        @Override
        protected List<FavoriteMusic> doInBackground() {
            // 先转换成目标对象集
            List<FavoriteMusic> list = new ArrayList<>();
            for (MusicInfo info : mListInfo) {
                if (Objects.isNull(info)) {
                    continue;
                }

                FavoriteMusic favoriteInfo =
                        new FavoriteMusic(info.mFileName, info.mFilePath);
                list.add(favoriteInfo);
            }

            // 当前模式有效性检查
            DataTaskModel model = (DataTaskModel) mOwnerRef.get();
            if (Objects.isNull(model)) {
                return null;
            }

            // 执行插入数据动作
            model.mFavoriteMusicDao.insert(list);
            return list;
        }
    }

    /**
     * 任务 | 删除收藏信息到收藏表
     * <p> {@link com.hcn.media_data.room.MediaDatabase}
     */
    protected static final class RemoveFavoriteTask extends BaseTask<String> {
        private final MusicInfo mMusicInfo;

        /**
         * 构造函数
         * <p> 如果参数 info 不为空，则表示当前只删除一个对象；
         *
         * @param model 所有者
         * @param info 插入数据对象
         * @param mainThreadResult 返回结果
         */
        public RemoveFavoriteTask(DataTaskModel model,
                                  @NonNull MusicInfo info,
                                  @NonNull Runnable mainThreadResult) {
            super(model, mainThreadResult);
            mMusicInfo = info;
        }

        @Override
        public String doInBackground() {
            // 当前模式有效性检查
            DataTaskModel model = (DataTaskModel) mOwnerRef.get();
            if (Objects.isNull(model)) {
                return null;
            }

            // 从数据库表中删除
            model.mFavoriteMusicDao.deleteInfo(mMusicInfo.mFilePath);
            return mMusicInfo.mFilePath;
        }
    }
}
