package com.hcn_library.util;

public interface ConstantDsp {
    int DEF_DSP_BANDS[][] = new int[][]{
            //user def.
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            //mNewsEqBandsValues
            {6, 8, 6, 2, 3, 0, -1, 0, -1, 6, 6, 8, 3, 2, 3, 4},
            //mJazzEqBandsValues
            {2, 2, 4, 3, 3, 4, -1, 0, -1, -3, -3, 2, -4, -8, -6, -4},
            //mCityEqBandsValues
            {5, 8, 5, 2, 4, 2, -2, -2, -2, -3, 0, -3, -2, 2, 1, -1},
            //mPopEqBandsValues
            {-2, 2, 0, 5, 5, 6, 6, 6, 4, 1, 1, 2, -2, -2, -4, -3},
            //mElectronicEqBandsValues
            {4, 5, 3, 3, 0, 1, 3, 4, 1, 1, 0, -1, -3, -4, -2, -2},
            //mClassicEqBandsValues
            {2, 0, -2, 1, 0, -1, -2, -2, 0, 1, 0, -1, -4, -3, -4, -3},
            //mMovieEqBandsValues
            {5, 2, 1, 3, 3, 0, -1, 0, 1, 2, 4, 4, 3, 5, 5, 0},
            //mRockEqBandsValues
            {5, 5, 6, 2, 0, 0, -1, 0, 1, 4, 0, 4, 5, 5, 5, 6},
            //mTechEqBandsValues
            {5, 5, 2, 4, 4, 0, 3, 0, 3, 4, 2, 4, 4, 4, 4, 3}
    };

    // mcx 客户，dsp内置，要做32段作假
    int DEF_DSP_BANDS_32[][] = new int[][]{
            //user def.
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//            //mNewsEqBandsValues
//            {6, 8, 6, 2, 3, 0, -1, 0, -1, 6, 6, 8, 3, 2, 3, 4},
//            //mJazzEqBandsValues
//            {2, 2, 4, 3, 3, 4, -1, 0, -1, -3, -3, 2, -4, -8, -6, -4},
//            //mCityEqBandsValues
//            {5, 8, 5, 2, 4, 2, -2, -2, -2, -3, 0, -3, -2, 2, 1, -1},
//            //mPopEqBandsValues
//            {-2, 2, 0, 5, 5, 6, 6, 6, 4, 1, 1, 2, -2, -2, -4, -3},
//            //mElectronicEqBandsValues
//            {4, 5, 3, 3, 0, 1, 3, 4, 1, 1, 0, -1, -3, -4, -2, -2},
//            //mClassicEqBandsValues
//            {2, 0, -2, 1, 0, -1, -2, -2, 0, 1, 0, -1, -4, -3, -4, -3},
//            //mMovieEqBandsValues
//            {5, 2, 1, 3, 3, 0, -1, 0, 1, 2, 4, 4, 3, 5, 5, 0},
//            //mRockEqBandsValues
//            {5, 5, 6, 2, 0, 0, -1, 0, 1, 4, 0, 4, 5, 5, 5, 6},
//            //mTechEqBandsValues
//            {5, 5, 2, 4, 4, 0, 3, 0, 3, 4, 2, 4, 4, 4, 4, 3}
    };
}
