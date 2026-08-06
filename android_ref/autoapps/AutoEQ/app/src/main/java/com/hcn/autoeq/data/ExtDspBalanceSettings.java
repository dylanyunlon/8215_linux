package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantExtDsp;

import java.util.Arrays;

public class ExtDspBalanceSettings implements ConstantExtDsp {

    private static final String TAG = ExtDspBalanceSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(ExtDspBalanceSettings.class.getSimpleName(), Log.DEBUG);

    private static final String EXT_DSP_BALANCE_FILE = "ext_dsp_balance"; // 各模式的均衡保存的文件名
    private static final String KEY_BALANCE_MODE = "ext_dsp_balance_mode"; // 均衡模式
    private static final String KEY_BALANCE_X = "ext_dsp_balance_x"; // 平衡x
    private static final String KEY_BALANCE_Y = "ext_dsp_balance_y"; // 平衡y

    public enum BALANCE_MODE {
        MAIN("MAIN", 3, 3), CO("CO", 11, 3), REAR("REAR", 7, 11), WHOLE("WHOLE", 7, 7);

        private String name;
        private int x, y;

        BALANCE_MODE(String name, int x, int y) {
            this.name = name;
            this.x = x;
            this.y = y;
        }

        public String getName() {
            return name;
        }

        public int getX() {
            return x;
        }

        public int getY() {
            return y;
        }
    }

    private Context context;
    private static ExtDspBalanceSettings extDspBalanceSettings = null;
    private SPUtils spUtils;

    public static ExtDspBalanceSettings getInstance(Context mContext) {
        if (null == extDspBalanceSettings) {
            extDspBalanceSettings = new ExtDspBalanceSettings(mContext);
        }
        return extDspBalanceSettings;
    }

    private ExtDspBalanceSettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(EXT_DSP_BALANCE_FILE);
    }

    public void saveBalance(String mode, int x, int y) {
        spUtils.put(KEY_BALANCE_MODE, mode);
        spUtils.put(KEY_BALANCE_X, x - 7);
        spUtils.put(KEY_BALANCE_Y, 7 - y, true);
        Log.d(TAG, String.format("saveBalance mode : %s, x : %d, y : %d", mode, x, y));
    }

    public int[] getBalance() {
        int x = 0, y = 0;
        String balanceMode = spUtils.getString(KEY_BALANCE_MODE);
        if (BALANCE_MODE.MAIN.getName().equals(balanceMode)) {
            x = BALANCE_MODE.MAIN.getX();
            y = BALANCE_MODE.MAIN.getY();
        } else if (BALANCE_MODE.CO.getName().equals(balanceMode)) {
            x = BALANCE_MODE.CO.getX();
            y = BALANCE_MODE.CO.getY();
        } else if (BALANCE_MODE.REAR.getName().equals(balanceMode)) {
            x = BALANCE_MODE.REAR.getX();
            y = BALANCE_MODE.REAR.getY();
        } else if (BALANCE_MODE.WHOLE.getName().equals(balanceMode)) {
            x = spUtils.getInt(KEY_BALANCE_X, ExtDspBalanceSettings.BALANCE_MODE.WHOLE.getX());
            y = spUtils.getInt(KEY_BALANCE_Y, ExtDspBalanceSettings.BALANCE_MODE.WHOLE.getY());
        }
        return new int[]{x + 7, 7 - y};
    }

    public void nativeBalance(int x, int y) {
        x = x - 7;
        y = 7 - y;
        final int BALANCE_VALUE_MAX = 7;
        final int BALANCE_DISTAND = 7;
        final int BALANCE_FURTHEST = 49;

        int FL, FR, RL, RR;
        int temp = 15;

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


    public void nativeBalanceDouble(int x, int y) {
        x = x - 7;
        y = 7 - y;
        final int BALANCE_VALUE_MAX = 7;
        final int BALANCE_DISTAND = 7;
        final int BALANCE_FURTHEST = 49;
        int double_M = 2;

        int FL, FR, RL, RR, SUB, CEN;
        int temp = 15;
        SUB = temp;
        Log.d(TAG, "nativeBalanceDouble: y = " + y);
        if (y >= 0) {
            CEN = temp;
        } else {
            CEN = temp + double_M * y - 1;
        }
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

        int[] data = new int[]{FL, FR, RL, RR, SUB, CEN};
        Log.d(TAG, String.format("nativeBalance data : start %s", Arrays.toString(data)));
        NativeHelper.getEq().setEqBalance(data);
    }

}
