/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.hcn.bluetooth.api;

import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.Date;

public class Utils {
    private static final String TAG = "Utils";

    private static final String platform_name = getSystemProperty("ro.build.product",
            "");

    private static final String TIMESTAMP_FORMAT = "yyyyMMdd'T'HHmmss";

    //蓝牙
    public static final String BT_PACKAGE_NAME = "com.autochips.bluetooth";
    public static final String BT_ACTIVITY_NAME =
            "com.autochips.bluetooth.MainBluetoothActivity";
    //蓝牙音乐
    public static final String BT_MUSIC_PACKAGE_NAME = "com.autochips.bluetooth";
    public static final String BT_MUSIC_ACTIVITY_NAME =
            "com.autochips.bluetooth.BtMusicActivity";

    public static final int MAX_TOAST_SIZE = 3;
    private static Toast[] mToasts = new Toast[MAX_TOAST_SIZE];
    private static int mToastIndex = 0;
    /**
     * 联系人姓名反向 值为：enable disable
     */
    private static final String KEY_REVERSE_CONTACT_NAME = "persist.sys.bt.reverse_name";
    private static String reverseName;
    //数据库用户类型
    public static final String ACCOUNT_TYPE = "com.android.bluetooth.pbapsink";
    private static int ThemeType = Integer.parseInt(
            getSystemProperty("persist.sys.etheme_god", "0"));

    //通过号码查询联系人姓名
    private static String SELECTION = ContactsContract.CommonDataKinds.Phone.NUMBER
            + "=? AND account_name=? AND account_type=?";

    static {
        updateReverseName();
    }

    /**
     * 是否是竖屏
     *
     * @param context
     * @return
     */
    public static boolean isPortrait(Context context) {
        if (context == null) return false;

        return context.getResources().getConfiguration().orientation
                == Configuration.ORIENTATION_PORTRAIT;
    }

    public static String dateFormat(String datestring) {

        StringBuilder sDate = new StringBuilder();
        sDate.append(datestring.substring(0, 4)).append("-").append(datestring.substring(4, 6))
                .append("-").append(datestring.substring(6, 8));

        return sDate.toString();
    }

    public static String timeFormat(String timestring) {

        StringBuilder sTime = new StringBuilder();
        sTime.append(timestring.substring(0, 2)).append(":").append(timestring.substring(2, 4))
                .append(":").append(timestring.substring(4, 6));

        return sTime.toString();
    }

    public static String getDateToString(long time) {
        Date d = new Date(time);
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat(TIMESTAMP_FORMAT);
        String timeString = simpleDateFormat.format(d);
        if (timeString != null) {
            String[] textarry = timeString.split("T");
            if (textarry != null && textarry.length > 0) {
                String dateformat = Utils.dateFormat(textarry[0]);
                String timeformat = Utils.timeFormat(textarry[1]);
                return (dateformat + " " + timeformat);
            }
        }
        return "";
    }

    public static String getContactNameByNumber(Context context, String number, String address) {
        long time = System.currentTimeMillis();
        String displayName = "";
        if (null == context || TextUtils.isEmpty(number) || TextUtils.isEmpty(address)) {
            return displayName;
        }
        Cursor cursor = null;
        try {
            ContentResolver resolver = context.getContentResolver();
            Uri uri = Phone.CONTENT_URI;
            String nameColumn = Phone.DISPLAY_NAME;
            if (isReverseName()) {
                nameColumn = Phone.DISPLAY_NAME;
            }
            String[] projection = new String[]{nameColumn,};
            cursor = resolver.query(uri, projection, SELECTION,
                    new String[]{number, address, ACCOUNT_TYPE}, null);
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    int nameColIdx = cursor.getColumnIndex(nameColumn);
                    displayName = cursor.getString(nameColIdx);
                    if (!TextUtils.isEmpty(displayName)) {
                        break;
                    }
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "getContactNameByNumber Exception!!!");
        } finally {
            Log.d(TAG,
                    "getContactNameByNumber: time=" + (System.currentTimeMillis() - time) + "ms");
            if (cursor != null) {
                cursor.close();
            }
        }
        return displayName;
    }

    public static void startApp(Context context, String packageName, String classname) {
        if (null == context) {
            return;
        }
        try {
            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.setComponent(new ComponentName(packageName, classname));
            intent.addCategory(Intent.CATEGORY_LAUNCHER);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void startBtClient(Context context, String extra) {
        if (null == context) {
            return;
        }
        try {
            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.setComponent(new ComponentName(BT_PACKAGE_NAME, BT_ACTIVITY_NAME));
            intent.putExtra("reason",extra);
            intent.addCategory(Intent.CATEGORY_LAUNCHER);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void showToast(Context context, int resid) {
        if (null == context) {
            return;
        }
        if (mToasts[mToastIndex] != null) {
            mToasts[mToastIndex].cancel();
        }
        mToasts[mToastIndex] = Toast.makeText(context, context.getString(resid),
                Toast.LENGTH_SHORT);
        mToasts[mToastIndex].show();
        mToastIndex = (mToastIndex + 1) % MAX_TOAST_SIZE;
    }

    public static void showToast(Context context, String text) {
        if (null == context) {
            return;
        }
        if (mToasts[mToastIndex] != null) {
            mToasts[mToastIndex].cancel();
        }
        mToasts[mToastIndex] = Toast.makeText(context, text, Toast.LENGTH_SHORT);
        mToasts[mToastIndex].show();
        mToastIndex = (mToastIndex + 1) % MAX_TOAST_SIZE;
    }

    public static boolean isNaviTopRunning(Context context) {
        if (null == context) {
            return false;
        }
        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        try {
            ComponentName cn = am.getRunningTasks(1).get(0).topActivity;
            if (cn.getPackageName().toLowerCase().contains("navi")
                    || cn.getPackageName().toLowerCase().contains("map")
                    || cn.getPackageName().toLowerCase().contains("igo")
                    || cn.getPackageName().toLowerCase().contains("sygic")
                    || cn.getPackageName().toLowerCase().contains("papago")
                    || cn.getPackageName().toLowerCase().contains("com.waze")) {
                return true;
            }
        } catch (SecurityException e) {

        }
        return false;
    }

    /**
     * 用于读取数据库时使用，避免每次调用getSystemProperty读取属性值
     *
     * @return
     */
    public static boolean isReverseName() {
        return "enable".equals(reverseName);
    }

    /**
     * 车载设置中可能更改属性值，下载电话本通话记录前更新reverseName的值
     */
    public static void updateReverseName() {
        reverseName = getSystemProperty(KEY_REVERSE_CONTACT_NAME, "disable");
    }

    public static String getSystemProperty(String property, String defaultValue) {
        String ret = null;
        try {
            Class<?> SystemProperties = Class.forName("android.os.SystemProperties");
            Method get = SystemProperties.getMethod("get", String.class, String.class);
            Object[] params = new Object[]{new String(property), defaultValue};
            ret = (String) (get.invoke(SystemProperties, params));
        } catch (Exception ignore) {
        }
        return ret;
    }

    public static void setSystemProperty(String property, String value) {
        try {
            Class<?> SystemProperties = Class.forName("android.os.SystemProperties");
            Method set = SystemProperties.getMethod("set", String.class, String.class);
            Object[] params = new Object[]{new String(property), value};
            set.invoke(SystemProperties, params);
        } catch (Exception ignore) {
        }
    }

    public static int getThemeType() {
        return ThemeType;
    }

    public static boolean isT5Platform() {
        return platform_name.equals("mercury-demo");
    }

    public static String getPlatform() {
        return platform_name;
    }

    /**
     * 8581平台创建自定义Hci日志存放路径
     */
    public static void CreateHciLogPath() {
        if (isPlatform8581()) {
            @SuppressLint("SdCardPath")
            String strPath = getSystemProperty("persist.vendor.bluetooth.btsnooppath", "/sdcard/goc/hci");
            boolean bRet = createOrExistsDir(strPath);
            Log.d(TAG, "Create HCI Log path=" + strPath + " bRet=" + bRet);
        }
    }

    public static boolean isPlatform8581() {
        return platform_name.startsWith("uis8581");
    }

    public static boolean createOrExistsDir(final String dirPath) {
        File file = isSpace(dirPath) ? null : new File(dirPath);
        return file != null && (file.exists() ? file.isDirectory() : file.mkdirs());
    }

    private static boolean isSpace(final String str) {
        if (str == null) {
            return true;
        }
        for (int i = 0, len = str.length(); i < len; ++i) {
            if (!Character.isWhitespace(str.charAt(i))) {
                return false;
            }
        }
        return true;
    }
}
