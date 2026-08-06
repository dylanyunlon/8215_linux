package com.hcn_library.util;


public interface NineConstantExtDsp extends ConstantExtDsp {
    // 混响模式
    int NINE_DSP_REVERB_STANDARD = 0;
    int NINE_DSP_REVERB_USER2 = 1;
    int NINE_DSP_REVERB_USER1 = 2;
    int NINE_DSP_REVERB_USER3 = 3;
    int NINE_DSP_REVERB_CLASSICAL = 4;
    int NINE_DSP_REVERB_NEWS = 5;
    int NINE_DSP_REVERB_POP = 6;
    int NINE_DSP_REVERB_CITY = 7;
    int NINE_DSP_REVERB_MOVIE = 8;
    int NINE_DSP_REVERB_ELECTRONIC = 9;
    int NINE_DSP_REVERB_ROCK = 10;
    int NINE_DSP_REVERB_TECHNO = 11;
    int NINE_DSP_REVERB_JAZZ = 12;
    int NINE_DSP_REVERB_SIZE = 13;
    int DEF_EQ_REVERB = EXT_DSP_REVERB_USER2;
    int NINE_DSP_REVERB_PREVIEW_START_INDEX = 4;


    int INDEX_LPF_HPF_F = 0x00;    //前左、前右
    int INDEX_LPF_HPF_R = 0x01;        //后左、后右
    int INDEX_LPF_HPF_CEN = 0x02;        //中置
    int INDEX_LPF_HPF_SUB = 0x03;    //重低音
    // 环绕增益
    int NINE_SURROUND_CHANEL_FL = 0x00;
    int NINE_SURROUND_CHANEL_FR = 0x01;
    int NINE_SURROUND_CHANEL_RL = 0x02;
    int NINE_SURROUND_CHANEL_RR = 0x03;

    int NINE_SURROUND_CHANEL_CEN = 0x04;
    int NINE_SURROUND_CHANEL_SUB = 0x05;


    // DTS 指令index
    int NINE_DTS_BASS_FRONT_FREQ = 14;
    int NINE_DTS_BASS_REAR_FREQ = 16;
    int NINE_DTS_BASS_SUB_FREQ = 15;
    int NINE_DTS_BASS_FRONT_LV = 11;
    int NINE_DTS_BASS_REAR_LV = 13;
    int NINE_DTS_BASS_SUB_LV = 12;
    int NINE_DTS_FOCUS_FRONT_LV = 18;
    int NINE_DTS_FOCUS_REAR_LV = 19;
    int NINE_DTS_FOCUS_CENTER_LV = 17;
    int NINE_DTS_PROCESS_MODEL = 1;
    int NINE_DTS_PHANTOM_CENTER_ENABLE = 3;
    int NINE_DTS_FOCUS_FRONT_REAR_LV = 20;
    int NINE_DTS_FOCUS_CENTER_REAR_LV = 21;
    int NINE_DTS_BYPASS = 22;


    // hlpf
    int NINE_CHANNEL_FRONT_HIGH = 1; // 前高
    int NINE_CHANNEL_FRONT_LOW = 2; // 前低
    int NINE_CHANNEL_REAR_HIGH = 3; // 后高
    int NINE_CHANNEL_REAR_LOW = 4; // 后低
    int NINE_CHANNEL_CENTER_HIGH = 5; // 中置低音高
    int NINE_CHANNEL_CENTER_LOW = 6; // 中置低音低
    int NINE_CHANNEL_SUBWOOFER_HIGH = 7; // 重低音高
    int NINE_CHANNEL_SUBWOOFER_LOW = 8; // 重低音低


    // 高低通过滤的默认值
    int NINE_HLPF_FRONT_REAR_FREQ_MIN = 20;
    int NINE_HLPF_FRONT_REAR_FREQ_MAX = 20000;
    int NINE_HLPF_SUBWOOFER_FREQ_MIN = 20;
    int NINE_HLPF_SUBWOOFER_FREQ_MAX = 400;

    int NINE_HLPF_INDEX_LPF_HPF_F_MIN_DEFAULT = 20;
    int NINE_HLPF_INDEX_LPF_HPF_F_MAX_DEFAULT = 20000;
    int NINE_HLPF_INDEX_LPF_HPF_R_MIN_DEFAULT = 20;
    int NINE_HLPF_INDEX_LPF_HPF_R_MAX_DEFAULT = 20000;
    int NINE_HLPF_INDEX_LPF_HPF_C_MIN_DEFAULT = 20;
    int NINE_HLPF_INDEX_LPF_HPF_C_MAX_DEFAULT = 20000;
    int NINE_HLPF_INDEX_LPF_HPF_SUB_MIN_DEFAULT = 20;
    int NINE_HLPF_INDEX_LPF_HPF_SUB_MAX_DEFAULT = 200;
    int NINE_HLPF_INDEX_LPF_HPF_CEN_MIN_DEFAULT = 630;
    int NINE_HLPF_INDEX_LPF_HPF_CEN_MAX_DEFAULT = 6300;

    int HLPF_QVALUE_DEFAULT = 1; // 斜率默认值

    //不是Index
    int NINE_DBB_CHANNEL_FLFR = 1;
    int NINE_DBB_CHANNEL_RLRR = 2;
    int NINE_DBB_CHANNEL_SUBWOOFER = 3;
    int NINE_DBB_CHANNEL_CEN = 4;
    int NINE_DEF_DBB_CHANNEL_FLFR = DBB_CHANNEL_FLFR;

    //ak7739 SubID
    int CMD_SUB_ID_EQ_BAND = 0x01;
    int CMD_SUB_ID_BALANCE = 0x02;
    int CMD_SUB_ID_ATTENUATE_INVERT_SPEAKER = 0x03;
    int CMD_SUB_ID_HPF_LPF = 0x04;
    int CMD_SUB_ID_DBB = 0x05;
    int CMD_SUB_ID_SURROUND = 0x06;
    int CMD_SUB_ID_LOUDNESS_STATUS = 0x07;
    int CMD_SUB_ID_LOUDNESS_BASSFILTER = 0X08;
    int CMD_SUB_ID_LOUDNESS_TREBLEFILTER = 0x09;
    int CMD_SUB_ID_LOUDNESS_BASS_MAXGAIN = 0x0A;
    int CMD_SUB_ID_LOUDNESS_TREBLE_MAXGAIN = 0x0B;
    int CMD_SUB_ID_DELAY = 0x0C;
    int CMD_SUB_ID_CALLERAKM_STATUS = 0x0D;
    int CMD_SUB_ID_CALLERAKM_REVERB_LEVEL = 0x00E;
    int CMD_SUB_ID_CALLERAKM_BASSBOOST = 0X0F;
    int CMD_SUB_ID_CALLERAKM_DYNAMIC_LEVEL = 0x011;
    int CMD_SUB_ID_CALLERAKM_SURROUND_LEVEL = 0X012;
    int CMD_SUB_ID_CALLERAKM_HIFIL_LEVEL = 0X013;
    int CMD_SUB_ID_CALLERAKM_SOUNDBALANCE_LEVEL = 0x016;

    int DEF_QVALUE = ("gb05".equals(EqUtils.getSkinName()) || EqUtils.isChip7739()) ? 2000 : 5000; // q值默认值（6225平台陈贵峰要求）
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

    int[][] DEF_EQ_16_FREQ_VALUES = new int[][]{
            {0, 32, 32},
            {1, 64, 64},
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
            {12, 6000, 6000},
            {13, 8000, 8000},
            {14, 12500, 12500},
            {15, 16000, 16000},
    };

    int DEF_EQ_32_FREQ_VALUES[][] = new int[][]{
            {0, 32, 20},
            {0, 32, 32},
            {1, 64, 40},
            {1, 64, 64},
            {2, 80, 70},
            {2, 80, 80},
            {3, 100, 90},
            {3, 100, 100},
            {4, 125, 110},
            {4, 125, 125},
            {5, 200, 150},
            {5, 200, 200},
            {6, 400, 300},
            {6, 400, 400},
            {7, 600, 500},
            {7, 600, 600},
            {8, 800, 700},
            {8, 800, 800},
            {9, 1000, 900},
            {9, 1000, 1000},
            {10, 2000, 1500},
            {10, 2000, 2000},
            {11, 4000, 3000},
            {11, 4000, 4000},
            {12, 6000, 5000},
            {12, 6000, 6000},
            {13, 8000, 7000},
            {13, 8000, 8000},
            {14, 12500, 10000},
            {14, 12500, 12500},
            {15, 16000, 14000},
            {15, 16000, 16000},
    };

    // 6625平台 gb04皮肤包 si47925dts 芯片 实际使用36段
    int[][] DEF_EQ_36_FREQ_VALUES = new int[][]{
            {0, 20, 20},
            {1, 25, 25},
            {2, 32, 32},
            {3, 40, 40},
            {4, 50, 50},
            {5, 65, 65},
            {6, 80, 80},
            {7, 100, 100},
            {8, 125, 125},
            {9, 160, 160},
            {10, 200, 200},
            {11, 250, 250},
            {12, 315, 315},
            {13, 400, 400},
            {14, 500, 500},
            {15, 630, 630},
            {16, 700, 700},
            {17, 800, 800},
            {18, 900, 900},
            {19, 1000, 1000},
            {20, 1200, 1200},
            {21, 1600, 1600},
            {22, 2000, 2000},
            {23, 2500, 2500},
            {24, 3200, 3200},
            {25, 4000, 4000},
            {26, 5000, 5000},
            {27, 6300, 6300},
            {28, 7000, 7000},
            {29, 8000, 8000},
            {30, 9000, 9000},
            {31, 10000, 10000},
            {32, 12500, 12500},
            {33, 16000, 16000},
            {34, 18000, 18000},
            {35, 20000, 20000},
    };
    int DEF_EQ_48_FREQ_VALUES[][] = new int[][]{
            {0, 32, 16},
            {0, 32, 32},
            {0, 32, 40},
            {1, 64, 50},
            {1, 64, 64},
            {1, 64, 70},
            {2, 80, 75},
            {2, 80, 80},
            {2, 80, 90},
            {3, 100, 95},
            {3, 100, 100},
            {3, 100, 110},
            {4, 125, 120},
            {4, 125, 125},
            {4, 125, 150},
            {5, 200, 175},
            {5, 200, 200},
            {5, 200, 250},
            {6, 400, 300},
            {6, 400, 400},
            {6, 400, 450},
            {7, 600, 500},
            {7, 600, 600},
            {7, 600, 650},
            {8, 800, 700},
            {8, 800, 800},
            {8, 800, 850},
            {9, 1000, 900},
            {9, 1000, 1000},
            {9, 1000, 1250},
            {10, 2000, 1500},
            {10, 2000, 2000},
            {10, 2000, 2500},
            {11, 4000, 3000},
            {11, 4000, 4000},
            {11, 4000, 4500},
            {12, 6000, 5000},
            {12, 6000, 6000},
            {12, 6000, 6500},
            {13, 8000, 7000},
            {13, 8000, 8000},
            {13, 8000, 9000},
            {14, 12500, 10000},
            {14, 12500, 12500},
            {14, 12500, 13500},
            {15, 16000, 14000},
            {15, 16000, 15000},
            {15, 16000, 16000},
    };

    //Used to DSP 14 bands
    int DEF_EQ_14_BANDS_VALUES[][][] = new int[][][]{
            {
                    //mClassicEqBandsValues
                    {6, 6, 8, 8, 4, 4, 2, 0, 2, 6, 2, -2, -2, -2},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mNewsEqBandsValues
                    {0, 6, 4, -2, 8, 8, 2, 0, 0, 6, 0, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mPopEqBandsValues
                    {0, 2, 4, 4, 8, 8, 0, 0, 0, 4, 0, 8, 6, 8},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mCityEqBandsValues
                    {0, 6, 6, 2, 6, 8, 0, 2, 0, 0, 2, 10, 0, 8},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mMovieEqBandsValues
                    {10, 10, 6, 6, 0, -6, -4, -4, 0, 0, -4, -4, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mElectronicEqBandsValues
                    {10, 8, 4, -2, -6, -2, -2, -2, -2, 8, 10, 10, 8, 4},
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
            },
            {
                    //mJazzEqBandsValues
                    {0, 6, 10, 4, 8, 4, 0, 0, 0, 2, 2, 10, 10, 10},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            }
    };

    //Used to DSP 14 bands
    int DEF_EQ_16_BANDS_VALUES[][][] = new int[][][]{
            {
                    //mClassicEqBandsValues
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -6, -6, -5, -5, -5, -8},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mNewsEqBandsValues
                    {-2, -5, -6, -4, -3, 0, 1, 6, 6, 2, 2, -3, -4, -5, -5, -5},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mPopEqBandsValues
                    {0, 0, 0, -1, -2, -4, -5, -5, -5, -5, -5, -3, -1, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mCityEqBandsValues
                    {3, 4, 6, 6, 0, -7, 0, 4, 4, 0, -8, 0, 7, 6, 2, 2},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mMovieEqBandsValues
                    {6, 4, 4, 4, 6, 6, 3, 3, 3, -2, -3, -3, -3, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mElectronicEqBandsValues
                    {7, 5, 2, 1, -1, -6, -5, -2, -1, 0, 5, 8, 6, 6, 7, 7},
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
            },
            {
                    //mJazzEqBandsValues
                    {0, 0, 0, 0, 0, 7, 7, 4, 3, 3, -2, 2, 4, 5, 5, 5},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            }
    };
    //Used to DSP 32 bands
    int DEF_EQ_32_BANDS_VALUES[][][] = new int[][][]{
            {
                    //mClassicEqBandsValues
                    {0, 0, 0, 0, 0, 0, 0, 0
                            , 0, 0, 0, 0, 0, 0, 0, 0
                            , 0, 0, 0, 0, -6, -6, -6, -6
                            , -5, -5, -5, -5, -5, -5, -8, -8},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}

            },
            {
                    //mNewsEqBandsValues
                    {-2, -2, -5, -5,
                            -6, -6, -4, -4,
                            -3, -3, 0, 0,
                            1, 1, 6, 6,
                            6, 6, 2, 2,
                            2, 2, -3, -3,
                            -4, -4, -5, -5,
                            -5, -5, -5, -5},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mPopEqBandsValues
                    {0, 0, 0, 0, 0, 0, -1, -1
                            , -2, -2, -4, -4, -5, -5, -5, -5
                            , -5, -5, -5, -5, -5, -5, -3, -3
                            , -1, -1, 0, 0, 0, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mCityEqBandsValues
                    {3, 3, 4, 4, 6, 6, 6, 6
                            , 0, 0, -7, -7, 0, 0, 4, 4
                            , 4, 4, 0, 0, -8, -8, 0, 0
                            , 7, 7, 6, 6, 2, 2, 2, 2},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mMovieEqBandsValues
                    {6, 6, 4, 4, 4, 4, 4, 4
                            , 6, 6, 6, 6, 3, 3, 3, 3
                            , 3, 3, -2, -2, -3, -3, -3, -3
                            , -3, -3, 0, 0, 0, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mElectronicEqBandsValues
                    {7, 7, 5, 5, 2, 2, 1, 1
                            , -1, -1, -6, -6, -5, -5, -2, -2
                            , -1, -1, 0, 0, 5, 5, 8, 8
                            , 6, 6, 6, 6, 7, 7, 7, 7},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}

            },
            {
                    //mRockEqBandsValues
                    {8, 8, 5, 5, 2, 2, -2, -2
                            , -4, -4, 7, 7, -2, -2, -2, -2
                            , -2, -2, 1, 1, 2, 2, 6, 6
                            , 6, 6, 7, 7, 9, 9, 9, 9},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mTechEqBandsValues
                    {-2, -2, 3, 3, 4, 4, 6, 6
                            , 6, 6, 2, 2, -4, -4, -4, -4
                            , -1, -1, -1, -1, 5, 5, 8, 8
                            , 8, 8, 8, 8, 10, 10, 10, 10},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mJazzEqBandsValues
                    {0, 0, 0, 0, 0, 0, 0, 0
                            , 0, 0, 7, 7, 7, 7, 4, 4
                            , 3, 3, 3, 3, -2, -2, 2, 2
                            , 4, 4, 5, 5, 5, 5, 5, 5},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE
                            , DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            }
    };

    // 需求，复用 ak7604c 32段频，古典 新闻 流行三个音效测试效果， q值为5， 频点采用6225 新制定的36段频点，多出来的四段用0填充。
    int DEF_EQ_36_BANDS_VALUES[][][] = new int[][][]{
            {
                    //classical
                    {0, 0, 0, 0
                            , 0, 0, 0, 0
                            , 0, 0, 0, 0
                            , 0, 0, 0, 0
                            , 0, 0, 0, 0
                            , -6, -6, -6, -6
                            , -5, -5, -5, -5
                            , -5, -5, -8, -8
                            , 0, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //news
                    {-2, -2, -5, -5,
                            -6, -6, -4, -4,
                            -3, -3, 0, 0,
                            1, 1, 6, 6,
                            6, 6, 2, 2,
                            2, 2, -3, -3,
                            -4, -4, -5, -5,
                            -5, -5, -5, -5
                            , 0, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //pop
                    {0, 0, 0, 0
                            , 0, 0, -1, -1
                            , -2, -2, -4, -4
                            , -5, -5, -5, -5
                            , -5, -5, -5, -5
                            , -5, -5, -3, -3
                            , -1, -1, 0, 0
                            , 0, 0, 0, 0
                            , 0, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //city
                    {2, 2, 2, 2
                            , 4, 4, 4, 4
                            , 3, 0, -3, -4
                            , -4, -2, -1, 0
                            , 2, 4, 4, 4
                            , 2, 0, -2, -8
                            , -2, 0, 0, 0
                            , 0, 0, 3, 3
                            , 3, 3, 3, 3},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //cinema
                    {-2, -2, 5, 5
                            , 4, 4, 4, 4
                            , 4, 3, 3, 3
                            , 3, 3, 3, 3
                            , 3, 3, 3, 4
                            , 2, -1, -2, -2
                            , -2, -3, -3, -3
                            , -2, -1, 0, 0
                            , 0, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //electric
                    {5, 5, 5, 5
                            , 5, 5, 5, 5
                            , 5, 4, -1, -2
                            , -4, -4, -4, -4
                            , -2, -1, 0, 0
                            , 0, 0, 4, 4
                            , 3, 3, 3, 3
                            , 4, 4, 4, 4
                            , 5, 5, 5, 5},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //rock
                    {6, 6, 5, 5
                            , 5, 5, 5, 5
                            , 2, -5, -4, 6
                            , 6, 2, 1, -2
                            , -2, -3, -3, 0
                            , 0, 0, 0, 2
                            , 2, 2, 2, 2
                            , 3, 3, 3, 6
                            , 6, 6, 6, 6}, {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //technology
                    {-4, -4, -3, 2
                            , 1, 2, 5, 7
                            , 7, 5, 4, 3
                            , 2, 2, 0, -3
                            , -4, -4, -2, 1
                            , 1, 2, 2, 2
                            , 2, 3, 4, 3
                            , 3, 2, 4, 4
                            , 6, 6, 6, 6},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //jazz
                    {0, 0, 0, 0
                            , 0, 0, 0, 0
                            , 0, 0, 0, 5
                            , 6, 4, 4, 4
                            , 5, 4, 3, 2
                            , 2, -2, -2, -2
                            , 1, 1, 1, 2
                            , 2, 3, 3, 5
                            , 5, 5, 5, 5},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}}};
    //Used to DSP 48 bands
    int DEF_EQ_48_BANDS_VALUES[][][] = new int[][][]{
            {
                    //mClassicEqBandsValues
                    {0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0
                            , -6, -6, -6
                            , -6, -6, -6
                            , -5, -5, -5
                            , -5, -5, -5
                            , -5, -5, -5
                            , -8, -8, -8},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mNewsEqBandsValues
                    {-2, -2, -2
                            , -5, -5, -5
                            , -6, -6, -6
                            , -4, -4, -4
                            , -3, -3, -3
                            , 0, 0, 0
                            , 1, 1, 1
                            , 6, 6, 6
                            , 6, 6, 6
                            , 2, 2, 2
                            , 2, 2, 2
                            , -3, -3, -3
                            , -4, -4, -4
                            , -5, -5, -5
                            , -5, -5, -5
                            , -5, -5, -5},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}

            },
            {
                    //mPopEqBandsValues
                    {0, 0, 0
                            , 0, 0, 0
                            , 0, 0, -1
                            , -1, -1, -1
                            , -2, -2, -2
                            , -4, -4, -4
                            , -5, -5, -5
                            , -5, -5, -5
                            , -5, -5, -5
                            , -5, -5, -5
                            , -5, -5, -5
                            , -3, -3, -3
                            , -1, -1, -1
                            , 0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mCityEqBandsValues
                    {3, 3, 3
                            , 4, 4, 4
                            , 6, 6, 6
                            , 6, 6, 6
                            , 0, 0, 0
                            , -7, -7, -7
                            , 0, 0, 0
                            , 4, 4, 4
                            , 4, 4, 4
                            , 0, 0, 0
                            , -8, -8, -8
                            , 0, 0, 0
                            , 7, 7, 7
                            , 6, 6, 6
                            , 2, 2, 2
                            , 2, 2, 2},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mMovieEqBandsValues
                    {6, 6, 6
                            , 4, 4, 4
                            , 4, 4, 4
                            , 4, 4, 4
                            , 6, 6, 6
                            , 6, 6, 6
                            , 3, 3, 3
                            , 3, 3, 3
                            , 3, 3, 3
                            , -2, -2, -2
                            , -3, -3, -3
                            , -3, -3, -3
                            , -3, -3, -3
                            , 0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mElectronicEqBandsValues
                    {7, 7, 7
                            , 5, 5, 5
                            , 2, 2, 2
                            , 1, 1, 1
                            , -1, -1, -1
                            , -6, -6, -6
                            , -5, -5, -5
                            , -2, -2, -2
                            , -1, -1, -1
                            , 0, 0, 0
                            , 5, 5, 5
                            , 8, 8, 8
                            , 6, 6, 6
                            , 6, 6, 6
                            , 7, 7, 7
                            , 7, 7, 7},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}

            },
            {
                    //mRockEqBandsValues
                    {8, 8, 8
                            , 5, 5, 5
                            , 2, 2, 2
                            , -2, -2, -2
                            , -4, -4, -4
                            , 7, 7, 7
                            , -2, -2, -2
                            , -2, -2, -2
                            , -2, -2, -2
                            , 1, 1, 1
                            , 2, 2, 2
                            , 6, 6, 6
                            , 6, 6, 6
                            , 7, 7, 7
                            , 9, 9, 9
                            , 9, 9, 9},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mTechEqBandsValues
                    {-2, -2, -2
                            , 3, 3, 3
                            , 4, 4, 4
                            , 6, 6, 6
                            , 6, 6, 6
                            , 2, 2, 2
                            , -4, -4, -4
                            , -4, -4, -4
                            , -1, -1, -1
                            , -1, -1, -1
                            , 5, 5, 5
                            , 8, 8, 8
                            , 8, 8, 8
                            , 8, 8, 8
                            , 10, 10, 10
                            , 10, 10, 10},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //mJazzEqBandsValues
                    {0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0
                            , 0, 0, 0
                            , 7, 7, 7
                            , 7, 7, 7
                            , 4, 4, 4
                            , 3, 3, 3
                            , 3, 3, 3
                            , -2, -2, -2
                            , 2, 2, 2
                            , 4, 4, 4
                            , 5, 5, 5
                            , 5, 5, 5
                            , 5, 5, 5},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            }
    };

    int DEF_EQ_36_BANDS_VALUES_7739[][][] = new int[][][]{
            {
                    //classical
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -3, -3, -3, -3, -1, -1, -1, -1, -1, -1, -1, -3, -3},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //news
                    {1, 1, 1, -1, -2, -4, -5, -3, -2, 0, 0, 0, 1, 1, 3, 3, 4, 3, 1, 1, 1, 2, 2, 2, -1, -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}
            },
            {
                    //pop
                    {1, 1, 1, 1, 1, 1, 1, 1, -1, -2, -2, -2, -2, -2, -2, -2, -2, -3, -3, -2, -2, -2, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //city
                    {2, 2, 2, 2, 4, 5, 5, 6, 3, -3, -4, -3, 1, 1, 3, 3, 3, 3, 0, 0, 0, -3, -5, 0, 1, 1, 4, 2, 2, 2, 1, 1, 1, 1, 1, 1},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //cinema
                    {3, 3, 3, 2, 2, 3, 5, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //electric
                    {4, 4, 4, 4, 4, 4, 2, 2, 2, -1, -2, -2, -2, -2, -2, -1, -1, -1, 0, 0, 0, 0, 2, 3, 3, 5, 3, 3, 2, 2, 2, 2, 2, 3, 3, 3},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //rock
                    {3, 3, 3, 3, 4, 4, 3, -2, -4, 2, 6, 2, -1, 0, -1, -1, -1, 0, 0, 0, 0, 1, 2, 2, 2, 3, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //technology
                    {0, 0, 0, -1, 2, 3, 4, 7, 5, 2, 2, 2, 2, -3, -3, -1, -1, -1, 0, 0, 0, 0, 3, 3, 3, 4, 4, 3, 3, 3, 4, 4, 4, 4, 4, 4},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}},
            {
                    //jazz
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, -1, -2, 0, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2},
                    {DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE,
                            DEF_QVALUE, DEF_QVALUE, DEF_QVALUE, DEF_QVALUE}}};


}