package com.hcn.autoradio.skin;

import android.annotation.ColorRes;
import android.annotation.DrawableRes;
import android.annotation.IntegerRes;
import android.annotation.NonNull;
import android.annotation.StringRes;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.os.Environment;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.hcn.autoradio.util.RadioUtils;
import com.hcn.skin.support.SkinCompatManager;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.io.File;
import java.util.Objects;

/**
 * @author simon
 */
public class SkinUtils {
    private static final String TAG = SkinUtils.class.getSimpleName();

    private static Context mContext;
    private static SkinCompatManager mSkinManager;

    /**
     * 扩展插件皮肤包属性定义
     */
    private static final String RADIO_SKIN_PROP = "persist.sys.radio.skins";
    private static final String RADIO_SKIN_PACKAGE_PREFIX = "com.hcn.radio.skins.";
    private static final String RADIO_SKIN_APK_PREFIX = "RadioSkins_";

    /**
     * 当前加载的扩展皮肤包定义
     * <p> 使用 {@link @SkinID} 约束当前皮肤变量的值；
     */
    @SkinID
    private static String mCurrentSkinID = SkinID.SKIN_NONE;


    /**
     * 加载扩展皮肤包
     *
     * <p> 加载前检查采用哪种方式进行加载；
     *
     * @param context 上下文环境
     */
    public static void init(@NonNull Context context) {
        if (mContext != null) {
            Log.w(TAG, "repeated calls to initialization functions!");
            return;
        }
        mContext = context;
        mSkinManager = SkinCompatManager.create(context);

        final String skinName = RadioUtils.getProp(RADIO_SKIN_PROP, "");
        String packageName = RADIO_SKIN_PACKAGE_PREFIX + skinName.toLowerCase();
        Log.d(TAG, "Radio skin init: skinName = " + skinName);
        if (!TextUtils.isEmpty(skinName)) {
            try {
                ApplicationInfo info = context.getPackageManager().getApplicationInfo(packageName, 0);
                if (info != null) {
                    //通过包名的方式加载扩展皮肤包
                    Log.d(TAG, "Load radio skin by packageName = " + packageName);
                    mSkinManager.loadSkinByPackage(packageName);
                    SkinUtils.mCurrentSkinID = skinName.toLowerCase();
                }
            } catch (PackageManager.NameNotFoundException e) {
                Log.d(TAG, "can't not find packageName: " + packageName);
                //通过路径的方式加载扩展皮肤包 /sdcard/skin/
                final String apkName = RADIO_SKIN_APK_PREFIX + skinName.toLowerCase() + ".apk";
                final String apkPath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "skin" + File.separator + apkName;
                if (isFileExists(apkPath)) {
                    Log.d(TAG, "Load radio skin by apk = " + apkPath);
                    mSkinManager.loadSkinSync(apkName);
                    SkinUtils.mCurrentSkinID = skinName.toLowerCase();
                } else {
                    Log.d(TAG, apkPath + ", does not exist!");
                    SkinUtils.mCurrentSkinID = SkinID.SKIN_NONE;
                }
            }
            Log.d(TAG, "mCurrentSkinID = " + SkinUtils.mCurrentSkinID);
        }
    }

    /**
     * 获取当前皮肤类型
     * <p> 用来给 UI 层动态创建对应的 UI 类对象；
     *
     * @return {@link @SkinID}
     */
    @NonNull
    @SkinID
    public static String getCurrentSkinID() {
        return SkinUtils.mCurrentSkinID;
    }

    /**
     * 主题查询
     */
    public static boolean isSkinID(final String skinID) {
        return Objects.equals(SkinUtils.mCurrentSkinID, skinID);
    }

    /**
     * 是否使用了皮肤包
     *
     * @return
     */
    public static boolean useSkinPackage() {
        return SkinCompatResources.getInstance().getSkinResources() != null;
    }

    /**
     * 当前应用程序上下文
     *
     * @return 上下文对象
     */
    public static Context context() {
        if (mContext == null) {
            throw new NullPointerException("The owner context is null!");
        }
        return mContext;
    }

    /**
     * 当前扩展皮肤包资源的上下文
     *
     * @return 上下文对象
     */
    public static Context getContext() {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getSkinContext();
        }
        return context();
    }

    /**
     * 获取皮肤包layout
     * @param layoutId
     * @return
     */
    public static View getLayout(int layoutId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getLayout(layoutId);
        }
        return LayoutInflater.from(context()).inflate(layoutId, null);
    }

    /**
     * 根据layoutId获取layout
     * @param layoutId
     * @param root
     * @return
     */
    public static View getLayout(int layoutId, ViewGroup root) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getLayout(layoutId, root);
        }
        View view = LayoutInflater.from(context()).inflate(layoutId, root);
        return view;
    }

    /**
     * 获取皮肤包layout
     * @param name
     * @return
     */
    public static View getLayout(String name) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getLayout(name);
        }
        int layoutId = mContext.getResources().getIdentifier(name, "layout", mContext.getPackageName());
        return LayoutInflater.from(context()).inflate(layoutId, null);
    }

    /**
     * 根据name获取layout
     * @param name
     * @param root
     * @return
     */
    public static View getLayout(String name, ViewGroup root) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getLayout(name, root);
        }
        int layoutId = mContext.getResources().getIdentifier(name, "layout", mContext.getPackageName());
        return LayoutInflater.from(context()).inflate(layoutId, root);
    }

    /**
     * 获取皮肤包里面的iresId
     *
     * @param resId
     * @return
     */
    public static int getId(@NonNull int resId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getSkinResId(resId);
        }
        return resId;
    }

    /**
     * 通过id名称来查找ResID
     * @param name
     * @return
     */
    public static int getId(String name) {
        return getId(name, "id");
    }


    /**
     * 获取皮肤包里面的iresId：根据id名称和类型
     * @param name
     * @param type
     * @return
     */
    public static int getId(String name, String type) {
        if (SkinUtils.useSkinPackage()) {
            return SkinCompatResources.getInstance().getSkinResId(name, type);
        }

        Context context = context();
        return context.getResources().getIdentifier(name, type, context.getPackageName());
    }

    /**
     * 获取皮肤包里面的String
     *
     * @param resId
     * @return
     */
    public static String getString(@StringRes int resId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getString(resId);
        }
        return context().getString(resId);
    }

    /**
     * 获取皮肤包里面的String
     *
     * @param resId
     * @return
     */
    public static CharSequence getText(@StringRes int resId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getText(resId);
        }
        return context().getText(resId);
    }

    /**
     * 获取颜色对象
     *
     * @param id 本地 id 名称
     */
    public static int getColor(@ColorRes int id) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getColor(id);
        }
        return context().getColor(id);
    }

    /**
     * 获取皮肤包里面resId
     *
     * @param view
     * @return
     */
    public static int getViewId(@NonNull View view) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getViewId(view);
        }
        return view.getId();
    }

    /**
     * 获取皮肤包里面Integer值
     */
    public static int getInteger(@IntegerRes int resId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getInteger(resId);
        }
        return context().getResources().getInteger(resId);
    }

    /**
     * 获取皮肤包里面的Drawable资源
     * @param resId
     * @return
     */
    public static Drawable getDrawable(@DrawableRes int resId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getDrawable(resId);
        }
        Context context = context();
        return context.getResources().getDrawable(resId, context.getTheme());
    }


    /**
     * Return the file by path.
     *
     * @param filePath The path of file.
     * @return the file
     */
    public static File getFileByPath(String filePath) {
        return TextUtils.isEmpty(filePath) ? null : new File(filePath);
    }

    /**
     * 返回文件是否存在
     *
     * @param file The file.
     * @return {@code true}: yes<br>{@code false}: no
     */
    public static boolean isFileExists(final File file) {
        return file != null && file.exists();
    }

    /**
     * 返回文件是否存在
     *
     * @param filePath The path of file.
     * @return {@code true}: yes<br>{@code false}: no
     */
    public static boolean isFileExists(final String filePath) {
        return isFileExists(getFileByPath(filePath));
    }
}
