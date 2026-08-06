package com.hcn.autoeq.data;

import android.audio.AudioEffect;
import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantCscAsp;

import java.util.Arrays;

public class SIExtDspBalanceSettings implements ConstantCscAsp {
    private Context context;
    private SPUtils spUtils;
    private static SIExtDspBalanceSettings cscAspBalanceSettings = null;
    private static final String TAG = SIExtDspBalanceSettings.class.getSimpleName();


    private static final int SI_DSP_BALANCE_MAX = 15;
    private static final int SI_DSP_BALANCE_DISTANCE = 7;
    private static final String SI_EXT_DSP_BALANCE_FILE = "si_dsp_balance"; // 各模式的均衡保存的文件名

    private static final String SI_EXT_BALANCE_MODE = "si_dsp_balance_mode"; // 均衡模式
    private static final String SI_EXT_DSP_BALANCE_X = "si_dsp_balance_x"; // 平衡x
    private static final String SI_EXT_DSP_BALANCE_Y = "si_dsp_balance_y"; // 平衡y

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

    /**
     * asp协议接口
     */
    private AudioEffect mAudioEffect = null;

    private SIExtDspBalanceSettings(Context context) {
        this.context = context;
        if (null == mAudioEffect) {
            mAudioEffect = AudioEffect.getInstance();
        }
        spUtils = SPUtils.getInstance(SI_EXT_DSP_BALANCE_FILE);
    }

    public static SIExtDspBalanceSettings getInstance(Context mContext) {
        if (null == cscAspBalanceSettings) {
            cscAspBalanceSettings = new SIExtDspBalanceSettings(mContext);
        }
        return cscAspBalanceSettings;
    }

    /**
     * 记录真实位置到sp
     *
     * @param x
     * @param y
     */
    public void saveBalance(String mode, int x, int y) {
        spUtils.put(SI_EXT_BALANCE_MODE, mode);
        spUtils.put(SI_EXT_DSP_BALANCE_X, x - SI_DSP_BALANCE_DISTANCE);
        spUtils.put(SI_EXT_DSP_BALANCE_Y, SI_DSP_BALANCE_DISTANCE - y, true);
    }

    public int[] getBalance() {
        int x = 0, y = 0;
        String balanceMode = spUtils.getString(SI_EXT_BALANCE_MODE);
        if (ExtDspBalanceSettings.BALANCE_MODE.MAIN.getName().equals(balanceMode)) {
            x = ExtDspBalanceSettings.BALANCE_MODE.MAIN.getX();
            y = ExtDspBalanceSettings.BALANCE_MODE.MAIN.getY();
        } else if (ExtDspBalanceSettings.BALANCE_MODE.CO.getName().equals(balanceMode)) {
            x = ExtDspBalanceSettings.BALANCE_MODE.CO.getX();
            y = ExtDspBalanceSettings.BALANCE_MODE.CO.getY();
        } else if (ExtDspBalanceSettings.BALANCE_MODE.REAR.getName().equals(balanceMode)) {
            x = ExtDspBalanceSettings.BALANCE_MODE.REAR.getX();
            y = ExtDspBalanceSettings.BALANCE_MODE.REAR.getY();
        } else if (ExtDspBalanceSettings.BALANCE_MODE.WHOLE.getName().equals(balanceMode)) {
            x = spUtils.getInt(SI_EXT_DSP_BALANCE_X, ExtDspBalanceSettings.BALANCE_MODE.WHOLE.getX());
            y = spUtils.getInt(SI_EXT_DSP_BALANCE_Y, ExtDspBalanceSettings.BALANCE_MODE.WHOLE.getY());
        }
        return new int[]{x + SI_DSP_BALANCE_DISTANCE, SI_DSP_BALANCE_DISTANCE - y};
    }

    public void nativeBalance(int x, int y) {
        x = x - SI_DSP_BALANCE_DISTANCE;
        y = SI_DSP_BALANCE_DISTANCE - y;
        final int BALANCE_VALUE_MAX = SI_DSP_BALANCE_DISTANCE;
        final int BALANCE_DISTAND = SI_DSP_BALANCE_DISTANCE;
        final int BALANCE_FURTHEST = SI_DSP_BALANCE_DISTANCE * SI_DSP_BALANCE_DISTANCE;

        int FL, FR, RL, RR, CEN;
        int temp = SI_DSP_BALANCE_MAX;

        if ((x >= 0) && (y >= 0)) {
            FL = (temp * (BALANCE_VALUE_MAX - x)) / BALANCE_DISTAND;
            FR = temp;
            RL = (temp * (BALANCE_VALUE_MAX - y) * (BALANCE_VALUE_MAX - x)) / BALANCE_FURTHEST;
            RR = (temp * (BALANCE_VALUE_MAX - y)) / BALANCE_DISTAND;
            CEN = temp;
        } else if ((x >= 0) && (y < 0)) {
            FL = (temp * (BALANCE_VALUE_MAX - x) * (y + BALANCE_VALUE_MAX)) / BALANCE_FURTHEST;
            FR = (temp * (y + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            RL = (temp * (BALANCE_VALUE_MAX - x)) / BALANCE_DISTAND;
            RR = temp;
            CEN = (temp * (y + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
        } else if ((x < 0) && (y >= 0)) {
            FL = temp;
            FR = (temp * (x + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            RL = (temp * (BALANCE_VALUE_MAX - y)) / BALANCE_DISTAND;
            RR = (temp * (x + BALANCE_VALUE_MAX) * (BALANCE_VALUE_MAX - y)) / BALANCE_FURTHEST;
            CEN = temp;
        } else {
            FL = (temp * (y + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            FR = (temp * (x + BALANCE_VALUE_MAX) * (y + BALANCE_VALUE_MAX)) / BALANCE_FURTHEST;
            RL = temp;
            RR = (temp * (x + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            CEN = (temp * (y + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
        }

        int[] data = new int[]{FL, FR, RL, RR, CEN, 15};
        NativeHelper.getEq().setEqBalance(data);
        Log.d(TAG, String.format("nativeBalance data : %s", Arrays.toString(data)));
    }
}
