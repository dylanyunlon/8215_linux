package com.hcn_library.util;

import android.content.ContentResolver;
import android.content.Context;
import android.provider.Settings;

public class SetupSharedData {

    private static SetupSharedData mInstance = null;
    private Context mContext = null;
    private ContentResolver mContentResolver = null;

    public static SetupSharedData getInstance(Context context) {
        if (mInstance == null) {
            mInstance = new SetupSharedData(context);
        }
        return mInstance;
    }

    private SetupSharedData(Context context) {
        mContext = context.getApplicationContext();
        mContentResolver = mContext.getContentResolver();
    }

    public void setIntValue(String key, int value) {
        Settings.System.putInt(mContentResolver, key, value);
    }

    public int getIntValue(String key, int defValue) {
        return Settings.System.getInt(mContentResolver, key, defValue);
    }

    public void setStringValue(String key, String value) {
        Settings.System.putString(mContentResolver, key, value);
    }

    public String getStringValue(String key) {
        return Settings.System.getString(mContentResolver, key);
    }
}
