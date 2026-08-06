package com.hcn_library.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.skin.support.utils.SkinListUtils;
import com.hcn_library.nativeextdsp.NativeHelper;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;

import java.util.Arrays;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;


public class NineDspBandSettings implements NineConstantExtDsp {
    // 存储扩展 DSP 频段设置的 SharedPreferences 文件名称
    private static final String DSP_BAND_SETTINGS_FILE = "ext_dsp_band";
    // 存储响度设置的键名
    private static final String KEY_LOUDNESS = "nine_dsp_loudness";
    // 存储混响类型的键名
    private static final String KEY_REVERB_TYPE = "reverb_type";
    // 存储环绕声设置的键名
    private static final String KEY_SURROUND = "nine_dsp_surround";
    // 上下文对象，用于资源访问等操作
    private Context context;
    // 用于存储和读取 SharedPreferences 数据的工具类实例
    private SPUtils sharedPreferencesUtils = SPUtils.getInstance(DSP_BAND_SETTINGS_FILE);
    // 日志标签，方便调试和追踪
    private static final String TAG = "NineDspBandSettings";
    // 存储用户频段设置的键名数组
    private static final String[] USER_BAND_KEYS = {"user0_band", "user1_band", "user2_band"};
    // 存储用户 Q 值设置的键名数组
    private static final String[] USER_QVALUE_KEYS = {"user0_qvalue", "user1_qvalue", "user2_qvalue"};
    // 当前自定义混响名称
    private String currentCustomReverbName = "";
    // 单例对象
    private static NineDspBandSettings instance = null;
    private int[][][] DEF_EQ_36_BANDS_VALUES;


    public static NineDspBandSettings getInstance(Context context) {
        if (instance == null) {
            instance = new NineDspBandSettings(context);
        }
        return instance;
    }


    // 保存环绕声设置到 SharedPreferences
    public void saveSurround(int surroundValue) {
        Log.d(TAG, String.format("saveSurround surround : %d", surroundValue));
        sharedPreferencesUtils.put(KEY_SURROUND, surroundValue, true);
    }


    // 从 SharedPreferences 获取环绕声设置
    public int getSurround() {
        return sharedPreferencesUtils.getInt(KEY_SURROUND, 0);
    }


    // 保存响度设置到 SharedPreferences
    public void saveLoudness(int loudnessValue) {
        Log.d(TAG, String.format("saveLoudness loudness : %d", loudnessValue));
        sharedPreferencesUtils.put(KEY_LOUDNESS, loudnessValue, true);
    }


    // 从 SharedPreferences 获取响度设置
    public int getLoudness() {
        return sharedPreferencesUtils.getInt(KEY_LOUDNESS, "gb05".equals(EqUtils.getSkinName()) ? 2 : 1);
    }


    // 调用 Native 方法设置环绕声
    public void nativeSurround(int surroundValue) {
        NativeHelper.getEq().setEqSurround(new int[]{surroundValue});
        Log.d(TAG, String.format("nativeSurround data : %s", surroundValue));
    }


    // 调用 Native 方法设置响度
    public void nativeLoudness(int loudnessValue) {
        NativeHelper.getEq().setEqLoudness(new int[]{loudnessValue});
        Log.d(TAG, String.format("nativeLoudness data : %s", loudnessValue));
    }


    // 构造函数，初始化上下文
    private NineDspBandSettings(Context context) {
        this.context = context;
        if (EqUtils.isChip7739()) {
            DEF_EQ_36_BANDS_VALUES = DEF_EQ_36_BANDS_VALUES_7739;
        } else {
            DEF_EQ_36_BANDS_VALUES = NineConstantExtDsp.DEF_EQ_36_BANDS_VALUES;
        }
    }


    // 保存混响类型设置到 SharedPreferences 并调用相关 Native 方法
    public void saveReverb(int reverbValue) {
        Log.d(TAG, "saveReverb :" + reverbValue);
        // 设置到标准模式，需要渐变处理
        if(reverbValue == 0) {
            int[] [] currentBandValue = getUserBandValue(getReverb());
            sharedPreferencesUtils.put(KEY_REVERB_TYPE, reverbValue, true);
            resetUserBand(currentBandValue);
            return;
        }
        sharedPreferencesUtils.put(KEY_REVERB_TYPE, reverbValue, true);
        nativeReverbType();
        nativeUserReverbType();
    }


    // 从 SharedPreferences 获取混响类型
    public int getReverb() {
        int reverbValue = sharedPreferencesUtils.getInt(KEY_REVERB_TYPE, 1);
        Log.d(TAG, "getReverb :" + reverbValue);
        return reverbValue;
    }
    // 设置当前自定义混响名称
    public void setCurrentCustomReverbName(String currentCustomReverbName) {
        this.currentCustomReverbName = currentCustomReverbName;
    }
    // 获取自定义混响名称
    public String getCurrentCustomReverbName() {
        return currentCustomReverbName;
    }

    // 调用 Native 方法设置混响类型（非用户自定义）
    public void nativeReverbType() {
        int reverb = getReverb() - NINE_DSP_REVERB_PREVIEW_START_INDEX;
        int bandTotal = EqUtils.getBandTotal();
        Log.d(TAG, "nativeReverbType: reverb = " + reverb + " bandTotal = " + bandTotal);
        if (reverb >= 9 || reverb < 0) {
            return;
        }
        int[] _gainValue = DEF_EQ_36_BANDS_VALUES[reverb][0];
        int[] _qValue = DEF_EQ_36_BANDS_VALUES[reverb][1];
        if (bandTotal == EqUtils.BAND_TOTAL_14) {
            _gainValue = DEF_EQ_14_BANDS_VALUES[reverb][0];
            _qValue = DEF_EQ_14_BANDS_VALUES[reverb][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
            _gainValue = DEF_EQ_16_BANDS_VALUES[reverb][0];
            _qValue = DEF_EQ_16_BANDS_VALUES[reverb][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
            _gainValue = DEF_EQ_32_BANDS_VALUES[reverb][0];
            _qValue = DEF_EQ_32_BANDS_VALUES[reverb][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_36) {
            _gainValue = DEF_EQ_36_BANDS_VALUES[reverb][0];
            _qValue = DEF_EQ_36_BANDS_VALUES[reverb][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
            _gainValue = DEF_EQ_48_BANDS_VALUES[reverb][0];
            _qValue = DEF_EQ_48_BANDS_VALUES[reverb][1];
        }
        int[] _Gain = dealBandValue(_gainValue);
        int[] _Q = dealBandValue(_qValue);
        for (int index = 0; index < EqUtils.getBandTotal(); index++) {
            nativeBand(index, _Gain[index], _Q[index]);
        }
    }


    // 调用 Native 方法设置用户自定义混响类型
    public void nativeUserReverbType() {
        int reverb = getReverb();
        Log.d(TAG, "nativeUserReverbType: " + reverb);
        if (reverb < NINE_DSP_REVERB_PREVIEW_START_INDEX) {
            int[][] userBandValue = getUserBandValue(reverb);
            int[] processedBandValues0 = dealBandValue(userBandValue[0]);
            int[] processedBandValues1 = dealBandValue(userBandValue[1]);
            for (int i = 0; i < processedBandValues0.length; i++) {
                nativeBand(i, processedBandValues0[i], processedBandValues1[i]);
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
        // 根据显示总段数和实际上传段数，判断处理条件,目的是只做九段实际上传段数；
        int paragraph = length / EqUtils.getBandTotal();// 段数
        int residue = length % EqUtils.getBandTotal();// 取余值
        int dealData = 0;//处理后的数据
        int dataIndex = 0;
        // 这里是对数据做处理，以求做到9段上传段数
        for (int realIndex = 0; realIndex < Math.min(EqUtils.getBandTotal(), length); realIndex++) {
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
        int paragraph = length / EqUtils.getBandTotal();// 段数
        int residue = length % EqUtils.getBandTotal();// 取余值
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
        int paragraph = length / EqUtils.getBandTotal();// 段数
        int residue = length % EqUtils.getBandTotal();// 取余值
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


    // 调用 Native 方法设置频段
    public void nativeBand(int index, int bandValue, int qValue) {
        int bandTotal = EqUtils.getBandTotal();
        int band = 0;
        int freq = 0;
        if (bandTotal == EqUtils.BAND_TOTAL_14) {
            band = DEF_EQ_14_FREQ_VALUES[index][0];
            freq = DEF_EQ_14_FREQ_VALUES[index][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
            band = DEF_EQ_16_FREQ_VALUES[index][0];
            freq = DEF_EQ_16_FREQ_VALUES[index][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
            band = DEF_EQ_32_FREQ_VALUES[index][0];
            freq = DEF_EQ_32_FREQ_VALUES[index][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_36) {
            band = DEF_EQ_36_FREQ_VALUES[index][0];
            freq = DEF_EQ_36_FREQ_VALUES[index][1];
        } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
            band = DEF_EQ_48_FREQ_VALUES[index][0];
            freq = DEF_EQ_48_FREQ_VALUES[index][1];
        }
        int[] data = {band, freq, bandValue, qValue};
        Log.d(TAG, String.format("nativeBand data : %s", Arrays.toString(data)) + " band setting: " + EqUtils.getBandTotal());
        NativeHelper.getEq().setEqBand(data);
    }


    // 获取用户自定义频段值
    public int[][] getUserBandValue(int reverbIndex) {
        int bandTotal = EqUtils.getBandTotal();
        int[][] bandValue = new int[2][bandTotal];
        if (reverbIndex == 0) {
            for (int i = 0; i < bandTotal; i++) {
                bandValue[0][i] = 0;
                bandValue[1][i] = DEF_QVALUE;
            }
        } else if (reverbIndex < NINE_DSP_REVERB_PREVIEW_START_INDEX) {
            int userIndex = reverbIndex - 1;
            String bandValueStr = sharedPreferencesUtils.getString(USER_BAND_KEYS[userIndex]);
            if (!"".equals(bandValueStr)) {
                String[] bandValues = bandValueStr.substring(bandValueStr.indexOf("[") + 1, bandValueStr.indexOf("]")).split(SkinListUtils.DEFAULT_JOIN_SEPARATOR);
                Log.d(TAG, "getUserBandValue: mStr.size = " + bandValues.length + " bandTotal = " + bandTotal);
                for (int i = 0; i < bandTotal; i++) {
                    bandValue[0][i] = i < bandValues.length ? Integer.parseInt(bandValues[i].trim()) : 0;
                }
            }
            String qValueStr = sharedPreferencesUtils.getString(USER_QVALUE_KEYS[userIndex]);
            if (!"".equals(qValueStr)) {
                String[] qValues = qValueStr.substring(qValueStr.indexOf("[") + 1, qValueStr.indexOf("]")).split(SkinListUtils.DEFAULT_JOIN_SEPARATOR);
                for (int i = 0; i < bandTotal; i++) {
                    bandValue[1][i] = i < qValues.length ? Integer.parseInt(qValues[i].trim()) : DEF_QVALUE;
                }
            } else {
                for (int i = 0; i < bandTotal; i++) {
                    bandValue[1][i] = DEF_QVALUE;
                }
            }
        } else {
            int defIndex = reverbIndex - NINE_DSP_REVERB_PREVIEW_START_INDEX;
            Log.d(TAG, "eq index: " + reverbIndex);
            if (bandTotal == EqUtils.BAND_TOTAL_14) {
                return DEF_EQ_14_BANDS_VALUES[defIndex];
            } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
                return DEF_EQ_16_BANDS_VALUES[defIndex];
            } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
                return DEF_EQ_32_BANDS_VALUES[defIndex];
            } else if (bandTotal == EqUtils.BAND_TOTAL_36) {
                return DEF_EQ_36_BANDS_VALUES[defIndex];
            } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
                return DEF_EQ_48_BANDS_VALUES[defIndex];
            }
        }
        Log.d(TAG, "eq index: " + reverbIndex + "getUserBandValue: " + Arrays.toString(bandValue[0]));
        return bandValue;
    }


    // 保存频段值到 SharedPreferences
    public void saveBandValue(int[][] bandValues) {
        int reverb = getReverb();
        if (reverb >= NINE_DSP_REVERB_PREVIEW_START_INDEX || reverb <= 0) {
            return;
        }
        int userIndex = reverb - 1;
        sharedPreferencesUtils.put(USER_BAND_KEYS[userIndex], Arrays.toString(bandValues[0]));
        sharedPreferencesUtils.put(USER_QVALUE_KEYS[userIndex], Arrays.toString(bandValues[1]), true);
        Log.d(TAG, "saveBandValue reverb: " + reverb + ", band value: " + Arrays.toString(bandValues[0]));
    }


    // 创建一个共享的 ScheduledExecutorService 实例
    private static ScheduledExecutorService executorService = new ScheduledThreadPoolExecutor(1);

    // 重置用户频段设置
    public void resetUserBand(int[][] currentUserBandValue) {
        int valueSize = EqUtils.getBandTotal();
        int reverb = getReverb();
        int userIndex = reverb - 1; // 第一个为标准模式
        int[] processedBandValues0 = dealBandValue(currentUserBandValue[0]);
        for (int i = 0; i < valueSize; i++) {
            sendDataWithGradient(i, processedBandValues0[i]);
        }
        if (reverb > 0 && reverb < NINE_DSP_REVERB_PREVIEW_START_INDEX) {
            sharedPreferencesUtils.remove(USER_BAND_KEYS[userIndex]);
            sharedPreferencesUtils.remove(USER_QVALUE_KEYS[userIndex]);
        }
    }

    // 重置的时候做渐变处理
    private void sendDataWithGradient(int index, int currentValue) {
        int targetValue = 0;
        int dataStep = (int) (Math.abs(currentValue) * 1f / 5 + 0.5f);
        Log.d(TAG, "sendDataWithGradient dataStep: " + dataStep + " currentValue: " + currentValue);
        if (currentValue == 0) {
            return;
        }
        // 根据 currentValue 的正负确定 dataStep 的正负，确保向 0 渐变
        dataStep = (currentValue >= 0) ? -dataStep : dataStep;
        int tempValue = currentValue;
        int delay = 100; // 初始延迟时间，单位为毫秒
        int maxTasks = 200; // 最大任务数量
        int taskCount = 0;
        try {
            while (true) {
                if (executorService.isShutdown()) {
                    Log.d(TAG, "sendDataWithGradient: executorService is shutdown");
                    break;
                }
                if (dataStep == 0) {
                    executorService.schedule(() -> nativeBand(index, targetValue, DEF_QVALUE), delay, TimeUnit.MILLISECONDS);
                    break;
                }
                tempValue += dataStep;
                if ((dataStep > 0 && tempValue >= targetValue) || (dataStep < 0 && tempValue <= targetValue)) {
                    executorService.schedule(() -> nativeBand(index, targetValue, DEF_QVALUE), delay, TimeUnit.MILLISECONDS);
                    break;
                }
                int finalTempValue = tempValue;
                executorService.schedule(() -> nativeBand(index, finalTempValue, DEF_QVALUE), delay, TimeUnit.MILLISECONDS);
                delay += 100; // 每次延迟时间递增，可根据需要调整
                taskCount++;
                if (taskCount >= maxTasks) {
                    break;
                }
            }
        } catch (Exception e) {
            // 处理异常，例如关闭服务时可能出现的异常
            e.printStackTrace();
        }
    }

    // 关闭 ScheduledExecutorService 的方法
    public static void shutdownExecutorService() {
        executorService.shutdown();
    }

    // 调用多个 Native 方法设置所有相关参数
    public void nativeAll() {
        nativeReverbType();
        nativeUserReverbType();
        nativeSurround(getSurround());
        nativeLoudness(getLoudness());
    }
}