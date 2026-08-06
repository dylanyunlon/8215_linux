package com.hcn.media_common.file;

import android.annotation.SuppressLint;
import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.text.TextUtils;

import androidx.annotation.NonNull;

import com.hcn.common.utils.HUriUtils;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_common.debug.LogUtil;

import java.io.File;
import java.util.Objects;

/**
 * 媒体 Uri 工具
 * <pre>
 *    主要是从 Uri 获取文件路径；
 *    后续建议直接使用 {@link com.hcn.auto_compat.file.MediaUtilsEx}
 * </pre>
 *
 * @author 65821
 * @deprecated 兼容历史平台版本
 */
public class MediaUriUtils {
    private static final String TAG = MediaUriUtils.class.getSimpleName();

    /**
     * 解析音频 Uri 文件路径
     * <p> 统一使用这个接口来解析，并兼容新旧软件；
     *
     * @param context 上下文环境
     * @param uri 需要解析的 uri 对象
     * @return 文件绝对路径
     */
    public static String parseAudioUriFilePath(@NonNull Context context, @NonNull Uri uri) {
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.P) {
            File file = HUriUtils.uri2File(uri);
            if (!Objects.isNull(file)) {
                return file.getAbsolutePath();
            }
        }

        return readMusicPath4Uri(context, uri);
    }

    /**
     * 解析视频 Uri 文件路径
     * <p> 统一使用这个接口来解析，并兼容新旧软件；
     *
     * @param context 上下文环境
     * @param uri 需要解析的 uri 对象
     * @return 文件绝对路径
     */
    public static String parseVideoUriFilePath(@NonNull Context context, @NonNull Uri uri) {
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.P) {
            File file = HUriUtils.uri2File(uri);
            if (!Objects.isNull(file)) {
                return file.getAbsolutePath();
            }
        }

        return readVideoPath4Uri(context, uri);
    }

    /**
     * 解析图片 Uri 文件路径
     * <p> 统一使用这个接口来解析，并兼容新旧软件；
     *
     * @param context 上下文环境
     * @param uri 需要解析的 uri 对象
     * @return 文件绝对路径
     */
    public static String parseImageUriFilePath(@NonNull Context context, @NonNull Uri uri) {
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.P) {
            File file = HUriUtils.uri2File(uri);
            if (!Objects.isNull(file)) {
                return file.getAbsolutePath();
            }
        }

        return readImagePath4Uri(context, uri) ;
    }

    /**
     * 从 Uri 获取音乐路径
     * <p> Android P 以后不再使用它；
     *
     * @param context 上下文环境
     * @param uri 目标 uri 对象
     * @return 对应的文件路径
     * @deprecated 过时的接口不在维护
     */
    @SuppressLint("NewApi")
    private static String readMusicPath4Uri(Context context, Uri uri) {
        String imagePath = null;
        LogUtil.d(TAG, "uri: " + uri);

        if (DocumentsContract.isDocumentUri(context, uri)) {
            String docId = DocumentsContract.getDocumentId(uri);
            assert uri != null;
            String authority = uri.getAuthority();

            LogUtil.d(TAG, "DocumentId: " + docId);
            LogUtil.d(TAG, "Authority: " + authority);

            if ("com.android.providers.media.documents".equals(authority)) {
                String id = docId.split(":")[1];
                String selection = MediaStore.Audio.Media._ID + "=" + id;
                imagePath = getMusicPath(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, selection);
            } else if ("com.android.providers.downloads.documents".equals(authority)) {
                Uri contentUri = ContentUris.withAppendedId(
                        Uri.parse("content://downloads/public_downloads"), Long.parseLong(docId));
                imagePath = getMusicPath(context, contentUri, null);

                LogUtil.e(TAG, "contentUri: " + contentUri + "  imagePath: " + imagePath);
            } else if ("com.android.externalstorage.documents".equals(authority)) {
                String[] split = docId.split(":");
                String type = split[0];
                if ("primary".equalsIgnoreCase(type)) {
                    imagePath = IConstant.PATH_FLASH + "/" + split[1];
                } else if ("sda".equalsIgnoreCase(type)
                        || "sdb".equalsIgnoreCase(type)
                        || "sdc".equalsIgnoreCase(type)
                        || "sdd".equalsIgnoreCase(type)
                        || "sde".equalsIgnoreCase(type)) {
                    imagePath = IConstant.PATH_USB + "/" + type + "/" + split[1];
                } else if ("sdcard1".equalsIgnoreCase(type)) {
                    imagePath = IConstant.PATH_SD + "/" + split[1];
                } else {
                    imagePath = getMusicPath(context, uri, null);
                    if (null != imagePath && imagePath.startsWith("/mnt/media_rw/")) {
                        imagePath = imagePath.replaceFirst("/mnt/media_rw/", "/storage/");
                    }
                }

                LogUtil.e(TAG, "[URL]ImagePath: " + imagePath);
            } else if (!TextUtils.isEmpty(authority)) {
                assert authority != null;
                if (authority.contains("com.android.mtp.documents")) {
                    LogUtil.d(TAG, "trigger USB MTP device!");
                }
            }
        } else {
            assert uri != null;
            if ("content".equalsIgnoreCase(uri.getScheme())) {
                imagePath = getMusicPath(context, uri, null);
                LogUtil.e(TAG, "[URL]ImagePath: " + imagePath);
            } else if ("file".equalsIgnoreCase(uri.getScheme())) {
                imagePath = uri.getPath();
                LogUtil.e(TAG, "[URL]ImagePath: " + imagePath);
            }
        }

        return imagePath;
    }

    /**
     * 获取音乐路径
     * <p> 不严谨的接口
     *
     * @param context
     * @param uri
     * @param selection
     * @return
     * @deprecated 过时的接口
     */
    @SuppressLint("Range")
    private static String getMusicPath(Context context, Uri uri, String selection) {
        String path = null;
        Cursor cursor = context.getContentResolver().query(uri, null, selection, null, null);
        if (cursor != null) {
            if (cursor.moveToFirst()) {
                path = cursor.getString(cursor.getColumnIndex(MediaStore.Audio.Media.DATA));
            }
            cursor.close();
        }
        return path;
    }

    /**
     * 从 Uri 获取视频路径
     * <p> Android P 以后不再使用它；
     *
     * @param context 上下文环境
     * @param uri 目标 uri 对象
     * @return 对应的文件路径
     * @deprecated 过时的接口不在维护
     */
    @SuppressLint("NewApi")
    private static String readVideoPath4Uri(Context context, Uri uri) {
        String imagePath = null;
        LogUtil.d(TAG, "intent.getData: " + uri);

        if (DocumentsContract.isDocumentUri(context, uri)) {
            String docId = DocumentsContract.getDocumentId(uri);
            LogUtil.d(TAG, "getDocumentId(uri): " + docId);

            assert uri != null;
            LogUtil.d(TAG, "uri.getAuthority(): " + uri.getAuthority());

            if ("com.android.providers.media.documents".equals(uri.getAuthority())) {
                LogUtil.e(TAG, uri.getAuthority());

                String id = docId.split(":")[1];
                String selection = MediaStore.Video.Media._ID + "=" + id;
                imagePath = getVideoPath(context, MediaStore.Video.Media.EXTERNAL_CONTENT_URI, selection);
            } else if ("com.android.providers.downloads.documents".equals(uri.getAuthority())) {
                LogUtil.e(TAG, uri.getAuthority());

                Uri contentUri = ContentUris.withAppendedId(
                        Uri.parse("content://downloads/public_downloads"), Long.parseLong(docId));
                imagePath = getVideoPath(context, contentUri, null);

                LogUtil.e(TAG, "contentUri: " + contentUri + ", imagePath: " + imagePath);
            } else if ("com.android.externalstorage.documents".equals(uri.getAuthority())) {
                String[] split = docId.split(":");
                String type = split[0];

                // primary 内置存储
                if ("primary".equalsIgnoreCase(type)) {
                    imagePath = IConstant.PATH_FLASH + "/" + split[1];
                } else if ("sda".equalsIgnoreCase(type)
                        || "sdb".equalsIgnoreCase(type)
                        || "sdc".equalsIgnoreCase(type)
                        || "sdd".equalsIgnoreCase(type)
                        || "sde".equalsIgnoreCase(type)) {
                    imagePath = IConstant.PATH_USB + "/" + type + "/" + split[1];
                } else if ("sdcard1".equalsIgnoreCase(type)) {
                    imagePath = IConstant.PATH_SD + "/" + split[1];
                } else {
                    imagePath = getVideoPath(context, uri, null);
                }

                LogUtil.e(TAG, "ImagePath: " + imagePath);
            }
        } else {
            assert uri != null;
            if ("content".equalsIgnoreCase(uri.getScheme())) {
                // 肯定不是这样暴力处理的，后面再研究下标准规则方式；
                if ("com.android.bluetooth.opp.fileprovider".equals(uri.getAuthority())) {
                    String path = uri.getPath();
                    if (path != null) {
                        if (path.startsWith("/bluetooth/")) {
                            imagePath = IConstant.PATH_FLASH + "/" + path.substring("/bluetooth/".length());
                        }
                    }
                } else {
                    imagePath = getVideoRealPathFromURI(context, uri);
                }

                LogUtil.e(TAG, "content:\\ " + imagePath);
            } else if ("file".equalsIgnoreCase(uri.getScheme())) {
                imagePath = uri.getPath();
            }
        }

        return imagePath;
    }

    /**
     * 获取视频路径
     * <p> 不严谨的接口
     *
     * @param context
     * @param uri
     * @param selection
     * @return
     * @deprecated 过时的接口
     */
    @SuppressLint("Range")
    private static String getVideoPath(Context context, Uri uri, String selection) {
        String path = null;
        Cursor cursor = context.getContentResolver().query(uri, null, selection, null, null);
        if (cursor != null) {
            if (cursor.moveToFirst()) {
                path = cursor.getString(cursor.getColumnIndex(MediaStore.Video.Media.DATA));
            }
            cursor.close();
        }
        return path;
    }

    /**
     * 获取视频路径
     * <p> 不严谨的接口
     *
     * @param context 上下文环境
     * @param uri uri 地址
     * @return
     * @deprecated 过时的接口
     */
    public static String getVideoRealPathFromURI(Context context, Uri uri) {
        String[] projection = {MediaStore.Video.Media.DATA};
        try {
            Cursor cursor = context.getContentResolver()
                    .query(uri, projection, null, null, null);
            if (cursor != null) {
                int column_index = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DATA);
                if (cursor.moveToFirst()) {
                    return cursor.getString(column_index);
                }
                cursor.close();
            }
        } catch (Exception ignored) {
        }
        return null;
    }

    /**
     * 获取图片绝对路径
     * <p> 不严谨的接口
     *
     * @param context 上下文环境
     * @param uri uri 地址
     * @return
     * @deprecated 过时的接口
     */
    private static String getImageAbsolutePath(Context context, Uri uri) {
        String[] projection = {MediaStore.Images.Media.DATA};
        Cursor cursor = context.getContentResolver().query(
                uri, projection, null, null, null);

        if (cursor != null && cursor.getCount() > 0) {
            try {
                int columnIndex = cursor.getColumnIndex(MediaStore.Images.Media.DATA);
                if (columnIndex != -1) {
                    cursor.moveToFirst();
                    return cursor.getString(columnIndex);
                }
            } finally {
                cursor.close();
            }
        }
        return null;
    }

    /**
     * 从 Uri 获取图片路径
     * <p> Android P 以后不再使用它；
     *
     * @param context 上下文环境
     * @param data 目标 uri 对象
     * @return 对应的文件路径
     * @deprecated 过时的接口不在维护
     */
    public static String readImagePath4Uri(Context context, Uri data) {
        if (data != null) {
            if (TextUtils.equals(data.getScheme(), "content")) {
                return getImageAbsolutePath(context, data);
            } else {
                return data.getPath();
            }
        }
        return null;
    }
}
