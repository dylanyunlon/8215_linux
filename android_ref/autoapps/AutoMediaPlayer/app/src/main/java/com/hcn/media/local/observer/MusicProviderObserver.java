package com.hcn.media.local.observer;

import static com.hcn.media.provider.MusicProvider.BASE_URI;

import android.app.Service;
import android.content.ContentValues;
import android.content.Context;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.AutoMediaPlayer.BuildConfig;
import com.hcn.common.misc.LogUtils;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.FavoriteManager;
import com.hcn.mediaservice.data.MusicInfo;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.Objects;


/**
 * 音乐事件观察者
 * <pre>
 *    给外部提供的API有控制音乐的接口；
 *    在这里对外部控制进行观察；
 * </pre>
 */
public class MusicProviderObserver {

    private final String TAG = this.getClass().getSimpleName();

    private final AppGlobalData mAppData;
    private final FavoriteManager fm;
    private final Context mContext;
    private final Reference<Service> mOwnerRef;
    private ContentObserver mProviderObserver;

    // 数据刷新防抖、延时等控制
    private Handler mHandler = null;
    private Runnable mUpdateAllDataRunnable = null;

    public MusicProviderObserver(Service service) {
        mOwnerRef = new WeakReference<>(service);
        fm = FavoriteManager.getInstance();
        mAppData = AppGlobalData.getInstance();
        mContext = mOwnerRef.get();
        mHandler = new Handler(Looper.getMainLooper());
    }

    public interface IProviderCallback {

        void onPlayModeUpdate(int playMode);

    }

    // 开启观察者
    public void startProviderObserver(IProviderCallback mCallback) {
        if (mContext == null) {
            return;
        }
        mProviderObserver = new ContentObserver(new Handler()) {
            @Override
            public void onChange(boolean selfChange, @Nullable Uri uri) {
                super.onChange(selfChange, uri);
                // 同步外部更新播放模式
                LogUtils.vTag(TAG, "onChange: uri " + uri.getLastPathSegment() + " selfChange " + selfChange);
                if (!selfChange && uri.toString().contains("playMode")) {
                    int PLAY_MODE = 0;
                    Cursor cursor = mContext.getContentResolver().query(uri, null, null, null, null);
                    if (cursor != null) {
                        if (cursor.moveToFirst()) {
                            PLAY_MODE = cursor.getInt(cursor.getColumnIndexOrThrow("playMode"));
                            mCallback.onPlayModeUpdate(PLAY_MODE);
                        }
                    }
                }
                // 同步外部更新收藏状态
                if (!selfChange && uri.toString().contains("favoriteState")) {
                    Cursor cursor = mContext.getContentResolver().query(uri, null, null, null, null);
                    if (cursor != null) {
                        if (cursor.moveToFirst()) {
                            int favoriteState = cursor.getInt(cursor.getColumnIndexOrThrow("favoriteState"));
                            MusicInfo info = mAppData.mCurrentMediaInfo;
                            if (Objects.isNull(info)) {
                                return;
                            }
                            if (favoriteState == 1) {
                                // 收藏成功与否
                                if (fm.addFavoriteMusic(info, false)) {
                                    info.mFavorite = true;
                                }
                            } else {
                                info.mFavorite = false;
                                fm.removeFavoriteMusic(info);
                            }
                        }
                    }
                }
            }
        };

        mContext.getContentResolver().registerContentObserver(BASE_URI, true, mProviderObserver);

        // 收藏事件监听
        FavoriteManager.IOperateListener mListener = new FavoriteManager.IOperateListener() {
            @Override
            public void onFavoriteEvent(int listType, String operate, Object obj0) {
                // 判断是否为 CurrentMediaInfo 状态变化，如不是则无需通知外部刷新
                if (obj0 instanceof FavoriteManager.InfoPackage) {
                    MusicInfo info = ((FavoriteManager.InfoPackage) obj0).info;
                    if (!Objects.isNull(info) && info != mAppData.mCurrentMediaInfo) {
                        LogUtils.vTag(TAG, "onFavoriteEvent: not current music ");
                        return;
                    }
                }

                LogUtils.vTag(TAG, "onFavoriteEvent: operate " + operate);
                switch (operate) {
                    case FavoriteManager.OPERATE_ADD:
                        updateFavoriteState(true);
                        break;
                    case FavoriteManager.OPERATE_REMOVE:
                        updateFavoriteState(false);
                        break;
                    default:
                        break;
                }
            }
        };

        fm.addOperateListener(FavoriteManager.Type.MUSIC, mListener);

        mUpdateAllDataRunnable = new Runnable() {
            @Override
            public void run() {
                // 全数据刷新前先重置数据，保证数据安全
                Uri resetUri = Uri.withAppendedPath(BASE_URI, "resetData");
                mContext.getContentResolver().update(resetUri, null, null, null);

                // 更新播放列表
                for (MusicInfo musicInfo : mAppData.musicPlaylist()) {
                    updateData("pathList", musicInfo.mFilePath);
                }

                // 更新当前播放位置
                updateData("currentPosition", mAppData.musicPlayPosition());

                // 更新播放模式
                updatePlayMode(mAppData.musicRepeatMode());

                MusicInfo currentInfo = mAppData.currentMediaInfo();
                if (currentInfo != null){
                    // 更新当前播放路径
                    updateData("currentPath", currentInfo.mFilePath);

                    // 更新收藏状态
                    updateFavoriteState(currentInfo.mFavorite);
                }
            }
        };
    }

    public void stopProviderObserver() {
        if (mContext == null || !BuildConfig.SUPPORT_MUSIC_PROVIDER) {
            return;
        }
        mContext.getContentResolver().unregisterContentObserver(mProviderObserver);
    }

    /**
     * 全数据刷新仅发生在触发列表播放、切换上下曲的时候
     * 刷新延迟200ms，做防抖处理
     */
    public void updateAllMusicData() {
        if (mHandler != null) {
            mHandler.removeCallbacks(mUpdateAllDataRunnable);
            mHandler.postDelayed(mUpdateAllDataRunnable, 200);
        }
    }

    // 更新播放模式
    public void updatePlayMode(int musicRepeatMode) {
        updateData("playMode", musicRepeatMode);
    }

    // 更新收藏状态
    public void updateFavoriteState(boolean isFavorite) {
        updateData("favoriteState", isFavorite);
    }

    // 更新频谱
    public void updateFrequencyData(@NonNull byte[] data) {
        updateData("currentFrequency", data);
    }

    // 泛型方法，用于更新任意类型的数据,简化 BuildConfig 控制用。
    private <T> void updateData(String uri, T value) {
        if (!BuildConfig.SUPPORT_MUSIC_PROVIDER || Objects.isNull(value)) {
            return;
        }

        ContentValues values = new ContentValues();

        if (value instanceof Integer) {
            values.put(uri, (Integer) value);
        } else if (value instanceof String) {
            values.put(uri, (String) value);
        } else if (value instanceof Boolean) {
            values.put(uri, (Boolean) value ? 1 : 0);
        } else if (value instanceof byte[]) {
            values.put(uri, (byte[]) value);
        } else {
            throw new IllegalArgumentException("Unsupported data type: " + value.getClass().getName());
        }

        Uri dataUri = Uri.withAppendedPath(BASE_URI, uri);
        mContext.getContentResolver().update(dataUri, values, null, null);
    }
}
