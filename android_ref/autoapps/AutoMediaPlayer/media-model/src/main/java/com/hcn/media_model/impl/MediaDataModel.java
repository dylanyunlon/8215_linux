package com.hcn.media_model.impl;

import android.content.Context;
import android.os.Message;
import android.provider.Settings;
import android.text.TextUtils;

import androidx.annotation.NonNull;

import com.hcn.common.lang.RunnableEx;
import com.hcn.common.utils.HFileUtils;
import com.hcn.config.HSettings;
import com.hcn.media.utils.LogUtils;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_data.FavoriteManager;
import com.hcn.media_data.room.FavoriteMusic;
import com.hcn.media_data.room.FavoriteMusicDao;
import com.hcn.media_data.ui.MediaPageState;
import com.hcn.media_data.ui.base.PageDataKV;
import com.hcn.media_model.impl.data.DataTaskModel;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * 媒体数据模型类
 * <pre>
 *    用来处理需要保存的数据信息；
 *    e.g. 存储恢复收藏列表、播放列表、播放状态等；
 * </pre>
 *
 * @author 65821
 */
class MediaDataModel extends DataTaskModel {
    /** Model 必须是唯一实例设计 **/
    private static MediaDataModel sInstance = null;

    /** DataModel 对外接口实例 **/
    public static MediaDataModel instance() {
        if (Objects.isNull(sInstance)) {
            throw new RuntimeException(
                    "Please initialize [MediaDataModel] Object!");
        }

        return sInstance;
    }

    /**
     * 初始化 DataModel 实例
     * <p> 数据模型暂时不需要访问外部模型，它只需要访问全局数据状态；
     *
     * @param context 上下文环境
     */
    public static void init(@NonNull Context context) {
        if (Objects.isNull(sInstance)) {
            sInstance = new MediaDataModel(context);
        } else {
            throw new RuntimeException(
                    "[MediaDataModel] already initialized!");
        }
    }

    /**
     * 最喜欢的音乐列表（数据库中的）
     * <pre>
     *    这个是和数据库信息同步的列表，不是正在显示的收藏列表；
     *    正在显示的收藏列表也是从这个里面提取的的；
     * </pre>
     */
    private final List<FavoriteMusic> mFavoriteMusicList = new ArrayList<>();

    /** 禁止构造无参对象 **/
    private MediaDataModel() {
        super(null, null);
        throw new RuntimeException(
                "Prohibit the construction of parameterless objects");
    }

    /**
     * MediaDataModel 构造函数
     * <p> 注意数据模型需要操作数据库，需要使用线程池；
     * @param context 上下文环境
     */
    public MediaDataModel(Context context) {
        super(context, "dataModel");

        // 页面数据初始化
        MediaPageState.instance();

        // 壁纸数据初始化
        MediaPageState.instance().getWallpaperData().initWallpaperData();

        // 暂时只处理音乐相关数据
        initMusicFavoriteInfo();
    }

    /**
     * 初始化收藏列表信息
     * <pre>
     *    从数据库读取收藏信息到 {@link FavoriteManager}
     *    收藏列表数据库对应访问 {@link FavoriteMusicDao}
     * </pre>
     */
    @SuppressWarnings("unchecked")
    private void initMusicFavoriteInfo() {
        // 不支持数据库操作
        if (!supportDatabaseOperate()) {
            return;
        }

        // 已经初始化过列表
        List<MusicInfo> musicInfos =
                FavoriteManager.getInstance().favoriteMusicList();
        if (!musicInfos.isEmpty()) {
            throw new RuntimeException("initMusicFavoriteInfo, errorCode=400!");
        }

        // 执行插入数据任务
        executeAsyncTask(
                new InitMusicFavoriteTask(this, new RunnableEx() {
                    @Override
                    public void run() {
                        Object obj = getObject();
                        assert obj instanceof List<?>;
                        List<FavoriteMusic> favoriteList = (List<FavoriteMusic>) obj;

                        // 数据库数据与版本不匹配
                        if (Objects.isNull(favoriteList)) {
                            // 这里需要删除数据库文件
                            LogUtil.w(TAG, "initMusicFavoriteInfo: db version not match!");
                            return;
                        }

                        // 保存数据到内存列表
                        mFavoriteMusicList.clear();
                        mFavoriteMusicList.addAll(favoriteList);

                        // 如果已经校验过，直接返回
                        FavoriteManager fm = FavoriteManager.getInstance();
                        if (fm.isInitCompleted(FavoriteManager.Type.MUSIC)) {
                            return;
                        }

                        // 数据库中没有最喜欢的音乐数据
                        if (favoriteList.isEmpty()) {
                            // 设置音乐收藏列表，设置初始化完成标记；
                            fm.initFavoriteMusicList(new ArrayList<>());
                            fm.setInitCompleted(FavoriteManager.Type.MUSIC, true);
                        } else {
                            // 触发数据库中的数据校验任务（检查所有数据）
                            checkFavoriteMusicData(favoriteList,
                                    CheckUpdateFavoriteTask.Reason.INITED);
                        }
                    }
                }));
    }

    /**
     * 检查最喜欢的音乐数据
     * <pre>
     *    List<FavoriteMusic> 是刚从数据库中读取出来的数据；
     *    我们需要检查这些数据当前是否有效（对应的文件是否存在）；
     * </pre>
     *
     * @param favoriteList  需要检查的数据
     * @param triggerReason 任务触发原因
     */
    @SuppressWarnings("unchecked")
    private void checkFavoriteMusicData(List<FavoriteMusic> favoriteList,
                                        @NonNull String triggerReason) {
        // 数据模式已经关闭
        if (isClosed()) {
            return;
        }

        // 不支持数据库操作
        if (!supportDatabaseOperate()) {
            return;
        }

        // 执行检查数据任务
        executeAsyncTask(new CheckUpdateFavoriteTask(this,
                new RunnableEx() {
                    @Override
                    public void run() {
                        Object obj = getObject();
                        assert obj instanceof List<?>;
                        List<MusicInfo> musicInfos = (List<MusicInfo>) obj;

                        // 添加校验后的数据到收藏列表
                        FavoriteManager fm = FavoriteManager.getInstance();
                        if (fm.isInitCompleted(FavoriteManager.Type.MUSIC)) {
                            // 已经初始化完成的情况，直接更新数据；
                            fm.updateFavoriteMusicList(musicInfos);
                        } else {
                            // 设置音乐收藏列表，设置初始化完成标记；
                            fm.initFavoriteMusicList(musicInfos);
                            fm.setInitCompleted(FavoriteManager.Type.MUSIC, true);

                            // 更新数据库信息中收藏文件的权重
                        }
                    }
                }).setListInfo(favoriteList)
                .setTriggerReason(triggerReason));
    }

    /**
     * 处理最喜欢列表选项插入操作
     * <p> 主要是执行更新数据库操作（数据库需要异步执行）；
     *
     * @param type 媒体类型
     * @param info 目标对象
     */
    @Override
    @SuppressWarnings("unchecked")
    public void insertFavoriteInfo(@FavoriteManager.Type int type, MusicInfo info) {
        if (isClosed()) {
            return;
        }

        // 不支持数据库操作
        if (!supportDatabaseOperate()) {
            return;
        }

        // 现阶段只处理音乐收藏列表
        if (type != FavoriteManager.Type.MUSIC) {
            return;
        }

        // 执行插入数据任务
        executeAsyncTask(
                new InsertFavoriteTask(this, info, new RunnableEx() {
                    @Override
                    public void run() {
                        Object obj = getObject();
                        if (Objects.isNull(obj)) {
                            return;
                        }

                        // 返回结果类型检查
                        assert obj instanceof List<?>;
                        List<FavoriteMusic> musicInfos = (List<FavoriteMusic>) obj;

                        // 填充到数据库关联的列表集（追加）
                        mFavoriteMusicList.addAll(musicInfos);
                    }
                }));
    }

    /**
     * 处理最喜欢列表选项移除操作
     * <p> 主要是执行更新数据库操作（数据库需要异步执行）；
     *
     * @param type 媒体类型
     * @param info 目标对象
     */
    @Override
    public void removeFavoriteInfo(@FavoriteManager.Type int type, MusicInfo info) {
        // 数据模式已经关闭
        if (isClosed()) {
            return;
        }

        // 不支持数据库操作
        if (!supportDatabaseOperate()) {
            return;
        }

        // 现阶段只处理音乐收藏列表
        if (type != FavoriteManager.Type.MUSIC) {
            return;
        }

        // 执行删除数据任务
        executeAsyncTask(
                new RemoveFavoriteTask(this, info, new RunnableEx() {
                    @Override
                    public void run() {
                        Object obj = getObject();
                        if (Objects.isNull(obj)) {
                            return;
                        }

                        // 返回结果类型检查
                        assert obj instanceof String;
                        String filePath = (String) obj;

                        // 从数据库关联的列表集移除 (2000 条数据平均不到 5ms 的耗时)
                        FavoriteMusic removeInfo = null;
                        for (FavoriteMusic musicinfo : mFavoriteMusicList) {
                            if (MiscUtils.reverseEquals(
                                    musicinfo.getFilePath(), filePath)) {
                                removeInfo = musicinfo;
                            }
                        }

                        // mFavoriteMusicList 中不可能存在空对象
                        if (!Objects.isNull(removeInfo)) {
                            mFavoriteMusicList.remove(removeInfo);
                        }
                    }
                }));
    }

    /**
     * 存储设备状态事件
     *
     * @param event  事件定义 {@link IMediaEvent}
     * @param wParam 附加参数 1
     * @param lParam 附加参数 2
     */
    @Override
    protected void onStorageDeviceEvent(int event, Object wParam, Object lParam) {
        switch (event) {
            case IMediaEvent.EVENT_MEDIA_MOUNTED:
                onStorageDeviceMounted(wParam);
                break;
            case IMediaEvent.EVENT_MEDIA_UNMOUNTED:
                onStorageDeviceUnmounted(wParam);
                break;
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
                onStorageDeviceScanStart(wParam);
                break;
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
                onStorageDeviceScanCompleted(wParam);
                break;
            default:
                break;
        }
    }

    /**
     * 存储设备挂载事件
     *
     * @see IMediaEvent#EVENT_MEDIA_MOUNTED
     * @param storagePath 存储设备路径
     */
    private void onStorageDeviceMounted(Object storagePath) {
        // 参数类型检查（必须是 String 类型）
        if (!(storagePath instanceof String)) {
            return;
        }

        String devicePath = (String) storagePath;
        LogUtil.v(TAG, "onStorageDeviceMounted: " + devicePath);

        // 关闭状态不处理
        if (isClosed()) {
            return;
        }

        // 检查音乐收藏信息是否有效
        tryCheckFavoriteMusicData(
                CheckUpdateFavoriteTask.Reason.MOUNTED,
                2000);
    }

    /**
     * 存储设备卸载事件
     * @see IMediaEvent#EVENT_MEDIA_UNMOUNTED
     * @param storagePath 存储设备路径
     */
    private void onStorageDeviceUnmounted(Object storagePath) {
        // 参数类型检查（必须是 String 类型）
        if (!(storagePath instanceof String)) {
            return;
        }

        String devicePath = (String) storagePath;
        LogUtil.v(TAG, "onStorageDeviceUnmounted: " + devicePath);

        // 检查音乐收藏信息是否有效
        tryCheckFavoriteMusicData(
                CheckUpdateFavoriteTask.Reason.UNMOUNTED,
                2000);
    }

    /**
     * 存储设备扫描开始
     * @see IMediaEvent#EVENT_MEDIA_LOADING_START
     * @param storagePath 存储设备路径
     */
    private void onStorageDeviceScanStart(Object storagePath) {
        // 参数类型检查（必须是 String 类型）
        if (!(storagePath instanceof String)) {
            return;
        }

        String devicePath = (String) storagePath;
        LogUtil.v(TAG, "onStorageDeviceScanStart: " + devicePath);
    }

    /**
     * 存储设备扫描完成
     * @see IMediaEvent#EVENT_MEDIA_LOADING_COMPLETE
     * @param storagePath 存储设备路径
     */
    private void onStorageDeviceScanCompleted(Object storagePath) {
        // 参数类型检查（必须是 String 类型）
        if (!(storagePath instanceof String)) {
            return;
        }

        String devicePath = (String) storagePath;
        LogUtil.v(TAG, "onStorageDeviceScanCompleted: " + devicePath);

        // 检查音乐收藏信息是否有效
        tryCheckFavoriteMusicData(
                CheckUpdateFavoriteTask.Reason.SCAN_COMPLETED,
                100);
    }

    /**
     * 尝试更新最喜欢音乐列表（2000 条数据理论上耗时不超过 500ms）
     * <p> 这个每次不管是哪个存储设备改变状态，我们都可以检查所有；
     *
     * @param tryCheckReason 尝试检查的原因
     * @param delayUpdateTime 延迟更新时间（ms）
     */
    private void tryCheckFavoriteMusicData(String tryCheckReason,
                                           long delayUpdateTime) {
        // 数据模式已经关闭
        if (isClosed()) {
            return;
        }

        // 不支持数据库操作
        if (!supportDatabaseOperate()) {
            return;
        }

        // 只有音乐收藏列表初始化完成才可以触发更新时间
        FavoriteManager fm = FavoriteManager.getInstance();
        if (fm.isInitCompleted(FavoriteManager.Type.MUSIC)) {
            if (mFavoriteMusicList.isEmpty()) {
                return;
            }

            if (delayUpdateTime > 0) {
                // 移除重复消息（只处理最新的）
                int msgWhat = MsgEx.MSG_UPDATE_FAVORITE_MUSIC_LIST;
                if (H0.hasMessages(msgWhat)) {
                    H0.removeMessages(msgWhat);
                }

                Message message = H0.obtainMessage(msgWhat, tryCheckReason);
                H0.sendMessageDelayed(message, delayUpdateTime);
                return;
            }

            checkFavoriteMusicData(mFavoriteMusicList, tryCheckReason);
        }
    }

    @Override
    public void closeDataModel() {
        super.closeDataModel();

        // 写页面数据到磁盘
        MediaPageState.instance()
                .saveAndCloseInstance();
    }

    /** 当前消息定义 **/
    private interface MsgEx {
        int MSG_IDLE = -1;

        // 关闭媒体数据库（测试用，禁止调用）
        int MSG_CLOSE_MEDIA_DATABASE = 1;

        // 更新最喜欢的音乐列表
        int MSG_UPDATE_FAVORITE_MUSIC_LIST = 2;
    }

    @Override
    protected void onHandleMessage(@NonNull Message msg) {
        super.onHandleMessage(msg);

        switch (msg.what) {
            case MsgEx.MSG_UPDATE_FAVORITE_MUSIC_LIST:
                // 更新消息必须带更新参数
                if (msg.obj instanceof String) {
                    onMsgUpdateFavoriteMusicList((String) msg.obj);
                }
                break;
            case MsgEx.MSG_CLOSE_MEDIA_DATABASE:
            case MsgEx.MSG_IDLE:
            default:
                break;
        }
    }

    /**
     * 更新最喜欢的音乐列表信息
     * @see MsgEx#MSG_UPDATE_FAVORITE_MUSIC_LIST
     *
     * @param updateReason 更新原因
     */
    private void onMsgUpdateFavoriteMusicList(@NonNull String updateReason) {
        tryCheckFavoriteMusicData(updateReason, -1);
    }
}
