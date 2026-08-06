package com.hcn.media_common.cache;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.util.LruCache;
import android.widget.AbsListView;
import android.widget.ImageView;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;

import androidx.annotation.DrawableRes;

import com.hcn.common.lang.RunnableEx;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.media_common.utils.Md5Utils;
import com.hcn.media_common.cache.DiskLruCache.Snapshot;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.utils.MediaID3Util;
import com.hcn.media_view.resx.IR;

import java.io.File;
import java.io.OutputStream;
import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;
import java.sql.Ref;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * 缩略图缓存工具
 * <p> 内存缓存 + 磁盘缓存 <br>
 * 这里面还有部分给视图设置 Bitmap/Drawable 的逻辑；
 *
 * @author 86158
 */
public class BitmapCache {
    private static final String TAG = "HBitmapCache";

    private static final int MEM_CACHE_MAX_SIZE = 20 * 1024 * 1024;
    private static final int DISKCACHE_MAX_SIZE = 40 * 1024 * 1024;
    private static BitmapCache S_INSTANCE;

    private final LruCache<String, HBmpPackage> mMemoryCache;
    private DiskLruCache mDiskLruCache = null;
    private final ExecutorService mImageThreadPool;

    private int mBgWidth = 150;
    private int mBgHeight = 88;

    /** 用来记录 Bitmap 是否已是提取过 **/
    public static final class HBmpPackage {
        public Bitmap mBitmap;

        // 是否提取过了: 存在提取过了, 但是 mBitmap 还是为 null 的情况<音视频文件不支持专辑封面>
        public boolean mExtracted;  // boolean 正常占用4个字节, 但是如果是应用到数组中它只占用一个字节。

        public HBmpPackage() {
            mBitmap = null;
            mExtracted = false;
        }

        // 这个不能做到精准计算, 因为 Bitmap 对象还有其它成员。
        public final int sizeOf() {
            if (mBitmap != null) {
                return mBitmap.getByteCount() + 8;
            }

            return 8;
        }
    }

    /** 获取唯一实例对象 **/
    public static BitmapCache getInstance() {
        if (Objects.isNull(S_INSTANCE)) {
            BitmapCache.init(HUtilsEx.getApp());
        }

        return S_INSTANCE;
    }

    /** 初始化 BitmapCache 全局对象 **/
    public static void init(Context context) {
        if (Objects.isNull(S_INSTANCE)) {
            S_INSTANCE = new BitmapCache(context);
        } else {
            throw new RuntimeException(
                    "[BitmapCache] already initialized!");
        }
    }

    private BitmapCache(Context context) {
        // 线程池
        mImageThreadPool = new ThreadPoolExecutor(0, 2,
                60, TimeUnit.SECONDS, new LinkedBlockingQueue<Runnable>());

        // 内存缓存
        mMemoryCache = new LruCache<String, HBmpPackage>(MEM_CACHE_MAX_SIZE) {

            @Override
            protected int sizeOf(String key, HBmpPackage objValue) {
                return objValue.sizeOf();
            }
        };

        // 磁盘缓存
        try {
            // 存储路径: /storage/emulated/0/.HBmpCache
            Context applicationContext = context.getApplicationContext();
            mDiskLruCache = DiskLruCache.open(
                    getDiskCacheDir(applicationContext, ".HBmpCache"),
                    getAppVersion(applicationContext), 1, DISKCACHE_MAX_SIZE);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static File getDiskCacheDir(Context context, String uniqueName) {
        String cachePath = Environment.getExternalStorageDirectory().getAbsolutePath();
        return new File(cachePath + File.separator + uniqueName);
    }

    public int getAppVersion(Context context) {
        try {
            PackageInfo info = context.getPackageManager().getPackageInfo(context.getPackageName(),
                    0);
            return info.versionCode;
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }

        return 1;
    }

    public void addHBmpPackageToMemoryCache(String key, HBmpPackage objValue) {
        HBmpPackage bmpPackage = mMemoryCache.get(key);

        if (null == bmpPackage) {
            mMemoryCache.put(key, objValue);
        } else {
            if (null == bmpPackage.mBitmap) {
                mMemoryCache.put(key, objValue);
            }
        }
    }

    public void addBitmapToDiskCache(String path, Bitmap bitmap) {
        if (null == mDiskLruCache) {
            return;
        }

        String key = Md5Utils.md5(path);

        try {
            if (null == mDiskLruCache.get(key)) {
                DiskLruCache.Editor editor = mDiskLruCache.edit(key);

                if (editor != null) {
                    OutputStream outputStream = editor.newOutputStream(0);

                    if (bitmap.compress(CompressFormat.JPEG, 100, outputStream)) {
                        editor.commit();
                    } else {
                        editor.abort();
                    }
                }

                mDiskLruCache.flush();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public HBmpPackage getHBmpPackageFromMemCache(String key) {
        return mMemoryCache.get(key);
    }

    // 存在缩略图在磁盘缓存
    public boolean existDiskCacheThumbnail(String path) {
        return (getBitmapFromDiskCache(path) != null);
    }

    private Bitmap getBitmapFromDiskCache(String path) {
        Bitmap bitmap = null;

        if (null == mDiskLruCache) {
            return null;
        }

        String key = Md5Utils.md5(path);
        try {
            Snapshot snapshot = mDiskLruCache.get(key);
            if (snapshot != null) {
                bitmap = BitmapFactory.decodeStream(snapshot.getInputStream(0));
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return bitmap;
    }

    /**
     * 加载显示目标缩略图
     *
     * @param path 目标媒体文件
     * @param imageView 缩略图显示视图
     * @param defaultImageResId 默认显示资源
     * @param waiting UI 等待显示（没有就动态加载）
     * @return 位图/NULL
     */
    public Bitmap loadNativeImage(String path,
                                  ImageView imageView,
                                  int defaultImageResId,
                                  boolean waiting) {
        return loadNativeImage(
                path, imageView, defaultImageResId, null, waiting);
    }

    /**
     * 加载显示目标缩略图
     *
     * @param path 目标媒体文件
     * @param imageView 缩略图显示视图
     * @param defaultImageResId 默认显示资源
     * @param callback 结果回调（解析到缩略图才会回调）
     * @param waiting UI 等待显示（没有就动态加载）
     * @return 位图/NULL
     */
    public Bitmap loadNativeImage(String path,
                                  ImageView imageView,
                                  int defaultImageResId,
                                  RunnableEx callback,
                                  boolean waiting) {
        Bitmap bitmap = null;
        boolean needLoad = true;
        HBmpPackage objValue = getHBmpPackageFromMemCache(path);

        if (objValue != null) {
            bitmap = objValue.mBitmap;
            needLoad = !objValue.mExtracted;
        }

        // 组合判断可能需要启动线程的情况
        if (null == objValue || null == bitmap) {
            if (defaultImageResId != 0) {
                if (imageView != null) {
                    imageView.setImageResource(defaultImageResId);
                }
            }

            // 条件 needLoad 可以过滤重复启动线程
            if (waiting && needLoad) {
                mImageThreadPool.execute(
                        new ImageRunnable(path, imageView, callback));
            }
        } else {
            if (imageView != null) {
                imageView.setImageBitmap(bitmap);

                // 通知目标存在缩略图信息
                if (callback != null) {
                    callback.callback(path);
                }
            }
        }

        return bitmap;
    }

    public Bitmap loadNativeImage(String path,
                                  ImageView imageView,
                                  int defaultImageResId,
                                  View background,
                                  int bgWidth,
                                  int bgHeight,
                                  View mask,
                                  boolean bWaiting) {
        mBgWidth = bgWidth;
        mBgHeight = bgHeight;
        Bitmap bitmap = null;
        boolean needLoad = true;
        HBmpPackage objValue = getHBmpPackageFromMemCache(path);

        if (objValue != null) {
            bitmap = objValue.mBitmap;
            needLoad = !objValue.mExtracted;
        }

        // 组合判断可能需要启动线程的情况
        if (null == objValue || null == bitmap) {

            if (defaultImageResId != 0) {
                if (imageView != null) {
                    imageView.setImageResource(defaultImageResId);
                }
            }

            // 条件 needLoad 可以过滤重复启动线程
            if (bWaiting && needLoad) {
                mImageThreadPool.execute(new ImageRunnable(path, imageView, background, mask));
            }
        } else {
            if (background != null) {
                mImageThreadPool.execute(new BlurRunnable(bitmap, background, mask));
            }

            if (imageView != null) {
                imageView.setImageBitmap(bitmap);
            }
        }

        return bitmap;
    }

    private class BlurRunnable implements Runnable {
        private Bitmap mBitmap;
        private Reference<View> mBgViewRef;
        private Reference<View> mMaskViewRef = null;

        public BlurRunnable(Bitmap bitmap, View bgView) {
            mBitmap = bitmap;
            mBgViewRef = new WeakReference<>(bgView);
        }

        public BlurRunnable(Bitmap bitmap, View bgView, View maskView) {
            mBitmap = bitmap;
            mBgViewRef = new WeakReference<>(bgView);
            mMaskViewRef = new WeakReference<>(maskView);
        }

        @SuppressLint("HandlerLeak")
        private final Handler mHandler = new Handler() {

            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);

                if (msg.obj instanceof Bitmap) {
                    Bitmap bitmap = (Bitmap) msg.obj;
                    View bgView = mBgViewRef.get();
                    View maskView = mMaskViewRef.get();

                    Context context = HUtilsEx.getApp().getApplicationContext();
                    Animation fadeoutAnim = AnimationUtils.loadAnimation(context, IR.Anim.fadein);

                    if (bgView != null) {
                        final Drawable newBitmapDrawable = new BitmapDrawable(bitmap);
                        bgView.setBackgroundDrawable(newBitmapDrawable);
                        bgView.startAnimation(fadeoutAnim);
                        bgView.setVisibility(View.VISIBLE);
                    }

                    if (maskView != null) {
                        maskView.startAnimation(fadeoutAnim);
                        maskView.setVisibility(View.VISIBLE);
                    }
                }
            }
        };

        @Override
        public void run() {
            int width = mBitmap.getWidth();
            int height = mBitmap.getHeight();
            LogUtil.v(TAG, " bmp: " + width + " x " + height + ", bg: " + mBgWidth + " x " + mBgHeight);

            if (mBgWidth > mBgHeight) {
                int tempHeight = (int) (width * ((float) mBgHeight / mBgWidth));
                if (tempHeight < height) {
                    height = tempHeight;
                }
            } else {
                int tempWidth = (int) (height * ((float) mBgWidth / mBgHeight));
                if (tempWidth < width) {
                    width = tempWidth;
                }
            }

            LogUtil.v(TAG, "Blur " + width + " x " + height);
            Bitmap overlay = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            Paint paint = new Paint();
            paint.setFlags(Paint.FILTER_BITMAP_FLAG);
            Canvas canvasOverlay = new Canvas(overlay);
            canvasOverlay.drawBitmap(mBitmap, 0, 0, paint);
            Bitmap blurBmp = BlurUtil.fastBlur(overlay, 6);

            Message msg = mHandler.obtainMessage();
            msg.obj = blurBmp;
            mHandler.sendMessage(msg);
        }
    }

    public Bitmap loadNativeImage(String path, ImageView imageView, AbsListView gridView,
            int defaultImageResId, boolean bWaiting) {
        Bitmap bitmap = null;
        boolean needLoad = true;
        HBmpPackage objValue = getHBmpPackageFromMemCache(path);

        if (objValue != null) {
            bitmap = objValue.mBitmap;
            needLoad = !objValue.mExtracted;
        }

        if (null == objValue || null == bitmap) {
            if (defaultImageResId != 0) {
                if (imageView != null) {
                    imageView.setImageResource(defaultImageResId);
                }
            }

            if (bWaiting && needLoad) {
                mImageThreadPool.execute(new GridViewRunnable(path, gridView));
            }
        } else {
            if (imageView != null) {
                imageView.setImageBitmap(bitmap);
            }
        }

        return bitmap;
    }

    public Bitmap loadNativeImage(MusicInfo info,
                                  ImageView imageView,
                                  NativeImageCallBack callback,
                                  int defaultImageResId,
                                  boolean bWaiting) {
        Bitmap bitmap = null;
        boolean needLoad = true;
        HBmpPackage objValue = getHBmpPackageFromMemCache(info.mFilePath);

        if (objValue != null) {
            bitmap = objValue.mBitmap;
            needLoad = !objValue.mExtracted;
        }

        if (null == objValue || null == bitmap) {
            if (defaultImageResId != 0) {
                if (imageView != null) {
                    imageView.setImageResource(defaultImageResId);
                }
            }

            if (bWaiting && needLoad) {
                mImageThreadPool.execute(new ImageCallbackRunnable(info, callback));
            }
        } else {
            if (imageView != null) {
                imageView.setImageBitmap(bitmap);
            }
        }

        return bitmap;
    }
    public Bitmap loadVideoInfoImage(String path,
                                     ImageView imageView,
                                     AbsListView gridView,
                                     Drawable defaultImage,
                                     boolean bWaiting) {
        Bitmap bitmap = null;
        boolean needLoad = true;
        HBmpPackage objValue = getHBmpPackageFromMemCache(path);

        if (objValue != null) {
            bitmap = objValue.mBitmap;
            needLoad = !objValue.mExtracted;
        }

        if (null == objValue || null == bitmap) {
            if (defaultImage != null) {
                if (imageView != null) {
                    imageView.setImageDrawable(defaultImage);
                }
            }

            if (bWaiting && needLoad) {
                mImageThreadPool.execute(new VideoGridViewRunnable(path, gridView));
            }
        } else {
            if (imageView != null) {
                imageView.setImageBitmap(bitmap);
            }
        }

        return bitmap;
    }

    @SuppressLint("UseCompatLoadingForDrawables")
    public Bitmap loadVideoInfoImage(String path,
                                     ImageView imageView,
                                     AbsListView gridView,
                                     @DrawableRes int defaultImageResId,
                                     boolean bWaiting) {
        Drawable defaultImage = null;
        if (imageView != null) {
            Context context = imageView.getContext();
            try {
                defaultImage = context.getDrawable(defaultImageResId);
            } catch (Exception ignored) {
            }
        }

        return loadVideoInfoImage(path, imageView, gridView, defaultImage, bWaiting);
    }

    public Bitmap loadVideoInfoImage(String path,
                                     ImageView imageView,
                                     Drawable defaultImage,
                                     boolean bWaiting) {
        Bitmap bitmap = null;
        boolean needLoad = true;
        HBmpPackage objValue = getHBmpPackageFromMemCache(path);

        if (objValue != null) {
            bitmap = objValue.mBitmap;
            needLoad = !objValue.mExtracted;
        }

        if (null == objValue || null == bitmap) {
            if (defaultImage != null) {
                if (imageView != null) {
                    imageView.setImageDrawable(defaultImage);
                }
            }

            if (bWaiting && needLoad) {
                mImageThreadPool.execute(new VideoImageViewRunnable(path, imageView));
            }
        } else {
            if (imageView != null) {
                imageView.setImageBitmap(bitmap);
            }
        }

        return bitmap;
    }

    public interface NativeImageCallBack {
        /**
         * 回调文件解析完成结果
         * <p> e.g. 专辑封面、视频插图等
         *
         * @param bitmap 位图
         * @param path 路径
         */
        void onImageLoader(Bitmap bitmap, String path);
    }

    @SuppressLint("HandlerLeak")
    private class ImageRunnable implements Runnable {
        private final String mPath;
        private final Reference<ImageView> mImageViewRef;
        private Reference<View> mBackgroundRef = null;
        private Reference<View> mMaskRef = null;
        private final Reference<RunnableEx> mCallbackRef;

        private final Handler mHandler = new Handler(Looper.getMainLooper()) {

            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);

                // 位图类型检查
                if (!(msg.obj instanceof Bitmap)) {
                    return;
                }

                Bitmap bitmap = (Bitmap) msg.obj;
                ImageView imageView = mImageViewRef.get();

                if (imageView != null
                        && ((String) imageView.getTag()).equals(mPath)) {
                    imageView.setImageBitmap(bitmap);

                    if (mBackgroundRef != null) {
                        mImageThreadPool.execute(
                                new BlurRunnable(bitmap, mBackgroundRef.get(),
                                        (mMaskRef != null) ? mMaskRef.get() : null));
                    }

                    // 解析到了缩略图，通知观察者
                    RunnableEx callback = mCallbackRef.get();
                    if (callback != null) {
                        callback.callback(mPath);
                    }
                }
            }
        };

        public ImageRunnable(String path, ImageView imageView) {
            this(path, imageView, null);
        }

        public ImageRunnable(String path, ImageView imageView, RunnableEx callback) {
            mPath = path;
            mImageViewRef = new WeakReference<>(imageView);
            mCallbackRef = new WeakReference<>(callback);
        }

        public ImageRunnable(String path, ImageView imageView, View bgView, View maskView) {
            this(path, imageView, null);

            mBackgroundRef = new WeakReference<>(bgView);
            mMaskRef = new WeakReference<>(maskView);
        }

        @Override
        public void run() {
            MediaID3Util.HBmpPackage bmpPackage =
                    MediaID3Util.retrieveMusicAlbumCover(mPath, false);

            HBmpPackage objValue = new HBmpPackage();
            objValue.mBitmap = bmpPackage.bmp;

            if (bmpPackage.bmp != null) {
                objValue.mExtracted = true;
                addHBmpPackageToMemoryCache(mPath, objValue);

                // 更新 UI 控件显示
                Message msg = mHandler.obtainMessage();
                msg.obj = bmpPackage.bmp;
                mHandler.sendMessage(msg);
            } else {
                // 如果媒体文件本身无有效图形资源, 修改提取标记
                switch (bmpPackage.reason) {
                    case MediaID3Util.REASON_NO_GRAPHIC_FOUND:
                    case MediaID3Util.REASON_NOT_BE_DECODED:
                        objValue.mExtracted = true;
                        addHBmpPackageToMemoryCache(mPath, objValue);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    @SuppressLint("HandlerLeak")
    private class GridViewRunnable implements Runnable {
        private String mPath;
        private Reference<AbsListView> mImageViewRef;

        private final Handler mHandler = new Handler() {

            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);

                if (msg.obj instanceof Bitmap) {
                    Bitmap bitmap = (Bitmap) msg.obj;
                    AbsListView gridView = mImageViewRef.get();

                    if (gridView != null) {
                        ImageView imageView = (ImageView) gridView.findViewWithTag(mPath);
                        if (imageView != null) {
                            imageView.setImageBitmap(bitmap);
                        }
                    }
                }
            }

        };

        public GridViewRunnable(String path, AbsListView gridView) {
            mPath = path;
            mImageViewRef = new WeakReference<>(gridView);
        }

        @Override
        public void run() {
            MediaID3Util.HBmpPackage bmpPackage = MediaID3Util.retrieveMusicAlbumCover(mPath, false);

            HBmpPackage objValue = new HBmpPackage();
            objValue.mBitmap = bmpPackage.bmp;

            if (bmpPackage.bmp != null) {
                objValue.mExtracted = true;
                addHBmpPackageToMemoryCache(mPath, objValue);

                Message msg = mHandler.obtainMessage();
                msg.obj = bmpPackage.bmp;
                mHandler.sendMessage(msg);
            } else {
                switch (bmpPackage.reason) {
                    case MediaID3Util.REASON_NO_GRAPHIC_FOUND:
                    case MediaID3Util.REASON_NOT_BE_DECODED:
                        objValue.mExtracted = true;
                        addHBmpPackageToMemoryCache(mPath, objValue);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    @SuppressLint("HandlerLeak")
    private class ImageCallbackRunnable implements Runnable {
        private final String mPath;
        private final Reference<MusicInfo> mInfoRef;
        private final Reference<NativeImageCallBack> mCallbackRef;

        private final Handler mHandler = new Handler() {

            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);

                if (msg.obj instanceof Bitmap) {
                    Bitmap bitmap = (Bitmap) msg.obj;
                    NativeImageCallBack callback = mCallbackRef.get();
                    if (callback != null) {
                        callback.onImageLoader(bitmap, mPath);
                    }
                }
            }
        };

        public ImageCallbackRunnable(MusicInfo info, NativeImageCallBack callback) {
            mPath = info.mFilePath;
            mInfoRef = new WeakReference<>(info);
            mCallbackRef = new WeakReference<>(callback);
        }

        @Override
        public void run() {
            MediaID3Util.HBmpPackage bmpPackage =
                    MediaID3Util.retrieveMusicAlbumCover(mPath, false);

            HBmpPackage objValue = new HBmpPackage();
            objValue.mBitmap = bmpPackage.bmp;

            if (null == bmpPackage.bmp) {
                MusicInfo info = mInfoRef.get();
                if (info != null) {
                    info.mID3Type = MusicInfo.ID3_TYPE_ERROR;
                }

                // 媒体文件无有效图形资源, 修改提取标记。
                switch (bmpPackage.reason) {
                    case MediaID3Util.REASON_NO_GRAPHIC_FOUND:
                    case MediaID3Util.REASON_NOT_BE_DECODED:
                        objValue.mExtracted = true;
                        addHBmpPackageToMemoryCache(mPath, objValue);
                        break;
                    default:
                        break;
                }

                return;
            }

            objValue.mExtracted = true;
            addHBmpPackageToMemoryCache(mPath, objValue);

            Message msg = mHandler.obtainMessage();
            msg.obj = bmpPackage.bmp;
            mHandler.sendMessage(msg);
        }
    }

    private class VideoImageViewRunnable implements Runnable {
        protected String mPath;
        private final Reference<ImageView> mImageViewRef;

        private final Handler mHandler = new Handler(Looper.getMainLooper()) {

            @Override
            public void handleMessage(Message msg) {
                onHandleMessage(msg);
            }
        };

        public VideoImageViewRunnable(String path, ImageView imageView) {
            mPath = path;
            mImageViewRef = new WeakReference<>(imageView);
        }

        @Override
        public void run() {
            Bitmap bitmap = getBitmapFromDiskCache(mPath);
            HBmpPackage objValue = new HBmpPackage();

            if (null == bitmap) {
                MediaID3Util.HBmpPackage bmpPackage =
                        MediaID3Util.retrieveVideoFrameAtTime(mPath, -1);

                if (bmpPackage.bmp != null) {
                    // 写磁盘缓存
                    bitmap = bmpPackage.bmp;
                    addBitmapToDiskCache(mPath, bmpPackage.bmp);
                } else {
                    // 媒体文件无有效图形资源, 修改提取标记。
                    switch (bmpPackage.reason) {
                        case MediaID3Util.REASON_NO_GRAPHIC_FOUND:
                        case MediaID3Util.REASON_NOT_BE_DECODED:
                            objValue.mBitmap = null;
                            objValue.mExtracted = true;
                            addHBmpPackageToMemoryCache(mPath, objValue);
                            break;
                        default:
                            break;
                    }

                    return;
                }
            }

            // 如果采集到数据
            int width = bitmap.getWidth();
            int height = bitmap.getHeight();
            int size = width * height * 4;

            ByteBuffer byteBuffer = ByteBuffer.allocate(size);
            bitmap.copyPixelsToBuffer(byteBuffer);
            byte[] buffer = byteBuffer.array();

            boolean maybeBlack = true;
            // 采样 900 个像素点(缩略图出来的是 150 * 150)
            for (int i = 0; i < buffer.length; i = i + 100) {
                if (buffer[i] > 0x1F || buffer[i + 1] > 0x1F || buffer[i + 2] > 0x1F) {
                    maybeBlack = false;
                    break;
                }
            }

            // 如果判定为黑色
            if (maybeBlack) {
                bitmap = null;
            }

            objValue.mBitmap = bitmap;
            objValue.mExtracted = true;
            addHBmpPackageToMemoryCache(mPath, objValue);

            Message msg = mHandler.obtainMessage();
            msg.obj = bitmap;
            mHandler.sendMessage(msg);
        }

        protected void onHandleMessage(Message msg) {
            if (!(msg.obj instanceof Bitmap)) {
                return;
            }

            Bitmap bitmap = (Bitmap) msg.obj;
            ImageView imageView = mImageViewRef.get();
            if (imageView != null) {
                imageView.setImageBitmap(bitmap);
            }
        }
    }

    @SuppressLint("HandlerLeak")
    private class VideoGridViewRunnable extends VideoImageViewRunnable {
        private final Reference<AbsListView> mListViewRef;

        public VideoGridViewRunnable(String path, AbsListView gridView) {
            super(path, null);
            mListViewRef = new WeakReference<>(gridView);
        }

        @Override
        protected void onHandleMessage(Message msg) {
            if (!(msg.obj instanceof Bitmap)) {
                return;
            }

            Bitmap bitmap = (Bitmap) msg.obj;
            AbsListView gridView = mListViewRef.get();
            if (gridView != null) {
                ImageView imageView = (ImageView) gridView.findViewWithTag(mPath);
                if (imageView != null) {
                    imageView.setImageBitmap(bitmap);
                }
            }
        }
    }

    // byte 数组转换 16 进制字符串。
    private static String bytes2Hex(byte[] src) {
        char[] res = new char[src.length << 1];
        final char[] hexDigits = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        for (int i = 0, j = 0; i < src.length; i++) {
            res[j++] = hexDigits[src[i] >>> 4 & 0x0F];
            res[j++] = hexDigits[src[i] & 0x0F];
        }
        return new String(res);
    }
}
