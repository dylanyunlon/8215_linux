
package com.hcn_library.util;

import java.lang.reflect.Array;
import java.util.Arrays;

public class Ak7739Utils {
    private static final String TAG = Ak7739Utils.class.getSimpleName();

    // 定义滤波器类型常量
    public static final int IIR2_LPF = 1;
    public static final int IIR2_HPF = 2;
    public static final int IIR2_BPF = 3;
    public static final int IIR2_NOTCH = 4;
    public static final int IIR2_PEAKING_EQ = 5;
    public static final int IIR2_LOW_SHELF = 6;
    public static final int IIR2_HIGH_SHELF = 7;
    public static final int IIR2_APF = 8;
    public static final int IIR2_1stLPF = 9;
    public static final int IIR2_1stHPF = 10;
    public static final int ALLPASSTHROUGH = 11;

    // 定义圆周率常量
    private static final double PI = 3.141592653589793238462643383279502884197456789;

    public static int fix32(double x, int shift) {
        // 缩放并四舍五入
        double scaledValue = x * (1L << (31 - shift));
        scaledValue += (x < 0) ? -0.5 : 0.5;

        // 范围限制
        int maxValue = (1 << 31) - 1;
        int minValue = -(1 << 31);
        scaledValue = Math.min(scaledValue, maxValue);
        scaledValue = Math.max(scaledValue, minValue);

        // 返回结果
        return (int) scaledValue;
    }

/*    public static int fix32(double x, int shift) {
        // 执行 x = x * pow(2, 31 - (float64_t)shift) + ((x < 0)? -0.5 : 0.5);
        x = x * Math.pow(2, 31 - (double) shift) + ((x < 0)? -0.5 : 0.5);

        // 执行 x = fmin(x, pow(2, 31) - 1);
        // Java 中使用 Math.min 方法来实现取最小值
        x = Math.min(x, Math.pow(2, 31) - 1);

        // 执行 x = fmax(x, -pow(2, 31));
        // Java 中使用 Math.max 方法来实现取最大值
        x = Math.max(x, -Math.pow(2, 31));

        // 执行 return (int32_t)x;
        return (int) x;
    }*/

/**
     * 将双精度浮点数进行缩放、取整和范围限制后转换为 32 位有符号整数
     *
     * @param x     输入的双精度浮点数
     * @param shift 缩放位移量
     * @return 转换后的 32 位有符号整数
     */

    public static int fix32H(double x, int shift) {
        // 缩放操作
        double scaledValue = x * Math.pow(2, 31 - shift);

        // 向下取整
        double flooredValue = Math.floor(scaledValue);

        // 范围限制
        int maxValue = (int) (Math.pow(2, 31) - 1);
        int minValue = -(int) Math.pow(2, 31);
        double limitedValue = Math.min(Math.max(flooredValue, minValue), maxValue);

        // 类型转换并返回结果
        return (int) limitedValue;
    }

/**
     * 处理双精度浮点数转换结果的低部分
     *
     * @param x     输入的双精度浮点数
     * @param shift 缩放位移量
     * @return 处理后低部分转换的 32 位有符号整数
     */

    public static int fix32L(double x, int shift) {
        // 缩放并减去高部分
        double scaledValue = x * Math.pow(2, 31 - shift);
        double lowPart = scaledValue - fix32H(x, shift);

        // 调用 fix32 函数进行最终转换
        return fix32(lowPart, 0);
    }

/**
     * 计算二阶 IIR 滤波器系数
     *
     * @param fc   滤波器的截止频率
     * @param type 滤波器的类型
     */

    public static int[] CalcIIR2Coef(double fc, int type) {
        int[] result = new int[5];
        double[] coeffTmp = new double[10];
        double z0, z1, z2, p0, p1, p2;
        double w, sn, cs, alpha;
        double fs = 48000;
        double Q = 0.707;

        // 计算中间变量
        w = 2 * PI * fc / fs;
        sn = Math.sin(w);
        cs = Math.cos(w);

        switch (type) {
            case IIR2_LPF:
                alpha = sn / (2 * Q);
                z0 = (1 - cs) / 2;
                z1 = 1 - cs;
                z2 = (1 - cs) / 2;
                p0 = 1 + alpha;
                p1 = -2 * cs;
                p2 = 1 - alpha;
                break;
            case IIR2_HPF:
                alpha = sn / (2 * Q);
                z0 = (1 + cs) / 2;
                z1 = -(1 + cs);
                z2 = (1 + cs) / 2;
                p0 = 1 + alpha;
                p1 = -2 * cs;
                p2 = 1 - alpha;
                break;
            case IIR2_1stLPF:
                p2 = z2 = 0.0;
                p1 = cs / (1 + sn);
                z0 = z1 = (1 - p1) / 2.0;
                p0 = -1.0;
                break;
            case IIR2_1stHPF:
                p2 = z2 = 0.0;
                p1 = (1 - sn) / cs;
                z0 = (1 + p1) / 2.0;
                z1 = -z0;
                p0 = -1.0;
                break;
            case ALLPASSTHROUGH:
            default:
                p1 = p2 = z1 = z2 = 0.0;
                p0 = z0 = 1.0;
                break;
        }

        // 计算归一化系数
        coeffTmp[0] = z2 / p0;
        coeffTmp[1] = z1 / p0;
        coeffTmp[2] = -p2 / p0;
        coeffTmp[3] = -p1 / p0;
        coeffTmp[4] = z0 / p0;

        // 将系数转换为固定点表示并存储到结果数组
        for (int i = 0; i < 5; i++) {
            result[i] = fix32(coeffTmp[i], 1);
        }
        return result;
    }


/**
     * 计算滤波器系数
     *
     * @param fc     截止频率
     * @param gain   增益
     * @param Q      品质因数
     * @param result 存储结果的数组
     * @param mode   模式，0 为单精度模式，其他为双精度模式
     */

    public static void cal_coef(double fc, double gain, double Q, int[] result, int mode) {
        if (result == null) {
            return;
        }

        double fs = 96000;
//        double A = Math.pow(10, gain / 40.0);
//        double omega = 2 * PI * fc / fs;
//        double sn = Math.sin(omega);
//        double cs = Math.cos(omega);
//        double alpha = sn / (2 * Q);
//
//        double p0 = 1 + (alpha / A);
//        double a2 = (1 - alpha * A) / p0;
//        double a1 = -2 * cs / p0;
//        double b2 = -(1 - alpha / A) / p0;
//        double b1 = 2 * cs / p0;
//        double a0 = (1 + alpha * A) / p0;

        System.out.printf("fc = %f, gain = %f, Q = %f\n", fc, gain, Q);

        double A = Math.pow(10, gain / 40.0);
        double omega = 2.0 * Math.PI * fc / fs;
        double sn = Math.sin(omega);
        double cs = Math.cos(omega);
        double alpha = sn / (2.0 * Q);

        double p0 = 1.0 + (alpha / A);
        double a2 = (1.0 - alpha * A) / p0;
        double a1 = -2.0 * cs / p0;
        double b2 = -(1.0 - alpha / A) / p0;
        double b1 = 2.0 * cs / p0;
        double a0 = (1.0 + alpha * A) / p0;

        if (gain == 0) {
            a2 = 0;
            a1 = 0;
            b2 = 0;
            b1 = 0;
            a0 = 1;
        }

        System.out.printf("%f :: b = %f, %f, %f, a = %f, %f\n", gain, a0, a1, a2, b1, b2);

        if (mode == 0) {
            // 单精度模式，从索引 2 开始存储结果
            if (result.length >= 7) {
                result[2] = fix32(a2, 1);
                result[3] = fix32(a1, 1);
                result[4] = fix32(b2, 1);
                result[5] = fix32(b1, 1);
                result[6] = fix32(a0, 1);

                if (fc == 0) {
                    result[2] = 0x000000;
                    result[3] = 0x000000;
                    result[4] = 0x000000;
                    result[5] = 0x000000;
                    result[6] = 0x400000;
                }
                System.out.printf("single:%08x, %08x, %08x, %08x, %08x\n", result[2], result[3], result[4], result[5], result[6]);
            }
        } else {
            // 双精度模式，从索引 2 开始存储结果
            if (result.length >= 12) {
                result[2] = fix32H(a2, 1);
                result[3] = fix32L(a2, 1);
                result[4] = fix32H(a1, 1);
                result[5] = fix32L(a1, 1);
                result[6] = fix32H(b2, 1);
                result[7] = fix32L(b2, 1);
                result[8] = fix32H(b1, 1);
                result[9] = fix32L(b1, 1);
                result[10] = fix32H(a0, 1);
                result[11] = fix32L(a0, 1);

                if (fc == 0) {
                    result[2] = 0x000000;
                    result[3] = 0x000000;
                    result[4] = 0x000000;
                    result[5] = 0x000000;
                    result[6] = 0x000000;
                    result[7] = 0x000000;
                    result[8] = 0x000000;
                    result[9] = 0x000000;
                    result[10] = 0x400000;
                    result[11] = 0x000000;
                }
                System.out.printf("double:%08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x\n",
                        result[2], result[3], result[4], result[5], result[6],
                        result[7], result[8], result[9], result[10], result[11]);
            }
        }
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

    //响度数值转换计算
    public static int loudnessLevelCal(double mg) {
        double tmp;
        int maxgain;

        if (mg >= 0) {
            tmp = mg / 32 / 6.02;
        } else {
            tmp = 1 - Math.pow(10, (mg / 20));
            tmp = 20 * Math.log10(tmp);
            tmp = tmp / 32 / 6.02;
        }
        maxgain = fix32(tmp, 0);
        return maxgain;
    }

    // 低频增强
    public static void bass_shelving(double fc, double Q, int[] data, int mode) {
        int[] result = new int[5];
        // 检查 result 数组是否为空
        if (result == null || result.length < 5) {
            System.err.println("Result array is null or not large enough.");
            return;
        }

        double fs = 96000;

        double omega = 2 * PI * fc / fs;
        double sn = Math.sin(omega);
        double cs = Math.cos(omega);
        double alpha = sn / (2 * Q);

//        double p1 = Math.cos(omega / 2) / Math.sin(omega / 2);
//        double a2 = 0.0;
//        double a1 = 1 / (1 + p1);
//        double b2 = 0.0;
//        double b1 = -(1 - p1) / (1 + p1);
//        double a0 = a1;

        double p0 = 1 + alpha;
        double a2 = (1 - cs) / (2 * p0);
        double a1 = (1 - cs) / p0;
        double b2 = -(1 - alpha) / p0;
        double b1 = 2 * (cs / p0);
        double a0 = (1 - cs) / (2 * p0);

        System.out.printf("hpf :: freq = %f :: %f, %f, %f, %f, %f\n", fc, a0, a1, a2, b1, b2);

        if (mode == 0) {
            // single precision mode
            result[0] = fix32(a2, 1);
            result[1] = fix32(a1, 1);
            result[2] = fix32(b2, 1);
            result[3] = fix32(b1, 1);
            result[4] = fix32(a0, 1);

            if (fc == 0) {
                result[0] = 0x000000;
                result[1] = 0x000000;
                result[2] = 0x000000;
                result[3] = 0x000000;
                result[4] = 0x400000;
            }
            System.out.printf("%s.single:%08x, %08x, %08x, %08x, %08x\n",
                    Thread.currentThread().getStackTrace()[1].getMethodName(),
                    result[0], result[1], result[2], result[3], result[4]);
        } else {
            // 处理其他模式
            System.out.printf("Unsupported mode: %d\n", mode);
        }
        System.arraycopy(result, 0, data, 1, 5);
    }


    public static void treble_shelving(double fc, double Q, int[] data, int mode) {
        int[] result = new int[5];
        // 检查结果数组是否为空或长度不足
        if (result == null || result.length < 5) {
            System.err.println("Result array is null or not large enough.");
            return;
        }

        // 采样率
        double fs = 96000;

        // 计算角频率
        double omega = 2 * PI * fc / fs;
        double sn = Math.sin(omega);
        double cs = Math.cos(omega);
        double alpha = sn / (2 * Q);

        // 计算滤波器系数
//        double p1 = Math.cos(omega / 2) / Math.sin(omega / 2);
//        double a2 = 0.0;
//        double a1 = p1 / (1 + p1);
//        double b2 = 0.0;
//        double b1 = -(1 - p1) / (1 + p1);
//        double a0 = p1 * (1 + p1);

        double p0 = 1 + alpha;
        double a2 = (1 + cs) / (2 * p0);
        double a1 = -(1 + cs) / p0;
        double b2 = -(1 - alpha) / p0;
        double b1 = 2 * cs / p0;
        double a0 = (1 + cs) / (2 * p0);

        // 打印滤波器系数信息
        System.out.printf("hpf :: freq = %f :: %f, %f, %f, %f, %f\n", fc, a0, a1, a2, b1, b2);

        if (mode == 0) {
            // 单精度模式
            result[0] = fix32(a2, 1);
            result[1] = fix32(a1, 1);
            result[2] = fix32(b2, 1);
            result[3] = fix32(b1, 1);
            result[4] = fix32(a0, 1);

            if (fc == 0) {
                result[0] = 0x000000;
                result[1] = 0x000000;
                result[2] = 0x000000;
                result[3] = 0x000000;
                result[4] = 0x400000;
            }
            // 打印单精度模式下转换后的系数
            System.out.printf("%s.single:%08x, %08x, %08x, %08x, %08x\n",
                    Thread.currentThread().getStackTrace()[1].getMethodName(),
                    result[0], result[1], result[2], result[3], result[4]);
        } else {
            // 处理不支持的模式
            System.out.printf("Unsupported mode: %d\n", mode);
        }
        System.arraycopy(result, 0, data, 1, 5);
    }


    public static int cal_gain(double gain) {
        double a = Math.pow(10, gain / 20.0);
        int data = fix32(a, 2);
        return data;
    }


    //滤波计算


    public static final int HPF = 0;
    public static final int LPF = 1;

    // 主函数
    public static void lpfHpfCalculate(int[] result, double fc, int slope, int type) {
        // 参数检查（根据需求取消注释）
        // if (result == null || (type != HPF && type != LPF) || !isValidSlope(slope)) {
        //     System.out.println("[AK7739] lpfHpfCalculate.params error.");
        //     return;
        // }

        Arrays.fill(result, 0); // 初始化数组为0

        double[] temp = new double[22];

        if (type == HPF) {
            switch (slope) {
                case 6:
                    calC6SlopeHPH1(result, fc);
                    break;
                case 12:
                    calC12SlopeHPH2(result, fc);
                    break;
                case 18:
                    calC18SlopeHPH3(result, fc);
                    break;
                case 24:
                    calC24SlopeHPH4(result, fc);
                    break;
                case 36:
                    calC36SlopeHPH6(result, fc);
                    break;
                case 48:
                    calC48SlopeHPH8(result, fc);
                    break;
                default:
                    setAllPassThrough(temp);
                    floatSwitchToInt32T(result, temp);
                    break;
            }
        } else if (type == LPF) {
            switch (slope) {
                case 6:
                    calC6SlopeLPH1(result, fc);
                    break;
                case 12:
                    calC12SlopeLPH2(result, fc);
                    break;
                case 18:
                    calC18SlopeLPH3(result, fc);
                    break;
                case 24:
                    calC24SlopeLPH4(result, fc);
                    break;
                case 36:
                    calC36SlopeLPH6(result, fc);
                    break;
                case 48:
                    calC48SlopeLPH8(result, fc);
                    break;
                default:
                    setAllPassThrough(temp);
                    floatSwitchToInt32T(result, temp);
                    break;
            }
        }

        // 打印结果（调试用）
        System.out.println();
        for (int i = 0; i < 20; ) {
            System.out.printf("result[%d]:0x%08x\t", i, result[i]);
            i++;
            if (i % 5 == 0) System.out.println();
        }
        System.out.println();
    }

    // 检查斜率是否有效
    private static boolean isValidSlope(int slope) {
        return slope == 6 || slope == 12 || slope == 18 || slope == 24 || slope == 36 || slope == 48;
    }

    // 设置全通滤波器系数
    private static void setAllPassThrough(double[] temp) {
        Arrays.fill(temp, 0.0);
        for (int i = 4; i < temp.length; i += 5) {
            temp[i] = 1.0;
        }
    }

    // 6dB/oct HPF
    private static void calC6SlopeHPH1(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double p1 = (1 - sn) / cs;

        result[0] = 0;
        result[1] = -(1 + p1) / 2;
        result[2] = 0;
        result[3] = p1;
        result[4] = (1 + p1) / 2;

        calcIIR2Coef(result, 5, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 10, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 15, fc, ALLPASSTHROUGH);

        floatSwitchToInt32T(target, result);
    }

    // 6dB/oct LPF
    private static void calC6SlopeLPH1(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double p0 = cs / (1 + sn);

        result[0] = 0;
        result[1] = (1 - p0) / 2;
        result[2] = 0;
        result[3] = p0;
        result[4] = (1 - p0) / 2;

        calcIIR2Coef(result, 5, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 10, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 15, fc, ALLPASSTHROUGH);

        floatSwitchToInt32T(target, result);
    }

    // 其他滤波器计算函数（类似实现，根据需要补充）
    private static void calC12SlopeHPH2(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double alpha = sn / (2 * Q);

        result[0] = (1 + cs) / 2 / (1 + alpha);
        result[1] = -(1 + cs) / (1 + alpha);
        result[2] = -(1 - alpha) / (1 + alpha);
        result[3] = 2 * cs / (1 + alpha);
        result[4] = (1 + cs) / 2 / (1 + alpha);

        calcIIR2Coef(result, 5, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 10, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 15, fc, ALLPASSTHROUGH);

        floatSwitchToInt32T(target, result);
    }

    private static void calC12SlopeLPH2(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double alpha = sn / (2 * Q);

        result[0] = (1 - cs) / 2 / (1 + alpha);
        result[1] = (1 - cs) / (1 + alpha);
        result[2] = -(1 - alpha) / (1 + alpha);
        result[3] = 2 * cs / (1 + alpha);
        result[4] = (1 - cs) / 2 / (1 + alpha);

        calcIIR2Coef(result, 5, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 10, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 15, fc, ALLPASSTHROUGH);

        floatSwitchToInt32T(target, result);
    }

    private static void calC18SlopeHPH3(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double t0 = Math.tan(Math.PI * fc / fs);
        double t1 = 1 + t0 + Math.pow(t0, 2);

        result[0] = 0;
        result[1] = -1 / (1 + t0);
        result[2] = 0;
        result[3] = (1 - t0) / (1 + t0);
        result[4] = 1 / (1 + t0);

        result[5] = 1 / t1;
        result[6] = -2 / t1;
        result[7] = (t0 - Math.pow(t0, 2) - 1)/ t1;
        result[8] = 2 * (1 - Math.pow(t0, 2)) / t1;
        result[9] = 1 / t1;

        calcIIR2Coef(result, 10, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 15, fc, ALLPASSTHROUGH);

        floatSwitchToInt32T(target, result);
    }

    private static void calC18SlopeLPH3(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double t0 = Math.tan(Math.PI * fc / fs);
        double t1 = 1 + t0 + Math.pow(t0, 2);

        result[0] = 0;
        result[1] = t0 / (1 + t0);
        result[2] = 0;
        result[3] = (1 - t0) / (1 + t0);
        result[4] = t0 / (1 + t0);

        result[5] = Math.pow(t0, 2) / t1;
        result[6] = 2 * Math.pow(t0, 2) / t1;
        result[7] = (t0 - Math.pow(t0, 2) - 1)/ t1;
        result[8] = 2 * (1 - Math.pow(t0, 2)) / t1;
        result[9] = Math.pow(t0, 2) / t1;

        calcIIR2Coef(result, 10, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 15, fc, ALLPASSTHROUGH);

        floatSwitchToInt32T(target, result);
    }

    private static void calC24SlopeHPH4(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double t0 = Math.tan(Math.PI * fc / fs);
        double t1 = 1 + 2 * Math.cos(Math.PI / 8) * t0 + Math.pow(t0, 2);
        double t2 = 1 + 2 * Math.cos(3 * Math.PI / 8) * t0 + Math.pow(t0, 2);

        result[0] = 1 / t1;
        result[1] = -2 / t1;
        result[2] = (2 * Math.cos(Math.PI / 8) * t0 - 1 - Math.pow(t0, 2)) / t1;
        result[3] = 2 * (1 - Math.pow(t0, 2)) / t1;
        result[4] = 1 / t1;

        result[5] = 1 / t2;
        result[6] = -2 / t2;
        result[7] = (2 * Math.cos(3 * Math.PI / 8) * t0 - 1 - Math.pow(t0, 2)) / t2;
        result[8] = 2 * (1 - Math.pow(t0, 2)) / t2;
        result[9] = 1 / t2;

        calcIIR2Coef(result, 10, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 15, fc, ALLPASSTHROUGH);

        floatSwitchToInt32T(target, result);
    }

    private static void calC24SlopeLPH4(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double t0 = Math.tan(Math.PI * fc / fs);
        double t1 = 1 + 2 * Math.cos(Math.PI / 8) * t0 + Math.pow(t0, 2);
        double t2 = 1 + 2 * Math.cos(3 * Math.PI / 8) * t0 + Math.pow(t0, 2);

        result[0] = Math.pow(t0, 2) / t1;
        result[1] = 2 * Math.pow(t0, 2) / t1;
        result[2] = (2 * Math.cos(Math.PI / 8) * t0 - 1 - Math.pow(t0, 2)) / t1;
        result[3] = 2 * (1 - Math.pow(t0, 2)) / t1;
        result[4] = Math.pow(t0, 2) / t1;

        result[5] = Math.pow(t0, 2) / t2;
        result[6] = 2 * Math.pow(t0, 2) / t2;
        result[7] = (2 * Math.cos(3 * Math.PI / 8) * t0 - 1 - Math.pow(t0, 2)) / t2;
        result[8] = 2 * (1 - Math.pow(t0, 2)) / t2;
        result[9] = Math.pow(t0, 2) / t2;

        calcIIR2Coef(result, 10, fc, ALLPASSTHROUGH);
        calcIIR2Coef(result, 15, fc, ALLPASSTHROUGH);

        floatSwitchToInt32T(target, result);
    }

    private static void calC36SlopeHPH6(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double t0 = Math.tan(Math.PI * fc / fs);
        double t1 = 1 + 2 * Math.cos(Math.PI / 12) * t0 + Math.pow(t0, 2);
        double t2 = 1 + 2 * Math.cos(3 * Math.PI / 12) * t0 + Math.pow(t0, 2);
        double t3 = 1 + 2 * Math.cos(5 * Math.PI / 12) * t0 + Math.pow(t0, 2);

        result[0] = 1 / t1;
        result[1] = -2 / t1;
        result[2] = (2 * Math.cos(Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t1;
        result[3] = 2 * (1 - Math.pow(t0, 2)) / t1;
        result[4] = 1 / t1;

        result[5] = 1 / t2;
        result[6] = -2 / t2;
        result[7] = (2 * Math.cos(3 * Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t2;
        result[8] = 2 * (1 - Math.pow(t0, 2)) / t2;
        result[9] = 1 / t2;

        result[10] = 1 / t3;
        result[11] = -2 / t3;
        result[12] = (2 * Math.cos(5 * Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t3;
        result[13] = 2 * (1 - Math.pow(t0, 2)) / t3;
        result[14] = 1 / t3;

        calcIIR2Coef(result, 15, fc, ALLPASSTHROUGH);

        floatSwitchToInt32T(target, result);
    }

    private static void calC36SlopeLPH6(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double t0 = Math.tan(Math.PI * fc / fs);
        double t1 = 1 + 2 * Math.cos(Math.PI / 12) * t0 + Math.pow(t0, 2);
        double t2 = 1 + 2 * Math.cos(3 * Math.PI / 12) * t0 + Math.pow(t0, 2);
        double t3 = 1 + 2 * Math.cos(5 * Math.PI / 12) * t0 + Math.pow(t0, 2);

        result[0] = Math.pow(t0, 2) / t1;
        result[1] = 2 * Math.pow(t0, 2) / t1;
        result[2] = (2 * Math.cos(Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t1;
        result[3] = 2 * (1 - Math.pow(t0, 2)) / t1;
        result[4] = Math.pow(t0, 2) / t1;

        result[5] = Math.pow(t0, 2) / t2;
        result[6] = 2 * Math.pow(t0, 2) / t2;
        result[7] = (2 * Math.cos(3 * Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t2;
        result[8] = 2 * (1 - Math.pow(t0, 2)) / t2;
        result[9] = Math.pow(t0, 2) / t2;

        result[10] = Math.pow(t0, 2) / t3;
        result[11] = 2 * Math.pow(t0, 2) / t3;
        result[12] = (2 * Math.cos(5 * Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t3;
        result[13] = 2 * (1 - Math.pow(t0, 2)) / t3;
        result[14] = Math.pow(t0, 2) / t3;

        calcIIR2Coef(result, 15, fc, ALLPASSTHROUGH);

        floatSwitchToInt32T(target, result);
    }

    private static void calC48SlopeHPH8(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double t0 = Math.tan(Math.PI * fc / fs);
        double t1 = 1 + 2 * Math.cos(Math.PI / 16) * t0 + Math.pow(t0, 2);
        double t2 = 1 + 2 * Math.cos(3 * Math.PI / 16) * t0 + Math.pow(t0, 2);
        double t3 = 1 + 2 * Math.cos(5 * Math.PI / 16) * t0 + Math.pow(t0, 2);
        double t4 = 1 + 2 * Math.cos(7 * Math.PI / 16) * t0 + Math.pow(t0, 2);

        result[0] = 1 / t1;
        result[1] = -2 / t1;
        result[2] = (2 * Math.cos(Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t1;
        result[3] = 2 * (1 - Math.pow(t0, 2)) / t1;
        result[4] = 1 / t1;

        result[5] = 1 / t2;
        result[6] = -2 / t2;
        result[7] = (2 * Math.cos(3 * Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t2;
        result[8] = 2 * (1 - Math.pow(t0, 2)) / t2;
        result[9] = 1 / t2;

        result[10] = 1 / t3;
        result[11] = -2 / t3;
        result[12] = (2 * Math.cos(5 * Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t3;
        result[13] = 2 * (1 - Math.pow(t0, 2)) / t3;
        result[14] = 1 / t3;

        result[15] = 1 / t4;
        result[16] = -2 / t4;
        result[17] = (2 * Math.cos(7 * Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t4;
        result[18] = 2 * (1 - Math.pow(t0, 2)) / t4;
        result[19] = 1 / t4;

        floatSwitchToInt32T(target, result);
    }

    private static void calC48SlopeLPH8(int[] target, double fc) {
        double[] result = new double[20];
        double fs = 96000.0;
        double Q = 0.707;
        double w = 2 * Math.PI * fc / fs;
        double sn = Math.sin(w);
        double cs = Math.cos(w);
        double t0 = Math.tan(Math.PI * fc / fs);
        double t1 = 1 + 2 * Math.cos(Math.PI / 16) * t0 + Math.pow(t0, 2);
        double t2 = 1 + 2 * Math.cos(3 * Math.PI / 16) * t0 + Math.pow(t0, 2);
        double t3 = 1 + 2 * Math.cos(5 * Math.PI / 16) * t0 + Math.pow(t0, 2);
        double t4 = 1 + 2 * Math.cos(7 * Math.PI / 16) * t0 + Math.pow(t0, 2);

        result[0] = Math.pow(t0, 2) / t1;
        result[1] = 2 * Math.pow(t0, 2) / t1;
        result[2] = (2 * Math.cos(Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t1;
        result[3] = 2 * (1 - Math.pow(t0, 2)) / t1;
        result[4] = Math.pow(t0, 2) / t1;

        result[5] = Math.pow(t0, 2) / t2;
        result[6] = 2 * Math.pow(t0, 2) / t2;
        result[7] = (2 * Math.cos(3 * Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t2;
        result[8] = 2 * (1 - Math.pow(t0, 2)) / t2;
        result[9] = Math.pow(t0, 2) / t2;

        result[10] = Math.pow(t0, 2) / t3;
        result[11] = 2 * Math.pow(t0, 2) / t3;
        result[12] = (2 * Math.cos(5 * Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t3;
        result[13] = 2 * (1 - Math.pow(t0, 2)) / t3;
        result[14] = Math.pow(t0, 2) / t3;

        result[15] = Math.pow(t0, 2) / t4;
        result[16] = 2 * Math.pow(t0, 2) / t4;
        result[17] = (2 * Math.cos(7 * Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t4;
        result[18] = 2 * (1 - Math.pow(t0, 2)) / t4;
        result[19] = Math.pow(t0, 2) / t4;

        floatSwitchToInt32T(target, result);
    }

        // IIR二阶滤波器系数计算
        private static void calcIIR2Coef(double[] result, int offset, double fc, int type) {
            if (type == ALLPASSTHROUGH) {
                for (int i = 0; i < 5; i++) {
                    if (offset + i < result.length) {
                        result[offset + i] = (i == 4) ? 1.0 : 0.0;
                    }
                }
            } else {
                // 其他滤波器类型的实现（根据需求补充）
            }
        }

        // 浮点数转Q31格式整数
        private static void floatSwitchToInt32T(int[] target, double[] result) {
            for (int i = 0; i < target.length && i < result.length; i++) {
                target[i] = fix32(result[i],1);
            }
        }

//        // 浮点数转Q31格式
//        private static int fix32(double value) {
//            return (int) (value * (1L << 31)); // Q31格式转换
//        }

/*        // 主函数测试
        public static void main(String[] args) {
            int[] result = new int[20];
            double fc = 1000.0; // 截止频率
            int slope = 6; // 斜率
            int type = HPF; // 滤波器类型

            lpfHpfCalculate(result, fc, slope, type);
        }
    }*/
}

