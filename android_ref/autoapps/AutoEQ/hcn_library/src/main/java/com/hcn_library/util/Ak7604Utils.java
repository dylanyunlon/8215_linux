package com.hcn_library.util;

import android.util.Log;

import java.util.Arrays;

public class Ak7604Utils {
    private static final String TAG = Ak7604Utils.class.getSimpleName();

    public static int[] calculate(double fc, double gain, double Q, int mode, int[] result) {
        double fs = 48000;

        double A = Math.pow(10., gain / 40.0);

        double omega = 2.0 * Math.PI * fc / fs;
        double sn = Math.sin(omega);
        double cs = Math.cos(omega);
        double alpha = sn / (2.0 * Q);


        double p0 = 1. + (alpha / A);
        double a2 = (1. - alpha * A) / p0;
        double a1 = -2. * cs / p0;
        double b2 = -(1. - alpha / A) / p0;
        double b1 = 2. * cs / p0;
        double a0 = (1. + alpha * A) / p0;

        Log.d(TAG, "hpf: gain = " + gain + " p0 = " + p0 + " a0 = " + a0 + " a1 = " + a1
                + " a2 = " + a2 + " b1 = " + b1 + " b2 = " + b2);
        if (mode == 0) {
            // Math.single precision mode
            result[0] = fix24(a2, 1);
            result[1] = fix24(a1, 1);
            result[2] = fix24(b2, 1);
            result[3] = fix24(b1, 1);
            result[4] = fix24(a0, 1);

            if (gain == 0) {
                result[0] = 0x000000;
                result[1] = 0x000000;
                result[2] = 0x000000;
                result[3] = 0x000000;
                result[4] = 0x400000;
            }
        } else {
            //double precision mode result[0~9]
            result[0] = fix24H(a2, 1);
            result[1] = fix24L(a2, 1);
            result[2] = fix24H(a1, 1);
            result[3] = fix24L(a1, 1);
            result[4] = fix24H(b2, 1);
            result[5] = fix24L(b2, 1);
            result[6] = fix24H(b1, 1);
            result[7] = fix24L(b1, 1);
            result[8] = fix24H(a0, 1);
            result[9] = fix24L(a0, 1);

            if (gain == 0) {
                result[0] = 0x000000;
                result[1] = 0x000000;
                result[2] = 0x000000;
                result[3] = 0x000000;
                result[4] = 0x000000;
                result[5] = 0x000000;
                result[6] = 0x000000;
                result[7] = 0x000000;
                result[8] = 0x400000;
                result[9] = 0x000000;
            }
        }
        Log.d(TAG, "calculate: " + Arrays.toString(result));
        return result;
    }

    public static int fix24(double x, int shift) {
        x = x * (1 << (23 - shift));
        x = Math.min(8388607., x);
        x = Math.max(-8388608., x);

        if (x < 0) {
            x += 16777216;
        }
        return (int) Math.max(0., Math.min(16777215., x + 0.5));

    }

    public static int fix24H(double x, int shift) {
        x = x * (1 << (15 - shift));
        x = Math.min(32767.99998474, x);
        x = Math.max(-32768, x);

        if (x < 0) x += 65536;
        x = (int) Math.max(0., Math.min(65535., x));
        return (int) x * 256;
    }


    public static int fix24L(double x, int shift) {
        x = x * (1 << (15 - shift));
        x = Math.min(32767.99998474, x);
        x = Math.max(-32768, x);
        if (x < 0) x += 65536;
        x = (x - (int) (x)) * 8388608;
        return (int) Math.max(0., Math.min(8388607., x + 0.5));
    }


    public static int[] hpf(double fc, int mode) {
        int[] result;
        double fs = 48000;
        double Q = 0.707;
        double omega = 2. * Math.PI * fc / fs;
        double sn = Math.sin(omega);
        double cs = Math.cos(omega);
        double alpha = sn / (2. * Q);

        double p0 = 1. + alpha;
        double a2 = (1. + cs) / 2. / p0;
        double a1 = -(1. + cs) / p0;
        double b2 = -(1. - alpha) / p0;
        double b1 = 2. * cs / p0;
        double a0 = (1. + cs) / 2. / p0;

        Log.d(TAG, "hpf: p0 = " + p0 + " a0 = " + a0 + " a1 = " + a1
                + " a2 = " + a2 + " b1 = " + b1 + " b2 = " + b2);
        if (mode == 0) {
            // Math.single precision mode
            result = new int[5];
            result[0] = fix24(a2, 1);
            result[1] = fix24(a1, 1);
            result[2] = fix24(b2, 1);
            result[3] = fix24(b1, 1);
            result[4] = fix24(a0, 1);

            if (fc == 0) {
                result[0] = 0x000000;
                result[1] = 0x000000;
                result[2] = 0x000000;
                result[3] = 0x000000;
                result[4] = 0x400000;
            }
        } else {
            result = new int[10];
            //double precision mode result[0~9]
            result[0] = fix24H(a2, 1);
            result[1] = fix24L(a2, 1);
            result[2] = fix24H(a1, 1);
            result[3] = fix24L(a1, 1);
            result[4] = fix24H(b2, 1);
            result[5] = fix24L(b2, 1);
            result[6] = fix24H(b1, 1);
            result[7] = fix24L(b1, 1);
            result[8] = fix24H(a0, 1);
            result[9] = fix24L(a0, 1);

            if (fc == 0) {
                result[0] = 0x000000;
                result[1] = 0x000000;
                result[2] = 0x000000;
                result[3] = 0x000000;
                result[4] = 0x000000;
                result[5] = 0x000000;
                result[6] = 0x000000;
                result[7] = 0x000000;
                result[8] = 0x400000;
                result[9] = 0x000000;
            }
        }
        Log.d(TAG, "hpf: " + translateTo16(result));
        return result;
    }


    public static int[] lpf(double fc, int mode) {
        int[] result;
        double Q = 0.707;
        double fs = 48000;

        double omega = 2. * Math.PI * fc / fs;
        double sn = Math.sin(omega);
        double cs = Math.cos(omega);
        double alpha = sn / (2. * Q);

        double p0 = 1. + alpha;
        double a2 = (1. - cs) / 2. / p0;
        double a1 = (1. - cs) / p0;
        double b2 = -(1. - alpha) / p0;
        double b1 = 2. * cs / p0;
        double a0 = (1. - cs) / 2. / p0;

        Log.d(TAG, "lpf: p0 = " + p0 + " a0 = " + a0 + " a1 = " + a1
            + " a2 = " + a2 + " b1 = " + b1 + " b2 = " + b2);

        if (mode == 0) {
            result = new int[5];
            // single precision mode
            result[0] = fix24(a2, 1);
            result[1] = fix24(a1, 1);
            result[2] = fix24(b2, 1);
            result[3] = fix24(b1, 1);
            result[4] = fix24(a0, 1);

            if (fc == 0) {
                result[0] = 0x000000;
                result[1] = 0x000000;
                result[2] = 0x000000;
                result[3] = 0x000000;
                result[4] = 0x400000;
            }
        } else {
            result = new int[10];
            //double precision mode result[0~9]
            result[0] = fix24H(a2, 1);
            result[1] = fix24L(a2, 1);
            result[2] = fix24H(a1, 1);
            result[3] = fix24L(a1, 1);
            result[4] = fix24H(b2, 1);
            result[5] = fix24L(b2, 1);
            result[6] = fix24H(b1, 1);
            result[7] = fix24L(b1, 1);
            result[8] = fix24H(a0, 1);
            result[9] = fix24L(a0, 1);
            if (fc == 0) {
                result[0] = 0x000000;
                result[1] = 0x000000;
                result[2] = 0x000000;
                result[3] = 0x000000;
                result[4] = 0x000000;
                result[5] = 0x000000;
                result[6] = 0x000000;
                result[7] = 0x000000;
                result[8] = 0x400000;
                result[9] = 0x000000;
            }
        }
        Log.d(TAG, "lpf: " + translateTo16(result));
        return result;
    }

    public static void hpfCalculate(int slope, int[] fc_table, int freq, int[] data) {
        for (int i = 0; i < 6; i++) {
            fc_table[i] = 0;    //设置直通
        }
        if (slope > 6) {
            slope = 6;
        }
        for (int i = 0; i < slope; i++) {
            fc_table[i] = freq;   //滤波器叠加，设置斜率
        }
        for (int i = 0; i < 6; i++) {
            //6 个高通滤波器 前5个double precision mode最后一个single
            int tempFreq = fc_table[i];
            if (i < 5) {
                int[] tempValue = hpf(tempFreq, 1); //计算前5个双精度滤波器
                for (int j = 0; j < tempValue.length; j++) {
                    data[2 + i * 10 + j] = tempValue[j];
                }
            } else {
                int[] tempValue = hpf(tempFreq, 0);//计算后1个单精度滤波器
                for (int j = 0; j < tempValue.length; j++) {
                    data[52 + j] = tempValue[j];
                }
            }
        }
        Log.d(TAG, "hpfCalculate: " + Arrays.toString(data));
    }


    public static void lpfCalculate(int slope, int[] fc_table, int freq, int[] data) {
        for (int i = 0; i < 6; i++) {
            fc_table[i] = 0;    //设置直通
        }
        if (slope > 6)
            slope = 6;
        for (int i = 0; i < slope; i++) {
            fc_table[i] = freq;   //滤波器叠加，设置斜率
        }
        for (int i = 0; i < 6; i++) {
            int tempFreq = fc_table[i];
            //6 个高通滤波器 前5个double precision mode最后一个single
            int[] tempValue = lpf(tempFreq, 0);//计算后1个单精度滤波器
            for (int j = 0; j < tempValue.length; j++) {
                data[2 + i * 5 + j] = tempValue[j];
            }
        }
        Log.d(TAG, "lpfCalculate: " + Arrays.toString(data));
    }

    public static void hpfSubCalculate(int slope, int[] fc_table, int freq, int[] data) {
        for (int i = 0; i < 6; i++) {
            fc_table[i] = 0;    //设置直通
        }
        if (slope > 6)
            slope = 6;
        for (int i = 0; i < slope; i++) {
            fc_table[i] = freq;   //滤波器叠加，设置斜率
        }
        for (int i = 0; i < 6; i++) {
            int tempFreq = fc_table[i];
            //6 个高通滤波器 前4个double precision mode 后2个single
            if (i < 4) {
                int[] tempValue = hpf(tempFreq, 1); //计算前4个双精度滤波器
                for (int j = 0; j < tempValue.length; j++) {
                    data[2 + i * 10 + j] = tempValue[j];
                }
            } else {
                int[] tempValue = hpf(tempFreq, 0);//计算后2个单精度滤波器
                for (int j = 0; j < tempValue.length; j++) {
                    data[42 + (i - 4) * 5 + j] = tempValue[j];
                }
            }
        }
        Log.d(TAG, "hpfSubCalculate: " + Arrays.toString(data));
    }

    public static void lpfSubCalculate(int slope, int[] fc_table, int freq, int[] data) {
        for (int i = 0; i < 6; i++) {
            fc_table[i] = 0;    //设置直通
        }
        if (slope > 6)
            slope = 6;
        for (int i = 0; i < slope; i++) {
            fc_table[i] = freq;   //滤波器叠加，设置斜率
        }
        for (int i = 0; i < 6; i++) {
            int tempFreq = fc_table[i];
            //6 个高通滤波器 前4个double precision mode 后2个single
            if (i < 4) {
                int[] tempValue = lpf(tempFreq, 1); //计算前4个双精度滤波器
                for (int j = 0; j < tempValue.length; j++) {
                    data[2 + i * 10 + j] = tempValue[j];
                }
            } else {
                int[] tempValue = lpf(tempFreq, 0);//计算后2个单精度滤波器
                for (int j = 0; j < tempValue.length; j++) {
                    data[42 + (i - 4) * 5 + j] = tempValue[j];
                }
            }
        }
        Log.d(TAG, "lpfSubCalculate: " + Arrays.toString(data));
    }


    public static StringBuffer translateTo16(int[] data) {
        StringBuffer buffer = new StringBuffer();
        for (int b : data) {
            String hex = String.format("%02X", b); // 将byte转换为16进制字符串
            buffer.append(hex);
            buffer.append("  ");
        }
        return buffer;
    }
}
