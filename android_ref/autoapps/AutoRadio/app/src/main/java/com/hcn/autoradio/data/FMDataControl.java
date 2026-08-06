package com.hcn.autoradio.data;

import static android.radio.RadioPlayer.BAND;
import static android.radio.RadioPlayer.EVENT_LIST_CHANGED;
import static android.radio.RadioPlayer.EVENT_PS_MESSAGE;
import static android.radio.RadioPlayer.EVENT_PTY_TYPE;
import static android.radio.RadioPlayer.EVENT_RDS_STATE;
import static android.radio.RadioPlayer.EVENT_RT_MESSAGE;
import static android.radio.RadioPlayer.EVENT_SCAN_DONE;

import android.Configures.HConfig;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.radio.RadioInfo;
import android.radio.RadioPlayer;
import android.radio.Utility;
import android.text.TextUtils;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;

import com.hcn.autoradio.IRadioServiceAPI;
import com.hcn.autoradio.R;
import com.hcn.autoradio.RadioMain;
import com.hcn.autoradio.audio.RadioAudioManager;
import com.hcn.autoradio.collection.FMCollectListDao;
import com.hcn.autoradio.collection.FMCollectionFunction;
import com.hcn.autoradio.service.FMPlugService;
import com.hcn.autoradio.skin.SkinID;
import com.hcn.autoradio.skin.SkinUtils;
import com.hcn.autoradio.skin.ThemeID;
import com.hcn.autoradio.util.LogoUtils;
import com.hcn.autoradio.util.RadioUtils;
import com.hcn.org.litepal.LitePalApplication;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Map.Entry;

public class FMDataControl extends RadioData {
    private static final String TAG = "FMDataControl";
    private Context mContext = null;
    private RadioPlayer mRadioPlayer = null;
    private Intent mServiceIntent = null;

    private RadioInfo mRadioInfo = null;
    private int[][] defaultFMPreset = {
            {87500, 90100, 98100, 106100, 107900, 87500},
            {87500, 90100, 98100, 106100, 108000, 87500},
            {87500, 90100, 98100, 106100, 108000, 87500},
            {65000, 67100, 69500, 71600, 74000, 65000},
            {87500, 90100, 98100, 106100, 108000, 87500},
            {76000, 80000, 83000, 86000, 90000, 76000},
            {87500, 90100, 98100, 106100, 108000, 87500},
            {76000, 83000, 98100, 106100, 108000, 76000}};

    private int[][] defaultAMPreset = {
            {530, 610, 1000, 1400, 1710, 530},
            {522, 603, 999, 1404, 1620, 522},
            {520, 600, 1000, 1400, 1710, 520},
            {531, 603, 999, 1404, 1620, 531},
            {531, 603, 999, 1404, 1629, 531},
            {522, 603, 999, 1404, 1629, 522},
            {531, 603, 999, 1404, 1602, 531},
            {520, 600, 1000, 1400, 1710, 520}};

    private Map<String, UpdateDataListener> mCallbackMap =
            new HashMap<String, UpdateDataListener>();

    private IRadioServiceAPI mFMPlugService = null;
    private FMServiceConnection mServiceConnection = null;

    private FMDataControlHandler mFMDataCtrlHandler = null;
    private SharedPreferences mRadioInfoPreferences = null;

    private SharedPreferences mSPRadioFreqName = null;
    private final String mXMLFreqName = "radio_rds";

    private static FMDataControl sRadioDataControl = null;

    private WindowManager mWindowManager = null;
    private WindowManager.LayoutParams mTAWindowParam = null;
    private View mTAView = null;
    private boolean isAddTAView = false;

    private static String mCustomName = null;
    private static String mProductName = null;
    private static String mPlatform = null;
    public final String mXMLFileName = "radio_info";
    private final String KEY_REGION = "currentRegion";
    private final String KEY_FREQ = "currentFreq";
    private final String KEY_INDEX = "currentIndex";
    private final String KEY_BAND = "currentBand";
    private int type;
    /**
     * 频点收藏功能
     */
    private FMCollectionFunction mFMCollectionFunc = null;
    public static boolean CONFIG_FREQ_COLLECT = false;
    /**
     * 功能1描述： RDS的PS信息有效时，显示位置与中间频率位置互换，即原频率位置显示PS信息， 原PS位置显示band及freq信息，例如FM1-87.5MHz
     */
    public static boolean CONFIG_PS_FREQ_EXCHANGE = false;
    /**
     * 功能2描述： RDS的PS信息有效时,保存频率到PRESET时会保存PS信息并且显示在PRESET上，代替频率， 双击PRESET可编辑PS或清除 此功能与功能3不共存
     */
    public static boolean CONFIG_PRESET_EDIT_NAME = false;
    /**
     * 功能3描述： PS位置除显示RDS频率名称外，还可显示自定义频率名称，自定义优先显示 此功能与功能1和2不共存 相关控件ID如下 R.id.tv_edit_rds_ps
     * R.id.butt_edit_name
     */
    public static boolean CONFIG_TEXTVIEW_PS_EDIT_NAME = false;
    /**
     * 功能4描述： RDS的PS信息有效时，更新PS信息到PRESET，同时显示频点在序号后面。 此功能需与功能2配合使用
     */
    public static boolean CONFIG_PRESET_FREQ_REPLACE_PS = false;

    private FMDataControl() {
        mFMDataCtrlHandler = new FMDataControlHandler(this);
        if (("REIS".equalsIgnoreCase(getCustomer())
                || "HARDSTONE".equalsIgnoreCase(getCustomer())
                || "Morocco".equalsIgnoreCase(getCustomer())
                || ("AC8257".equals(getPlatform()) && getProductName().endsWith("XT"))
                || ("AC8227L".equals(getPlatform()) && getProductName().endsWith("XT")))
                && E_THEME_GOD != ThemeID.E_THEME_ID_206
                && E_THEME_GOD != ThemeID.E_THEME_ID_405) {
            CONFIG_PS_FREQ_EXCHANGE = true;
            CONFIG_PRESET_EDIT_NAME = true;
            //摩洛哥客户特殊要求20230717
            if ("Morocco".equalsIgnoreCase(getCustomer())) {
                CONFIG_PRESET_FREQ_REPLACE_PS = true;
            }
        }
        if (SkinUtils.useSkinPackage()) {
            initFMDataControlForSkin();
        } else {
            initFMDataControlForMcc();
        }
    }
    public void initFMDataControlForSkin() {

        switch (SkinUtils.getCurrentSkinID()) {
            case SkinID.SKIN_DZ17:
                CONFIG_PRESET_FREQ_REPLACE_PS = true;
                CONFIG_PRESET_EDIT_NAME = true;
                LogoUtils.initialize();
                break;
            case SkinID.SKIN_XT144:
                CONFIG_PS_FREQ_EXCHANGE = false;
                CONFIG_PRESET_EDIT_NAME = true;
                CONFIG_PRESET_FREQ_REPLACE_PS = true;
                CONFIG_TEXTVIEW_PS_EDIT_NAME = true;
                break;
            case SkinID.SKIN_N91:
                CONFIG_FREQ_COLLECT = true;
                break;
            default:
                break;
        }
    }

    public void initFMDataControlForMcc() {
        switch (RadioData.E_THEME_GOD) {
            case ThemeID.E_THEME_ID_405:
                CONFIG_TEXTVIEW_PS_EDIT_NAME = true;
                //mcc405-mnc001 收藏列表
                if (E_THEME_SUB == 1) {
                    CONFIG_FREQ_COLLECT = true;
                }
                break;
            default:
                break;
        }
    }

    public void initFMDataControl(Context mContext) {
        this.mContext = mContext;
        if (null == mRadioPlayer) {
            mRadioPlayer = RadioPlayer.getRadioPlayer();
        }
        setRegion(mRadioPlayer.getRegion());
    }

    @Override
    public void setRegion(int mRegion) {
        super.setRegion(mRegion);
        if (!new File(new StringBuilder(mContext.getDataDir().getAbsolutePath()).append(
                "/shared_prefs/").append(mXMLFileName).append(".xml").toString()).exists()) {
            writeRadioInfo(KEY_REGION, mRegion);
            mRadioPlayer.setPreset(BAND.FM1, defaultFMPreset[mRegion]);
            mRadioPlayer.setPreset(BAND.FM2, defaultFMPreset[mRegion]);
            mRadioPlayer.setPreset(BAND.FM3, defaultFMPreset[mRegion]);
            mRadioPlayer.setPreset(BAND.AM1, defaultAMPreset[mRegion]);
        } else {
            int savedRegion = readRadioInfo(KEY_REGION, -1);
            if (savedRegion == -1) {
                writeRadioInfo(KEY_REGION, mRegion);
            } else {
                if (savedRegion != mRegion) {
                    if (mRadioInfoPreferences == null) {
                        mRadioInfoPreferences = mContext.getSharedPreferences(mXMLFileName,
                                Context.MODE_PRIVATE);
                    }
                    synchronized (mRadioInfoPreferences) {
                        SharedPreferences.Editor editor = mRadioInfoPreferences.edit();
                        editor.putInt(KEY_REGION, mRegion);
                        editor.remove(KEY_FREQ);
                        editor.remove(KEY_BAND);
                        editor.remove(KEY_INDEX);
                        editor.commit();
                    }
                    mRadioPlayer.setPreset(BAND.FM1, defaultFMPreset[mRegion]);
                    mRadioPlayer.setPreset(BAND.FM2, defaultFMPreset[mRegion]);
                    mRadioPlayer.setPreset(BAND.FM3, defaultFMPreset[mRegion]);
                    mRadioPlayer.setPreset(BAND.AM1, defaultAMPreset[mRegion]);
                }
            }
        }
    }

    public static String getCustomer() {//add by bli
        if (mCustomName == null) {
            mCustomName = RadioUtils.getProp("persist.sys.customer", "");
        }
        return mCustomName;
    }

    public static String getProductName() {
        //海外板卡要求RDS编辑
        if (mProductName == null) {
            mProductName = RadioUtils.getProp("ro.fota.device", "");
        }
        return mProductName;
    }

    public static String getPlatform() {
        if (mPlatform == null) {
            mPlatform = RadioUtils.getProp("ro.mediatek.platform", "");
        }
        return mPlatform;
    }

    public boolean isOverseasVersion() {//add by bli
        return HConfig.isOverseaVersion();
    }

    public boolean isExitOnBackKey() {
        return "1".equals(RadioUtils.getProp("persist.sys.media_exit", "1"));
    }

    public static int getUIType() {
        return E_THEME_GOD;
    }

    public static FMDataControl getInstance() {
        if (sRadioDataControl == null) {
            sRadioDataControl = new FMDataControl();
        }

        return sRadioDataControl;
    }

    public void initRadioPlayerEventListener() {
        if (null != mRadioPlayer) {
            mRadioPlayer.setOnEventListener(new RadioPlayer.OnEventListener() {
                @Override
                public void onEvent(int i, RadioInfo radioInfo) {
                    if (mRadioInfo != null && mRadioInfo.toString().equals(
                            radioInfo.toString())) {
                        return;
                    }
                    if ((radioInfo.mUiBand.startsWith("FM") && IsAMrange(radioInfo.mFreq))
                            || (radioInfo.mUiBand.startsWith("AM") && IsFMrange(
                            radioInfo.mFreq))) {
                        Log.d(TAG, "onEvent: error RadioInfo!! mUiBand=" + radioInfo.mUiBand + " mFreq=" + radioInfo.mFreq);
                        return;
                    }
                    /**
                     *此处忽略RDS相关信息，使用下面onEvent回调处理
                     */
                    handleRadioInfo(radioInfo, true);

                    // notify radiomain update
                    mFMDataCtrlHandler.sendMessage(mFMDataCtrlHandler.obtainMessage(
                            FMDataControlHandler.WM_WHAT_DATA_CHANGE,
                            UpdateDataListener.UPDATE_DATA_INFO, 0x00));
                }

                @Override
                public void onEvent(int i, String s) {
                    if (i == EVENT_LIST_CHANGED) {
                        if (getRegion() != mRadioPlayer.getRegion()) {
                            setRegion(mRadioPlayer.getRegion());
                            mRadioParameters = new Utility.RadioParameters();
                            dataChangeNotify(null, UpdateDataListener.UPDATE_DATA_RANGE);
                            if (currentBand() > BAND_FM_3) {
                                setFreq(defaultAMPreset[getRegion()][0], 0x00);
                            } else {
                                setFreq(defaultFMPreset[getRegion()][0], 0x00);
                            }
                        }

                        if (s != null) {
                            if (s.startsWith("FM-1")) {
                                fillPreset(BAND_FM_1,
                                        mRadioPlayer.getPreset(BandToFrameworkBand(BAND_FM_1)));
                            } else if (s.startsWith("FM-2")) {
                                fillPreset(BAND_FM_2,
                                        mRadioPlayer.getPreset(BandToFrameworkBand(BAND_FM_2)));
                            } else if (s.startsWith("FM-3")) {
                                fillPreset(BAND_FM_3,
                                        mRadioPlayer.getPreset(BandToFrameworkBand(BAND_FM_3)));
                            } else if (s.startsWith("AM-1")) {
                                fillPreset(BAND_AM_1,
                                        mRadioPlayer.getPreset(BandToFrameworkBand(BAND_AM_1)));
                            }
                            mFMDataCtrlHandler.sendMessage(mFMDataCtrlHandler.obtainMessage(
                                    FMDataControlHandler.WM_WHAT_DATA_CHANGE,
                                    UpdateDataListener.UPDATE_DATA_FREQLIST, 0x00));
                        }
                    } else if (i == EVENT_SCAN_DONE) {
                        if (String.valueOf(RadioData.SEEK_ALL).equals(s)) {//AS finish
                            int[] presets = readPresetList(currentBand());
                            setFreq(presets[0], 0x00);
                        }
                    } else if (i == EVENT_PS_MESSAGE) {
                        if ((mRdsInfo & 0x04) != 0x00) {
                            if (null == s) {
                                mRdsPS = "";
                            } else {
                                mRdsPS = s;
                            }
                        }
                        mFMDataCtrlHandler.sendMessage(mFMDataCtrlHandler.obtainMessage(
                                FMDataControlHandler.WM_WHAT_DATA_CHANGE,
                                UpdateDataListener.UPDATE_DATA_RDS_INFO, 0x00));
                    } else if (i == EVENT_RT_MESSAGE) {
                        if ((mRdsInfo & 0x04) != 0x00) {
                            if (null == s) {
                                mRdsRT = "";
                            } else {
                                mRdsRT = s;
                            }
                        }
                        mFMDataCtrlHandler.sendMessage(mFMDataCtrlHandler.obtainMessage(
                                FMDataControlHandler.WM_WHAT_DATA_CHANGE,
                                UpdateDataListener.UPDATE_DATA_RDS_INFO, 0x00));
                    } else if (i == EVENT_RDS_STATE) {
                        mRdsInfo = Integer.parseInt(s);
                        if ((mRdsInfo & 0x04) == 0x00) {
                            mRdsRT = "";
                            mRdsPS = "";
                            mPtyType = 0x00;
                        }
                        mFMDataCtrlHandler.sendMessage(mFMDataCtrlHandler.obtainMessage(
                                FMDataControlHandler.WM_WHAT_DATA_CHANGE,
                                UpdateDataListener.UPDATE_DATA_RDS_INFO, 0x00));
                    } else if (i == EVENT_PTY_TYPE) {
                        if ((mRdsInfo & 0x04) != 0x00) {
                            mPtyType = Integer.parseInt(s);
                        }
                        mFMDataCtrlHandler.sendMessage(mFMDataCtrlHandler.obtainMessage(
                                FMDataControlHandler.WM_WHAT_DATA_CHANGE,
                                UpdateDataListener.UPDATE_DATA_RDS_INFO, 0x00));
                    }
                }
            });
        }
    }

    public void unInitRadioPlayEventListener() {
        if (null != mRadioPlayer) {
            mRadioPlayer.setOnEventListener(null);
        }
    }

    // open
    public boolean openDataService() {
        if (sRadioDataControl == null) {
            return false;
        }

        // remote service
        try {
            mServiceIntent = new Intent(mContext, FMPlugService.class);
            if (null == mServiceConnection) {
                mContext.startService(mServiceIntent);
                mServiceConnection = new FMServiceConnection();
                mContext.bindService(mServiceIntent, mServiceConnection, Context.BIND_AUTO_CREATE);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        initRadioPlayerEventListener();
        return true;
    }

    // close
    public void closeDataService() {
        if (null != mServiceConnection) {
            try {
                mContext.unbindService(mServiceConnection);
            } catch (IllegalArgumentException ignored) {
            }
        }
        mContext.stopService(mServiceIntent);
    }

    public void Band(int band) {
        if (band == BAND_AM_1 && (RadioUtils.getRadioModel().equalsIgnoreCase(
                RadioUtils.RADIO_INSIDE) || RadioUtils.getRadioModel().equals(RadioUtils.RADIO_QN8035))) {
            band = BAND_FM_1;//inside Radio not support AM
        }
        setFreq(band, lastFreq(band), lastIndx(band));
    }

    public void AS() {
        if (null == mRadioPlayer) {
            return;
        }

        mRadioPlayer.autoScan(mRadioPlayer.getUiband(), BAND_STATION_TOTAL);
    }

    public void PS() {
        if (null == mRadioPlayer) {
            return;
        }

        mRadioPlayer.presetScan(3);
    }

    public void Local() {
        if (mRadioPlayer == null) {
            return;
        }

        mRadioPlayer.local(!isLocal());
    }

    public void setLocal(boolean enable) {
        mIsLocal = enable;
        if (mRadioPlayer != null) {
            mRadioPlayer.local(enable);
        }
    }

    public void seekDown() {
        if (null == mRadioPlayer) {
            return;
        }

        mRadioPlayer.seekDown(mRadioPlayer.getUiband());
    }

    public void seekUp() {
        if (null == mRadioPlayer) {
            return;
        }

        mRadioPlayer.seekUp(mRadioPlayer.getUiband());
    }

    public void stepUp() {
        if (null == mRadioPlayer) {
            return;
        }
        int freq;
        if (IsFMrange(mCurrentFreq)) {
            freq = mCurrentFreq - mRadioParameters.FmStep;
            if (freq < mRadioParameters.FmMin) {
                freq = mRadioParameters.FmMax;
            }
        } else {
            freq = mCurrentFreq - mRadioParameters.AmStep;
            if (freq < mRadioParameters.AmMin) {
                freq = mRadioParameters.AmMax;
            }
        }
        setFreq(freq);
    }

    public void stepDown() {
        if (null == mRadioPlayer) {
            return;
        }

        int freq;
        if (IsFMrange(mCurrentFreq)) {
            freq = mCurrentFreq + mRadioParameters.FmStep;
            if (freq > mRadioParameters.FmMax) {
                freq = mRadioParameters.FmMin;
            }
        } else {
            freq = mCurrentFreq + mRadioParameters.AmStep;
            if (freq > mRadioParameters.AmMax) {
                freq = mRadioParameters.AmMin;
            }
        }
        setFreq(freq);
    }

    public void presetUp() {
        int currentIndex = currentIndx();
        int[] presets = readPresetList(currentBand());
        if (currentIndex >= 0x01) {
            setFreq(presets[currentIndex - 1], currentIndex - 1);
        } else {
            setFreq(presets[BAND_STATION_TOTAL - 1], BAND_STATION_TOTAL - 1);
        }
    }

    public void presetDown() {
        int currentIndex = currentIndx();
        int[] presets = readPresetList(currentBand());
        if (currentIndex < BAND_STATION_TOTAL - 1) {
            setFreq(presets[currentIndex + 1], currentIndex + 1);
        } else {
            setFreq(presets[0], 0);
        }

    }

    public void scan() {
        if (null == mRadioPlayer) {
            return;
        }

        mRadioPlayer.seekPlay(mRadioPlayer.getUiband());//SEEK_Play
    }

    public void handleRadioInfo(RadioInfo radioInfo, boolean ignoreRDS) {
        if (null == radioInfo) {
            return;
        }
        mRadioInfo = radioInfo;
        mScanType = radioInfo.mScanType;
        mIsLocal = radioInfo.mIsLocal;
        mIsStereo = radioInfo.mIsStereo;
        if (mScanType == RadioData.PRESET_PLAY) {
            mCurrentIndx = radioInfo.mIndexof;
        }

        syncBandWithFramework();

        if (mCurrentFreq != radioInfo.mFreq) {
            mCurrentFreq = radioInfo.mFreq;
            mBandLastFreq[mCurrentBand] = mCurrentFreq;

            if (mScanType >= RadioData.SEEK_ALL && mScanType != RadioData.PRESET_PLAY) {
                mCurrentIndx = getIndexFromPresetList(mCurrentBand, mCurrentFreq);
                mBandLastIndx[mCurrentBand] = mCurrentIndx;
            }

            dataChangeNotify(null, UpdateDataListener.EVENT_FREQ_CHANGE);
        }

        if (!ignoreRDS) {
            try {
                mRdsInfo = Integer.parseInt(radioInfo.mRDSstate);
                if ((mRdsInfo & 0x04) != 0x00) {
                    if (null == radioInfo.mPSname) {
                        mRdsPS = "";
                    } else {
                        mRdsPS = radioInfo.mPSname;
                    }
                    if (null == radioInfo.mRTtype) {
                        mRdsRT = "";
                    } else {
                        mRdsRT = radioInfo.mRTtype;
                    }
                    mPtyType = Integer.parseInt(radioInfo.mPTYtype);
                } else {
                    mRdsPS = "";
                    mRdsRT = "";
                    mPtyType = 0x00;
                }
            } catch (Exception e) {

            }
        }
    }

    public void syncBandWithFramework() {
        if (null == mRadioPlayer) {
            return;
        }
        RadioPlayer.BAND band = mRadioPlayer.getUiband();
        int tempBand = BAND_FM_1;
        if (band == RadioPlayer.BAND.FM1) {
            tempBand = BAND_FM_1;
        } else if (band == RadioPlayer.BAND.FM2) {
            tempBand = BAND_FM_2;
        } else if (band == RadioPlayer.BAND.FM3) {
            tempBand = BAND_FM_3;
        } else if (band == RadioPlayer.BAND.AM1) {
            tempBand = BAND_AM_1;
        }

        if (tempBand != mCurrentBand) {
            dataChangeNotify(null, UpdateDataListener.EVENT_BAND_CHANGE);
        }
        mCurrentBand = tempBand;
    }

    public void setFreq(int freq) {
        setFreq(freq, -1);
    }

    public void setFreq(int freq, int index) {
        setFreq(mCurrentBand, freq, index);
    }

    public void setFreq(int band, int freq, int index) {
        if (null == mRadioPlayer) {
            return;
        }
        if (index == -1) {
            index = getIndexFromPresetList(band, freq);
        }
        mCurrentIndx = index;
        mBandLastIndx[band] = index;
        mRadioPlayer.setUibandIndexFreq(BandToFrameworkBand(band), index, freq);
    }

    public String getFreqUnit(int freq) {
        if (freq < 10000) {
            return SkinUtils.getString(R.string.khz);
        } else {
            return SkinUtils.getString(R.string.mhz);
        }
    }

    public String getFormatFreq(int freq, boolean withUnit) {
        StringBuilder strFreq = new StringBuilder();
        if (freq < 10000) {
            strFreq.append(freq);
            if (withUnit) {
                strFreq.append(" ").append(SkinUtils.getString(R.string.khz));
            }
        } else {
            strFreq.append(String.format(Locale.ENGLISH, "%.02f", freq * 0.001));
            if (withUnit) {
                strFreq.append(" ").append(SkinUtils.getString(R.string.mhz));
            }
        }
        return strFreq.toString();
    }

    public RadioInfo getRadioInfo() {
        if (null == mRadioPlayer) {
            return null;
        }
        return mRadioPlayer.getRadioInfo();
    }

    public int getIndexFromPresetList(int band, int freq) {
        int[] presets = readPresetList(band);
        int index = -1;
        for (int i = 0; i < presets.length; i++) {
            if (freq == presets[i]) {
                index = i;
                break;
            }
        }
        return index;
    }

    public int[] readPresetList(int whichBand) {
        if (mPresetFreqs[whichBand][0] == 0x00) {
            fillPreset(whichBand, mRadioPlayer.getPreset(BandToFrameworkBand(whichBand)));
        }
        return mPresetFreqs[whichBand];
    }

    public void fillPreset(int band, int[] presets) {
        int len = presets.length;
        if (len < BAND_STATION_TOTAL) {
            if (band > BAND_FM_3) {
                Arrays.fill(mPresetFreqs[band], mRadioParameters.AmMin);
            } else {
                Arrays.fill(mPresetFreqs[band], mRadioParameters.FmMin);
            }
            System.arraycopy(presets, 0, mPresetFreqs[band], 0, len);
            mRadioPlayer.setPreset(BandToFrameworkBand(band), mPresetFreqs[band]);
        } else {
            System.arraycopy(presets, 0, mPresetFreqs[band], 0, BAND_STATION_TOTAL);
        }
    }

    public void savePreset(int freq, int index) {
        if (FMDataControl.CONFIG_PRESET_EDIT_NAME && isFMBand()) {
            if (!TextUtils.isEmpty(mRdsPS)) {
                writeRdsPs(String.valueOf(freq), mRdsPS);
            }
        }
        if (mRadioPlayer != null && index >= 0) {
            int[] presets = readPresetList(currentBand());
            presets[index] = freq;
            mRadioPlayer.setPreset(mRadioPlayer.getUiband(), presets);
            setFreq(freq, index);//update index to framework
        }
    }

    public void setPTY(int type) {
        if (mRadioPlayer != null) {
            mRadioPlayer.setPTY(type);
        }
        this.type = type;
    }
    public int getPty() {
        return this.type;
    }
    public void setTA(boolean enable) {
        mIsTA = enable;
        if (mRadioPlayer != null) {
            mRadioPlayer.setTA(enable);
        }
        if (enable) {
            writeRadioInfo("TA_Enable", 0x01);
        } else {
            writeRadioInfo("TA_Enable", 0x00);
        }
    }

    public void setAF(boolean enable) {
        mIsAF = enable;
        if (mRadioPlayer != null) {
            mRadioPlayer.setAF(enable);
        }
        if (enable) {
            writeRadioInfo("AF_Enable", 0x01);
        } else {
            writeRadioInfo("AF_Enable", 0x00);
        }
    }

    public int readRadioInfo(String key, int def) {
        if (mRadioInfoPreferences == null) {
            mRadioInfoPreferences = mContext.getSharedPreferences(mXMLFileName,
                    Context.MODE_PRIVATE);
        }
        return mRadioInfoPreferences.getInt(key, def);
    }

    public void writeRadioInfo(String key, int val) {
        if (mRadioInfoPreferences == null) {
            mRadioInfoPreferences = mContext.getSharedPreferences(mXMLFileName,
                    Context.MODE_PRIVATE);
        }
        synchronized (mRadioInfoPreferences) {
            SharedPreferences.Editor editor = mRadioInfoPreferences.edit();
            editor.putInt(key, val);
            editor.apply();
        }
    }

    public void saveRadioData(boolean immediately) {
        if (mRadioInfoPreferences == null) {
            mRadioInfoPreferences = mContext.getSharedPreferences(mXMLFileName,
                    Context.MODE_PRIVATE);
        }
        int saved_freq = mRadioInfoPreferences.getInt(KEY_FREQ, 87500);
        int saved_band = mRadioInfoPreferences.getInt(KEY_BAND, RadioData.BAND_FM_1);
        int saved_index = readRadioInfo(KEY_INDEX, -1);
        if (currentFreq() != saved_freq || currentBand() != saved_band
                || currentIndx() != saved_index) {
            Log.d(TAG, "saveRadioData--->band=" + currentBand() + " index=" + currentIndx() + " freq=" + currentFreq());
            synchronized (mRadioInfoPreferences) {
                SharedPreferences.Editor editor = mRadioInfoPreferences.edit();
                editor.putInt(KEY_FREQ, currentFreq());
                editor.putInt(KEY_BAND, currentBand());
                editor.putInt(KEY_INDEX, currentIndx());
                if (immediately) {
                    editor.commit();
                } else {
                    editor.apply();
                }
            }
        }
    }

    public String readRdsPs(String freq, String def) {
        String tmp = "";
        if (mSPRadioFreqName == null) {
            mSPRadioFreqName = mContext.getSharedPreferences(mXMLFreqName, Context.MODE_PRIVATE);
        }
        tmp = mSPRadioFreqName.getString(freq + "edit", def);
        if (tmp.length() <= 0) {
            tmp = mSPRadioFreqName.getString(freq, def);
        }
        return tmp;
    }

    public void writeRdsPs(String freq, String RdsPs) {
        if (mSPRadioFreqName == null) {
            mSPRadioFreqName = mContext.getSharedPreferences(mXMLFreqName, Context.MODE_PRIVATE);
        }
        SharedPreferences.Editor editor = mSPRadioFreqName.edit();
        editor.putString(freq, RdsPs);
        editor.apply();
    }

    public void deleteRdsPs(String freq) {
        if (mSPRadioFreqName == null) {
            mSPRadioFreqName = mContext.getSharedPreferences(mXMLFreqName, Context.MODE_PRIVATE);
        }
        SharedPreferences.Editor editor = mSPRadioFreqName.edit();
        editor.remove(freq);
        editor.apply();
    }

    // FMPlugService
    private final class FMServiceConnection implements ServiceConnection {

        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            mFMPlugService =  IRadioServiceAPI.Stub.asInterface(binder);
            Log.d(TAG, "onServiceConnected: FMServiceConnection");
            if (mFMPlugService != null) {
                dataChangeNotify(null, UpdateDataListener.UPDATE_DATA_INFO);
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            Log.d(TAG, "onServiceDisconnected: FMServiceConnection");
            mFMPlugService = null;
        }
    }

    public IRadioServiceAPI getFMPlugService() {
        return mFMPlugService;
    }

    // Handler
    private static final class FMDataControlHandler extends Handler {
        public static final int WM_WHAT_DATA_CHANGE = 1;

        private WeakReference<FMDataControl> mWRFmDataControl = null;

        public FMDataControlHandler(FMDataControl fmDC) {
            if (fmDC != null) {
                mWRFmDataControl = new WeakReference<FMDataControl>(fmDC);
            }
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            if (mWRFmDataControl != null) {
                FMDataControl fmDc = mWRFmDataControl.get();

                if (null != fmDc) {
                    switch (msg.what) {
                        case WM_WHAT_DATA_CHANGE:
                            fmDc.dataChangeNotify(null, msg.arg1);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    // notify ui update callback listener
    public interface UpdateDataListener {
        int UPDATE_DATA_RANGE = 1;
        int UPDATE_DATA_FREQLIST = 2;
        int UPDATE_DATA_INFO = 3;
        int UPDATE_DATA_CLOSE_TA = 4;
        int UPDATE_DATA_RDS_INFO = 5;
        int EVENT_FREQ_CHANGE = 6;
        int EVENT_BAND_CHANGE = 7;

        void updateRadioUIElement(int nType);
    }

    // register listener
    public void registerDataChangeListener(String key,
            UpdateDataListener listener) {

        if (null != mCallbackMap) {
            if (!mCallbackMap.containsKey(key)) {
                mCallbackMap.put(key, listener);
            }
        }
    }

    // unregister listener
    public void unRegisterDataChangeListener(String key) {

        if (null != mCallbackMap) {
            if (mCallbackMap.containsKey(key)) {
                mCallbackMap.remove(key);
            }
        }
    }

    // callback notify
    // if (null == key) -notify all
    public void dataChangeNotify(String key, int nType) {

        UpdateDataListener listener = null;

        if (null != mCallbackMap) {

            if (null == key) {

                Iterator<Entry<String, UpdateDataListener>> iter = mCallbackMap
                        .entrySet().iterator();

                while (iter.hasNext()) {
                    @SuppressWarnings("rawtypes")
                    Map.Entry entry = (Map.Entry) iter.next();
                    listener = (UpdateDataListener) entry.getValue();
                    if (null != listener) {
                        listener.updateRadioUIElement(nType);
                    } else {
                        iter.remove();
                    }
                }
            } else {
                listener = mCallbackMap.get(key);
                if (null != listener) {
                    listener.updateRadioUIElement(nType);
                }
            }
        }
    }

    // [是 TA/交通警告 窗口显示]
    public boolean isTAWindowShow() {
        return isAddTAView;
    }
    public static Handler handler = new Handler();
    Runnable runnable = new Runnable() {
        @Override
        public void run() {
            int mRet = RadioAudioManager.getInstance().requestAudioFocus(
                    AudioManager.AUDIOFOCUS_GAIN_TRANSIENT, AudioAttributes.USAGE_NOTIFICATION,
                    AudioAttributes.CONTENT_TYPE_SONIFICATION);
            Log.d(TAG, "run: " + mRet);
            if (mRet == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
                RadioAudioManager.getInstance().startRender();
            } else {
                handler.postDelayed(runnable, 500);
            }
        }
    };


    public void showTAWindow() {
        Log.d(TAG, "showTAWindow: AudioFocusType=" + RadioAudioManager.getInstance().getAudioFocusType());
        if (RadioAudioManager.getInstance().getAudioFocusType()
                != AudioManager.AUDIOFOCUS_GAIN) {
            handler.postDelayed(runnable, 500);
        }

        if (!RadioUtils.isReversing()) {//不在倒车状态
            if (mWindowManager == null) {
                mWindowManager = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
            }
            if (mTAWindowParam == null) {
                mTAWindowParam = new WindowManager.LayoutParams();
                if (Build.VERSION.SDK_INT >= 26) {
                    mTAWindowParam.type = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
                } else {
                    mTAWindowParam.type = WindowManager.LayoutParams.TYPE_PHONE;
                }
                mTAWindowParam.gravity = Gravity.CENTER;
                mTAWindowParam.width = WindowManager.LayoutParams.WRAP_CONTENT;
                mTAWindowParam.height = WindowManager.LayoutParams.WRAP_CONTENT;
                mTAWindowParam.format = PixelFormat.RGBA_8888;

                LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(
                        Context.LAYOUT_INFLATER_SERVICE);
                mTAView = inflater.inflate(R.layout.radio_rds_ta, null);
                mTAView.setOnTouchListener(new View.OnTouchListener() {
                    boolean mTouchInView = false;

                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        if (event.getAction() == MotionEvent.ACTION_DOWN) {
                            mTouchInView = touchEventInView(v, event);
                        } else if (event.getAction() == MotionEvent.ACTION_UP) {
                            if (mTouchInView && touchEventInView(v, event)) {
                                Intent i = new Intent(Intent.ACTION_MAIN);
                                i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                                i.setClassName(mContext, RadioMain.class.getName());
                                mContext.startActivity(i);
                            }
                        } else if (event.getAction() == MotionEvent.ACTION_MOVE) {
                        }
                        return true;
                    }
                });

                mTAView.findViewById(R.id.close_ta).setOnClickListener(new View.OnClickListener() {

                    @Override
                    public void onClick(View v) {
                        // TODO Auto-generated method stub
                        setTA(false);
                        removeTAWindow();
                        dataChangeNotify(null, UpdateDataListener.UPDATE_DATA_CLOSE_TA);
                    }
                });
            }
            if (!isAddTAView) {
                isAddTAView = true;
                mWindowManager.addView(mTAView, mTAWindowParam);
            }
        }
    }

    public void removeTAWindow() {
        Log.d(TAG, "removeTAWindow: AudioFocusType=" + RadioAudioManager.getInstance().getAudioFocusType());
        handler.removeCallbacksAndMessages(null);
        if (RadioAudioManager.getInstance().getAudioFocusType()
                != AudioManager.AUDIOFOCUS_GAIN) {
            RadioAudioManager.getInstance().releaseAudioFocus();
            RadioAudioManager.getInstance().stopRender();
        }
        hideTAWindow();
    }

    public void hideTAWindow() {
        if (isAddTAView && mWindowManager != null) {
            mWindowManager.removeViewImmediate(mTAView);
            isAddTAView = false;
        }
    }

    public static boolean touchEventInView(View view, MotionEvent ev) {
        if (view == null || ev == null) {
            return false;
        }
        int[] loc = new int[2];
        Rect rect;

        view.getLocationOnScreen(loc);
        rect = new Rect(0, 0, view.getWidth(), view.getHeight());
        rect.offsetTo(loc[0], loc[1]);
        return rect.contains((int) ev.getRawX(), (int) ev.getRawY());
    }

    /**
     * 初始化收藏功能场景的数据库
     */
    public void initFMCollectionFunction() {
        if (mFMCollectionFunc == null && CONFIG_FREQ_COLLECT) {
            LitePalApplication.initialize(mContext);
            FMCollectListDao.initFMCollectListDateBase(mContext);
            mFMCollectionFunc = FMCollectionFunction.getInstance();
            mFMCollectionFunc.init();
        }
    }

    /**
     * 获取当前波段的正在播放的频点
     *
     * @return
     */
    public String getCurBandFreq() {
        int freq = currentFreq();
        if (freq != 0) {
            if (currentBand() < BAND_AM_1) {
                return String.format(Locale.ENGLISH, "%.02f", freq * 0.001);
            } else {
                return String.valueOf(freq);
            }
        } else {
            return String.valueOf(freq);
        }
    }

    /**
     * 是否处于电台搜索中
     * @return
     */
    public boolean isAS() {
        int scanType = getScanType();
        return scanType == RadioData.SEEK_ALL;
    }

    /**
     * 是否处于电台浏览中
     *
     * @return
     */
    public boolean isPS() {
        int scanType = getScanType();
        return scanType == RadioData.PRESET_PLAY;
    }

    /**
     * 是否在进行搜台
     *
     * @return
     */
    public boolean isScan() {
        int scanType = getScanType();
        return scanType == RadioData.SEEK_PLAY;
    }

    /**
     * 是否在进行上下一个有效台收搜
     *
     * @return
     */
    public boolean isSeek() {
        int scanType = getScanType();
        return scanType == RadioData.SEEK_UP || scanType == RadioData.SEEK_DOWN;
    }

    /**
     * 收藏电台
     * @param band
     * @param freq
     * @param pos
     */
    public boolean collectFreq(int band, String freq, int pos) {
        if (mFMCollectionFunc == null) {
            return false;
        }
        if (band < BAND_FM_1 || band >= BAND_SIZE || TextUtils.isEmpty(freq) || pos < 0) {
            Log.v(TAG, " collectFreq, param error!!!"
                    + " band=" + band
                    + " BAND_SIZE=" + BAND_SIZE
                    + " pos=" + pos
                    + " freq=" + freq
            );
           return false;
        }
        Log.v(TAG, " collectFreq, band = " + band + " pos = " + pos + " freq = " + freq);
        if (mFMCollectionFunc.isFreqCollected(band, freq)
                || mFMCollectionFunc.isFreqCollected(band, pos)) {
            return true;
        } else {
            return mFMCollectionFunc.collectFreq(band, freq, pos);
        }
    }

    public boolean collectFreq(int band, String freq) {
        if (mFMCollectionFunc == null) {
            return false;
        }
        if (band < BAND_FM_1 || band >= BAND_SIZE || TextUtils.isEmpty(freq)) {
            Log.v(TAG, " collectFreq param error!!!"
                    + " band=" + band
                    + " BAND_SIZE=" + BAND_SIZE
                    + " freq=" + freq
            );
            return false;
        }

        Log.v(TAG, " collectFreq, band = " + freq + " pos = " + freq);
        if (!mFMCollectionFunc.isFreqCollected(band, freq)) {
            return mFMCollectionFunc.collectFreq(band, freq, -1);
        }
        return true;
    }

    public List<String> getCollectedFreqList(int band) {
        if (mFMCollectionFunc == null) {
            return null;
        }
        return mFMCollectionFunc.getCollectedFreqList(band);
    }

    public boolean deleteCollectFreq(int band, int pos) {
        if (mFMCollectionFunc == null) {
            return false;
        }
        if (band < BAND_FM_1 || band >= BAND_SIZE || pos < 0) {
            Log.v(TAG, " deleteCollectFreq(band, pos) failed, param error"
                    + " band=" + band
                    + " BAND_SIZE=" + BAND_SIZE
                    + " pos=" + pos
            );
            return false;
        }
        Log.v(TAG, " deleteCollectFreq, band = " + band + " pos = " + pos);
        mFMCollectionFunc.deleteCollectedFreq(band, pos);
        return true;
    }

    public boolean deleteCollectFreq(int band, String freq) {
        if (mFMCollectionFunc == null) {
            return false;
        }
        if (band < BAND_FM_1 || band >= BAND_SIZE || TextUtils.isEmpty(freq)) {
            Log.v(TAG, " deleteCollectFreq2(band, freq) failed, param error"
                    + " band=" + band
                    + " BAND_SIZE=" + BAND_SIZE
                    + " freq=" + freq
            );
            return false;
        }
        Log.v(TAG, " deleteCollectFreq, band = " + band + " pos = " + freq);
        mFMCollectionFunc.deleteCollectedFreq(band, freq);
        return true;
    }

    public boolean deleteCollectFreqAll(int band) {
        if (mFMCollectionFunc == null) {
            return false ;
        }
        if (band < BAND_FM_1 || band >= BAND_SIZE) {
            Log.v(TAG, " deleteCollectFreq(band) failed, param error "
                    + " band=" + band
                    + " BAND_SIZE=" + BAND_SIZE
            );
            return false;
        }
        Log.v(TAG, " deleteCollectFreqAll, band = " + band);
        mFMCollectionFunc.deleteCollectedFreqAll(band);
        return true;
    }

    public boolean isCollectedFreq(int band, int pos) {
        if (mFMCollectionFunc == null) {
            return false;
        }
        return mFMCollectionFunc.isFreqCollected(band, pos);
    }

    public boolean isCollectedFreq(int band, String freq) {
        if (mFMCollectionFunc == null) {
            return false;
        }
        return mFMCollectionFunc.isFreqCollected(band, freq);
    }


    public void gotoFreq(String freq) {
        if (!TextUtils.isEmpty(freq)) {
            int tmp_freq = 0;
            if (isFMBand()) {
                //后装是1000
                tmp_freq = (int) (Float.parseFloat(freq) * 1000);
            } else {
                tmp_freq = Integer.parseInt(freq);
            }
            if (tmp_freq != 0) {
                Log.v(TAG, " gotoFreq tmp_freq=" + tmp_freq + " freq=" + freq);
                //5位数的
                setFreq(tmp_freq);
            }
        } else {
            Log.v(TAG, " gotoFreq failed freq=" + freq);
        }
    }

    public void initCurrentFreq() {
        if ("1".equals(RadioUtils.getProp("sys.radio.rds", "0"))) {
            Log.d(TAG, "onCreate: Traffic on!");
            handleRadioInfo(getRadioInfo(), false);
            setFreq(mCurrentBand, mCurrentFreq, -1);
        } else {
            mCurrentFreq = readRadioInfo("currentFreq", -1);
            mCurrentBand = readRadioInfo("currentBand", RadioData.BAND_FM_1);
            if (mCurrentFreq == -1) {
                mCurrentFreq = FMDataControl.mRadioParameters.FmMin;
                mCurrentIndx = 0;
            } else {
                mCurrentIndx = readRadioInfo("currentIndex", -1);
            }
            mBandLastFreq[mCurrentBand] = mCurrentFreq;
            setFreq(mCurrentBand, mCurrentFreq, mCurrentIndx);
        }
    }
    public void setReg(boolean enable) {
        mIsReg = enable;
        if (mRadioPlayer != null) {
            mRadioPlayer.setREG(enable);
        }
        if (enable) {
            writeRadioInfo("Reg_Enable", 0x01);
        } else {
            writeRadioInfo("Reg_Enable", 0x00);
        }
    }

    public void setFMRssiThreshold(int rssi){
        if (mRadioPlayer != null) {
            mRadioPlayer.setFMRssiThreshold(rssi);
        }
    }

    public void setAMRssiThreshold(int rssi){
        if (mRadioPlayer != null) {
            mRadioPlayer.setAMRssiThreshold(rssi);
        }
    }
}
