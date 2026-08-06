package com.hcn_library.util;

public interface SIConstantExtDsp extends ConstantExtDsp {

    // HPF
    int INDEX_LPF_HPF_F = 0x00;    //前左、前右
    int INDEX_LPF_HPF_R = 0x01;        //后左、后右
    int INDEX_LPF_HPF_CEN = 0x02;        //中置
    int INDEX_LPF_HPF_SUB = 0x03;    //重低音

    int SI_CHANNEL_FRONT_HIGH = 1; // 前高
    int SI_CHANNEL_FRONT_LOW = 2; // 前低
    int SI_CHANNEL_REAR_HIGH = 3; // 后高
    int SI_CHANNEL_REAR_LOW = 4; // 后低
    int SI_CHANNEL_SUBWOOFER_HIGH = 5; // 重低音高
    int SI_CHANNEL_SUBWOOFER_LOW = 6; // 重低音低
    int SI_CHANNEL_CENTER_HIGH = 7; // 中置低音高
    int SI_CHANNEL_CENTER_LOW = 8; // 中置低音低


    int SI_HLPF_FRONT_REAR_FREQ_MIN = 20;
    int SI_HLPF_FRONT_REAR_FREQ_MAX = 20000;
    int SI_HLPF_SUBWOOFER_FREQ_MIN = 20;
    int SI_HLPF_SUBWOOFER_FREQ_MAX = 400;

    int SI_HLPF_INDEX_LPF_HPF_F_MIN_DEFAULT = 20;
    int SI_HLPF_INDEX_LPF_HPF_F_MAX_DEFAULT = 20000;
    int SI_HLPF_INDEX_LPF_HPF_R_MIN_DEFAULT = 20;
    int SI_HLPF_INDEX_LPF_HPF_R_MAX_DEFAULT = 20000;
    int SI_HLPF_INDEX_LPF_HPF_SUB_MIN_DEFAULT = 20;
    int SI_HLPF_INDEX_LPF_HPF_SUB_MAX_DEFAULT = 200;
    int SI_HLPF_INDEX_LPF_HPF_CEN_MIN_DEFAULT = 630;
    int SI_HLPF_INDEX_LPF_HPF_CEN_MAX_DEFAULT = 6300;

    int DEF_QVALUE = EqUtils.getBandTotal() <= 16 ? 2000 : 5000; // q值默认值（8257平台陈贵峰要求）
//    int DEF_QVALUE = 2000; // q值默认值（8257平台陈贵峰要求）


    // 注意：底层支持16段，目前界面只做14段
    // 0：native 所需的 band
    // 1：native 所需的 freq
    // 2：线性控件所需
    int[][] SI_DEF_EQ_14_FREQ_VALUES = new int[][]{
            {0, 30, 30},
            {1, 63, 63},
            {2, 80, 80},
            {3, 100, 100},
            {4, 125, 125},
            {5, 160, 160},
            {6, 200, 200},
            {7, 300, 300},
            {8, 400, 400},
            {9, 630, 630},
            {10, 1000, 1000},
            {11, 3150, 3150},
            {12, 8000, 8000},
            {13, 16000, 16000}
    };

    int[][] SI_DEF_EQ_16_FREQ_VALUES = new int[][]{
            {0, 20, 20},
            {1, 30, 30},
            {2, 50, 50},
            {3, 80, 80},
            {4, 125, 125},
            {5, 200, 200},
            {6, 320, 320},
            {7, 500, 500},
            {8, 800, 800},
            {9, 1000, 1000},
            {10, 2000, 2000},
            {11, 3000, 3000},
            {12, 5000, 5000},
            {13, 8000, 8000},
            {14, 12000, 12000},
            {15, 20000, 20000},
    };


    int SI_DEF_EQ_32_FREQ_VALUES[][] = new int[][]{
            {0, 20, 20}, {1, 25, 25}, {2, 32, 32}, {3, 40, 40},
            {4, 50, 50}, {5, 65, 65}, {6, 80, 80}, {7, 100, 100},
            {8, 125, 125}, {9, 160, 160}, {10, 200, 200}, {11, 250, 250},
            {12, 315, 315}, {13, 400, 400}, {14, 500, 500}, {15, 630, 630},
            {16, 800, 800}, {17, 1000, 1000}, {18, 1200, 1200}, {19, 1600, 1600},
            {20, 2000, 2000}, {21, 2500, 2500}, {22, 3200, 3200},
            {23, 4000, 4000}, {24, 5000, 5000}, {25, 6300, 6300},
            {26, 8000, 8000}, {27, 10000, 10000}, {28, 12500, 12500},
            {29, 16000, 16000}, {30, 18000, 18000}, {31, 20000, 20000}
    };

    int SI_DEF_EQ_48_FREQ_VALUES[][] = new int[][]{
            {0, 20, 20}, {1, 23, 23}, {2, 28, 28}, {3, 31, 31},{4, 36, 36},
            {5, 42, 42}, {6, 48, 48}, {7, 55, 55}, {8, 65, 65},{9, 75, 75},
            {10, 85, 85}, {11, 100, 100}, {12, 125, 125}, {13, 135, 135}, {14, 156, 156},
            {15, 180, 180}, {16, 210, 210}, {17, 245, 245}, {18, 280, 280}, {19, 320, 320},
            {20, 375, 375}, {21, 400, 400},{22, 500, 500}, {23, 600, 600}, {24, 700, 700},
            {25, 800, 800}, {26, 900, 900}, {27, 1000, 1000},{28, 1250, 1250}, {29, 1400, 1400},
            {30, 1700, 1700},{31, 2000, 2000}, {32, 2400, 2400}, {33, 2800, 2800},{34, 3000, 3000},
            {35, 3500, 3500}, {36, 4000, 4000}, {37, 4200, 4200}, {38, 5000, 5000}, {39, 6200, 6200},
            {40, 7000, 7000}, {41, 8000, 8000}, {42, 9000, 9000}, {43, 10000, 10000}, {44, 12500, 12500},
            {45, 16000, 16000}, {46, 18000, 18000}, {47, 20000, 20000}
    };

    int SI_DEF_EQ_36_FREQ_VALUES[][] = new int[][]{
            {0, 20, 20}, {1, 23, 23}, {2, 28, 28}, {3, 36, 36},
            {4, 42, 42}, {5, 55, 55}, {6, 65, 65}, {7, 85, 85},
            {8, 100, 100}, {9, 125, 125}, {10, 135, 135}, {11, 180, 180},
            {12, 210, 210}, {13, 280, 280}, {14, 320, 320}, {15, 375, 375},
            {16, 500, 500}, {17, 600, 600}, {18, 700, 700}, {19, 900, 900},
            {20, 1000, 1000}, {21, 1400, 1400}, {22, 1700, 1700}, {23, 2000, 2000},
            {24, 2400, 2400}, {25, 2800, 2800}, {26, 3500, 3500}, {27, 4000, 4000},
            {28, 5000, 5000}, {29, 6200, 6200}, {30, 7000, 7000}, {31, 9000, 9000},
            {32, 12500, 12500}, {33, 16000, 16000}, {34, 18000, 18000}, {35, 20000, 20000}};

    //Used to DSP 14 bands
    int SI_DEF_EQ_14_BANDS_VALUES[][][] = new int[][][]{
            {
                    //mNewsEqBandsValues
                    {0, 6, 4, -2, 8, 8, 2, 0, 0, 6, 0, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mJazzEqBandsValues
                    {0, 6, 10, 4, 8, 4, 0, 0, 0, 2, 2, 10, 10, 10},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mCityEqBandsValues
                    {0, 6, 6, 2, 6, 8, 0, 2, 0, 0, 2, 10, 0, 8},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mPopEqBandsValues
                    {0, 2, 4, 4, 8, 8, 0, 0, 0, 4, 0, 8, 6, 8},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mElectronicEqBandsValues
                    {10, 8, 4, -2, -6, -2, -2, -2, -2, 8, 10, 10, 8, 4},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mClassicEqBandsValues
                    {6, 6, 8, 8, 4, 4, 2, 0, 2, 6, 2, -2, -2, -2},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mMovieEqBandsValues
                    {10, 10, 6, 6, 0, -6, -4, -4, 0, 0, -4, -4, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mRockEqBandsValues
                    {8, 10, 10, 6, 0, 0, 0, 0, 0, 6, 0, 8, 8, 8},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mTechEqBandsValues
                    {4, 2, 0, -2, 0, 4, 6, 8, 8, 10, 6, 8, 8, 10},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            }
    };

    int SI_DEF_EQ_16_BANDS_VALUES[][][] = new int[][][]{
            {
                    //mNewsEqBandsValues
                    {-2, -5, -6, -4, -3, 0, 1, 6, 6, 2, 2, -3, -4, -5, -5, -5},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mJazzEqBandsValues
                    {0, 0, 0, 0, 0, 7, 7, 4, 3, 3, -2, 2, 4, 5, 5, 5},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mCityEqBandsValues
                    {3, 4, 6, 6, 0, -7, 0, 4, 4, 0, -8, 0, 7, 6, 2, 2},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mPopEqBandsValues
                    {0, 0, 0, -1, -2, -4, -5, -5, -5, -5, -5, -3, -1, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mElectronicEqBandsValues
                    {7, 5, 2, 1, -1, -6, -5, -2, -1, 0, 5, 8, 6, 6, 7, 7},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mClassicEqBandsValues
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -6, -6, -5, -5, -5, -8},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mMovieEqBandsValues
                    {6, 4, 4, 4, 6, 6, 3, 3, 3, -2, -3, -3, -3, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mRockEqBandsValues
                    {8, 5, 2, -2, -4, 7, -2, -2, -2, 1, 2, 6, 6, 7, 9, 9},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mTechEqBandsValues
                    {-2, 3, 4, 6, 6, 2, -4, -4, -1, -1, 5, 8, 8, 8, 10, 10},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            }
    };

    //Used to DSP 32 bands
    int SI_DEF_EQ_32_BANDS_VALUES[][][] = new int[][][]{
            {
                    //mNewsEqBandsValues
                    {-2, 0, 2, 0, 2, 4, 6, 8,
                            6, 4, 6, 8, 6, 6, 4, 6,
                            2, 0, 0, 0, 2, 0, 0, 0,
                            2, 0, 0, 2, 2, 2, 0, 2},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mJazzEqBandsValues
                    {4, 6, 2, 2, 0, -4, -2, -4,
                            -4, -2, -4, -2, -4, -4, -8, -10,
                            -6, 0, 0, -6, -4, -6, -4, -6,
                            0, 2, 0, 0, 2, 2, 2, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mCityEqBandsValues
                    {-4, -4, -2, 0, 2, 4, 4, 2,
                            4, 6, 4, 6, 6, 8, 6, 4,
                            2, 2, 2, 4, 6, 2, 0, 2,
                            2, 0, 0, 0, 0, 2, 0, 2},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mPopEqBandsValues
                    {-2, 0, 2, 4, 6, 8, 6, 8,
                            8, 8, 6, 6, 6, 4, 0, -4,
                            -4, -2, -2, -4, -2, -4, -2, -4,
                            -2, 0, 0, -2, -2, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mElectronicEqBandsValues
                    {8, 6, 6, 6, 4, 0, -2, 0,
                            -2, -6, -4, -6, -4, -2, 0, 8,
                            10, 10, 8, 8, 8, 10, 8, 10,
                            10, 10, 10, 8, 8, 8, 8, 10},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mClassicEqBandsValues
                    {0, 0, -2, -2, 0, 0, -2, -2,
                            -2, 0, 0, 0, 0, 0, 0, -10,
                            -8, -8, -10, -12, -12, -8, -6, -6,
                            -8, -8, -8, -8, -10, -10, -10, -10},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mMovieEqBandsValues
                    {10, 6, 8, 10, 8, 6, 4, 6,
                            4, 6, 4, 0, 2, -6, -4, -4,
                            0, 0, -4, -4, -4, -2, -4, 0,
                            2, 0, 2, 0, 2, 0, 2, 2},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mRockEqBandsValues
                    {4, 2, 4, 0, 2, -8, -6, -8,
                            -4, -8, -4, -6, -4, 0, 4, 6,
                            4, 2, 8, 8, 6, 4, 2, 8,
                            6, 4, 4, 6, 6, 4, 8, 4},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mTechEqBandsValues
                    {4, 2, 4, 2, 2, 0, 0, 0,
                            0, -2, 0, -2, 0, 2, 4, 8,
                            10, 10, 12, 8, 8, 10, 8, 8,
                            8, 10, 10, 10, 12, 12, 10, 12},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            }
    };

    //Used to DSP 48 bands
    int SI_DEF_EQ_48_BANDS_VALUES[][][] = new int[][][]{
            {
                    //mNewsEqBandsValues
                    {0, 0, 0, 4, 0, 0, 0, 0,
                            0, 8, 0, 2, 0, 0, 0, 0,
                            0, -2, -4, -2, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 2, 0, 0, 0, 0, 4,
                            0, 6, 0, 0, 0, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mJazzEqBandsValues
                    {6, 4, 4, 2, 4, 0, -2, 0,
                            0, 2, -2, -6, -6, -6, -6, -6,
                            -2, -2, -4, -2, -2, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 2,
                            2, 4, 2, 2, 0, 2, 2, 4,
                            6, 6, 8, 10, 8, 10, 8, 6},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mCityEqBandsValues
                    {-4, -4, -2, 0, 2, 4, 4, 2,
                            4, 6, 4, 6, 6, 8, 6, 4,
                            2, 2, 2, 4, 6, 2, 0, 2,
                            2, 0, 0, 0, 0, 2, 0, 2,
                            -4, -2, -2, -4, -2, -4, -2, -4,
                            -2, 0, 0, -2, -2, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mPopEqBandsValues
                    {0, 2, 2, 4, 4, 4, 4, 4,
                            4, 8, 6, 4, 8, 6, 6, 4,
                            2, -2, -4, -2, 0, 0, -2, -4,
                            0, 0, -2, 0, -2, -2, -2, 0,
                            -2, 0, 2, 2, 4, 4, 4, 4,
                            6, 6, 8, 10, 8, 10, 8, 6},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mElectronicEqBandsValues
                    {8, 6, 6, 6, 4, 0, -2, 0,
                            -2, -6, -4, -6, -4, -2, 0, 8,
                            10, 10, 8, 8, 8, 10, 8, 10,
                            10, 10, 10, 8, 8, 8, 8, 10,
                            -8, -8, -10, -12, -12, -8, -6, -6,
                            -8, -8, -8, -8, -10, -10, -10, -10},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mClassicEqBandsValues
                    {6, 4, 2, 4, 2, 2, 0, 2,
                            2, 6, 2, 2, 0, 0, 0, 0,
                            0, -2, -4, -2, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 2, 0, 0, 2, 2, 4,
                            6, 6, 8, 8, 8, 8, 6, 4},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mMovieEqBandsValues
                    {6, 2, 4, 6, 4, 2, 0, 2,
                            4, 6, 4, 0, 2, -6, -4, -4,
                            0, 0, -4, -4, -4, -2, -4, 0,
                            2, 0, 2, 0, 2, 0, 2, 2,
                            4, 2, 8, 8, 6, 4, 2, 8,
                            6, 4, 4, 6, 6, 4, 8, 4},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mRockEqBandsValues
                    {8, 6, 6, 4, 4, 4, 4, 6,
                            6, 4, 6, -2, 0, -4, -8, -10,
                            -10, -6, -8, -6, -6, -4, -2, -4,
                            -4, 0, -2, -4, -2, -2, -4, -2,
                            -2, 0, -2, 0, 0, 2, 2, 0,
                            6, 2, 12, 6, 0, -6, -8, -10},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mTechEqBandsValues
                    {4, 2, 4, 2, 2, 0, 0, 0,
                            0, -2, 0, -2, 0, 2, 4, 8,
                            10, 10, 12, 8, 8, 10, 8, 8,
                            8, 10, 10, 10, 12, 12, 10, 12,
                            4, 2, 8, 8, 6, 4, 2, 8,
                            6, 4, 4, 6, 6, 4, 8, 4},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            }
    };

    int SI_DEF_EQ_36_BANDS_VALUES[][][] = new int[][][]{{
            //mNewsEqBandsValues
            {0, 0, 0, 4
            , 0, 0, 0, 0
            , 0, 8, 0, 2
            , 0, 0, 0, 0
            , 0, -2, -4, -2
            , 0, 0, 0, 0
            , 0, 0, 0, 0
            , 0, 0, 0, 0
            , 0, 0, 2, 0}
            , {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}}, {
            //mJazzEqBandsValues
            {6, 4, 4, 2
            , 4, 0, -2, 0
            , 0, 2, -2, -6
            , -6, -6, -6, -6
            , -2, -2, -4, -2
            , -2, 0, 0, 0
            , 0, 0, 0, 0
            , 0, 0, 0, 2
            , 2, 4, 2, 2}
            , {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}}, {
            //mCityEqBandsValues
            {-4, -4, -2, 0
            , 2, 4, 4, 2
            , 4, 6, 4, 6
            , 6, 8, 6, 4
            , 2, 2, 2, 4
            , 6, 2, 0, 2
            , 2, 0, 0, 0
            , 0, 2, 0, 2
            , -4, -2, -2, -4}
            , {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}}, {
            //mPopEqBandsValues
            {0, 2, 2, 4
            , 4, 4, 4, 4
            , 4, 8, 6, 4
            , 8, 6, 6, 4
            , 2, -2, -4, -2
            , 0, 0, -2, -4
            , 0, 0, -2, 0
            , -2, -2, -2, 0
            , -2, 0, 2, 2}
            , {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}}, {
            //mElectronicEqBandsValues
            {8, 6, 6, 6
            , 4, 0, -2, 0
            , -2, -6, -4, -6
            , -4, -2, 0, 8
            , 10, 10, 8, 8
            , 8, 10, 8, 10
            , 10, 10, 10, 8
            , 8, 8, 8, 10
            , -8, -8, -10, -12}
            , {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}}, {
            //mClassicEqBandsValues
            {6, 4, 2, 4
            , 2, 2, 0, 2
            , 2, 6, 2, 2
            , 0, 0, 0, 0
            , 0, -2, -4, -2
            , 0, 0, 0, 0
            , 0, 0, 0, 0
            , 0, 0, 0, 0
            , 0, 0, 2, 0}
            , {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}}, {
            //mMovieEqBandsValues
            {6, 2, 4, 6
            , 4, 2, 0, 2
            , 4, 6, 4, 0
            , 2, -6, -4, -4
            , 0, 0, -4, -4
            , -4, -2, -4, 0
            , 2, 0, 2, 0
            , 2, 0, 2, 2
            , 4, 2, 8, 8}
            , {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}}, {
            //mRockEqBandsValues
            {8, 6, 6, 4
            , 4, 4, 4, 6
            , 6, 4, 6, -2
            , 0, -4, -8, -10
            , -10, -6, -8, -6
            , -6, -4, -2, -4
            , -4, 0, -2, -4
            , -2, -2, -4, -2
            , -2, 0, -2, 0}
            , {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}}, {
            //mTechEqBandsValues
            {4, 2, 4, 2
            , 2, 0, 0, 0
            , 0, -2, 0, -2
            , 0, 2, 4, 8
            , 10, 10, 12, 8
            , 8, 10, 8, 8
            , 8, 10, 10, 10
            , 12, 12, 10, 12
            , 4, 2, 8, 8}
            , {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}}};
}
