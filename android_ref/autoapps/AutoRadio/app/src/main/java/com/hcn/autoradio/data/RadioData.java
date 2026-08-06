package com.hcn.autoradio.data;

import android.radio.RadioPlayer;
import android.radio.Utility;

import com.hcn.autoradio.R;
import com.hcn.autoradio.skin.ThemeID;
import com.hcn.autoradio.util.RadioUtils;

import java.util.Arrays;

public class RadioData {
    // band
    public static final int BAND_FM_1 = 0;
    public static final int BAND_FM_2 = 1;
    public static final int BAND_FM_3 = 2;
    public static final int BAND_AM_1 = 3;
    public static final int BAND_SIZE = 4;

    // region
    public static final int REGION_USA = 0;
    public static final int REGION_Europe = 1;
    public static final int REGION_Latin = 2;
    public static final int REGION_OIRT = 3;
    public static final int REGION_China = 4;
    public static final int REGION_JAPAN = 5;
    public static final int REGION_AUSTRALIA = 6;
    public static final int REGION_Latin2 = 7;//76--108Mhz

    protected int mRegion;
    public static Utility.RadioParameters mRadioParameters = new Utility.RadioParameters();

    //scan type
    public static final int SEEK_ALL = 1;//AS
    public static final int SEEK_PLAY = 2;//SCAN
    public static final int SEEK_UP = 3;
    public static final int SEEK_DOWN = 4;
    public static final int PRESET_PLAY = 5;//PS

    protected int mScanType;

    public int mCurrentBand;
    public int mCurrentFreq;
    public int mCurrentIndx;

    public int[] mBandLastFreq = new int[BAND_SIZE];
    public int[] mBandLastIndx = {-1, -1, -1, -1, -1};

    // channel
    public int BAND_STATION_TOTAL;
    public static final int PAGE_STATION_NUM = 0x06;
    public int[][] mPresetFreqs = null;

    protected boolean mIsLocal;
    protected boolean mIsStereo;

    public boolean mIsSupportRDS = true;
    public boolean mIsAF;
    public boolean mIsTA;
    public boolean mIsReg;
    public int mPtyType;
    public int mRdsInfo;
    public String mRdsPS;
    public String mRdsRT;

    public static final int E_THEME_GOD = RadioUtils.getIntProp("persist.sys.etheme_god", 0);
    public static final int E_THEME_SUB = RadioUtils.getIntProp("persist.sys.etheme_sub", 0);

    RadioData() {
        mCurrentBand = BAND_FM_1;
        mCurrentFreq = mRadioParameters.FmMin;
        mCurrentIndx = 0;
        if (E_THEME_GOD == ThemeID.E_THEME_ID_206) {
            BAND_STATION_TOTAL = 8;
        } else {
            BAND_STATION_TOTAL = 18;
        }
        if (null == mPresetFreqs) {
            mPresetFreqs = new int[BAND_SIZE][BAND_STATION_TOTAL];
        }

        mIsLocal = false;
        mIsStereo = false;
        mScanType = 0;

        mIsAF = false;
        mIsTA = false;
        mIsReg = false;
        mPtyType = 0;
        mRdsInfo = 0;
    }

    public int currentBand() {
        return mCurrentBand;
    }

    public int currentFreq() {
        return mCurrentFreq;
    }

    public int currentIndx() {
        return mCurrentIndx;
    }

    public int lastFreq(int band) {
        if (mBandLastFreq[band] == 0x00) {
            if (band <= BAND_FM_3) {
                return mRadioParameters.FmMin;
            } else {
                return mRadioParameters.AmMin;
            }
        }
        return mBandLastFreq[band];
    }

    public int lastIndx(int band) {
        return mBandLastIndx[band];
    }

    public boolean isLocal() {
        return mIsLocal;
    }

    public boolean isStereo() {
        return mIsStereo;
    }

    public int getScanType() {
        return mScanType;
    }

    public int getRegion() {
        return mRegion;
    }

    public void setRegion(int mRegion) {
        this.mRegion = mRegion;
        Arrays.fill(mBandLastFreq, 0x00);
        Arrays.fill(mBandLastIndx, -1);

    }

    public boolean IsAMrange(int freq) {
        return (freq >= mRadioParameters.AmMin) && (freq <= mRadioParameters.AmMax);
    }

    public boolean IsFMrange(int freq) {
        return (freq >= mRadioParameters.FmMin) && (freq <= mRadioParameters.FmMax);
    }

    public boolean isFMBand() {
        return mCurrentBand <= BAND_FM_3;
    }

    public RadioPlayer.BAND BandToFrameworkBand(int iBand) {
        RadioPlayer.BAND mBand = RadioPlayer.BAND.FM1;
        if (iBand >= BAND_FM_1 && iBand < BAND_SIZE) {
            if (iBand == BAND_FM_1) {
                mBand = RadioPlayer.BAND.FM1;
            } else if (iBand == BAND_FM_2) {
                mBand = RadioPlayer.BAND.FM2;
            } else if (iBand == BAND_FM_3) {
                mBand = RadioPlayer.BAND.FM3;
            } else if (iBand == BAND_AM_1) {
                mBand = RadioPlayer.BAND.AM1;
            }
        }
        return mBand;
    }

    public String BandToString(int band) {

        String strBand = "FM-1";

        switch (band) {
            case BAND_FM_1:
                strBand = "FM-1";
                break;
            case BAND_FM_2:
                strBand = "FM-2";
                break;
            case BAND_FM_3:
                strBand = "FM-3";
                break;
            case BAND_AM_1:
                strBand = "AM-1";
                break;
            default:
                break;
        }
        return strBand;
    }

    public int BandToImage(int band) {

        int R_ID = R.drawable.radio_fm1;

        switch (band) {
            case BAND_FM_1:
                R_ID = R.drawable.radio_fm1;
                break;
            case BAND_FM_2:
                R_ID = R.drawable.radio_fm2;
                break;
            case BAND_FM_3:
                R_ID = R.drawable.radio_fm3;
                break;
            case BAND_AM_1:
                R_ID = R.drawable.radio_am1;
                break;
            default:
                break;
        }
        return R_ID;
    }

    public int UnitToImage() {
        int R_ID;
        if (isFMBand()) {
            R_ID = R.drawable.radio_mhz;
        } else {
            R_ID = R.drawable.radio_khz;
        }
        return R_ID;
    }
}
