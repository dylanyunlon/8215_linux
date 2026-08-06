package com.hcn.autoeq.data;

import android.audio.AudioEffect;
import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantCscAsp;
import com.hcn.autoeq.util.EqUtils;

import java.util.Arrays;

public class CscAspBalanceSettings implements ConstantCscAsp {
    private Context context;
    private SPUtils spUtils;
    private static CscAspBalanceSettings cscAspBalanceSettings = null;
    private static final String TAG = CscAspBalanceSettings.class.getSimpleName();

    public static final int CSC_ASP_BALANCE_DISTANCE = 9;
    private static final String EXT_CSC_ASP_BALANCE_FILE = "csc_asp_balance"; // 各模式的均衡保存的文件名
    private static final String KEY_CSC_ASP_BALANCE_X = "csc_asp_balance_x"; // 平衡x
    private static final String KEY_CSC_ASP_BALANCE_Y = "csc_asp_balance_y"; // 平衡y

    /**
     * asp协议接口
     */
    private AudioEffect mAudioEffect = null;

    private CscAspBalanceSettings(Context context) {
        this.context = context;
        if (null == mAudioEffect) {
            mAudioEffect = AudioEffect.getInstance();
        }
        spUtils = SPUtils.getInstance(EXT_CSC_ASP_BALANCE_FILE);
    }

    public static CscAspBalanceSettings getInstance(Context mContext) {
        if (null == cscAspBalanceSettings) {
            cscAspBalanceSettings = new CscAspBalanceSettings(mContext);
        }
        return cscAspBalanceSettings;
    }

    /**
     * 记录真实位置到sp
     *
     * @param x
     * @param y
     */
    public void saveCscAspBalance(int x, int y) {
        spUtils.put(KEY_CSC_ASP_BALANCE_X, x - CSC_ASP_BALANCE_DISTANCE);
        spUtils.put(KEY_CSC_ASP_BALANCE_Y, CSC_ASP_BALANCE_DISTANCE - y, true);
        Log.d(TAG, String.format("saveCscAspBalance, x : %d, y : %d", x, y));
    }


    /**
     * 获取真实位置到sp
     */
    public int[] getCscAspBalance() {
        int x = 0, y = 0;
        x = spUtils.getInt(KEY_CSC_ASP_BALANCE_X, 0);
        y = spUtils.getInt(KEY_CSC_ASP_BALANCE_Y, 0);
        Log.d(TAG, String.format("getCscAspBalance, x : %d, y : %d", x, y));
        return new int[]{x + CSC_ASP_BALANCE_DISTANCE, CSC_ASP_BALANCE_DISTANCE - y};
    }

    public void setCscAspBalance(int x, int y, Boolean saveVal) {
        x = x - CSC_ASP_BALANCE_DISTANCE;
        y = CSC_ASP_BALANCE_DISTANCE - y;
        final int BALANCE_VALUE_MAX = CSC_ASP_BALANCE_DISTANCE;
        final int BALANCE_DISTAND = CSC_ASP_BALANCE_DISTANCE;
        final int BALANCE_FURTHEST = CSC_ASP_BALANCE_DISTANCE * CSC_ASP_BALANCE_DISTANCE;

        int FL, FR, RL, RR;
        int temp = EqUtils.BALANCE_DEPTH;

        if ((x >= 0) && (y >= 0)) {
            FL = (temp * (BALANCE_VALUE_MAX - x)) / BALANCE_DISTAND;
            FR = temp;
            RL = (temp * (BALANCE_VALUE_MAX - y) * (BALANCE_VALUE_MAX - x)) / BALANCE_FURTHEST;
            RR = (temp * (BALANCE_VALUE_MAX - y)) / BALANCE_DISTAND;
        } else if ((x >= 0) && (y < 0)) {
            FL = (temp * (BALANCE_VALUE_MAX - x) * (y + BALANCE_VALUE_MAX)) / BALANCE_FURTHEST;
            FR = (temp * (y + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            RL = (temp * (BALANCE_VALUE_MAX - x)) / BALANCE_DISTAND;
            RR = temp;
        } else if ((x < 0) && (y >= 0)) {
            FL = temp;
            FR = (temp * (x + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            RL = (temp * (BALANCE_VALUE_MAX - y)) / BALANCE_DISTAND;
            RR = (temp * (x + BALANCE_VALUE_MAX) * (BALANCE_VALUE_MAX - y)) / BALANCE_FURTHEST;
        } else {
            FL = (temp * (y + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            FR = (temp * (x + BALANCE_VALUE_MAX) * (y + BALANCE_VALUE_MAX)) / BALANCE_FURTHEST;
            RL = temp;
            RR = (temp * (x + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
        }

        int[] data = new int[]{FL, FR, RL, RR};

        NativeHelper.getEq().setEqBalance(data);
        Log.d(TAG, String.format("nativeBalance data : %s", Arrays.toString(data)));
    }
}
