package com.hcn.autoeq.util;

import android.util.Log;

import com.hcn.autoeq.bean.SIDspBiQuadParams;
import com.hcn.autoeq.bean.SIDspBiQuadParamsExt;
import com.hcn.autoeq.nativeextdsp.Ak7604;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class SI47925EqDealDataUtil {
    private static final String TAG = SI47925EqDealDataUtil.class.getSimpleName();

    static double PI = 3.14159265358979323846;
    static double FS = 48000;

    private static final int CROSSOVER_THRU = 0; //0
    private static final int CROSSOVER_6DB_LPF = 1; // 1
    private static final int CROSSOVER_6DB_HPF = 2; //2
    private static final int CROSSOVER_12DB_LPF = 3; //3
    private static final int CROSSOVER_12DB_HPF = 4; //4
    private static final int CROSSOVER_18DB_LPF = 5; //5
    private static final int CROSSOVER_18DB_HPF = 6; //6
    private static final int CROSSOVER_24DB_LPF = 7;  //7
    private static final int CROSSOVER_24DB_HPF = 8; // 8
    private static final int CROSSOVER_36DB_LPF = 9; // 8
    private static final int CROSSOVER_36DB_HPF = 10; // 10
    private static final int CROSSOVER_48DB_LPF = 11; // 11
    private static final int CROSSOVER_48DB_HPF = 12; //12;

    static NumFormat numFormat = new NumFormat(true, 16, 8);

    public enum FILTER_TYPE {
        FILTER_LOW_PASS,
        FILTER_HIGH_PASS,
        FILTER_BAND_PASS,
        FILTER_ALL_PASS,
        FILTER_NOTCH,
        FILTER_PEAK_EQ,
        FILTER_HIGH_SHELF,
        FILTER_LOW_SHELF,
    }


    public enum FILTER_PARAM_TYPE {
        FILTER_PARAM_Q,
        FILTER_PARAM_S,
        FILTER_PARAM_BW,
    }


    public static int calcDelayBinary(double delay) {
        return convert_to_binary_point(delay, numFormat.is_signed, numFormat.total_bits, numFormat.fractional_bits);
    }


    /**
     * 该方法的作用，就是为了使浮点数，转化为某制式的整数（二进制）
     * 如：
     *
     * @param value           5.5
     * @param is_signed       false
     * @param total_bits      16
     * @param fractional_bits 8
     * @return 0000 0101  1000 0000   ,即0x0580
     */
    public static int convert_to_binary_point(double value, boolean is_signed, int total_bits, int fractional_bits) {
        // The final binary point representation.
        long bin_rep = 0;
        int integral_bits = total_bits - fractional_bits;
        double remaining = value;

        // Decimal value of LSB
        double precision = 1.0 / (1 << fractional_bits);
        // Lower limit
        double lower_limit = 0;
        // Upper limit
        double upper_limit = 0;
        int i = 0;
        int mask = 0;

        if (is_signed) {
            lower_limit = -1 * (1 << (integral_bits - 1));
            upper_limit = ((1 << (total_bits - 1)) - 1) * precision;
        } else {
            upper_limit = ((1 << total_bits) - 1) * precision;
        }

        // bounds checking
        if (value <= lower_limit) {
            // return most negative value, which is msb only set

            return is_signed ? (1 << (total_bits - 1)) : 0;
        }

        if (value >= upper_limit) {

            if (is_signed) {
                // For signed values, largest is all bits set except MSB
                // 2^(total_bits-1) - 1
                return (1 << (total_bits - 1)) - 1;
            } else {
                // Unsigned max is all bits set.
                // 2^(total_bits) - 1
                return (1 << total_bits) - 1;
            }
        }

        // Value within bounds, continue checking.
        if (remaining < 0) {
            // most negative number in 2's complement.
            bin_rep = (1L << (total_bits - 1));
            // printf("bin_rep: %d %x remaining: %f %x \n", bin_rep, bin_rep, remaining,
            // remaining);
            remaining = (float) bin_rep + remaining;
            // printf("bin_rep: %d %x remaining: %f %x \n", bin_rep, bin_rep, remaining,
            // remaining);
        }

        // remaining is now a positive number to convert
        bin_rep = Math.round(remaining / precision);

        mask = 0;
        for (i = 0; i < total_bits; i++) {
            mask |= (1 << i);
        }
        return (int) (bin_rep & mask);
    }


    static class NumFormat {
        boolean is_signed;
        int total_bits;
        int fractional_bits;

        NumFormat(boolean is_signed, int total_bits, int fractional_bits) {
            this.is_signed = is_signed;
            this.total_bits = total_bits;
            this.fractional_bits = fractional_bits;
        }
    }

    public static SIDspBiQuadParams peakingCoef(FILTER_TYPE filterType, double filter, FILTER_PARAM_TYPE paramType, double param, double fc) {
        SIDspBiQuadParams result = new SIDspBiQuadParams();
        // Biquad coefficients
        double a0 = 1.0, a1 = 0.0, a2 = 0.0, b0 = 1.0, b1 = 0.0, b2 = 0.0;
        // Normalized biquad coefficients
        double a1N, a2N, b0N, b1N, b2N;

        double bnExtrema;
        int shift;

        double a = Math.pow(10, (filter / 40.0));
        double w0 = 2 * PI * fc / FS;
        double cs = Math.cos(w0);
        double sn = Math.sin(w0);

        double alpha = 0;
        double beta = 0;

        switch (paramType) {
            case FILTER_PARAM_Q:
                alpha = sn / (2.0 * param);
                break;
            case FILTER_PARAM_BW: {
                double q = fc / param;
                alpha = sn / (2.0 * q);
                //alpha = sn*Math.Sinh(NaturalLog(2)/2*param_value*w0/sn);
                break;
            }
            case FILTER_PARAM_S:
                alpha = sn / (2.0 * Math.sqrt((a + 1.0 / a) * (1.0 / param - 1) + 2));
                beta = Math.sqrt((a * a + 1) / param - (a - 1) * (a - 1));
                break;
        }

        switch (filterType) {
            case FILTER_LOW_PASS:
                b0 = (1 - cs) / 2.0;
                b1 = 1 - cs;
                b2 = (1 - cs) / 2.0;
                a0 = 1 + alpha;
                a1 = -2.0 * cs;
                a2 = 1 - alpha;
                break;
            case FILTER_HIGH_PASS:
                b0 = (1 + cs) / 2.0;
                b1 = -1 - cs;
                b2 = (1 + cs) / 2.0;
                a0 = 1 + alpha;
                a1 = -2 * cs;
                a2 = 1 - alpha;
                break;
            case FILTER_BAND_PASS:
                b0 = alpha;
                b1 = 0;
                b2 = -alpha;
                a0 = 1 + alpha;
                a1 = -2 * cs;
                a2 = 1 - alpha;
                break;
            case FILTER_NOTCH:
                b0 = 1;
                b1 = -2 * cs;
                b2 = 1;
                a0 = 1 + alpha;
                a1 = -2 * cs;
                a2 = 1 - alpha;
                break;
            case FILTER_ALL_PASS:
                b0 = 1 - alpha;
                b1 = -2 * cs;
                b2 = 1 + alpha;
                a0 = 1 + alpha;
                a1 = -2 * cs;
                a2 = 1 - alpha;
                break;
            case FILTER_PEAK_EQ:
                b0 = 1 + alpha * a;
                b1 = -2 * cs;
                b2 = 1 - alpha * a;
                a0 = 1 + alpha / a;
                a1 = -2 * cs;
                a2 = 1 - alpha / a;
                break;
            case FILTER_HIGH_SHELF:
                b0 = a * ((a + 1) + (a - 1) * cs + beta * sn);
                b1 = -2 * a * ((a - 1) + (a + 1) * cs);
                b2 = a * ((a + 1) + (a - 1) * cs - beta * sn);
                a0 = (a + 1) - (a - 1) * cs + beta * sn;
                a1 = 2 * ((a - 1) - (a + 1) * cs);
                a2 = (a + 1) - (a - 1) * cs - beta * sn;
                break;
            case FILTER_LOW_SHELF:
                b0 = a * ((a + 1) - (a - 1) * cs + beta * sn);
                b1 = 2 * a * ((a - 1) - (a + 1) * cs);
                b2 = a * ((a + 1) - (a - 1) * cs - beta * sn);
                a0 = (a + 1) + (a - 1) * cs + beta * sn;
                a1 = -2 * ((a - 1) + (a + 1) * cs);
                a2 = (a + 1) + (a - 1) * cs - beta * sn;
                break;
        }

        // Normalize to a0
        a1N = -a1 / a0;
        a2N = -a2 / a0;
        b0N = b0 / a0;
        b1N = b1 / a0;
        b2N = b2 / a0;

        bnExtrema = Math.max(Math.max(Math.abs(b0N), Math.abs(b1N)), Math.abs(b2N));

        shift = 0;
        if (bnExtrema > 2)
            shift = (int) Math.floor(Math.log(bnExtrema) / Math.log(2));
        else if (b0N >= 2)
            b0N = 1.9999999999999;
        else if (b1N >= 2)
            b1N = 1.9999999999999;
        else if (b2N >= 2)
            b2N = 1.9999999999999;

        //D_ASSERT_MSG(shift < 31, "ERROR: should use pow() instead to avoid errors!\n");
        b0N = b0N / (1 << shift);
        b1N = b1N / (1 << shift);
        b2N = b2N / (1 << shift);

        Log.d(TAG, String.format("1peakingCoef data,a1N:[%f],a2N:[%f],b0N:[%f],b1N:[%f],b2N:[%f],shift:[%d]", a1N, a2N, b0N, b1N, b2N, shift));
        result.setA1(convert_to_binary_point(a1N, true, 24, 22));
        result.setA2(convert_to_binary_point(a2N, true, 24, 22));
        result.setB0(convert_to_binary_point(b0N, true, 24, 22));
        result.setB1(convert_to_binary_point(b1N, true, 24, 22));
        result.setB2(convert_to_binary_point(b2N, true, 24, 22));
        result.setbShift(shift);
        Log.d(TAG, String.format("2peakingCoef data,a1N:[%x],a2N:[%x],b0N:[%x],b1N:[%x],b2N:[%x],shift:[%d]", result.getA1(), result.getA2(),
                result.getB0(), result.getB1(), result.getB2(), result.getbShift()));
        return result;

    }

    public static SIDspBiQuadParams lpfCalcBinary(double freq_hz) {
        return peakingCoef(FILTER_TYPE.FILTER_LOW_PASS, 0, FILTER_PARAM_TYPE.FILTER_PARAM_Q, 0.707, freq_hz);
    }

    public static SIDspBiQuadParams hpfCalcBinary(double freq_hz) {
        return peakingCoef(FILTER_TYPE.FILTER_HIGH_PASS, 0, FILTER_PARAM_TYPE.FILTER_PARAM_Q, 0.707, freq_hz);
    }

    public static List<SIDspBiQuadParamsExt> SIDspHpfLpfFilter(int slope, double freq_hz, double sample_rate) {
        double Q_value = 0.7071068;
        double temp_EQ_Q = 0;
        double EQ_p0 = 0;
        double EQ_p1 = 0;
        double a1 = 0;
        double Temp_p1 = 0;
        double fs = sample_rate;
        // Normalized biquad coefficients
        double a1N = 0, a2N = 0, b0N = 0, b1N = 0, b2N = 0;
        double bnExtrema = 0;
        int shift = 0;
        int i = 0;
        double EQ_a02 = 0, EQ_a01 = 0, EQ_b02 = 0, EQ_b01 = 0, EQ_a00 = 0;
        double EQ_a12 = 0, EQ_a11 = 0, EQ_b12 = 0, EQ_b11 = 0, EQ_a10 = 0;
        double EQ_a22 = 0, EQ_a21 = 0, EQ_b22 = 0, EQ_b21 = 0, EQ_a20 = 0;
        double EQ_a32 = 0, EQ_a31 = 0, EQ_b32 = 0, EQ_b31 = 0, EQ_a30 = 0;
        double EQ_W = 0, EQ_sn = 0, EQ_cs = 0, EQ_alpha = 0;
        SIDspBiQuadParamsExt reg_coeffs;
        List<SIDspBiQuadParamsExt> siDspBiQuadParamsList = new ArrayList<>();

        EQ_W = (2 * Math.PI * freq_hz) / sample_rate;
        EQ_sn = Math.sin(EQ_W);
        EQ_cs = Math.cos(EQ_W);
        EQ_alpha = EQ_sn / (2.0 * Q_value);

        if (slope == CROSSOVER_THRU || slope == CROSSOVER_6DB_HPF
                || slope == CROSSOVER_6DB_LPF || slope == CROSSOVER_12DB_HPF
                || slope == CROSSOVER_12DB_LPF) {
            switch (slope) {
                case CROSSOVER_THRU:
                    EQ_a02 = 0;
                    EQ_a01 = 0;
                    EQ_b02 = 0;
                    EQ_b01 = 0;
                    EQ_a00 = 1;
                    break;
                case CROSSOVER_6DB_LPF:
                    EQ_p1 = EQ_cs / (1 + EQ_sn);
                    Temp_p1 = EQ_cs / (1 + EQ_sn);      //LFP
                    a1 = (1 - Temp_p1) / 2.0;
                    EQ_a02 = 0;
                    EQ_a01 = (1 - EQ_p1) / 2.0;
                    EQ_b02 = 0;
                    EQ_b01 = EQ_p1;
                    EQ_a00 = a1;
                    break;
                case CROSSOVER_6DB_HPF:
                    EQ_p1 = (1 - EQ_sn) / EQ_cs;
                    Temp_p1 = (1 - EQ_sn) / EQ_cs;    //HFP
                    a1 = -(1 + Temp_p1) / 2.0;
                    EQ_p1 = (1 - EQ_sn) / EQ_cs;
                    EQ_a02 = 0.0;
                    EQ_a01 = -(1 + EQ_p1) / 2.0;
                    EQ_b02 = 0.0;
                    EQ_b01 = EQ_p1;
                    EQ_a00 = -a1;
                    break;
                case CROSSOVER_12DB_LPF:
                    EQ_p0 = 1 + EQ_alpha;
                    EQ_a02 = (1 - EQ_cs) / 2 / EQ_p0;
                    EQ_a01 = (1 - EQ_cs) / EQ_p0;
                    EQ_b02 = -(1 - EQ_alpha) / EQ_p0;
                    EQ_b01 = 2 * EQ_cs / EQ_p0;
                    EQ_a00 = (1 - EQ_cs) / 2 / EQ_p0;
                    break;
                case CROSSOVER_12DB_HPF:
                    EQ_p0 = 1 + EQ_alpha;
                    EQ_a02 = (1 + EQ_cs) / 2 / EQ_p0;
                    EQ_a01 = -(1 + EQ_cs) / EQ_p0;
                    EQ_b02 = -(1 - EQ_alpha) / EQ_p0;
                    EQ_b01 = 2 * EQ_cs / EQ_p0;
                    EQ_a00 = (1 + EQ_cs) / 2 / EQ_p0;
                    break;
                default:
                    break;
            }
            EQ_a12 = 0;
            EQ_a11 = 0;
            EQ_b12 = 0;
            EQ_b11 = 0;
            EQ_a10 = 1;
            EQ_a22 = 0;
            EQ_a21 = 0;
            EQ_b22 = 0;
            EQ_b21 = 0;
            EQ_a20 = 1;
            EQ_a32 = 0;
            EQ_a31 = 0;
            EQ_b32 = 0;
            EQ_b31 = 0;
            EQ_a30 = 1;
        } else if (slope == CROSSOVER_18DB_LPF) {
            double t0, t1, t2;
            t0 = Math.tan(Math.PI * freq_hz / fs);
            t1 = 1 + 2 * Math.cos(Math.PI / 3) * t0 + Math.pow(t0, 2);  //
            EQ_p1 = EQ_cs / (1 + EQ_sn);
            Temp_p1 = EQ_cs / (1 + EQ_sn);      //LFP
            a1 = (1 - Temp_p1) / 2.0;
            EQ_a02 = 0;
            EQ_a01 = (1 - EQ_p1) / 2.0;
            EQ_b02 = 0;
            EQ_b01 = EQ_p1;
            EQ_a00 = a1;
            EQ_a12 = Math.pow(t0, 2) / t1;   //EQ_a02=t0^2/t1;
            EQ_a11 = 2 * Math.pow(t0, 2) / t1;
            EQ_b12 = (2 * Math.cos(Math.PI / 3) * t0 - 1 - Math.pow(t0, 2)) / t1;
            EQ_b11 = 2 * (1 - Math.pow(t0, 2)) / t1;
            EQ_a10 = Math.pow(t0, 2) / t1;
            EQ_a22 = 0;
            EQ_a21 = 0;
            EQ_b22 = 0;
            EQ_b21 = 0;
            EQ_a20 = 1;
            EQ_a32 = 0;
            EQ_a31 = 0;
            EQ_b32 = 0;
            EQ_b31 = 0;
            EQ_a30 = 1;
        } else if (slope == CROSSOVER_18DB_HPF) {
            double t0, t1, t2;
            t0 = Math.tan(Math.PI * freq_hz / fs);
            t1 = 1 + 2 * Math.cos(1 * Math.PI / 3) * t0 + Math.pow(t0, 2);  //
            EQ_p1 = (1 - EQ_sn) / EQ_cs;
            Temp_p1 = (1 - EQ_sn) / EQ_cs;    //HFP
            a1 = -(1 + Temp_p1) / 2.0;
            EQ_p1 = (1 - EQ_sn) / EQ_cs;
            EQ_a02 = 0.0;
            EQ_a01 = -(1 + EQ_p1) / 2.0;
            EQ_b02 = 0.0;
            EQ_b01 = EQ_p1;
            EQ_a00 = -a1;
            EQ_a12 = 1 / t1;
            EQ_a11 = -2 / t1;
            EQ_b12 = (2 * Math.cos(Math.PI / 3) * t0 - 1 - Math.pow(t0, 2)) / t1;
            EQ_b11 = 2 * (1 - Math.pow(t0, 2)) / t1;
            EQ_a10 = 1 / t1;
            EQ_a22 = 0;
            EQ_a21 = 0;
            EQ_b22 = 0;
            EQ_b21 = 0;
            EQ_a20 = 1;
            EQ_a32 = 0;
            EQ_a31 = 0;
            EQ_b32 = 0;
            EQ_b31 = 0;
            EQ_a30 = 1;
        } else if (slope == CROSSOVER_24DB_LPF) {
            double t0, t1, t2;
            t0 = Math.tan(Math.PI * freq_hz / fs);
            t1 = 1 + 2 * Math.cos(Math.PI / 8) * t0 + Math.pow(t0, 2);  //
            t2 = 1 + 2 * Math.cos(3 * Math.PI / 8) * t0 + Math.pow(t0, 2);        //
            EQ_a02 = Math.pow(t0, 2) / t1;   //EQ_a02=t0^2/t1;
            EQ_a01 = 2 * Math.pow(t0, 2) / t1;
            EQ_b02 = (2 * Math.cos(Math.PI / 8) * t0 - 1 - Math.pow(t0, 2)) / t1;
            EQ_b01 = 2 * (1 - Math.pow(t0, 2)) / t1;
            EQ_a00 = Math.pow(t0, 2) / t1;
            EQ_a12 = Math.pow(t0, 2) / t2;
            EQ_a11 = 2 * Math.pow(t0, 2) / t2;
            EQ_b12 = (2 * Math.cos(3 * Math.PI / 8) * t0 - 1 - Math.pow(t0, 2)) / t2;
            EQ_b11 = 2 * (1 - Math.pow(t0, 2)) / t2;
            EQ_a10 = Math.pow(t0, 2) / t2;
            EQ_a22 = 0;
            EQ_a21 = 0;
            EQ_b22 = 0;
            EQ_b21 = 0;
            EQ_a20 = 1;
            EQ_a32 = 0;
            EQ_a31 = 0;
            EQ_b32 = 0;
            EQ_b31 = 0;
            EQ_a30 = 1;
        } else if (slope == CROSSOVER_24DB_HPF) {
            double t0, t1, t2;
            t0 = Math.tan(Math.PI * freq_hz / fs);
            t1 = 1 + 2 * Math.cos(1 * Math.PI / 8) * t0 + Math.pow(t0, 2);  //
            t2 = 1 + 2 * Math.cos(3 * Math.PI / 8) * t0 + Math.pow(t0, 2);        //
            EQ_a02 = 1 / t1;
            EQ_a01 = -2 / t1;
            EQ_b02 = (2 * Math.cos(Math.PI / 8) * t0 - 1 - Math.pow(t0, 2)) / t1;
            EQ_b01 = 2 * (1 - Math.pow(t0, 2)) / t1;
            EQ_a00 = 1 / t1;
            EQ_a12 = 1 / t2;
            EQ_a11 = -2 / t2;
            EQ_b12 = (2 * Math.cos(3 * Math.PI / 8) * t0 - 1 - Math.pow(t0, 2)) / t2;
            EQ_b11 = 2 * (1 - Math.pow(t0, 2)) / t2;
            EQ_a10 = 1 / t2;
            EQ_a22 = 0;
            EQ_a21 = 0;
            EQ_b22 = 0;
            EQ_b21 = 0;
            EQ_a20 = 1;
            EQ_a32 = 0;
            EQ_a31 = 0;
            EQ_b32 = 0;
            EQ_b31 = 0;
            EQ_a30 = 1;
        } else if (slope == CROSSOVER_36DB_LPF) {
            double t0, t1, t2, t3;
            t0 = Math.tan(Math.PI * freq_hz / fs);
            t1 = 1 + 2 * Math.cos(Math.PI / 12) * t0 + Math.pow(t0, 2);
            t2 = 1 + 2 * Math.cos(3 * Math.PI / 12) * t0 + Math.pow(t0, 2);
            t3 = 1 + 2 * Math.cos(5 * Math.PI / 12) * t0 + Math.pow(t0, 2);
            EQ_a02 = Math.pow(t0, 2) / t1;
            EQ_a01 = 2 * Math.pow(t0, 2) / t1;
            EQ_b02 = (2 * Math.cos(Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t1;
            EQ_b01 = 2 * (1 - Math.pow(t0, 2)) / t1;
            EQ_a00 = Math.pow(t0, 2) / t1;
            EQ_a12 = Math.pow(t0, 2) / t2;
            EQ_a11 = 2 * Math.pow(t0, 2) / t2;
            EQ_b12 = (2 * Math.cos(3 * Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t2;
            EQ_b11 = 2 * (1 - Math.pow(t0, 2)) / t2;
            EQ_a10 = Math.pow(t0, 2) / t2;
            EQ_a22 = Math.pow(t0, 2) / t3;
            EQ_a21 = 2 * Math.pow(t0, 2) / t3;
            EQ_b22 = (2 * Math.cos(5 * Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t3;
            EQ_b21 = 2 * (1 - Math.pow(t0, 2)) / t3;
            EQ_a20 = Math.pow(t0, 2) / t3;
            EQ_a32 = 0;
            EQ_a31 = 0;
            EQ_b32 = 0;
            EQ_b31 = 0;
            EQ_a30 = 1;

        } else if (slope == CROSSOVER_36DB_HPF) {
            double t0, t1, t2, t3;
            t0 = Math.tan(Math.PI * freq_hz / fs);
            t1 = 1 + 2 * Math.cos(Math.PI / 12) * t0 + Math.pow(t0, 2);
            t2 = 1 + 2 * Math.cos(3 * Math.PI / 12) * t0 + Math.pow(t0, 2);
            t3 = 1 + 2 * Math.cos(5 * Math.PI / 12) * t0 + Math.pow(t0, 2);
            EQ_a02 = 1 / t1;
            EQ_a01 = -2 / t1;
            EQ_b02 = (2 * Math.cos(Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t1;
            EQ_b01 = 2 * (1 - Math.pow(t0, 2)) / t1;
            EQ_a00 = 1 / t1;
            EQ_a12 = 1 / t2;
            EQ_a11 = -2 / t2;
            EQ_b12 = (2 * Math.cos(3 * Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t2;
            EQ_b11 = 2 * (1 - Math.pow(t0, 2)) / t2;
            EQ_a10 = 1 / t2;
            EQ_a22 = 1 / t3;
            EQ_a21 = -2 / t3;
            EQ_b22 = (2 * Math.cos(5 * Math.PI / 12) * t0 - 1 - Math.pow(t0, 2)) / t3;
            EQ_b21 = 2 * (1 - Math.pow(t0, 2)) / t3;
            EQ_a20 = 1 / t3;
            EQ_a32 = 0;
            EQ_a31 = 0;
            EQ_b32 = 0;
            EQ_b31 = 0;
            EQ_a30 = 1;
        } else if (slope == CROSSOVER_48DB_LPF) {
            double t0, t1, t2, t3, t4;
            t0 = Math.tan(Math.PI * freq_hz / fs);
            t1 = 1 + 2 * Math.cos(Math.PI / 16) * t0 + Math.pow(t0, 2);
            t2 = 1 + 2 * Math.cos(3 * Math.PI / 16) * t0 + Math.pow(t0, 2);
            t3 = 1 + 2 * Math.cos(5 * Math.PI / 16) * t0 + Math.pow(t0, 2);
            t4 = 1 + 2 * Math.cos(7 * Math.PI / 16) * t0 + Math.pow(t0, 2);
            EQ_a02 = Math.pow(t0, 2) / t1;
            EQ_a01 = 2 * Math.pow(t0, 2) / t1;
            EQ_b02 = (2 * Math.cos(Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t1;
            EQ_b01 = 2 * (1 - Math.pow(t0, 2)) / t1;
            EQ_a00 = Math.pow(t0, 2) / t1;
            EQ_a12 = Math.pow(t0, 2) / t2;
            EQ_a11 = 2 * Math.pow(t0, 2) / t2;
            EQ_b12 = (2 * Math.cos(3 * Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t2;
            EQ_b11 = 2 * (1 - Math.pow(t0, 2)) / t2;
            EQ_a10 = Math.pow(t0, 2) / t2;
            EQ_a22 = Math.pow(t0, 2) / t3;
            EQ_a21 = 2 * Math.pow(t0, 2) / t3;
            EQ_b22 = (2 * Math.cos(5 * Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t3;
            EQ_b21 = 2 * (1 - Math.pow(t0, 2)) / t3;
            EQ_a20 = Math.pow(t0, 2) / t3;
            EQ_a32 = Math.pow(t0, 2) / t4;
            EQ_a31 = 2 * (Math.pow(t0, 2) / t4);
            EQ_b32 = (2 * Math.cos(Math.PI * 7 / 16) * t0 - 1 - Math.pow(t0, 2)) / t4;
            EQ_b31 = 2 * (1 - Math.pow(t0, 2)) / t4;
            EQ_a30 = Math.pow(t0, 2) / t4;
        } else if (slope == CROSSOVER_48DB_HPF) {
            double t0, t1, t2, t3, t4;
            t0 = Math.tan(Math.PI * freq_hz / fs);
            t1 = 1 + 2 * Math.cos(Math.PI / 16) * t0 + Math.pow(t0, 2);
            t2 = 1 + 2 * Math.cos(3 * Math.PI / 16) * t0 + Math.pow(t0, 2);
            t3 = 1 + 2 * Math.cos(5 * Math.PI / 16) * t0 + Math.pow(t0, 2);
            t4 = 1 + 2 * Math.cos(7 * Math.PI / 16) * t0 + Math.pow(t0, 2);
            EQ_a02 = 1 / t1;
            EQ_a01 = -2 / t1;
            EQ_b02 = (2 * Math.cos(Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t1;
            EQ_b01 = 2 * (1 - Math.pow(t0, 2)) / t1;
            EQ_a00 = 1 / t1;
            EQ_a12 = 1 / t2;
            EQ_a11 = -2 / t2;
            EQ_b12 = (2 * Math.cos(3 * Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t2;
            EQ_b11 = 2 * (1 - Math.pow(t0, 2)) / t2;
            EQ_a10 = 1 / t2;
            EQ_a22 = 1 / t3;
            EQ_a21 = -2 / t3;
            EQ_b22 = (2 * Math.cos(5 * Math.PI / 16) * t0 - 1 - Math.pow(t0, 2)) / t3;
            EQ_b21 = 2 * (1 - Math.pow(t0, 2)) / t3;
            EQ_a20 = 1 / t3;
            EQ_a32 = 1 / t4;
            EQ_a31 = -(1 / t4) * 2;
            EQ_b32 = (2 * Math.cos(Math.PI * 7 / 16) * t0 - Math.pow(t0, 2) - 1) / t4;
            EQ_b31 = 2 * (1 - Math.pow(t0, 2)) / t4;
            EQ_a30 = 1 / t4;

        }
        for (i = 0; i < 4; i++) {
            if (0 == i) {
                a1N = EQ_b01;
                a2N = EQ_b02;
                b0N = EQ_a00;
                b1N = EQ_a01;
                b2N = EQ_a02;
            } else if (1 == i) {
                a1N = EQ_b11;
                a2N = EQ_b12;
                b0N = EQ_a10;
                b1N = EQ_a11;
                b2N = EQ_a12;
            } else if (2 == i) {
                a1N = EQ_b21;
                a2N = EQ_b22;
                b0N = EQ_a20;
                b1N = EQ_a21;
                b2N = EQ_a22;
            } else if (3 == i) {
                a1N = EQ_b31;
                a2N = EQ_b32;
                b0N = EQ_a30;
                b1N = EQ_a31;
                b2N = EQ_a32;
            }

            // Deal with values that are too large.
            bnExtrema = Math.max(Math.max(Math.abs(b0N), Math.abs(b1N)), Math.abs(b2N));

            shift = 0;
            if (bnExtrema > 2)
                shift = (int) Math.floor(Math.log(bnExtrema) / Math.log(2));
            else if (b0N >= 2)
                b0N = 1.9999999999999;
            else if (b1N >= 2)
                b1N = 1.9999999999999;
            else if (b2N >= 2)
                b2N = 1.9999999999999;

            //D_ASSERT_MSG(shift < 31, "ERROR: should use Math.pow() instead to avoid errors!\n");
            b0N = b0N / (1 << shift);
            b1N = b1N / (1 << shift);
            b2N = b2N / (1 << shift);
            reg_coeffs = new SIDspBiQuadParamsExt();
            reg_coeffs.setA1(a1N);
            reg_coeffs.setA2(a2N);
            reg_coeffs.setB0(b0N);
            reg_coeffs.setB1(b1N);
            reg_coeffs.setB2(b2N);
            reg_coeffs.setbShift(shift);
            siDspBiQuadParamsList.add(reg_coeffs);
        }

        return siDspBiQuadParamsList;
    }


    /**
     * 浮点数转16位的二进制
     *
     * @param x
     * @param shift
     * @return
     */
    public static int fix16(double x, int shift) {
        //x = x * (1 << (23 - shift));  // pow(2, 23 - shift);
        x = x * (Math.pow(2, 16 - shift));
        x = Math.min(32767, x);
        x = Math.max(-32768.0, x);
        if (x < 0)
            x += 65536;
        return (int) Math.max(0., Math.min(65535, x + 0.555));
    }


    /**
     * 浮点数转24位的二进制
     *
     * @param x
     * @param shift
     * @return
     */
    private static int fix24(double x, int shift) {
        x = x * (Math.pow(2, 24 - shift));
        x = Math.min(8388607.0, x);
        x = Math.max(-8388608.0, x);
        if (x < 0)
            x += 16777216;
        return (int) Math.max(0., Math.min(16777215.0, x + 0.555));
    }

}
