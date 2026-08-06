package com.hcn.autoeq.util;

public interface ConstantEq {
    String EQ_SAVE_DSP = "eq_attribute_dsp"; //频率段保存
    String EQ_SAVE_ASP = "eq_attribute_asp"; //延时保存文件
    String EQ_SAVE_BALANCE = "eq_attribute_balance"; //声场相关保存

    //Balance Data Key.
    String STATUS_BALANCE_X = "status_balance_x";//ASP平衡x
    String STATUS_BALANCE_Y = "status_balance_y";//ASP平衡y
    String STATUS_REVERB_TYPE = "status_reverb_type";
    String STATUS_BASS_BOOST = "status_bass_boost"; //低音
    String STATUS_TREBLE = "status_treble"; //高音
    String STATUS_LOUDNESS = "status_loudness"; //混响
    String STATUS_SURROUND = "status_surround"; //环绕音
    String STATUS_SUBWOOFER = "status_subwoofer"; //混响
    String STATUS_ASP_BAND = "status_asp_band"; //波段
    String STATUS_DSP_BAND = "status_dsp_band"; //波段
    String STATUS_DSP_POWER = "status_dsp_power"; //波段

    //ASP Band Fragment.
    int EQ_REVERB_USER = 0;
    int EQ_REVERB_NEWS = 1;
    int EQ_REVERB_JAZZ = 2;
    int EQ_REVERB_CITY = 3;
    int EQ_REVERB_POP = 4;
    int EQ_REVERB_ELECTRONIC = 5;
    int EQ_REVERB_CLASSIZ = 6;
    int EQ_REVERB_MOVIE = 7;
    int EQ_REVERB_ROCK = 8;
    int EQ_REVERB_TECHNO = 9;
    int EQ_REVERB_SIZE = 10;

    int DEFINE_SEEKBAR_MAX = 14;
}
