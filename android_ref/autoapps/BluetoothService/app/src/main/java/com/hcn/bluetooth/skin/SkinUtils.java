package com.hcn.bluetooth.skin;

import android.annotation.NonNull;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.os.Environment;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.hcn.skin.support.SkinCompatManager;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
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
    private static final String BLUETOOTH_SKIN_PROP = "persist.sys.bluetooth.skins";
    private static final String BLUETOOTH_SKIN_PACKAGE_PREFIX = "com.hcn.bluetooth.skins.";
    private static final String BLUETOOTH_SKIN_APK_PREFIX = "BTSkins_";

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
    public static void init(Context context) {
        if (mContext != null) {
            Log.w(TAG, "repeated calls to initialization functions!");
            return;
        }
        mContext = context;
        mSkinManager = SkinCompatManager.create(context);

        final String skinName = getSystemProp(BLUETOOTH_SKIN_PROP, "");
        String packageName = BLUETOOTH_SKIN_PACKAGE_PREFIX + skinName;
        Log.d(TAG, "Bluetooth skin init: skinName = " + skinName);
        if (!TextUtils.isEmpty(skinName)) {
            try {
                ApplicationInfo info = context.getPackageManager().getApplicationInfo(packageName, 0);
                if (info != null) {
                    //通过包名的方式加载扩展皮肤包
                    Log.d(TAG, "Load bluetooth skin by packageName = " + packageName);
                    mSkinManager.loadSkinByPackage(packageName);
                    SkinUtils.mCurrentSkinID = skinName.toLowerCase();
                }
            } catch (PackageManager.NameNotFoundException e) {
                Log.d(TAG, "can't not find packageName: " + packageName);
                //通过路径的方式加载扩展皮肤包 /sdcard/skin/
                final String apkName = BLUETOOTH_SKIN_APK_PREFIX + skinName.toLowerCase() + ".apk";
                final String apkPath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "skin" + File.separator + apkName;
                if (isFileExists(apkPath)) {
                    Log.d(TAG, "Load bluetooth skin by apk = " + apkPath);
                    mSkinManager.loadSkinSync(apkName);
                    SkinUtils.mCurrentSkinID = skinName.toLowerCase();
                } else {
                    Log.d(TAG, apkPath + ", does not exist!");
                    SkinUtils.mCurrentSkinID = SkinID.SKIN_NONE;
                }
            }
        }
    }

    /**
     * 获取当前皮肤类型
     * <p> 用来给 UI 层动态创建对应的 UI 类对象；
     *
     * @return {@link @SkinID}
     */
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
     * 获取皮肤包里面的resId
     *
     * @param resId
     * @return
     */
    public static int getId(int resId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getId(resId);
        }
        return resId;
    }

    /**
     * 获取皮肤包里面的String
     *
     * @param resId
     * @return
     */
    public static String getString(int resId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getString(resId);
        }
        return context().getString(resId);
    }

    /**
     * 获取颜色对象
     *
     * @param id 本地 id 名称
     */
    public static int getColor(int id) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getColor(id);
        }
        return context().getColor(id);
    }

    public static int getInteger(int id) {
        if (isResourcesExist(id) > 0) {
            return SkinCompatResources.getInstance().getInteger(id);
        }
        return context().getResources().getInteger(id);
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
     * 获取皮肤包里面resId
     *
     * @return
     */
    public static int getResId(int resId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getSkinResId(resId);
        }
        return resId;
    }

    public static Drawable getDrawable(int resId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getDrawable(resId);
        }
        return context().getResources().getDrawable(resId,null);
    }

    /**
     * 根据layoutId获取layout
     * @param layoutId
     * @return
     */
    public static View getLayout(int layoutId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getLayout(layoutId);
        }
        View view = LayoutInflater.from(context()).inflate(layoutId, null);
        return view;
    }

    /**
     * 根据layoutId获取layout
     * @param layoutId
     * @param root
     * @return
     */
    public static View getLayout(int layoutId,ViewGroup root) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getLayout(layoutId, root);
        }
        View view = LayoutInflater.from(context()).inflate(layoutId, root);
        return view;
    }

    /**
     * 根据name获取layout
     * @param name
     * @return
     */
    public static View getLayout(String name) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getLayout(name);
        }
        int layoutId = mContext.getResources().getIdentifier(name, "layout", mContext.getPackageName());
        View view = LayoutInflater.from(context()).inflate(layoutId, null);
        return view;
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
     * Whether Resources is Exist in skin
     *
     * @return non 0 means exist,0 means not exist
     */
    public static int isResourcesExist(int resId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getSkinResId(resId);
        }
        return 0;
    }

    /**
     * Whether Resources is Exist in skin
     *
     * @return non 0 means exist,0 means not exist
     */
    public static int isResourcesExist(String resName, String resType) {
        if (useSkinPackage()) {
            try {
                if (TextUtils.isEmpty(resName) || TextUtils.isEmpty(resType)) {
                    return 0;
                }
                Resources skinRes = SkinCompatResources.getInstance().getSkinResources();
                String skinPkgName = SkinCompatResources.getInstance().getSkinPkgName();
                return skinRes.getIdentifier(resName, resType, skinPkgName);
            } catch (Exception e) {
                return 0;
            }
        }
        return 0;
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

    /**
     * 获取系统属性值
     *
     * @param key
     * @param defaultValue
     * @return
     */
    public static int getIntProp(String key, int defaultValue) {
        int result = defaultValue;
        try {
            Class<?> systemProperties = Class.forName("android.os.SystemProperties");
            Method get = systemProperties.getMethod("getInt", String.class, int.class);
            Object[] params = new Object[]{key, defaultValue};
            result = (int) (get.invoke(systemProperties, params));
        } catch (Exception e) {

        }
        return result;
    }

    /**
     * 获取系统属性值
     *
     * @param key
     * @param defaultValue
     * @return
     */
    public static String getSystemProp(String key, String defaultValue) {
        String result = "";
        try {
            Class<?> SystemProperties = Class.forName("android.os.SystemProperties");
            Method get = SystemProperties.getMethod("get", String.class, String.class);
            Object[] params = new Object[]{key, defaultValue};
            result = (String) (get.invoke(SystemProperties, params));
        } catch (ClassNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return result;
    }

    /**
     * 设置系统属性值
     *
     * @param property
     * @param value
     */
    public static void setSystemProp(String property, String value) {
        try {
            Class<?> systemProperties = Class.forName("android.os.SystemProperties");
            Method set = systemProperties.getMethod("set", String.class, String.class);
            Object[] params = new Object[]{new String(property), value};
            set.invoke(systemProperties, params);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
