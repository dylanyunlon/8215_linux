package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.hcn.autoeq.bean.FyDspHLPFFreq;
import com.hcn.autoeq.bean.FyDspHLPFSlope;
import com.hcn.autoeq.bean.FyDspOutputChannel;
import com.hcn.autoeq.bean.FyDspOutputMode;
import com.hcn.autoeq.nativeextdsp.FY7604;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantFyDsp;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class FyDspHLPFSettings extends FyDspBaseSettings implements ConstantFyDsp {

    private static final String TAG = FyDspHLPFSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(FyDspHLPFSettings.class.getSimpleName(), Log.DEBUG);

    private static final String FY_DSP_HLPF_FILE = "v2_fy_dsp_hlpf"; // 各模式的衰减及反相保存的文件名

    private Context context;
    private static FyDspHLPFSettings fyDspHLPFSettings = null;

    public static FyDspHLPFSettings getInstance(Context context) {
        if (null == fyDspHLPFSettings) {
            fyDspHLPFSettings = new FyDspHLPFSettings(context);
        }
        return fyDspHLPFSettings;
    }

    private FyDspHLPFSettings(Context context) {
        super(FY_DSP_HLPF_FILE);
        this.context = context;
    }

    public void saveOutputMode(FyDspOutputMode fyDspOutputMode) {
        Log.d(TAG, "saveOutputMode output mode : " + fyDspOutputMode.name());
        spUtils.put("key_output_mode", fyDspOutputMode.name());
    }

    public FyDspOutputMode getOutputMode() {
        return FyDspOutputMode.valueOf(getSpUtils().getString("key_output_mode", DEF_OUTPUT_MODE.name()));
    }

    public void saveOutputChannel(FyDspOutputChannel fyDspOutputChannel, FyDspOutputMode fyDspOutputMode) {
        Log.d(TAG, "saveOutputChannel output channel : " + fyDspOutputChannel.name());
        spUtils.put("key_output_channel_" + fyDspOutputMode.name(), fyDspOutputChannel.name());
    }

    public FyDspOutputChannel getOutputChannel(FyDspOutputMode fyDspOutputMode) {
        FyDspOutputChannel DEF_OUTPUT_CHANNEL; // 默认输出通道，不通的输出模式有不通的输出模式
        if (fyDspOutputMode == FyDspOutputMode.WAY2) {
            DEF_OUTPUT_CHANNEL = FyDspOutputChannel.WAY2_F;
        } else if (fyDspOutputMode == FyDspOutputMode.WAY3) {
            DEF_OUTPUT_CHANNEL = FyDspOutputChannel.WAY3_F;
        } else if (fyDspOutputMode == FyDspOutputMode.CHANNEL51) {
            DEF_OUTPUT_CHANNEL = FyDspOutputChannel.CHANNEL51_F;
        } else {
            DEF_OUTPUT_CHANNEL = FyDspOutputChannel.WAY6_F;
        }

        return FyDspOutputChannel.valueOf(getSpUtils().getString("key_output_channel_" + fyDspOutputMode, DEF_OUTPUT_CHANNEL.name()));
    }

    // 各模式的频率默认值
    private static final List<FyDspHLPFFreq> WAY2_F = Arrays.stream(FyDspHLPFFreq.values()).filter(fyDspHLPFFreq -> fyDspHLPFFreq.getFreq() >= 5000).collect(Collectors.toList());
    private static final List<FyDspHLPFFreq> WAY2_R = Arrays.stream(FyDspHLPFFreq.values()).filter(fyDspHLPFFreq -> fyDspHLPFFreq.getFreq() >= 63 && fyDspHLPFFreq.getFreq() <= 5000).collect(Collectors.toList());
    private static final List<FyDspHLPFFreq> WAY2_SUBWOOFER_CENTER = Arrays.stream(FyDspHLPFFreq.values()).filter(fyDspHLPFFreq -> fyDspHLPFFreq.getFreq() >= 20 && fyDspHLPFFreq.getFreq() <= 160).collect(Collectors.toList());

    private static final List<FyDspHLPFFreq> WAY3_F = Arrays.stream(FyDspHLPFFreq.values()).filter(fyDspHLPFFreq -> fyDspHLPFFreq.getFreq() >= 5000).collect(Collectors.toList());
    private static final List<FyDspHLPFFreq> WAY3_R = Arrays.stream(FyDspHLPFFreq.values()).filter(fyDspHLPFFreq -> fyDspHLPFFreq.getFreq() >= 630 && fyDspHLPFFreq.getFreq() <= 6300).collect(Collectors.toList());
    private static final List<FyDspHLPFFreq> WAY3_SUBWOOFER_CENTER = Arrays.stream(FyDspHLPFFreq.values()).filter(fyDspHLPFFreq -> fyDspHLPFFreq.getFreq() >= 63 && fyDspHLPFFreq.getFreq() <= 630).collect(Collectors.toList());

    private static final List<FyDspHLPFFreq> CHANNEL51_F = Arrays.stream(FyDspHLPFFreq.values()).filter(fyDspHLPFFreq -> fyDspHLPFFreq.getFreq() <= 20000).collect(Collectors.toList());
    private static final List<FyDspHLPFFreq> CHANNEL51_R = Arrays.stream(FyDspHLPFFreq.values()).filter(fyDspHLPFFreq -> fyDspHLPFFreq.getFreq() <= 20000).collect(Collectors.toList());
    private static final List<FyDspHLPFFreq> CHANNEL51_SUBWOOFER = Arrays.stream(FyDspHLPFFreq.values()).filter(fyDspHLPFFreq -> fyDspHLPFFreq.getFreq() >= 20 && fyDspHLPFFreq.getFreq() <= 160).collect(Collectors.toList());
    private static final List<FyDspHLPFFreq> CHANNEL51_CENTER = Arrays.stream(FyDspHLPFFreq.values()).filter(fyDspHLPFFreq -> fyDspHLPFFreq.getFreq() >= 630 && fyDspHLPFFreq.getFreq() <= 6300).collect(Collectors.toList());

    // 20 -> 20k
    private static final List<FyDspHLPFFreq> WAY6_F = Arrays.stream(FyDspHLPFFreq.values()).collect(Collectors.toList());
    private static final List<FyDspHLPFFreq> WAY6_R = Arrays.stream(FyDspHLPFFreq.values()).collect(Collectors.toList());
    private static final List<FyDspHLPFFreq> WAY6_SUBWOOFER_CENTER = Arrays.stream(FyDspHLPFFreq.values()).collect(Collectors.toList());

    // 频率范围，不同输出模式，不同的通道，对应不同的频率范围
    public List<FyDspHLPFFreq> getFreqList(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel) {
        if (fyDspOutputMode == FyDspOutputMode.WAY2) {
            if (fyDspOutputChannel == FyDspOutputChannel.WAY2_F) {
                return WAY2_F;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY2_R) {
                return WAY2_R;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY2_SUBWOOFER_CENTER) {
                return WAY2_SUBWOOFER_CENTER;
            } else {
                return WAY2_F;
            }
        } else if (fyDspOutputMode == FyDspOutputMode.WAY3) {
            if (fyDspOutputChannel == FyDspOutputChannel.WAY3_F) {
                return WAY3_F;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY3_R) {
                return WAY3_R;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY3_SUBWOOFER_CENTER) {
                return WAY3_SUBWOOFER_CENTER;
            } else {
                return WAY3_F;
            }
        } else if (fyDspOutputMode == FyDspOutputMode.CHANNEL51) {
            if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_F) {
                return CHANNEL51_F;
            } else if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_R) {
                return CHANNEL51_R;
            } else if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_SUBWOOFER) {
                return CHANNEL51_SUBWOOFER;
            } else if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_CENTER) {
                return CHANNEL51_CENTER;
            } else {
                return CHANNEL51_F;
            }
        } else {
            if (fyDspOutputChannel == FyDspOutputChannel.WAY6_F) {
                return WAY6_F;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY6_R) {
                return WAY6_R;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY6_SUBWOOFER_CENTER) {
                return WAY6_SUBWOOFER_CENTER;
            } else {
                return WAY6_F;
            }
        }
    }

    public List<FyDspHLPFSlope> getSlopeList() {
        return Arrays.stream(FyDspHLPFSlope.values()).collect(Collectors.toList());
    }

    // 设置单个通道
    public void nativeHLPF(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel
            , FyDspHLPFFreq fyDspHPFFreq, FyDspHLPFFreq fyDspLPFFreq, FyDspHLPFSlope fyDspHPFSlope, FyDspHLPFSlope fyDspLPFSlope) {
        Log.d(TAG, "nativeHLPF output mode : " + fyDspOutputMode + ", output channel : " + fyDspOutputChannel
                + ", hpf freq : " + fyDspHPFFreq + ", lpf freq : " + fyDspLPFFreq
                + ", hpf slope : " + fyDspHPFSlope + ", lpf slope : " + fyDspLPFSlope);
        int channelH = 0, channelL = 0, freqH = 0, freqL = 0, slopeH = 0, slopeL = 0;

        if (fyDspOutputMode == FyDspOutputMode.WAY2) {
            if (fyDspOutputChannel == FyDspOutputChannel.WAY2_F) {
                channelH = 1;
                channelL = 2;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY2_R) {
                channelH = 3;
                channelL = 4;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY2_SUBWOOFER_CENTER) {
                channelH = 7;
                channelL = 8;
            }
        } else if (fyDspOutputMode == FyDspOutputMode.WAY3) {
            if (fyDspOutputChannel == FyDspOutputChannel.WAY3_F) {
                channelH = 1;
                channelL = 2;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY3_R) {
                channelH = 3;
                channelL = 4;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY3_SUBWOOFER_CENTER) {
                channelH = 7;
                channelL = 8;
            }
        } else if (fyDspOutputMode == FyDspOutputMode.CHANNEL51) {
            if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_F) {
                channelH = 1;
                channelL = 2;
            } else if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_R) {
                channelH = 3;
                channelL = 4;
            } else if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_CENTER) {
                channelH = 5;
                channelL = 6;
            } else if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_SUBWOOFER) {
                channelH = 7;
                channelL = 8;
            }
        } else if (fyDspOutputMode == FyDspOutputMode.WAY6) {
            if (fyDspOutputChannel == FyDspOutputChannel.WAY6_F) {
                channelH = 1;
                channelL = 2;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY6_R) {
                channelH = 3;
                channelL = 4;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY6_SUBWOOFER_CENTER) {
                channelH = 7;
                channelL = 8;
            }
        }

        freqH = fyDspHPFFreq.getFreq();
        slopeH = fyDspHPFSlope.ordinal(); // 设置到底层时，是索引

        freqL = fyDspLPFFreq.getFreq();
        slopeL = fyDspLPFSlope.ordinal(); // 设置到底层时，是索引

        int[] dataH = new int[]{FY7604.FY_CMD_SUB_ID_LPF_HPF, channelH, freqH, slopeH};
        int[] dataL = new int[]{FY7604.FY_CMD_SUB_ID_LPF_HPF, channelL, freqL, slopeL};

        Log.d(TAG, String.format("nativeHLPF dataH : %s, dataL : %s", Arrays.toString(dataH), Arrays.toString(dataL)));

        NativeHelper.getEq().setEqHpfLpf(dataH);
        NativeHelper.getEq().setEqHpfLpf(dataL);

        // way2/way3时，需要一起发送 5 6
        if ((fyDspOutputMode == FyDspOutputMode.WAY2 && fyDspOutputChannel == FyDspOutputChannel.WAY2_SUBWOOFER_CENTER)
                || (fyDspOutputMode == FyDspOutputMode.WAY3 && fyDspOutputChannel == FyDspOutputChannel.WAY3_SUBWOOFER_CENTER)
                || (fyDspOutputMode == FyDspOutputMode.WAY6 && fyDspOutputChannel == FyDspOutputChannel.WAY6_SUBWOOFER_CENTER)) {
            dataH = new int[]{FY7604.FY_CMD_SUB_ID_LPF_HPF, 5, freqH, slopeH};
            dataL = new int[]{FY7604.FY_CMD_SUB_ID_LPF_HPF, 6, freqL, slopeL};

            Log.d(TAG, String.format("nativeHLPF dataH : %s, dataL : %s", Arrays.toString(dataH), Arrays.toString(dataL)));

            NativeHelper.getEq().setEqHpfLpf(dataH);
            NativeHelper.getEq().setEqHpfLpf(dataL);
        }
    }

    // 设置某输出模式下的所有通道
    public void nativeHLPFAllChannel(final FyDspOutputMode fyDspOutputMode) {
        FyDspOutputChannel[] channel;
        if (fyDspOutputMode == FyDspOutputMode.WAY2) {
            channel = new FyDspOutputChannel[]{FyDspOutputChannel.WAY2_F, FyDspOutputChannel.WAY2_R, FyDspOutputChannel.WAY2_SUBWOOFER_CENTER};
        } else if (fyDspOutputMode == FyDspOutputMode.WAY3) {
            channel = new FyDspOutputChannel[]{FyDspOutputChannel.WAY3_F, FyDspOutputChannel.WAY3_R, FyDspOutputChannel.WAY3_SUBWOOFER_CENTER};
        } else if (fyDspOutputMode == FyDspOutputMode.CHANNEL51) {
            channel = new FyDspOutputChannel[]{FyDspOutputChannel.CHANNEL51_F, FyDspOutputChannel.CHANNEL51_R, FyDspOutputChannel.CHANNEL51_CENTER, FyDspOutputChannel.CHANNEL51_SUBWOOFER};
        } else {
            channel = new FyDspOutputChannel[]{FyDspOutputChannel.WAY6_F, FyDspOutputChannel.WAY6_R, FyDspOutputChannel.WAY6_SUBWOOFER_CENTER};
        }

        Stream.of(channel).forEach(fyDspOutputChannel -> {
            int hpfFreq = getHPFFreq(fyDspOutputMode, fyDspOutputChannel);
            int lpfFreq = getLPFFreq(fyDspOutputMode, fyDspOutputChannel);
            int hpfSlope = getHPFSlope(fyDspOutputMode, fyDspOutputChannel);
            int lpfSlope = getLPFSlope(fyDspOutputMode, fyDspOutputChannel);
            nativeHLPF(fyDspOutputMode, fyDspOutputChannel
                    , FyDspHLPFFreq.findByValue(hpfFreq), FyDspHLPFFreq.findByValue(lpfFreq)
                    , FyDspHLPFSlope.findByValue(hpfSlope), FyDspHLPFSlope.findByValue(lpfSlope));
        });
    }

    public void saveFreq(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel
            , FyDspHLPFFreq fyDspHPFFreq, FyDspHLPFFreq fyDspLPFFreq) {
        Log.d(TAG, "saveFreq output mode : " + fyDspOutputMode + ", output channel : " + fyDspOutputChannel
                + ", hpf freq : " + fyDspHPFFreq + ", lpf freq : " + fyDspLPFFreq);
        spUtils.put("key_hpf_freq_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name(), fyDspHPFFreq.getFreq());
        spUtils.put("key_lpf_freq_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name(), fyDspLPFFreq.getFreq());
    }

    public int getHPFFreq(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel) {
        Log.d(TAG, "getHPFFreq output mode : " + fyDspOutputMode + ", output channel : " + fyDspOutputChannel);
        FyDspHLPFFreq fyDspHLPFFreq = getDefaultFreq(fyDspOutputMode, fyDspOutputChannel, true);
        return getSpUtils().getInt("key_hpf_freq_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name(), fyDspHLPFFreq.getFreq());
    }

    public int getLPFFreq(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel) {
        Log.d(TAG, "getLPFFreq output mode : " + fyDspOutputMode + ", output channel : " + fyDspOutputChannel);
        FyDspHLPFFreq fyDspHLPFFreq = getDefaultFreq(fyDspOutputMode, fyDspOutputChannel, false);
        return getSpUtils().getInt("key_lpf_freq_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name(), fyDspHLPFFreq.getFreq());
    }

    public void saveSlope(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel
            , FyDspHLPFSlope fyDspHPFSlope, FyDspHLPFSlope fyDspLPFSlope) {
        Log.d(TAG, "saveSlope output mode : " + fyDspOutputMode + ", output channel : " + fyDspOutputChannel
                + ", hpf slope : " + fyDspHPFSlope + ", lpf slope : " + fyDspLPFSlope);
        spUtils.put("key_hpf_slope_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name(), fyDspHPFSlope.getSlope());
        spUtils.put("key_lpf_slope_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name(), fyDspLPFSlope.getSlope());
    }

    public int getHPFSlope(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel) {
//        Log.d(TAG, "getHPFSlope output mode : " + fyDspOutputMode + ", output channel : " + fyDspOutputChannel);
        // slope 默认值
        FyDspHLPFSlope fyDspHLPFSlope = getDefaultSlope(fyDspOutputMode, fyDspOutputChannel, true);
        return getSpUtils().getInt("key_hpf_slope_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name(), fyDspHLPFSlope.getSlope());
    }

    public int getLPFSlope(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel) {
//        Log.d(TAG, "getLPFSlope output mode : " + fyDspOutputMode + ", output channel : " + fyDspOutputChannel);
        // slope 默认值
        FyDspHLPFSlope fyDspHLPFSlope = getDefaultSlope(fyDspOutputMode, fyDspOutputChannel, false);
        return getSpUtils().getInt("key_lpf_slope_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name(), fyDspHLPFSlope.getSlope());
    }

    public void resetFreq(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel) {
        spUtils.remove("key_hpf_freq_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name());
        spUtils.remove("key_lpf_freq_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name());
    }

    public void resetSlope(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel) {
        spUtils.remove("key_hpf_slope_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name());
        spUtils.remove("key_lpf_slope_" + fyDspOutputMode.name() + "_" + fyDspOutputChannel.name());
    }

    // 获取 slope 默认值
    private FyDspHLPFSlope getDefaultSlope(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel, boolean isHpf) {
        FyDspHLPFSlope fyDspHLPFSlope;
        if (fyDspOutputMode == FyDspOutputMode.WAY2) {
            fyDspHLPFSlope = FyDspHLPFSlope.SLOPE_12;
        } else if (fyDspOutputMode == FyDspOutputMode.WAY3) {
            fyDspHLPFSlope = FyDspHLPFSlope.SLOPE_12;
        } else if (fyDspOutputMode == FyDspOutputMode.CHANNEL51) {
            if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_F) {
                fyDspHLPFSlope = FyDspHLPFSlope.SLOPE_0;
            } else if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_R) {
                fyDspHLPFSlope = FyDspHLPFSlope.SLOPE_0;
            } else {
                fyDspHLPFSlope = FyDspHLPFSlope.SLOPE_12;
            }
        } else {
            if (fyDspOutputChannel == FyDspOutputChannel.WAY6_F) {
                fyDspHLPFSlope = FyDspHLPFSlope.SLOPE_0;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY6_R) {
                fyDspHLPFSlope = FyDspHLPFSlope.SLOPE_0;
            } else {
                fyDspHLPFSlope = isHpf ? FyDspHLPFSlope.SLOPE_12 : FyDspHLPFSlope.SLOPE_24;
            }
        }
        return fyDspHLPFSlope;
    }

    // 获取 freq 默认值
    private FyDspHLPFFreq getDefaultFreq(FyDspOutputMode fyDspOutputMode, FyDspOutputChannel fyDspOutputChannel, boolean isHpf) {
        FyDspHLPFFreq fyDspHLPFFreq;
        if (fyDspOutputMode == FyDspOutputMode.WAY2 || fyDspOutputMode == FyDspOutputMode.WAY3 || fyDspOutputMode == FyDspOutputMode.CHANNEL51) {
            // HPF 默认值为数据的第一项，LPF 默认值为数据的最后一项
            List<FyDspHLPFFreq> freqList = getFreqList(fyDspOutputMode, fyDspOutputChannel);
            return isHpf ? freqList.get(0) : freqList.get(freqList.size() - 1);
        } else {
            if (fyDspOutputChannel == FyDspOutputChannel.WAY6_F) {
                fyDspHLPFFreq = isHpf ? FyDspHLPFFreq.FREQ_4000 : FyDspHLPFFreq.FREQ_4000;
            } else if (fyDspOutputChannel == FyDspOutputChannel.WAY6_R) {
                fyDspHLPFFreq = isHpf ? FyDspHLPFFreq.FREQ_80 : FyDspHLPFFreq.FREQ_4000;
            } else {
                fyDspHLPFFreq = isHpf ? FyDspHLPFFreq.FREQ_20 : FyDspHLPFFreq.FREQ_80;
            }
        }
        return fyDspHLPFFreq;
    }
}
