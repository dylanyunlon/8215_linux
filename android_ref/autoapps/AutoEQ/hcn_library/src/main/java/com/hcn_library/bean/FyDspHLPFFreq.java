package com.hcn_library.bean;

import com.hcn_library.util.SystemUtils;

import java.util.Arrays;
import java.util.Locale;

public enum FyDspHLPFFreq {
    FREQ_20(20),
    FREQ_25(25),
    FREQ_32(32),
    FREQ_40(40),
    FREQ_50(50),
    FREQ_63(63),
    FREQ_80(80),
    FREQ_100(100),
    FREQ_125(125),
    FREQ_160(160),
    FREQ_200(200),
    FREQ_250(250),
    FREQ_320(320),
    FREQ_400(400),
    FREQ_500(500),
    FREQ_630(630),
    FREQ_800(800),
    FREQ_1000(1000),
    FREQ_1250(1250),
    FREQ_1600(1600),
    FREQ_2000(2000),
    FREQ_2500(2500),
    FREQ_3200(3200),
    FREQ_4000(4000),
    FREQ_5000(5000),
    FREQ_6300(6300),
    FREQ_8000(8000),
    FREQ_10000(10000),
    FREQ_12500(12500),
    FREQ_16000(16000),
    FREQ_20000(20000);

    private int freq;

    FyDspHLPFFreq(int freq) {
        this.freq = freq;
    }

    public int getFreq() {
        return freq;
    }

    public String getText() {
        if (freq >= 1000) {
            return SystemUtils.subZeroAndDot(String.format(Locale.getDefault(), "%.2f", freq / 1000f)) + " KHz";
        }
        return String.format(Locale.getDefault(), "%d Hz", freq);
    }

    public static FyDspHLPFFreq findByValue(int value) {
        return Arrays.stream(FyDspHLPFFreq.values()).filter(fyDspHLPFFreq -> fyDspHLPFFreq.getFreq() == value).findFirst().get();
    }

}
