package com.hcn.autoradio;

import static com.hcn.autoradio.util.RadioUtils.RADIO_SI4754;
import static com.hcn.autoradio.util.RadioUtils.RADIO_TEA6851;
import static com.hcn.autoradio.util.RadioUtils.RDS_INSIDE;

import android.app.Activity;
import android.app.Application;
import android.content.res.Configuration;
import android.provider.Settings;
import android.util.Log;

import com.hcn.autoradio.audio.RadioAudioManager;
import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.skin.SkinUtils;
import com.hcn.autoradio.skin.ThemeUtilsEx;
import com.hcn.autoradio.util.RadioUtils;

import java.util.LinkedList;
import java.util.List;


public class FMApplication extends Application {
    private static final String TAG = "FMApplication";
    private FMDataControl mFMDataControl = null;
    private List<Activity> mActivityList = new LinkedList<Activity>();

    // add activity to list
    public void addActivity(Activity object) {

        if (!mActivityList.contains(object)) {
            mActivityList.add(object);
        }
    }

    // remove activity from list
    public void removeActivity(Activity object) {
        if (mActivityList.contains(object)) {
            mActivityList.remove(object);
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate version:" + RadioUtils.getVersionName(this));
        SkinUtils.init(this);
        Configuration config = getResources().getConfiguration();
        ScreenSpec.mFullScreenWidth = config.screenWidthDp;
        ScreenSpec.mFullScreenHeight = config.screenHeightDp;
        ScreenSpec.mScreenDensity = config.densityDpi / 160f;
        ScreenSpec.mOrientation = config.orientation;

        mFMDataControl = FMDataControl.getInstance();
        if (null != mFMDataControl) {
            mFMDataControl.initFMDataControl(getApplicationContext());
        }

        //获取收音芯片类型
        String strRadioMode = RadioUtils.initRadioModel();
        if (RADIO_TEA6851.equals(strRadioMode)
                || RADIO_SI4754.equals(strRadioMode)) {
            mFMDataControl.mIsSupportRDS = false;
        } else if (mFMDataControl.isOverseasVersion()) {
            mFMDataControl.mIsSupportRDS = (1 == Settings.System.getInt(getContentResolver(),
                    RDS_INSIDE, 1));
        } else {
            mFMDataControl.mIsSupportRDS = (1 == Settings.System.getInt(getContentResolver(),
                    RDS_INSIDE, 0));
        }
        ThemeUtilsEx.init(this);
        RadioAudioManager.init(this);
        RadioAudioManager.getInstance().createRenderThread();
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
    }
}


