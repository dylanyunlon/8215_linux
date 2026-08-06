package com.hcn.media_common.utils;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.media.MediaMetadataRetriever;
import android.text.TextUtils;

import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_base.HMediaConfig;

import java.io.File;
import java.io.IOException;
import java.util.Locale;

/**
 * ID3 工具类
 * @author 65821
 */
public final class MediaID3Util {
    public static final String TAG = "MediaID3Util";

    /** 解析包 Bitmap 为 null 的原因 / 没有有效图形资源发现 **/
    public static final int REASON_NO_GRAPHIC_FOUND = -100;

    /** 解析包 Bitmap 为 null 的原因 / 不能被解码的图形资源 **/
    public static final int REASON_NOT_BE_DECODED = -101;

    /** Bitmap Package **/
    public static final class HBmpPackage {
        public Bitmap bmp;
        public int reason; // 如果 bmp 是 null, 就需要查找原因。

        public HBmpPackage() {
            bmp = null;
            reason = 0;
        }
    }

    /** [检索目标 ID3 信息] **/
    public static void retrieveTargetID3Info(MusicInfo info) {
        if (null == info || TextUtils.isEmpty(info.mFilePath)) {
            return;
        }

        File file = new File(info.mFilePath);
        if (!file.exists()) {
            return;
        }

        String filePath = info.mFilePath;
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();

        try {
            retriever.setDataSource(filePath);

            String title = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_TITLE);
            String album = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_ALBUM);
            String artist = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_ARTIST);
            String duration = retriever.extractMetadata(
                    MediaMetadataRetriever.METADATA_KEY_DURATION);

            info.mID3Type = MusicInfo.ID3_TYPE_EXTRACTED;
            if (TextUtils.isEmpty(title)) {
                info.mTitle = info.mFileName;
            } else {
                info.mTitle = title;
            }

            if (TextUtils.isEmpty(album)) {
                info.mAlbum = "<Unknown>";
            } else {
                info.mAlbum = album;
            }

            if (TextUtils.isEmpty(artist)) {
                info.mArtist = "<Unknown>";
            } else {
                info.mArtist = artist;
            }

            if (TextUtils.isEmpty(duration)) {
                info.mTotalTime = 0;
            } else {
                info.mTotalTime = Integer.parseInt(duration);
            }
        } catch (Exception ex) {
            info.mID3Type = MusicInfo.ID3_TYPE_ERROR;
        } finally {
            try {
                retriever.release();
            } catch (IOException ignored) {
            }
        }
    }

    /**
     * 封面专辑宽度限制
     * <pre>
     *    太大了会占用内存信息
     *    正常来说专辑封面基本是正方形的
     * <pre>
     */
    private static final int COVER_W_LIMIT = 300;
    private static final int COVER_H_LIMIT = 300;
    private static final int MAX_COVER_WH_LIMIT = 512;

    /** [检索音乐专辑封面] */
    public static HBmpPackage retrieveMusicAlbumCover(String filePath, boolean originalImage) {
        HBmpPackage bmpPackage = new HBmpPackage();
        Bitmap bitmap = null;
        Bitmap target = null;

        if (TextUtils.isEmpty(filePath)) {
            bmpPackage.reason = -1;
            return bmpPackage;
        }

        File file = new File(filePath);
        if (!file.exists()) {
            bmpPackage.reason = -2;
            return bmpPackage;
        }

        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            retriever.setDataSource(filePath);

            // 此方法找到与数据源相关联的可选图形或相册/封面艺术.
            byte[] art = retriever.getEmbeddedPicture();
            if (art != null && art.length > 0) {
                bitmap = BitmapFactory.decodeByteArray(art, 0, art.length);
                if (null != bitmap) {
                    int width = bitmap.getWidth();
                    int height = bitmap.getHeight();
                    boolean reduceQuality = width > COVER_W_LIMIT || height > COVER_H_LIMIT;
                    if (reduceQuality && !originalImage) {
                        if (width > MAX_COVER_WH_LIMIT
                                && height > MAX_COVER_WH_LIMIT) {
                            width = MAX_COVER_WH_LIMIT;
                            height = MAX_COVER_WH_LIMIT;
                        } else {
                            width = COVER_W_LIMIT;
                            height = COVER_H_LIMIT;
                        }

                        // 降低图片质量，避免数据太大；
                        target = Bitmap.createBitmap(width, height, bitmap.getConfig());
                        Canvas canvas = new Canvas(target);
                        canvas.drawBitmap(bitmap, null,
                                new Rect(0, 0, target.getWidth(), target.getHeight()), null);
                    }
                } else {
                    bmpPackage.reason = REASON_NOT_BE_DECODED;
                }
            } else {
                bmpPackage.reason = REASON_NO_GRAPHIC_FOUND;
            }
        } catch (Exception ex) {
            LogUtil.e(TAG, "retrieveMusicAlbumCover: " + ex.toString());
        } finally {
            if (bitmap != null) {
                if (target != null) {
                    bitmap.recycle();
                    bitmap = null;

                    bmpPackage.bmp = target;
                } else {
                    bmpPackage.bmp = bitmap;
                }
            }

            try {
                retriever.release();
            } catch (IOException ignored) {
            }
        }

        return bmpPackage;
    }

    // [检索视频特定显示帧]
    public static HBmpPackage retrieveVideoFrameAtTime(String filePath, long time) {
        HBmpPackage bmpPackage = new HBmpPackage();
        Bitmap bitmap = null;
        Bitmap target = null;

        if (TextUtils.isEmpty(filePath)) {
            bmpPackage.reason = -1;
            return bmpPackage;
        }

        File file = new File(filePath);
        if (!file.exists()) {
            bmpPackage.reason = -2;
            return bmpPackage;
        }

        // 获取文件后缀
        boolean useSoftDecoder;
        String strSuffix = ".suffix";
        int pos = filePath.lastIndexOf('.');

        if (pos != -1) {
            strSuffix = filePath.substring(pos) + ".";
            strSuffix = strSuffix.toLowerCase(Locale.getDefault());
            useSoftDecoder = HMediaConfig.VITAMIO_VIDEO_FRAME_SUFFIX.contains(strSuffix);
        } else {
            bmpPackage.reason = -3;
            return bmpPackage;
        }

        // [特定格式使用 Vitamio 提供的接口]
        io.vov.vitamio.MediaMetadataRetriever vitamioRetriever = null;
        MediaMetadataRetriever retriever = null;
        if (useSoftDecoder) {
            // 参考仓库：
            // https://github.com/wseemann/FFmpegMediaMetadataRetriever
            vitamioRetriever = new io.vov.vitamio.MediaMetadataRetriever(null);
        } else {
            retriever = new MediaMetadataRetriever();
        }

        // [获取视频特定显示帧位图]
        try {
            String duration = "";
            if (useSoftDecoder) {
                vitamioRetriever.setDataSource(filePath);
                duration = vitamioRetriever.extractMetadata(
                        io.vov.vitamio.MediaMetadataRetriever.METADATA_KEY_DURATION);
            } else {
                retriever.setDataSource(filePath);
                duration = retriever.extractMetadata(
                        MediaMetadataRetriever.METADATA_KEY_DURATION);
            }

            int durationMs = 0;
            if (!TextUtils.isEmpty(duration)) {
                durationMs = Integer.parseInt(duration);
            }

            if (time < 0) {
                long timeMs = 5 * 1000;
                if (durationMs < timeMs) {
                    timeMs = durationMs;
                }

                if (useSoftDecoder) {
                    // .flv 不能用 vitamio 来获取视频显示帧, 初步发现 libffmpeg 报错
                    bitmap = vitamioRetriever.getFrameAtTime(timeMs * 1000);
                } else {
                    bitmap = retriever.getFrameAtTime(timeMs * 1000);

                    if (null == bitmap) {
                        // avi 特殊处理下，支持的越多越好
                        if (".avi.".equals(strSuffix)) {
                            LogUtil.e(TAG, "retrieveVideoFrameAtTime: "
                                    + "getFrameAtTime = null,  file = " + filePath);

                            // 硬解码不支持
                            retriever.release();
                            retriever = null;

                            // 尝试软解码获取
                            useSoftDecoder = true;
                            vitamioRetriever = new io.vov.vitamio.MediaMetadataRetriever(null);
                            vitamioRetriever.setDataSource(filePath);
                            bitmap = vitamioRetriever.getFrameAtTime(timeMs * 1000);
                        }
                    }
                }
            } else {
                if (useSoftDecoder) {
                    bitmap = vitamioRetriever.getFrameAtTime(time);
                } else {
                    bitmap = retriever.getFrameAtTime(time);
                }
            }

            if (bitmap != null) {
                target = Bitmap.createBitmap(150, 150, bitmap.getConfig());
                Canvas canvas = new Canvas(target);
                canvas.drawBitmap(bitmap, null,
                        new Rect(0, 0, target.getWidth(), target.getHeight()),
                        null);

                bmpPackage.bmp = target;
            } else {
                bmpPackage.reason = REASON_NO_GRAPHIC_FOUND;
            }
        } catch (Exception ex) {
            LogUtil.e(TAG, "retrieveVideoFrameAtTime: " + ex.toString());
        } finally {
            if (null != bitmap) {
                bitmap.recycle();
                bitmap = null;
            }

            if (useSoftDecoder) {
                if (null != vitamioRetriever) {
                    vitamioRetriever.release();
                    vitamioRetriever = null;
                }
            } else {
                try {
                    retriever.release();
                } catch (IOException ignored) {
                }
            }
        }

        return bmpPackage;
    }
}
