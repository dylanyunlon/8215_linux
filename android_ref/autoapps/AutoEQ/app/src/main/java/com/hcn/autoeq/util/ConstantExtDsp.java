package com.hcn.autoeq.util;

public interface ConstantExtDsp {

    // band
    int EXT_DSP_BAND_GAIN_MAX = 24; // +-12的时候，全部拉到最高时测量12db；+-14的时候，测量是14db

    int EXT_DSP_REVERB_NEWS = 0;
    int EXT_DSP_REVERB_JAZZ = 1;
    int EXT_DSP_REVERB_CITY = 2;
    int EXT_DSP_REVERB_POP = 3;
    int EXT_DSP_REVERB_ELECTRONIC = 4;
    int EXT_DSP_REVERB_CLASSIZ = 5;
    int EXT_DSP_REVERB_MOVIE = 6;
    int EXT_DSP_REVERB_ROCK = 7;
    int EXT_DSP_REVERB_TECHNO = 8;
    int EXT_DSP_REVERB_SIZE = 9;
    int EXT_DSP_REVERB_USER0 = 10;
    int EXT_DSP_REVERB_USER1 = 11;
    int EXT_DSP_REVERB_USER2 = 12;

    int DEF_EQ_REVERB = EXT_DSP_REVERB_USER0;
    int DEF_QVALUE = 2000; // q值默认值（8257平台陈贵峰要求）

    // attenuate
    int DEF_ATTENUATE = 0; // balance 衰减的默认值

    int DEF_SURROUND_ENABLE_STATUS = 0; // surround 默认关闭
    // loudness 默认开启（8257平台陈贵峰要求），8581平台默认关闭（2023-12-4 邮件需求）
    int DEF_LOUDNESS_ENABLE_STATUS = EqUtils.getDefaultLoudness();

    // hlpf
    int CHANNEL_FRONT_HIGH = 1; // 前高
    int CHANNEL_FRONT_LOW = 2; // 前低
    int CHANNEL_REAR_HIGH = 3; // 后高
    int CHANNEL_REAR_LOW = 4; // 后低
    int CHANNEL_SUBWOOFER_HIGH = 5; // 重低音高
    int CHANNEL_SUBWOOFER_LOW = 6; // 重低音低

    // dbb
    int DBB_FREQ_MAX = 400;
    int DBB_FREQ_MIN = 20;
    int DBB_GAIN_MAX = 15;
    int DBB_GAIN_MIN = 0;

    // 此值不是界面所需的索引，是对应到 native 接口
    int DBB_CHANNEL_FLFR = 1;
    int DBB_CHANNEL_RLRR = 2;
    int DBB_CHANNEL_SUBWOOFER = 3;
    int DEF_DBB_CHANNEL_FLFR = DBB_CHANNEL_FLFR;

    // 高低通过滤的默认值
    int HLPF_FRONT_REAR_FREQ_MIN = 20;
    int HLPF_FRONT_REAR_FREQ_MAX = 20000;
    int HLPF_SUBWOOFER_FREQ_MIN = 20;
    int HLPF_SUBWOOFER_FREQ_MAX = 400;

    int HLPF_FRONT_REAR_FREQ_MIN_DEFAULT = 20;
    int HLPF_FRONT_REAR_FREQ_MAX_DEFAULT = 20000;
    int HLPF_SUBWOOFER_FREQ_MIN_DEFAULT = 20;
    int HLPF_SUBWOOFER_FREQ_MAX_DEFAULT = 200;

    int HLPF_QVALUE_DEFAULT = 700; // 斜率默认值

    // 注意：底层支持16段，目前界面只做14段
    // 0：native 所需的 band
    // 1：native 所需的 freq
    // 2：线性控件所需
    int[][] DEF_EQ_14_FREQ_VALUES = new int[][]{
            {0, 30, 32},
            {1, 60, 64},
            {2, 80, 80},
            {3, 100, 100},
            {4, 125, 125},
            {5, 200, 200},
            {6, 400, 400},
            {7, 600, 600},
            {8, 800, 800},
            {9, 1000, 1000},
            {10, 2000, 2000},
            {11, 4000, 4000},
            {12, 8000, 8000},
            {13, 12500, 16000}
    };

    int DEF_EQ_32_FREQ_VALUES[][] = new int[][]{
            {0, 30, 20},
            {0, 30, 25},
            {0, 30, 30},
            {0, 30, 40},
            {1, 60, 50},
            {1, 60, 60},
            {2, 80, 80},
            {3, 100, 100},
            {4, 125, 125},
            {5, 200, 175},
            {5, 200, 200},
            {5, 200, 250},
            {6, 400, 300},
            {6, 400, 400},
            {7, 600, 500},
            {7, 600, 700},
            {8, 800, 800},
            {9, 1000, 1000},
            {9, 1000, 1200},
            {10, 1600, 1500},
            {10, 1600, 2000},
            {11, 2500, 2400},
            {11, 2500, 3000},
            {12, 4000, 4000},
            {12, 4000, 5000},
            {13, 6300, 6000},
            {14, 8000, 8000},
            {14, 8000, 10000},
            {15, 12500, 12000},
            {15, 12500, 14000},
            {15, 12500, 16000},
            {15, 12500, 18000}
    };
    int DEF_EQ_48_FREQ_VALUES[][] = new int[][]{
            {0, 30, 16},
            {0, 30, 20},
            {0, 30, 25},
            {0, 30, 30},
            {0, 30, 35},
            {0, 30, 40},
            {1, 60, 50},
            {1, 60, 60},
            {1, 60, 70},
            {2, 80, 80},
            {3, 100, 95},
            {3, 100, 100},
            {4, 125, 125},
            {4, 125, 150},
            {5, 200, 175},
            {5, 200, 200},
            {5, 200, 235},
            {5, 200, 250},
            {6, 400, 300},
            {6, 400, 375},
            {6, 400, 400},
            {7, 600, 500},
            {7, 600, 600},
            {7, 600, 700},
            {8, 800, 800},
            {8, 800, 920},
            {9, 1000, 1100},
            {9, 1000, 1200},
            {9, 1000, 1300},
            {10, 1600, 1700},
            {10, 1600, 2000},
            {11, 2500, 2400},
            {11, 2500, 2800},
            {12, 4000, 3200},
            {12, 4000, 3800},
            {12, 4000, 4400},
            {12, 4000, 5000},
            {13, 6300, 6000},
            {13, 6300, 7000},
            {14, 8000, 8000},
            {14, 8000, 9500},
            {14, 8000, 11000},
            {15, 12500, 12500},
            {15, 12500, 14000},
            {15, 12500, 15000},
            {15, 12500, 16000},
            {15, 12500, 17000},
            {15, 12500, 18000}
    };

    //Used to DSP 14 bands
    int DEF_EQ_14_BANDS_VALUES[][][] = new int[][][]{
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

    //Used to DSP 32 bands
    int DEF_EQ_32_BANDS_VALUES[][][] = new int[][][]{
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
    int DEF_EQ_48_BANDS_VALUES[][][] = new int[][][]{
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

}
