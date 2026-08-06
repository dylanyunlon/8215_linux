package com.hcn.media.api;

import android.content.ContentValues;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.pm.ProviderInfo;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.util.Log;

import androidx.annotation.Nullable;

import com.hcn.media.base.IExternalControlCallback;
import com.hcn.media.utils.LogUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * 音乐信息对外API
 * <pre>
 *    给外部提供的API，能控制音乐相关的信息；
 *    目前可控信息有播放时间、路径、频谱、列表、播放模式、随机索引、收藏状态；
 *    封装控制方法接口给外部调用；
 * </pre>
 */
public class HMusicProviderApi {

    private final String TAG = this.getClass().getSimpleName();

    public static final String AUTHORITY = "com.hcn.media.provider.MusicProvider";
    public static final Uri BASE_URI = Uri.parse("content://" + AUTHORITY);
    public static boolean ProviderAvailable = false;

    private Context mContext;
    private IExternalControlCallback mCallback;
    private ContentObserver mProviderObserver;

    public HMusicProviderApi(Context context, IExternalControlCallback callback) {
        this.mContext = context;
        this.mCallback = callback;
    }

    public void isProviderAvailable() {
        // 判断 ContentProvider 是否可用，不可用时被外部调用方法会引起崩溃
        PackageManager pm = mContext.getPackageManager();
        ProviderInfo providerInfo = pm.resolveContentProvider(AUTHORITY, 0);
        ProviderAvailable = (providerInfo != null);
        Log.w(TAG, "isProviderAvailable: ProviderAvailable " + ProviderAvailable);
    }


    public void startObserving() {
        isProviderAvailable();
        mProviderObserver = new ContentObserver(new Handler()) {
            @Override
            public void onChange(boolean selfChange, @Nullable Uri uri) {
                super.onChange(selfChange, uri);
                // 数据变更时，获取最新列表
                List<String> updatedPaths = getPathlist();
                if (mCallback != null) {
                    mCallback.onPlaylistUpdated(updatedPaths);
                    mCallback.onPlayModeUpdated(getPlayMode());
                    mCallback.onCurrentPathUpdated(getCurrentPath());
                    mCallback.onCurrentPositionUpdated(getCurrentPosition());
                    mCallback.onFavoriteStateUpdated(getFavoriteState());
                }
            }
        };
        if (ProviderAvailable) {
            LogUtils.eTag(TAG, "ContentObserver registered successfully.");
            mContext.getContentResolver().registerContentObserver(BASE_URI, true, mProviderObserver);
        } else {
            LogUtils.eTag(TAG, "ContentObserver registered failed.");
        }
    }

    public void stopObserving() {
        if (mProviderObserver == null) {
            return;
        }
        mContext.getContentResolver().unregisterContentObserver(mProviderObserver);
    }

    public List<String> getPathlist() {
        List<String> pathList = getDataList("pathList", String.class);
        if (pathList == null) {
            LogUtils.eTag(TAG, "pathList is null");
            return new ArrayList<>();
        }
        return pathList;
    }

    public int getPlayMode() {
        Integer playMode = getData("playMode", Integer.class);
        if (playMode == null) {
            LogUtils.eTag(TAG, "playMode is null");
            return 0;
        }
        return playMode;
    }

    public String getCurrentPath() {
        String currentPath = getData("currentPath", String.class);
        if (currentPath == null) {
            LogUtils.eTag(TAG, "currentPath is null");
            return "";
        }
        return currentPath;
    }

    public int getCurrentPosition() {
        Integer currentPosition = getData("currentPosition", Integer.class);
        if (currentPosition == null) {
            LogUtils.eTag(TAG, "currentPosition is null");
            return 0;
        }
        return currentPosition;
    }

    public int getFavoriteState() {
        Integer favoriteState = getData("favoriteState", Integer.class);
        if (favoriteState == null) {
            LogUtils.eTag(TAG, "favoriteState is null");
            return 0;
        }
        return favoriteState;
    }


    public void setPlayMode(int playMode) {
        if (!ProviderAvailable) {
            LogUtils.eTag(TAG, "Failed to find provider");
            return;
        }
        ContentValues values = new ContentValues();
        Uri playModeUri = Uri.withAppendedPath(BASE_URI, "playMode");
        // 0:顺序播放 2:单曲循环 3:随机播放
        switch (playMode) {
            case 0:
                values.put("playMode", 0);
                break;
            case 2:
                values.put("playMode", 2);
                break;
            case 3:
                values.put("playMode", 3);
                break;
        }
        mContext.getContentResolver().update(playModeUri, values, null, null);
    }

    public void setFavoriteState(int favoriteState) {
        if (!ProviderAvailable) {
            LogUtils.eTag(TAG, "Failed to find provider");
            return;
        }
        ContentValues values = new ContentValues();
        Uri favoriteStateUri = Uri.withAppendedPath(BASE_URI, "favoriteState");
        values.put("favoriteState", favoriteState);
        mContext.getContentResolver().update(favoriteStateUri, values, null, null);
    }

    // 通用方法用于获取单个值
    public <T> T getData(String columnName, Class<T> returnType) {
        T result = null;
        Cursor cursor = null;
        try {
            Uri dataUri = Uri.withAppendedPath(BASE_URI, columnName);
            cursor = mContext.getContentResolver().query(dataUri, null, null, null, null);

            if (cursor != null) {
                if (cursor.moveToFirst()) {
                    int columnIndex = cursor.getColumnIndexOrThrow(columnName);
                    if (returnType == Integer.class) {
                        result = returnType.cast(cursor.getInt(columnIndex));
                    } else if (returnType == String.class) {
                        result = returnType.cast(cursor.getString(columnIndex));
                    }
                }
            }
        } catch (NullPointerException e) {
            LogUtils.eTag(TAG, "Null pointer exception when accessing provider: " + e.getMessage());
        } catch (IllegalArgumentException e) {
            LogUtils.eTag(TAG, "Invalid argument: " + e.getMessage());
        } catch (Exception e) {
            LogUtils.eTag(TAG, "Unexpected error: " + e.getMessage());
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return result;
    }

    // 通用方法用于获取列表数据
    public <T> List<T> getDataList(String columnName, Class<T> returnType) {
        List<T> resultList = new ArrayList<>();
        Cursor cursor = null;
        try {
            Uri dataUri = Uri.withAppendedPath(BASE_URI, columnName);
            cursor = mContext.getContentResolver().query(dataUri, null, null, null, null);

            if (cursor != null) {
                while (cursor.moveToNext()) {
                    int columnIndex = cursor.getColumnIndexOrThrow(columnName);
                    if (returnType == Integer.class) {
                        resultList.add(returnType.cast(cursor.getInt(columnIndex)));
                    } else if (returnType == String.class) {
                        resultList.add(returnType.cast(cursor.getString(columnIndex)));
                    }
                }
            }
        } catch (NullPointerException e) {
            LogUtils.eTag(TAG, "Null pointer exception when accessing provider: " + e.getMessage());
        } catch (IllegalArgumentException e) {
            LogUtils.eTag(TAG, "Invalid argument: " + e.getMessage());
        } catch (Exception e) {
            LogUtils.eTag(TAG, "Unexpected error: " + e.getMessage());
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return resultList;
    }

}
