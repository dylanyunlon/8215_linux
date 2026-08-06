package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.hcn.autoeq.bean.Band;
import com.hcn.autoeq.bean.BandData;
import com.hcn.autoeq.bean.FyDspBandMode;
import com.hcn.autoeq.nativeextdsp.FY7604;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantFyDsp;

import java.util.ArrayList;
import java.util.List;

public class FyDspBandSettings extends FyDspBaseSettings implements ConstantFyDsp {

    private static final String TAG = FyDspBandSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(FyDspBandSettings.class.getSimpleName(), Log.DEBUG);

    private static final String FY_DSP_BAND_FILE = "v2_fy_dsp_band"; // 各模式的音效保存的文件名

    private Context context;
    private static FyDspBandSettings extDspBandSettings = null;

    public static FyDspBandSettings getInstance(Context context) {
        if (null == extDspBandSettings) {
            extDspBandSettings = new FyDspBandSettings(context);
        }
        return extDspBandSettings;
    }

    private FyDspBandSettings(Context context) {
        super(FY_DSP_BAND_FILE);
        this.context = context;
    }

    public void saveBandMode(FyDspBandMode fyDspBandMode) {
        Log.d(TAG, "saveBandMode band mode : " + fyDspBandMode.name());
        spUtils.put("key_band_mode", fyDspBandMode.name());
    }

    // 此数据不需要从 USER 模式的 sp 中获取
    public FyDspBandMode getBandMode() {
        return FyDspBandMode.valueOf(spUtils.getString("key_band_mode", DEF_BAND_MODE.name()));
    }

    public void saveBandQ(Band band) {
        Log.d(TAG, "saveBandQ band : " + band);
        int index = band.getIndex();
        spUtils.put("key_band_q_" + index, band.getQ());
    }

    public void resetCustomBands() {
        List<Band> bandList = BandData.initCustomBands();
        for (int i = 0; i < bandList.size(); i++) {
            Band band = bandList.get(i);
//            Log.d(TAG, "resetCustomBands band : " + band);
            getSpUtils().put("key_band_index_" + i, band.getIndex());
            getSpUtils().put("key_band_gain_" + i, band.getGain());
            getSpUtils().put("key_band_q_" + i, band.getQ());
            getSpUtils().put("key_band_freq_" + i, band.getFreq());
            getSpUtils().put("key_band_type_" + i, band.getType());
            getSpUtils().put("key_band_bypass_" + i, band.getBypass());
        }
    }

    public void saveBands(List<Band> bandList) {
        for (int i = 0; i < bandList.size(); i++) {
            Band band = bandList.get(i);
//            Log.d(TAG, "saveBands band : " + band);
            spUtils.put("key_band_index_" + i, band.getIndex());
            spUtils.put("key_band_gain_" + i, band.getGain());
            spUtils.put("key_band_q_" + i, band.getQ());
            spUtils.put("key_band_freq_" + i, band.getFreq());
            spUtils.put("key_band_type_" + i, band.getType());
            spUtils.put("key_band_bypass_" + i, band.getBypass());
        }
    }

    public List<Band> getBands(FyDspBandMode fyDspBandMode) {
        List<Band> bandList = new ArrayList<>();

        if (fyDspBandMode == FyDspBandMode.CUSTOM) { // 用户模式的值，从sp文件中获取
            final List<Band> customDefaultBandList = BandData.initCustomBands();
            final int size = customDefaultBandList.size();

            for (int i = 0; i < size; i++) {
                Band userDefaultBand = customDefaultBandList.get(i);

                Band band = new Band();
                bandList.add(band);
                band.setIndex(getSpUtils().getInt("key_band_index_" + i, userDefaultBand.getIndex()));
                band.setGain(getSpUtils().getInt("key_band_gain_" + i, userDefaultBand.getGain()));
                band.setQ(getSpUtils().getInt("key_band_q_" + i, userDefaultBand.getQ()));
                band.setFreq(getSpUtils().getInt("key_band_freq_" + i, userDefaultBand.getFreq()));
                band.setType(getSpUtils().getInt("key_band_type_" + i, userDefaultBand.getType()));
                band.setBypass(getSpUtils().getInt("key_band_bypass_" + i, userDefaultBand.getBypass()));
            }
        } else {
            bandList = FyDspBandMode.getBands(fyDspBandMode);
        }
        return bandList;
    }

    public void nativeBand(Band band) {
        Log.d(TAG, "nativeBand band : " + band);
        int[] data = {FY7604.FY_CMD_SUB_ID_SET_PEQ_SINGLE
                , band.getIndex(), band.getGain() * 10, band.getQ(), band.getFreq(), band.getType(), band.getBypass()};
        NativeHelper.getEq().setEqBand(data);
    }

    public void nativeBands(List<Band> bands) {
        Log.d(TAG, "nativeBands bands : " + bands.toString());
        List<Integer> data = new ArrayList<>();
        data.add(FY7604.FY_CMD_SUB_ID_SET_PEQ_32ALL);
        bands.forEach(b -> {
            data.add(b.getGain() * 10);
            data.add(b.getQ());
            data.add(b.getFreq());
            data.add(b.getType());
            data.add(b.getBypass());
        });
        NativeHelper.getEq().setEqBands(data.stream().mapToInt(Integer::intValue).toArray());
    }

    public void nativeBandQ(Band band) {
        Log.d(TAG, "nativeBandQ band : " + band);
        int[] data = {FY7604.FY_CMD_SUB_ID_SET_PEQ_SINGLE
                , band.getIndex(), band.getGain() * 10, band.getQ(), band.getFreq(), band.getType(), band.getBypass()};
        NativeHelper.getEq().setEqBand(data);
    }
}
