package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantExtDsp;
import com.hcn.autoeq.util.ECDConstantExtDsp;
import com.hcn.autoeq.util.EqUtils;

import java.util.Arrays;

public class ExtDspBandSettings implements ConstantExtDsp {

    private static final String TAG = ExtDspBandSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(ExtDspBandSettings.class.getSimpleName(), Log.DEBUG);

    private static final String EXT_DSP_BAND_FILE = "ext_dsp_band"; // 各模式的音效保存的文件名
    private static final String KEY_REVERB_TYPE = "reverb_type"; // 音效模式的 key
    private static final String KEY_USER_BAND[] = {"user0_band", "user1_band", "user2_band"}; // 音效频率的 key
    private static final String KEY_USER_QVALUE[] = {"user0_qvalue", "user1_qvalue", "user2_qvalue"}; // 音效 q 值的 key

    private Context context;
    private static ExtDspBandSettings extDspBandSettings = null;
    private SPUtils spUtils;

    public static ExtDspBandSettings getInstance(Context mContext) {
        if (null == extDspBandSettings) {
            extDspBandSettings = new ExtDspBandSettings(mContext);
        }
        return extDspBandSettings;
    }

    private ExtDspBandSettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(EXT_DSP_BAND_FILE);
    }

    public void saveReverb(int reverb) {
        spUtils.put(KEY_REVERB_TYPE, reverb, true);

        // 各波段设置
        nativeReverbType();
        nativeUserReverbType();
    }

    // 默认混响模式
    public void nativeReverbType() {
        int reverb = getReverb();
        int bandTotal = EqUtils.getBandTotal();
        Log.d(TAG, "nativeReverbType: reverb = " + reverb + " bandTotal = " + bandTotal);
        // 设置每一段的默认值
        if (reverb < EXT_DSP_REVERB_SIZE) {
            if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())){
                int[] _gainValue = ECDConstantExtDsp.DEF_EQ_16_BANDS_VALUES[reverb][0];
                int[] _qValue = ECDConstantExtDsp.DEF_EQ_16_BANDS_VALUES[reverb][1];
                if (bandTotal == EqUtils.BAND_TOTAL_14) {
                    _gainValue = ECDConstantExtDsp.DEF_EQ_14_BANDS_VALUES[reverb][0];
                    _qValue = ECDConstantExtDsp.DEF_EQ_14_BANDS_VALUES[reverb][1];
                } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
                    _gainValue = ECDConstantExtDsp.DEF_EQ_16_BANDS_VALUES[reverb][0];
                    _qValue = ECDConstantExtDsp.DEF_EQ_16_BANDS_VALUES[reverb][1];
                } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                    _gainValue = ECDConstantExtDsp.DEF_EQ_32_BANDS_VALUES[reverb][0];
                    _qValue = ECDConstantExtDsp.DEF_EQ_32_BANDS_VALUES[reverb][1];
                } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                    _gainValue = ECDConstantExtDsp.DEF_EQ_48_BANDS_VALUES[reverb][0];
                    _qValue = ECDConstantExtDsp.DEF_EQ_48_BANDS_VALUES[reverb][1];
                }
                int[] _Gain = dealBandValue(_gainValue);
                int[] _Q = dealBandValue(_qValue);
                for (int index = 0; index < EqUtils.getBandTotal(); index++) {
                    nativeBand(index, _Gain[index], _Q[index]);
                }
            } else {
                for (int index = 0; index < bandTotal; index++) {
                    if (bandTotal == EqUtils.BAND_TOTAL_14) {
                        nativeBand(index, DEF_EQ_14_BANDS_VALUES[reverb][0][index], DEF_EQ_14_BANDS_VALUES[reverb][1][index]);
                    } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                        nativeBand(index, DEF_EQ_32_BANDS_VALUES[reverb][0][index], DEF_EQ_32_BANDS_VALUES[reverb][1][index]);
                    } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                        nativeBand(index, DEF_EQ_48_BANDS_VALUES[reverb][0][index], DEF_EQ_48_BANDS_VALUES[reverb][1][index]);
                    }
                }
            }
        }
    }

    // 用户自定义模式
    public void nativeUserReverbType() {
        int reverb = getReverb();
        Log.d(TAG, "nativeUserReverbType: " + reverb);
        // 设置每一段的默认值
        if (reverb > EXT_DSP_REVERB_SIZE) {
            int[][] bandValue = getUserBandValue(reverb);
            int[] _gainValue = bandValue[0];
            int[] _qValue = bandValue[1];
            if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())) {
                int[] _Gain = dealBandValue(_gainValue);
                int[] _Q = dealBandValue(_qValue);
                for (int i = 0; i < _gainValue.length; i++) {
                    nativeBand(i, _Gain[i], _Q[i]);
                }
            } else {
                for (int i = 0; i < _gainValue.length; i++) {
                    nativeBand(i, _gainValue[i], _qValue[i]);
                }
            }
        }
    }

    /**
     * 处理所有的Band值
     */
    public static int[] dealBandValue(int[] data) {
        int[] realBassGain = new int[EqUtils.getBandTotal()];
        for (int i = 0; i < realBassGain.length; i++) {
            realBassGain[i] = 0;
        }
        int length = data.length;
        int paragraph = length / EqUtils.getBandTotal();
        int residue = length % EqUtils.getBandTotal();
        int dealData = 0;
        int dataIndex = 0;
        for (int realIndex = 0; realIndex < Math.min(EqUtils.getBandTotal(), length); realIndex++) {
            if (paragraph <= 0) {
                realBassGain[realIndex] = data[realIndex];
            } else {
                int re = residue > 0 ? 1 : 0;
                int iii = 0;
                for (int ii = 0; paragraph + re > ii; ii++, iii++) {
                    if (length <= dataIndex) {
                        break;
                    }
                    dealData = data[dataIndex];
                    dataIndex++;
                }
                realBassGain[realIndex] = dealData;
                dealData = 0;
                if (residue > 0) {
                    residue--;
                }
            }

        }
        Log.d(TAG, "data = " + Arrays.toString(data));
        Log.d(TAG, "realBassGain = " + Arrays.toString(realBassGain));
        return realBassGain;
    }

    /**
     * 根据原始的index值（如：第34段，实际我们最大值只有9段），需要对其进行处理，返回34段改变后，实际九段变化的对应index位置的值
     *
     * @param data        处理前的数据
     * @param originIndex 原始index值
     * @return
     */
    public static int getDealBandValue(int[] data, int originIndex) {
        int value = 0;
        int length = data.length;
        int paragraph = length / EqUtils.getBandTotal();
        int residue = length % EqUtils.getBandTotal();
        if (length <= originIndex) {

        } else if (paragraph <= 0) {
            value = data[originIndex];

        } else {
            int number = ((1 + paragraph) * residue > originIndex ? 1 : 0) + paragraph;
            int startIndex = 0;
            if (residue * (paragraph + 1) > originIndex) {
                startIndex = originIndex - originIndex % (paragraph + 1);
            } else {
                startIndex = originIndex - (originIndex - residue * (paragraph + 1)) % paragraph;
            }
            for (int i = number; i > 0; i--, startIndex++) {
                if (startIndex >= length) {
                    continue;
                }
                value = data[originIndex];
            }
        }
        Log.d(TAG, "getDealBandValue:" + value);
        return value;
    }

    /**
     * 根据原始的index值（如：第34段，实际我们最大值只有9段），需要实际九段变化的对应index位置的值
     *
     * @param data        处理前的数据
     * @param originIndex 原始index值
     * @return
     */
    public static int getDealBandIndex(int[] data, int originIndex) {
        int value = 0;
        int length = data.length;
        int paragraph = length / EqUtils.getBandTotal();
        int residue = length % EqUtils.getBandTotal();
        if (length <= originIndex) {

        } else if (paragraph <= 0) {
            value = originIndex;
        } else {
            if (residue * (paragraph + 1) >= originIndex) {
                value = originIndex / (paragraph + 1);
            } else {
                value = (originIndex - residue * (paragraph + 1)) / paragraph + residue;
            }
        }
        Log.d(TAG, "getDealBandIndex:" + value);
        return value;
    }


    public int getReverb() {
        return spUtils.getInt(KEY_REVERB_TYPE, DEF_EQ_REVERB);
    }

    // 设置各 Band 值
    public void nativeBand(int index, int gain, int qValue) {
        int bandTotal = EqUtils.getBandTotal();
        int band = 0;
        int freq = 0;
        if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())) {
            if (bandTotal == EqUtils.BAND_TOTAL_14) {
                band = ECDConstantExtDsp.DEF_EQ_14_FREQ_VALUES[index][0];
                freq = ECDConstantExtDsp.DEF_EQ_14_FREQ_VALUES[index][1];
            } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
                band = ECDConstantExtDsp.DEF_EQ_16_FREQ_VALUES[index][0];
                freq = ECDConstantExtDsp.DEF_EQ_16_FREQ_VALUES[index][1];
            } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                band = ECDConstantExtDsp.DEF_EQ_32_FREQ_VALUES[index][0];
                freq = ECDConstantExtDsp.DEF_EQ_32_FREQ_VALUES[index][1];
            } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                band = ECDConstantExtDsp.DEF_EQ_48_FREQ_VALUES[index][0];
                freq = ECDConstantExtDsp.DEF_EQ_48_FREQ_VALUES[index][1];
            }
        } else {
            if (bandTotal == EqUtils.BAND_TOTAL_14) {
                band = ConstantExtDsp.DEF_EQ_14_FREQ_VALUES[index][0];
                 freq = ConstantExtDsp.DEF_EQ_14_FREQ_VALUES[index][1];
            } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
                band = ECDConstantExtDsp.DEF_EQ_16_FREQ_VALUES[index][0];
                freq = ECDConstantExtDsp.DEF_EQ_16_FREQ_VALUES[index][1];
            } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                band = ConstantExtDsp.DEF_EQ_32_FREQ_VALUES[index][0];
                freq = ConstantExtDsp.DEF_EQ_32_FREQ_VALUES[index][1];
            } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                band = ConstantExtDsp.DEF_EQ_48_FREQ_VALUES[index][0];
                freq = ConstantExtDsp.DEF_EQ_48_FREQ_VALUES[index][1];
            }
        }

        int[] data = {band, freq, gain, qValue};
        Log.d(TAG, String.format("nativeBand data : %s", Arrays.toString(data)));
        NativeHelper.getEq().setEqBand(data);
    }

    // 获取当前用户模式各Band值.
    public int[][] getUserBandValue(int reverb) {
        int bandTotal = EqUtils.getBandTotal();
        int[][] bandValue = new int[2][bandTotal];
        if (reverb > EXT_DSP_REVERB_SIZE) {
            // USER 模式获取保存值
            String userBand = spUtils.getString(KEY_USER_BAND[reverb - EXT_DSP_REVERB_USER0]);
            if (!"".equals(userBand)) {
                userBand = userBand.substring(userBand.indexOf("[") + 1, userBand.indexOf("]"));
                String[] mStr = userBand.split(",");
                Log.d(TAG, "getUserBandValue: mStr.size = " + mStr.length + " bandTotal = " + bandTotal);
                for (int i = 0; i < bandTotal; i++) {
                    //获取各Band值
                    if (mStr.length - 1 >= i) {
                        bandValue[0][i] = Integer.parseInt(mStr[i].trim());
                    } else if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())){
                        bandValue[0][i] = 0;
                    }
                }
            }

            String userQValue = spUtils.getString(KEY_USER_QVALUE[reverb - EXT_DSP_REVERB_USER0]);
            if (!"".equals(userQValue)) {
                userQValue = userQValue.substring(userQValue.indexOf("[") + 1, userQValue.indexOf("]"));
                String[] mStr = userQValue.split(",");
                for (int i = 0; i < bandTotal; i++) {
                    //获取各Band值
                    if (mStr.length - 1 >= i) {
                        bandValue[1][i] = Integer.parseInt(mStr[i].trim());
                    } else if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())) {
                        bandValue[1][i] = DEF_QVALUE;
                    }
                }
            } else { // 设置默认q值
                for (int i = 0; i < bandTotal; i++) {
                    //获取各Band值
                    bandValue[1][i] = DEF_QVALUE;
                }
            }
        } else if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())) {
            if (bandTotal == EqUtils.BAND_TOTAL_14) {
                return ECDConstantExtDsp.DEF_EQ_14_BANDS_VALUES[reverb];
            } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
                return ECDConstantExtDsp.DEF_EQ_16_BANDS_VALUES[reverb];
            } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                return ECDConstantExtDsp.DEF_EQ_32_BANDS_VALUES[reverb];
            } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                return ECDConstantExtDsp.DEF_EQ_48_BANDS_VALUES[reverb];
            }
        } else {
            if (bandTotal == EqUtils.BAND_TOTAL_14) {
                return DEF_EQ_14_BANDS_VALUES[reverb];
            } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
                return ECDConstantExtDsp.DEF_EQ_16_BANDS_VALUES[reverb];
            } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                return DEF_EQ_32_BANDS_VALUES[reverb];
            } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                return DEF_EQ_48_BANDS_VALUES[reverb];
            }
        }
        Log.d(TAG, "getUserBandValue: " + Arrays.toString(bandValue));
        return bandValue;
    }

    public void saveBandValue(int[][] bandValue) {
        int reverb = extDspBandSettings.getReverb();
        if (reverb > EXT_DSP_REVERB_SIZE) {
            // 只保存 User 模式下的值
            spUtils.put(KEY_USER_BAND[reverb - EXT_DSP_REVERB_USER0], Arrays.toString(bandValue[0]));
            spUtils.put(KEY_USER_QVALUE[reverb - EXT_DSP_REVERB_USER0], Arrays.toString(bandValue[1]), true);
        }
    }

    public void resetUserBand() {
        int valueSize = EqUtils.getBandTotal();
        for (int i = 0; i < valueSize; i++) {
            nativeBand(i, 0, DEF_QVALUE);
        }
        spUtils.clear();
        spUtils.put(KEY_REVERB_TYPE, DEF_EQ_REVERB, true);
    }

}
