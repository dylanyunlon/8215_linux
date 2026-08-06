package com.hcn_library.bean;

import android.content.Context;


import com.hcn_library.hcn_library.R;

import java.util.stream.IntStream;

public enum FyDspLoudness {

    LOUDNESS_0(0),
    LOUDNESS_1(4),
    LOUDNESS_2(6),
    LOUDNESS_3(8),
    LOUDNESS_4(10);

    private int loudness;

    FyDspLoudness(int loudness) {
        this.loudness = loudness;
    }

    public int getLoudness() {
        return loudness;
    }

    public static String format(Context context, FyDspLoudness fyDspLoudness) {
        switch (fyDspLoudness) {
            case LOUDNESS_0:
                return context.getString(com.hcn_library.hcn_library.R.string.app_sound_off);
            default:
                int loudnessIndex = IntStream.range(0, FyDspLoudness.values().length).filter(i -> fyDspLoudness == FyDspLoudness.values()[i]).findFirst().orElse(0);
                return context.getString(R.string.fydsp_bassboost_loudness_level, loudnessIndex);
        }
    }

}
