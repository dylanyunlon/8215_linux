package com.hcn.media.provider;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.common.misc.LogUtils;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * 音乐信息提供者
 * <pre>
 *    将音乐的相关信息存储在内存中，供第三方调用；
 *    目前可控信息有播放时间、路径、频谱、列表、播放模式、随机索引、收藏状态；
 * </pre>
 */
public class MusicProvider extends ContentProvider {

    private final String TAG = this.getClass().getSimpleName();

    public static final String AUTHORITY = "com.hcn.media.provider.MusicProvider";
    public static final Uri BASE_URI = Uri.parse("content://" + AUTHORITY);

    /**
     * 延迟刷新机制
     * 通过handler来合并频繁多次的刷新，减少开销
     */
    public static final int NOTIFY_DELAY = 500;
    public static int CURRENT_MATCH = 0;

    private Handler mHandler = null;
    private Runnable mNotifyRunnable = null;

    /**
     * 定义 URI 匹配句柄
     * 后续可拓展更多内容
     */
    public static final int RESET_DATA = 0;
    public static final int CURRENT_TIME = 1;
    public static final int CURRENT_PATH = 2;
    public static final int CURRENT_FREQUENCY = 3;
    public static final int PATH_LIST = 4;
    public static final int PLAY_MODE = 5;
    public static final int CURRENT_POSITION = 6;
    public static final int FAVORITE_STATE = 7;

    /**
     * 音乐信息相关数据
     * 以静态变量的方式保存在内存中
     */
    public static int time;
    public static String path;
    public static byte[] frequency;
    public static List<String> pathList;
    public static int playMode;
    public static int currentPosition;
    public static int favoriteState;

    public static final UriMatcher URI_MATCHER = new UriMatcher(UriMatcher.NO_MATCH) {
        {
            addURI(AUTHORITY, "resetData", RESET_DATA);
            addURI(AUTHORITY, "currentTime", CURRENT_TIME);
            addURI(AUTHORITY, "currentPath", CURRENT_PATH);
            addURI(AUTHORITY, "currentFrequency", CURRENT_FREQUENCY);
            addURI(AUTHORITY, "pathList", PATH_LIST);
            addURI(AUTHORITY, "playMode", PLAY_MODE);
            addURI(AUTHORITY, "currentPosition", CURRENT_POSITION);
            addURI(AUTHORITY, "favoriteState", FAVORITE_STATE);
        }
    };

    @Override
    public boolean onCreate() {
        // CopyOnWriteArrayList 可以有效避免 ConcurrentModificationException
        pathList = new CopyOnWriteArrayList<>();
        mHandler = new Handler(Looper.getMainLooper());
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {
        MatrixCursor cursor;
        switch (URI_MATCHER.match(uri)) {
            case CURRENT_TIME:
                cursor = new MatrixCursor(new String[]{"currentTime"});
                cursor.addRow(new Object[]{time});
                break;
            case CURRENT_PATH:
                cursor = new MatrixCursor(new String[]{"currentPath"});
                cursor.addRow(new Object[]{path});
                break;
            case CURRENT_FREQUENCY:
                cursor = new MatrixCursor(new String[]{"currentFrequency"});
                cursor.addRow(new Object[]{frequency});
                break;
            case PATH_LIST:
                cursor = new MatrixCursor(new String[]{"pathList"});
                for (String path : pathList) {
                    cursor.addRow(new Object[]{path});
                }
                break;
            case PLAY_MODE:
                cursor = new MatrixCursor(new String[]{"playMode"});
                cursor.addRow(new Object[]{playMode});
                break;
            case CURRENT_POSITION:
                cursor = new MatrixCursor(new String[]{"currentPosition"});
                cursor.addRow(new Object[]{currentPosition});
                break;
            case FAVORITE_STATE:
                cursor = new MatrixCursor(new String[]{"favoriteState"});
                cursor.addRow(new Object[]{favoriteState});
                break;
            default:
                throw new IllegalArgumentException("Unknown URI: " + uri);
        }
        return cursor;
    }

    @Nullable
    @Override
    public String getType(@NonNull Uri uri) {
        return null;
    }


    @Override
    public Uri insert(Uri uri, ContentValues values) {

        return null;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        if (CURRENT_MATCH != URI_MATCHER.match(uri)) {
            // 避免批量写入时重复打印
            CURRENT_MATCH = URI_MATCHER.match(uri);
            LogUtils.vTag(TAG, "update: " + uri + " match: " + URI_MATCHER.match(uri));
        }

        // 刷新防抖处理
        if (mNotifyRunnable != null) {
            mHandler.removeCallbacks(mNotifyRunnable);
        }

        mNotifyRunnable = new Runnable() {
            @Override
            public void run() {
                getContext().getContentResolver().notifyChange(uri, null);
                LogUtils.vTag(TAG, "Notify Success ");
            }
        };

        switch (URI_MATCHER.match(uri)) {
            case RESET_DATA:
                resetData();
                return 0;
            case CURRENT_TIME:
                time = values.getAsInteger("currentTime");
                break;
            case CURRENT_PATH:
                path = values.getAsString("currentPath");
                break;
            case CURRENT_FREQUENCY:
                frequency = values.getAsByteArray("currentFrequency");
                break;
            case PATH_LIST:
                pathList.add(values.getAsString("pathList"));
                break;
            case PLAY_MODE:
                playMode = values.getAsInteger("playMode");
                break;
            case CURRENT_POSITION:
                currentPosition = values.getAsInteger("currentPosition");
                break;
            case FAVORITE_STATE:
                favoriteState = values.getAsInteger("favoriteState");
                break;
            default:
                throw new IllegalArgumentException("Unknown URI: " + uri);
        }

        mHandler.postDelayed(mNotifyRunnable, NOTIFY_DELAY);

        return 0;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {

        return 0;
    }

    public void resetData() {
        pathList.clear();
        time = 0;
        path = null;
        frequency = null;
        playMode = 0;
        currentPosition = 0;
        favoriteState = 0;
        CURRENT_MATCH = -1;
        LogUtils.vTag(TAG, "Reset Data ");
    }
}

