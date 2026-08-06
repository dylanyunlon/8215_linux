package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.hcn.autoeq.bean.FyDspBalanceMode;
import com.hcn.autoeq.bean.FyDspOutputMode;
import com.hcn.autoeq.nativeextdsp.FY7604;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantFyDsp;

import java.util.Arrays;

public class FyDspBalanceSettings extends FyDspBaseSettings implements ConstantFyDsp {

    private static final String TAG = FyDspBalanceSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(FyDspBalanceSettings.class.getSimpleName(), Log.DEBUG);

    private static final String FY_DSP_BALANCE_FILE = "v2_fy_dsp_balance"; // 各模式的均衡保存的文件名
    private static final String KEY_BALANCE_MODE = "fy_dsp_balance_mode"; // 均衡模式
    private static final String KEY_BALANCE_X = "fy_dsp_balance_x"; // 平衡x
    private static final String KEY_BALANCE_Y = "fy_dsp_balance_y"; // 平衡y
    private static final String KEY_UI_MODE = "fy_dsp_ui_mode"; // ui 模式（balance or delay）

    // UI 模式
    public enum UI_MODE {
        BALANCE, DELAY
    }

    private Context context;
    private static FyDspBalanceSettings extDspBalanceSettings = null;

    public static FyDspBalanceSettings getInstance(Context context) {
        if (null == extDspBalanceSettings) {
            extDspBalanceSettings = new FyDspBalanceSettings(context);
        }
        return extDspBalanceSettings;
    }

    private FyDspBalanceSettings(Context context) {
        super(FY_DSP_BALANCE_FILE);
        this.context = context;
    }

    public void saveBalanceOrDelayUIMode(UI_MODE uiMode) {
        spUtils.put(KEY_UI_MODE, uiMode.ordinal());
    }

    public int getBalanceOrDelayUIMode() {
        return spUtils.getInt(KEY_UI_MODE, UI_MODE.BALANCE.ordinal());
    }

    public void saveBalance(String mode, int x, int y) {
        spUtils.put(KEY_BALANCE_MODE, mode);
        spUtils.put(KEY_BALANCE_X, x - 7);
        spUtils.put(KEY_BALANCE_Y, 7 - y, true);
        Log.d(TAG, String.format("saveBalance mode : %s, x : %d, y : %d", mode, x, y));
    }

    public int[] getBalance() {
        int x = 0, y = 0;
        String balanceMode = getSpUtils().getString(KEY_BALANCE_MODE);
        if (FyDspBalanceMode.MAIN.getName().equals(balanceMode)) {
            x = FyDspBalanceMode.MAIN.getX();
            y = FyDspBalanceMode.MAIN.getY();
        } else if (FyDspBalanceMode.CO.getName().equals(balanceMode)) {
            x = FyDspBalanceMode.CO.getX();
            y = FyDspBalanceMode.CO.getY();
        } else if (FyDspBalanceMode.REAR.getName().equals(balanceMode)) {
            x = FyDspBalanceMode.REAR.getX();
            y = FyDspBalanceMode.REAR.getY();
        } else if (FyDspBalanceMode.WHOLE.getName().equals(balanceMode)) {
            x = getSpUtils().getInt(KEY_BALANCE_X, FyDspBalanceMode.WHOLE.getX());
            y = getSpUtils().getInt(KEY_BALANCE_Y, FyDspBalanceMode.WHOLE.getY());
        } else if (FyDspBalanceMode.LEFT.getName().equals(balanceMode)) {
            x = FyDspBalanceMode.LEFT.getX();
            y = FyDspBalanceMode.LEFT.getY();
        } else if (FyDspBalanceMode.CENTER.getName().equals(balanceMode)) {
            x = FyDspBalanceMode.CENTER.getX();
            y = FyDspBalanceMode.CENTER.getY();
        } else if (FyDspBalanceMode.RIGHT.getName().equals(balanceMode)) {
            x = FyDspBalanceMode.RIGHT.getX();
            y = FyDspBalanceMode.RIGHT.getY();
        }
        return new int[]{x + 7, 7 - y};
    }

    /**
     * 传入的参数：
     * 0,0     7,0      14,0
     * <p>
     * 0,7     7,7      14,7
     * <p>
     * 0,14   7,14      14,14
     */
    public void nativeBalance(int x, int y, FyDspOutputMode fyDspOutputMode) {
        x = x - 7;
        y = 7 - y;
        /**
         * 转换成±7后：
         * -7,7      0,7       7,7
         *
         * -7,0      0,0       7,0
         *
         * -7,-7     0,-7      7,-7
         */
        final int BALANCE_VALUE_MAX = 7;
        final int BALANCE_DISTAND = 7;
        final int BALANCE_FURTHEST = 49;

        int FL, FR, RL, RR, CENTER, SUBWOOFER;
        final int temp = 15;

        if ((x >= 0) && (y >= 0)) { // 右上角
            FL = (temp * (BALANCE_VALUE_MAX - x)) / BALANCE_DISTAND;
            FR = temp;
            RL = (temp * (BALANCE_VALUE_MAX - y) * (BALANCE_VALUE_MAX - x)) / BALANCE_FURTHEST;
            RR = (temp * (BALANCE_VALUE_MAX - y)) / BALANCE_DISTAND;
        } else if ((x >= 0) && (y < 0)) { // 右下角
            FL = (temp * (BALANCE_VALUE_MAX - x) * (y + BALANCE_VALUE_MAX)) / BALANCE_FURTHEST;
            FR = (temp * (y + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            RL = (temp * (BALANCE_VALUE_MAX - x)) / BALANCE_DISTAND;
            RR = temp;
        } else if ((x < 0) && (y >= 0)) { // 左上角
            FL = temp;
            FR = (temp * (x + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            RL = (temp * (BALANCE_VALUE_MAX - y)) / BALANCE_DISTAND;
            RR = (temp * (x + BALANCE_VALUE_MAX) * (BALANCE_VALUE_MAX - y)) / BALANCE_FURTHEST;
        } else { // 左下角
            FL = (temp * (y + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            FR = (temp * (x + BALANCE_VALUE_MAX) * (y + BALANCE_VALUE_MAX)) / BALANCE_FURTHEST;
            RL = temp;
            RR = (temp * (x + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
        }

        if (fyDspOutputMode == FyDspOutputMode.WAY6) { // 6路
            // CENTER 始终是15不变
            CENTER = temp;
            // SUBWOOFER 始终是15不变
            SUBWOOFER = temp;
        } else if (fyDspOutputMode == FyDspOutputMode.CHANNEL51) { // 5.1声道
            // 在中间位置时，中置是15，往前排（上）调不变，往后排（下）调是递减15->0?
            CENTER = (temp * (y + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            if (CENTER > temp) CENTER = temp;
            // SUBWOOFER 始终是15不变
            SUBWOOFER = temp;
        } else { // WAY 2 和 WAY 3
            // 在中间位置时，中置和重低音都是15
            // 越往左边，重低音越小，越往右中置越小
            CENTER = (temp * (BALANCE_VALUE_MAX - y) * (BALANCE_VALUE_MAX - x)) / BALANCE_FURTHEST;
            if (CENTER > temp) CENTER = temp;

            SUBWOOFER = (temp * (x + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            if (SUBWOOFER > temp) SUBWOOFER = temp;
        }

        int[] data = new int[]{FY7604.FY_CMD_SUB_ID_BALANCE, FL, FR, RL, RR, CENTER, SUBWOOFER};
        NativeHelper.getEq().setEqBalance(data);
        Log.d(TAG, String.format("nativeBalance data : %s", Arrays.toString(data)));
    }

}
