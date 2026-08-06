package com.hcn.autoeq.bean;

import android.content.Context;

import com.hcn.autoeq.R;

import java.util.Arrays;

public enum FyDspHLPFSlope {

    SLOPE_0(0),
    SLOPE_6(6),
    SLOPE_12(12),
    SLOPE_18(18),
    SLOPE_24(24),
    SLOPE_36(36),
    SLOPE_48(48);

    private int slope;

    FyDspHLPFSlope(int slope) {
        this.slope = slope;
    }

    public int getSlope() {
        return slope;
    }

    public static String format(Context context, FyDspHLPFSlope fyDspHLPFSlope) {
        switch (fyDspHLPFSlope) {
            case SLOPE_0:
                return context.getString(R.string.fydsp_hlpf_slope_0);
            default:
                return String.valueOf(fyDspHLPFSlope.getSlope()) + "dB/oct";
        }
    }

    public static FyDspHLPFSlope findByValue(int value) {
        return Arrays.stream(FyDspHLPFSlope.values()).filter(fyDspHLPFSlope -> fyDspHLPFSlope.getSlope() == value).findFirst().get();
    }

}
