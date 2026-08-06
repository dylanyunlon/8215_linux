package com.hcn.autoeq.bean;

import android.content.Context;

import com.hcn.autoeq.R;

import java.util.List;

public enum FyDspBandMode {
    // 注意：此名字对应 fydsp_fragment_band_mode.xml 中的 tag
    CUSTOM, STANDARD, NEWS, JAZZ, CITY, POP, ELECTRONIC, CLASSICS, MOVIE, ROCK, TECHNO;

    public static String format(Context context, FyDspBandMode fyDspBandMode) {
        switch (fyDspBandMode) {
            case CUSTOM:
                return context.getString(R.string.reverb_custom);
            case STANDARD:
                return context.getString(R.string.reverb_standard);
            case NEWS:
                return context.getString(R.string.reverb_news);
            case JAZZ:
                return context.getString(R.string.reverb_jazz);
            case CITY:
                return context.getString(R.string.reverb_city);
            case POP:
                return context.getString(R.string.reverb_pop);
            case ELECTRONIC:
                return context.getString(R.string.reverb_electronic);
            case CLASSICS:
                return context.getString(R.string.reverb_classiz);
            case MOVIE:
                return context.getString(R.string.reverb_movie);
            case ROCK:
                return context.getString(R.string.reverb_rock);
            case TECHNO:
                return context.getString(R.string.reverb_techno);
        }
        return "";
    }

    public static List<Band> getBands(FyDspBandMode fyDspBandMode) {
        switch (fyDspBandMode) {
            case CUSTOM:
                return BandData.initCustomBands();
            case STANDARD:
                return BandData.initStandardBands();
            case NEWS:
                return BandData.initNewsBands();
            case JAZZ:
                return BandData.initJazzBands();
            case CITY:
                return BandData.initCityBands();
            case POP:
                return BandData.initPopBands();
            case ELECTRONIC:
                return BandData.initElectronicBands();
            case CLASSICS:
                return BandData.initClassicsBands();
            case MOVIE:
                return BandData.initMovieBands();
            case ROCK:
                return BandData.initRockBands();
            case TECHNO:
                return BandData.initTechnoBands();
            default:
                return BandData.initCustomBands();
        }
    }
}
