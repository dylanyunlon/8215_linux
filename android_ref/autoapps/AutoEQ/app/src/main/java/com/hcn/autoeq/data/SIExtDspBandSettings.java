package com.hcn.autoeq.data;

import static com.hcn.autoeq.util.ConstantCscAsp.BAND_TOTAL;

import android.content.Context;
import android.graphics.Paint;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantExtDsp;
import com.hcn.autoeq.util.SIConstantExtDsp;
import com.hcn.autoeq.util.EqUtils;

import java.util.Arrays;

public class SIExtDspBandSettings implements SIConstantExtDsp {

    private static final String TAG = SIExtDspBandSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(SIExtDspBandSettings.class.getSimpleName(), Log.DEBUG);

    private static final String SI_EXT_DSP_BAND_FILE = "si_ext_dsp_band"; // 各模式的音效保存的文件名
    private static final String SI_KEY_REVERB_TYPE = "si_reverb_type"; // 音效模式的 key
    private static final String SI_KEY_USER_BAND[] = {"user0_band", "user1_band", "user2_band"}; // 音效频率的 key
    private static final String SI_KEY_USER_QVALUE[] = {"user0_qvalue", "user1_qvalue", "user2_qvalue"}; // 音效 q 值的 key

    private Context context;
    private static SIExtDspBandSettings extDspBandSettings = null;
    private SPUtils spUtils;

    //真实的band总数；
    private static final int REAL_BAND_TOTAL = EqUtils.getBandTotal();

    //用来存储band值（用来上传的）
    private int bassGain[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    //用来中心频率值；（用来上传的）
    private static int centerFre[] = {63, 100, 160, 300, 630, 1000, 3150, 8000, 16000};

    public static SIExtDspBandSettings getInstance(Context mContext) {
        if (null == extDspBandSettings) {
            extDspBandSettings = new SIExtDspBandSettings(mContext);
        }
        return extDspBandSettings;
    }

    private SIExtDspBandSettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(SI_EXT_DSP_BAND_FILE);
    }

    public void saveReverb(int reverb) {
        spUtils.put(SI_KEY_REVERB_TYPE, reverb, true);

        // 各波段设置
        nativeReverbType();
        nativeUserReverbType();
    }

    // 默认混响模式
    public void nativeReverbType() {
        int reverb = getReverb();
        int bandTotal = EqUtils.getBandTotal();
        if (reverb < EXT_DSP_REVERB_SIZE) {
            int[] _gainValue = SI_DEF_EQ_14_BANDS_VALUES[reverb][0];
            int[] _qValue = SI_DEF_EQ_14_BANDS_VALUES[reverb][1];
            if (bandTotal == EqUtils.BAND_TOTAL_14) {
                _gainValue = SI_DEF_EQ_14_BANDS_VALUES[reverb][0];
                _qValue = SI_DEF_EQ_14_BANDS_VALUES[reverb][1];
            } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                _gainValue = SI_DEF_EQ_32_BANDS_VALUES[reverb][0];
                _qValue = SI_DEF_EQ_32_BANDS_VALUES[reverb][1];
            } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                _gainValue = SI_DEF_EQ_48_BANDS_VALUES[reverb][0];
                _qValue = SI_DEF_EQ_48_BANDS_VALUES[reverb][1];
            } else if (bandTotal == EqUtils.BAND_TOTAL_36) {
                _gainValue = SI_DEF_EQ_36_BANDS_VALUES[reverb][0];
                _qValue = SI_DEF_EQ_36_BANDS_VALUES[reverb][1];
            } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
                _gainValue = SI_DEF_EQ_16_BANDS_VALUES[reverb][0];
                _qValue = SI_DEF_EQ_16_BANDS_VALUES[reverb][1];
            }
            int[] _Gain = dealBandValue(_gainValue);
            int[] _Q = dealBandValue(_qValue);
            for (int index = 0; index < REAL_BAND_TOTAL; index++) {
                nativeBand(index, _Gain[index], _Q[index]);
            }
        }
    }

    // 用户自定义模式
    public void nativeUserReverbType() {
        int reverb = getReverb();
        // 设置每一段的默认值
        if (reverb > EXT_DSP_REVERB_SIZE) {
            int[][] bandValue = getUserBandValue(reverb);
            int[] _gainValue = bandValue[0];
            int[] _qValue = bandValue[1];
            int[] _Gain = dealBandValue(_gainValue);
            int[] _Q = dealBandValue(_qValue);
            for (int i = 0; i < REAL_BAND_TOTAL; i++) {
                nativeBand(i, _Gain[i], _Q[i]);
            }
        }
    }

    public int getReverb() {
        return spUtils.getInt(SI_KEY_REVERB_TYPE, DEF_EQ_REVERB);
    }

    // 设置各 Band 值
    public void nativeBand(int index, int gain, int qValue) {
        int freq = 0;
        int band = 0;
        int bandTotal = EqUtils.getBandTotal();
        if (bandTotal == EqUtils.BAND_TOTAL_14) {
            band = SIConstantExtDsp.SI_DEF_EQ_14_FREQ_VALUES[index][0];
            freq = SIConstantExtDsp.SI_DEF_EQ_14_FREQ_VALUES[index][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
            band = SIConstantExtDsp.SI_DEF_EQ_32_FREQ_VALUES[index][0];
            freq = SIConstantExtDsp.SI_DEF_EQ_32_FREQ_VALUES[index][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
            band = SIConstantExtDsp.DEF_EQ_48_FREQ_VALUES[index][0];
            freq = SIConstantExtDsp.DEF_EQ_48_FREQ_VALUES[index][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_36) {
            band = SIConstantExtDsp.SI_DEF_EQ_36_FREQ_VALUES[index][0];
            freq = SIConstantExtDsp.SI_DEF_EQ_36_FREQ_VALUES[index][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
            band = SIConstantExtDsp.SI_DEF_EQ_16_FREQ_VALUES[index][0];
            freq = SIConstantExtDsp.SI_DEF_EQ_16_FREQ_VALUES[index][1];
        }
        int[] data = {band, freq, gain, qValue};
        Log.d(TAG, String.format("nativeBand data : %s", Arrays.toString(data)));
        NativeHelper.getEq().setEqBand(data);
    }

    /**
     * 处理所有的Band值
     */
    public static int[] dealBandValue(int[] data) {
        // 9段上传段数
        int[] realBassGain = new int[EqUtils.getBandTotal()];
        for (int i = 0; i < realBassGain.length; i++) {
            realBassGain[i] = 0;
        }
        int length = data.length;
        // 根据显示总段数和实际上传段数，判断处理条件,目的是只做九段实际上传段数；
        int paragraph = length / REAL_BAND_TOTAL;// 段数
        int residue = length % REAL_BAND_TOTAL;// 取余值
        int dealData = 0;//处理后的数据
        int dataIndex = 0;
        // 这里是对数据做处理，以求做到9段上传段数
        for (int realIndex = 0; realIndex < Math.min(REAL_BAND_TOTAL, length); realIndex++) {
            if (paragraph <= 0) {
                // 如果段数小于等于0，即显示总段数小于实际上传段数，就依次加入
                realBassGain[realIndex] = data[realIndex];
            } else {
                // 如果段数大于0，即总段数大于实际段数,那就把多出相接近的数值相加取平均值
                int re = residue > 0 ? 1 : 0;
                int iii = 0;
                for (int ii = 0; paragraph + re > ii; ii++, iii++) {
                    if (length <= dataIndex) {
                        break;
                    }
                    //2024.06.24 做一点小小的改动，不学asp,取平均值了;而是采用7604的作假，获取最后的值（保留目前的算式，这样可以避免重复提交同个index的上传）
                    dealData = data[dataIndex];
                    dataIndex++;
                }
                //2024.06.24 做一点小小的改动，不学asp,取平均值了;而是采用7604的作假，获取最后的值（保留目前的算式，这样可以避免重复提交同个index的上传）
                realBassGain[realIndex] = dealData;
                dealData = 0;
                if (residue > 0) {
                    residue--;
                }
            }

        }
        Log.d(TAG, Arrays.toString(data));
        Log.d(TAG, Arrays.toString(realBassGain));
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
        //需要返回的值
        int value = 0;
        int length = data.length;
        // 根据显示总段数和实际上传段数，判断处理条件,目的是只做九段实际上传段数；
        int paragraph = length / REAL_BAND_TOTAL;// 段数
        int residue = length % REAL_BAND_TOTAL;// 取余值
        if (length <= originIndex) {
            return value;
        }
        if (paragraph <= 0) {
            // 如果段数小于等于0，即显示总段数小于实际上传段数，就不做任何处理；
            value = data[originIndex];
            Log.d(TAG, "getDealBandValue:" + value);
            return value;
        } else {
            // 如果段数大于0，即总段数大于实际段数,那就遍历一遍数据；把包含 有原始index值的 处理数据返回；
            //因为知道index值及其数据组的长度，因此可以直接定位 包含 有原始index值的 处理数据；
            //先确定数量
            int number = ((1 + paragraph) * residue > originIndex ? 1 : 0) + paragraph;
            int startIndex = 0;
            //然后确定起点位置
            if (residue * (paragraph + 1) > originIndex) {
                startIndex = originIndex - originIndex % (paragraph + 1);
            } else {
                startIndex = originIndex - (originIndex - residue * (paragraph + 1)) % paragraph;
            }
            //最后返回值
            for (int i = number; i > 0; i--, startIndex++) {
                if (startIndex >= length) {
                    continue;
                }
                //2024.06.24 做一点小小的改动，不学asp,取平均值了;而是采用7604的作假，获取最后的值（保留目前的算式，这样可以避免重复提交同个index的上传）
                value = data[originIndex];
            }
            Log.d(TAG, "getDealBandValue:" + value);
            return value;
        }
    }

    /**
     * 根据原始的index值（如：第34段，实际我们最大值只有9段），需要实际九段变化的对应index位置的值
     *
     * @param data        处理前的数据
     * @param originIndex 原始index值
     * @return
     */
    public static int getDealBandIndex(int[] data, int originIndex) {
        //需要返回的值
        int value = 0;
        int length = data.length;
        // 根据显示总段数和实际上传段数，判断处理条件,目的是只做九段实际上传段数；
        int paragraph = length / REAL_BAND_TOTAL;// 段数
        int residue = length % REAL_BAND_TOTAL;// 取余值
        if (length <= originIndex) {
            Log.d(TAG, "getDealBandIndex:" + value);
            return value;
        }
        if (paragraph <= 0) {
            // 如果段数小于等于0，即显示总段数小于实际上传段数，就不做任何处理；
            value = originIndex;
            Log.d(TAG, "getDealBandIndex:" + value);
            return value;
        } else {
            if (residue * (paragraph + 1) >= originIndex) {
                value = originIndex / (paragraph + 1);
            } else {
                value = (originIndex - residue * (paragraph + 1)) / paragraph + residue;
            }
            Log.d(TAG, "getDealBandIndex:" + value);
            return value;
        }
    }


    // 获取当前用户模式各Band值.
    public int[][] getUserBandValue(int reverb) {
        int bandTotal = EqUtils.getBandTotal();
        int[][] bandValue = new int[2][bandTotal];
        if (reverb > EXT_DSP_REVERB_SIZE) {
            // USER 模式获取保存值
            String userBand = spUtils.getString(SI_KEY_USER_BAND[reverb - EXT_DSP_REVERB_USER0]);
            if (!"".equals(userBand)) {
                userBand = userBand.substring(userBand.indexOf("[") + 1, userBand.indexOf("]"));
                String[] mStr = userBand.split(",");
                for (int i = 0; i < bandTotal; i++) {
                    //获取各Band值
                    if (i < mStr.length) {
                        bandValue[0][i] = Integer.parseInt(mStr[i].trim());
                    } else {
                        bandValue[0][i] = 0;
                    }
                }
            }

            String userQValue = spUtils.getString(SI_KEY_USER_QVALUE[reverb - EXT_DSP_REVERB_USER0]);
            if (!"".equals(userQValue)) {
                userQValue = userQValue.substring(userQValue.indexOf("[") + 1, userQValue.indexOf("]"));
                String[] mStr = userQValue.split(",");
                for (int i = 0; i < bandTotal; i++) {
                    //获取各Band值
                    if (i < mStr.length) {
                        bandValue[1][i] = Integer.parseInt(mStr[i].trim());
                    } else {
                        bandValue[1][i] = DEF_QVALUE;
                    }
                }
            } else { // 设置默认q值
                for (int i = 0; i < bandTotal; i++) {
                    //获取各Band值
                    bandValue[1][i] = DEF_QVALUE;
                }
            }
        } else {
            if (bandTotal == EqUtils.BAND_TOTAL_14) {
                return SI_DEF_EQ_14_BANDS_VALUES[reverb];
            } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                return SI_DEF_EQ_32_BANDS_VALUES[reverb];
            } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                return SI_DEF_EQ_48_BANDS_VALUES[reverb];
            } else if (bandTotal == EqUtils.BAND_TOTAL_36) {
                return SI_DEF_EQ_36_BANDS_VALUES[reverb];
            }  if (bandTotal == EqUtils.BAND_TOTAL_16) {
                return SI_DEF_EQ_16_BANDS_VALUES[reverb];
            }
        }
        return bandValue;
    }

    public void saveBandValue(int[][] bandValue) {
        int reverb = extDspBandSettings.getReverb();
        if (reverb > EXT_DSP_REVERB_SIZE) {
            // 只保存 User 模式下的值
            spUtils.put(SI_KEY_USER_BAND[reverb - EXT_DSP_REVERB_USER0], Arrays.toString(bandValue[0]));
            spUtils.put(SI_KEY_USER_QVALUE[reverb - EXT_DSP_REVERB_USER0], Arrays.toString(bandValue[1]), true);
        }
    }

    public void resetUserBand() {
        spUtils.clear();
        spUtils.put(SI_KEY_REVERB_TYPE, DEF_EQ_REVERB, true);
    }

}
