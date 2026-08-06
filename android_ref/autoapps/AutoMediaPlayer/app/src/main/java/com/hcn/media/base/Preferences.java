package com.hcn.media.base;

import android.content.Context;
import android.content.SharedPreferences;

import androidx.annotation.NonNull;

import com.hcn.media_base.constant.IMusicState;

/**
 * 偏好数据存储
 * <p> 当前进程的私有数据状态记忆体；
 *
 * @author 65821
 */
public class Preferences {

    /** 多媒体 UI 状态键值 **/
    private static final String KEY_MEDIAPLAYER = "media-player";

    /** 多媒体播放模式键值 **/
    private static final String KEY_REPEAT_MODE = "repeatmode";

    /** 读取指定的私有状态值从媒体播放器偏好存储 **/
    public static int readIntFromSharedPreferences(@NonNull Context context,
                                                    String key,
                                                    int defValue) {
        SharedPreferences shareData =
                context.getSharedPreferences(
                        KEY_MEDIAPLAYER, Context.MODE_PRIVATE);
        return shareData.getInt(key, defValue);
    }

    /** 保存指定的私有状态值到媒体播放器偏好存储 **/
    public static void writeIntToSharedPreferences(@NonNull Context context,
                                                    String key,
                                                    int value) {
        SharedPreferences.Editor shareData =
                context.getSharedPreferences(
                        KEY_MEDIAPLAYER, Context.MODE_PRIVATE).edit();
        shareData.putInt(key, value);
        shareData.apply();
    }

    /** 读取媒体播放循环模式 **/
    public static int readPlayRepeatMode(@NonNull Context context,
                                         int type) {
        SharedPreferences sharedPreferences
                = context.getSharedPreferences(
                KEY_REPEAT_MODE, Context.MODE_PRIVATE);
        if (type == IMusicState.MEDIA_TYPE_MUSIC) {
            return sharedPreferences.getInt("music", IMusicState.REPEAT_MODE_QUEUE);
        } else {
            return sharedPreferences.getInt("video", IMusicState.REPEAT_MODE_QUEUE);
        }
    }

    /** 保存媒体播放循环模式 **/
    public static void writePlayRepeatMode(@NonNull Context context,
                                           int type,
                                           int repeatMode) {
        SharedPreferences.Editor editor
                = context.getSharedPreferences(
                KEY_REPEAT_MODE, Context.MODE_PRIVATE).edit();
        if (type == IMusicState.MEDIA_TYPE_MUSIC) {
            editor.putInt("music", repeatMode);
        } else {
            editor.putInt("video", repeatMode);
        }
        editor.apply();
    }
}
