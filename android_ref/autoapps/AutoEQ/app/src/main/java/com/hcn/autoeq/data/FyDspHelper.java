package com.hcn.autoeq.data;

import android.content.Context;

import com.hcn.autoeq.bean.Band;
import com.hcn.autoeq.bean.FyDspBandMode;
import com.hcn.autoeq.bean.FyDspOutputMode;

import java.util.List;

public class FyDspHelper {

    public static void nativeAllData(Context context) {
        FyDspHLPFSettings fyDspHLPFSettings = FyDspHLPFSettings.getInstance(context);
        FyDspOutputMode fyDspOutputMode = fyDspHLPFSettings.getOutputMode();

        FyDspBandSettings fyDspBandSettings = FyDspBandSettings.getInstance(context);
        FyDspBandMode fyDspBandMode = fyDspBandSettings.getBandMode();
        List<Band> bandList = fyDspBandSettings.getBands(fyDspBandMode);
        fyDspBandSettings.nativeBands(bandList);

        FyDspBalanceSettings fyDspBalanceSettings = FyDspBalanceSettings.getInstance(context);
        int[] balance = fyDspBalanceSettings.getBalance();
        fyDspBalanceSettings.nativeBalance(balance[0], balance[1], fyDspOutputMode);

        FyDspDelaySettings fyDspDelaySettings = FyDspDelaySettings.getInstance(context);
        fyDspDelaySettings.nativeAll();

        FyDspSurroundSettings fyDspSurroundSettings = FyDspSurroundSettings.getInstance(context);
        fyDspSurroundSettings.nativeLoudness(fyDspSurroundSettings.getLoudness());
        fyDspSurroundSettings.nativeBassBoost(fyDspSurroundSettings.getBassBoostFreqCh12(), fyDspSurroundSettings.getBassBoostGainCh12()
                , fyDspSurroundSettings.getBassBoostFreqCh34(), fyDspSurroundSettings.getBassBoostGainCh34());

        FyDspAttenuateSettings fyDspAttenuateSettings = FyDspAttenuateSettings.getInstance(context);
        fyDspAttenuateSettings.nativeAll("LF", "RF", "LR", "RR", "CENTER", "SUBWOOFER");

        fyDspHLPFSettings.nativeHLPFAllChannel(fyDspOutputMode);
    }
}
