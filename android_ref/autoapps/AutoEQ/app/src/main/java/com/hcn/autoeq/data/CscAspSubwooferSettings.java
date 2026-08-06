package com.hcn.autoeq.data;

import android.audio.AudioEffect;
import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantCscAsp;

import java.util.Arrays;

public class CscAspSubwooferSettings implements ConstantCscAsp {
    public static final String CSC_ASP_LP_FFR_EQ = "-12";
    private static final String TAG = CscAspSubwooferSettings.class.getSimpleName();

    public static final int DEFAULT_CSC_ASP_SURROUND_FRE = 0;
    public static final int DEFAULT_CSC_ASP_SURROUND_LPF_PHASE_ON = 0;

    private static final String CSC_ASP_SURROUND_FILE = "csc_asp_surround_file"; // 环绕保存的文件名
    private static final String KEY_CSC_ASP_SURROUND_FRE = "key_csc_asp_surround_fre";

    private Context context;
    private static CscAspSubwooferSettings cscAspSubwooferSettings = null;

    private SPUtils spUtils;

    private CscAspSubwooferSettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(CSC_ASP_SURROUND_FILE);
    }

    public static CscAspSubwooferSettings getInstance(Context mContext) {
        if (null == cscAspSubwooferSettings) {
            cscAspSubwooferSettings = new CscAspSubwooferSettings(mContext);
        }
        return cscAspSubwooferSettings;
    }

    public void saveSurround(int fre) {
        Log.d(TAG, String.format("saveSurround enable : %d", fre));
        spUtils.put(KEY_CSC_ASP_SURROUND_FRE, fre);
        nativeSurround(fre);
    }


    public int getSurroundFre() {
        return spUtils.getInt(KEY_CSC_ASP_SURROUND_FRE, DEFAULT_CSC_ASP_SURROUND_FRE);
    }


    public void nativeSurround(int Fc) {
        int[] data = new int[]{Fc, DEFAULT_CSC_ASP_SURROUND_LPF_PHASE_ON};
        NativeHelper.getEq().setEqSurround(data);
        Log.d(TAG, String.format("csc_asp_nativeBalance data : %s", Arrays.toString(data)));
    }

}
