package com.hcn.autoeq.util;

public interface ConstantAsp {
    int[][] DEF_ASP_BANDS_CD3313 = new int[][]{
            //user def
            {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
            //mNewsEqBandsValues
            {5, 5, 6, 8, 7, 7, 4, 6, 7, 5, 8, 7},
            //mJazzEqBandsValues
            {10, 10, 9, 10, 9, 10, 6, 5, 8, 8, 5, 6},
            //mCityEqBandsValues
            {7, 6, 10, 6, 9, 6, 7, 6, 9, 9, 10, 8},
            //mPopEqBandsValues
            {5, 5, 8, 6, 10, 8, 10, 8, 10, 9, 10, 10},
            //mElectronicEqBandsValues
            {10, 9, 9, 10, 9, 10, 9, 10, 10, 9, 10, 10},
            //mClassicEqBandsValues
            {6, 5, 6, 6, 9, 10, 5, 4, 5, 5, 4, 5},
            //mMovieEqBandsValues
            {8, 4, 10, 10, 9, 10, 8, 5, 8, 5, 6, 5},
            //mRockEqBandsValues
            {6, 8, 7, 8, 7, 8, 10, 10, 9, 10, 9, 9},
            //mTechEqBandsValues
            {10, 9, 10, 9, 10, 10, 9, 10, 10, 9, 10, 8}
    };
    int[][] DEF_ASP_BANDS_ZL3560 = new int[][]{
            //user def
            {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
            //mNewsEqBandsValues
            {7, 11, 13, 13, 7, 12, 7, 7, 7, 7, 7, 7},
            //mJazzEqBandsValues
            {7, 13, 13, 12, 7, 9, 9, 11, 11, 11, 11, 11},
            //mCityEqBandsValues
            {7, 12, 12, 10, 7, 7, 7, 12, 7, 7, 12, 12},
            //mPopEqBandsValues
            {7, 10, 13, 13, 7, 7, 7, 8, 13, 12, 13, 13},
            //mElectronicEqBandsValues
            {13, 10, 5, 6, 6, 12, 12, 13, 13, 12, 11, 11},
            //mClassicEqBandsValues
            {10, 11, 9, 9, 7, 10, 9, 6, 6, 6, 6, 6},
            //mMovieEqBandsValues
            {12, 11, 7, 5, 6, 7, 5, 5, 7, 7, 7, 7},
            //mRockEqBandsValues
            {12, 13, 7, 7, 11, 7, 12, 12, 12, 12, 12, 12},
            //mTechEqBandsValues
            {10, 7, 7, 10, 11, 12, 10, 11, 12, 12, 13, 13}
    };

    // 魏波需求：只有8581平台的zl3560芯片默认值仿造dsp芯片的默认值
    int[][] DEF_ASP_BANDS = EqUtils.is8581() && EqUtils.ASP_CHIP_ZL3560.equals(EqUtils.getEqChipType()) ? DEF_ASP_BANDS_ZL3560 : DEF_ASP_BANDS_CD3313;

}
