package com.hcn.media_model.eq;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

public class SharePreferencesTools {
    private static SharePreferencesTools mInstance;

    private SharedPreferences mSharedPreferences;
    private Editor mEditor;
    private String mFirst = "first";
    private String mTwo = "two";
    private String mThree = "three";
    private String mFour = "four";
    private String mFive = "five";
    private String mSix = "six";
    private String mseven = "seven";
    private String mBass = "bass";
    private String mVirt = "virt";

    private SharePreferencesTools() {
    }

    public static SharePreferencesTools getSharePreferencesTools() {
        if (mInstance == null) {
            mInstance = new SharePreferencesTools();
        }
        return mInstance;
    }

    public void init(Context context) {
        if (mSharedPreferences == null || mEditor == null) {
            mSharedPreferences = context.getSharedPreferences("eqs", Context.MODE_PRIVATE);
            mEditor = mSharedPreferences.edit();
        }
    }

    public void setFirstEQPreference(int first) {
        mEditor.putInt(mFirst, first);
        mEditor.commit();
    }

    public void setTwoEQPreference(int two) {
        mEditor.putInt(mTwo, two);
        mEditor.commit();
    }

    public void setThreeEQPreference(int three) {
        mEditor.putInt(mThree, three);
        mEditor.commit();
    }

    public void setFourEQPreference(int four) {
        mEditor.putInt(mFour, four);
        mEditor.commit();
    }

    public void setFiveEQPreference(int five) {
        mEditor.putInt(mFive, five);
        mEditor.commit();
    }

    public void setSixEQPreference(int six) {
        mEditor.putInt(mSix, six);
        mEditor.commit();
    }

    public void setSevenEQPreference(int seven) {
        mEditor.putInt(mseven, seven);
        mEditor.commit();
    }

    public int getModePreference() {
        return mSharedPreferences.getInt("mode", 1);
    }

    public void setModePreference(int values) {
        mEditor.putInt("mode", values);
        mEditor.commit();
    }

    public int getEQTwoPreference() {
        return mSharedPreferences.getInt(mTwo, 10);
    }

    public int getEQThreePreference() {
        return mSharedPreferences.getInt(mThree, 10);
    }

    public int getEQFourPreference() {
        return mSharedPreferences.getInt(mFour, 10);
    }

    public int getEQFivePreference() {
        return mSharedPreferences.getInt(mFive, 10);
    }

    public int getEQSixPreference() {
        return mSharedPreferences.getInt(mSix, 0);
    }

    public int getEQSevenPreference() {
        return mSharedPreferences.getInt(mseven, 0);
    }

    public int getEQFirstPreference() {
        return mSharedPreferences.getInt(mFirst, 10);
    }

    public int getBassPreference() {
        return mSharedPreferences.getInt(mBass, 0);
    }

    public void setBassPreference(int bass) {
        mEditor.putInt(mBass, bass);
        mEditor.commit();
    }

    public int getVirtPreference() {
        return mSharedPreferences.getInt(mVirt, 0);
    }

    public void setVirtPreference(int bass) {
        mEditor.putInt(mVirt, bass);
        mEditor.commit();
    }

    public void uninit() {
        mSharedPreferences = null;
        mEditor = null;
    }
}
