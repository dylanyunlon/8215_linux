package com.hcn_library.data;

import static com.hcn_library.util.EqUtils.KEY_SKIN;

import android.audio.AudioEffect;
import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn_library.nativeextdsp.NativeHelper;
import com.hcn_library.util.Ak7739Utils;
import com.hcn_library.util.ConstantCscAsp;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.SystemUtils;

import java.util.Arrays;


public class NineDspBalanceSettings implements ConstantCscAsp {
    // 存储平衡模式的键名
    private static final String BALANCE_MODE_KEY = "nine_dsp_balance_mode";
    // 平衡计算中的距离常量，可能用于坐标转换等操作
    private static final int BALANCE_DISTANCE = 7;
    // 存储平衡设置的 SharedPreferences 文件名称
    private static final String BALANCE_SETTINGS_FILE = "nine_dsp_balance";
    // 平衡计算中的最大值，可能用于范围限定等操作
    private static final int BALANCE_MAX = 15;
    // 存储平衡设置中 X 坐标的键名
    private static final String BALANCE_X_KEY = "nine_dsp_balance_x";
    // 存储平衡设置中 Y 坐标的键名
    private static final String BALANCE_Y_KEY = "nine_dsp_balance_y";
    // 日志标签，方便调试和追踪
    private static final String TAG = "NineDspBalanceSettings";
    // 单例对象
    private static NineDspBalanceSettings instance;
    // 上下文对象，用于资源访问等操作
    private Context context;
    // 音频效果对象，可能用于音频处理
    private AudioEffect audioEffect;
    // 用于存储和读取 SharedPreferences 数据的工具类实例
    private SPUtils sharedPreferencesUtils = SPUtils.getInstance(BALANCE_SETTINGS_FILE);


    // 平衡模式的枚举类型，包含不同模式及其坐标信息
    public enum BALANCE_MODE {
        // 主模式，可能代表一种默认或主要的平衡模式
        MAIN("MAIN", 3, 3),
        // 协同模式，可能代表一种协同处理的平衡模式
        CO("CO", 11, 3),
        // 后置模式，可能代表一种强调后置声道的平衡模式
        REAR("REAR", 7, 11),
        // 整体模式，可能代表一种整体平衡的模式
        WHOLE("WHOLE", 7, 7);

        private String name;
        private int x;
        private int y;

        BALANCE_MODE(String str, int i, int i2) {
            name = str;
            x = i;
            y = i2;
        }

        // 获取平衡模式的名称
        public String getName() {
            return name;
        }

        // 获取平衡模式的 X 坐标
        public int getX() {
            return x;
        }

        // 获取平衡模式的 Y 坐标
        public int getY() {
            return y;
        }
    }


    // 构造函数，初始化上下文和音频效果对象
    private NineDspBalanceSettings(Context context) {
        this.context = context;
        if (null == audioEffect) {
            audioEffect = AudioEffect.getInstance();
        }
    }


    // 获取单例实例
    public static NineDspBalanceSettings getInstance(Context context) {
        if (instance == null) {
            instance = new NineDspBalanceSettings(context);
        }
        return instance;
    }


    // 保存平衡设置到 SharedPreferences
    public void saveBalance(String balanceMode, int balanceX, int balanceY) {
        // 存储平衡模式
        sharedPreferencesUtils.put(BALANCE_MODE_KEY, balanceMode);
        // 存储调整后的 X 坐标，可能基于 BALANCE_DISTANCE 进行了某种转换
        sharedPreferencesUtils.put(BALANCE_X_KEY, balanceX - BALANCE_DISTANCE);
        // 存储调整后的 Y 坐标，可能基于 BALANCE_DISTANCE 进行了某种转换
        sharedPreferencesUtils.put(BALANCE_Y_KEY, BALANCE_DISTANCE - balanceY, true);
        Log.d(TAG, "saveBalance x: " +  + (balanceX - BALANCE_DISTANCE) + " y: " + (BALANCE_DISTANCE - balanceY));
    }


    // 获取平衡设置
    public int[] getBalance() {
        // 从 SharedPreferences 中获取存储的平衡模式名称
        String modeName = sharedPreferencesUtils.getString(BALANCE_MODE_KEY);
        int x, y;
        // 根据不同的平衡模式设置对应的 X 和 Y 坐标
        if (BALANCE_MODE.MAIN.getName().equals(modeName)) {
            x = BALANCE_MODE.MAIN.getX();
            y = BALANCE_MODE.MAIN.getY();
        } else if (BALANCE_MODE.CO.getName().equals(modeName)) {
            x = BALANCE_MODE.CO.getX();
            y = BALANCE_MODE.CO.getY();
        } else if (BALANCE_MODE.REAR.getName().equals(modeName)) {
            x = BALANCE_MODE.REAR.getX();
            y = BALANCE_MODE.REAR.getY();
        } else if (BALANCE_MODE.WHOLE.getName().equals(modeName)) {
            // 对于 WHOLE 模式，可能从 SharedPreferences 中获取 X 和 Y 坐标，若不存在则使用默认值
            x = sharedPreferencesUtils.getInt(BALANCE_X_KEY, BALANCE_MODE.WHOLE.getX());
            y = sharedPreferencesUtils.getInt(BALANCE_Y_KEY, BALANCE_MODE.WHOLE.getY());
        } else {
            x = 0;
            y = 0;
        }
        Log.d(TAG, "getBalance balanceMode: " + modeName + " x: " + x + " y: " + y);
        // 对获取的坐标进行调整后返回
        return new int[]{x + BALANCE_DISTANCE, BALANCE_DISTANCE - y};
    }


    // 调用 Native 方法设置平衡，根据输入的坐标计算并设置平衡参数
    public void nativeBalance(int inputX, int inputY) {
        if ("gb05".equals(EqUtils.getSkinName())) {
            nativeBalanceDouble(inputX, inputY);
            return;
        }

        // 对输入的 X 坐标进行调整
        int adjustedX = inputX - BALANCE_DISTANCE;
        // 对输入的 Y 坐标进行调整
        int adjustedY = BALANCE_DISTANCE - inputY;
        // 平衡计算中的最大值
        final int MAX_BALANCE_VALUE = BALANCE_DISTANCE;
        // 平衡计算中的距离常量
        final int DISTANCE = BALANCE_DISTANCE;
        // 平衡计算中的距离平方
        final int DISTANCE_SQUARED = BALANCE_DISTANCE * BALANCE_DISTANCE;

        int frontLeft, frontRight, rearLeft, rearRight, center;
        int temp = BALANCE_MAX;

        // 根据调整后的 X 和 Y 坐标的正负情况，计算不同声道的平衡参数
        if ((adjustedX >= 0) && (adjustedY >= 0)) {
            frontLeft = (temp * (MAX_BALANCE_VALUE - adjustedX)) / DISTANCE;
            frontRight = temp;
            rearLeft = (temp * (MAX_BALANCE_VALUE - adjustedY) * (MAX_BALANCE_VALUE - adjustedX)) / DISTANCE_SQUARED;
            rearRight = (temp * (MAX_BALANCE_VALUE - adjustedY)) / DISTANCE;
            center = temp;
        } else if ((adjustedX >= 0) && (adjustedY < 0)) {
            frontLeft = (temp * (MAX_BALANCE_VALUE - adjustedX) * (adjustedY + MAX_BALANCE_VALUE)) / DISTANCE_SQUARED;
            frontRight = (temp * (adjustedY + MAX_BALANCE_VALUE)) / DISTANCE;
            rearLeft = (temp * (MAX_BALANCE_VALUE - adjustedX)) / DISTANCE;
            rearRight = temp;
            center = (temp * (adjustedY + MAX_BALANCE_VALUE)) / DISTANCE;
        } else if ((adjustedX < 0) && (adjustedY >= 0)) {
            frontLeft = temp;
            frontRight = (temp * (adjustedX + MAX_BALANCE_VALUE)) / DISTANCE;
            rearLeft = (temp * (MAX_BALANCE_VALUE - adjustedY)) / DISTANCE;
            rearRight = (temp * (adjustedX + MAX_BALANCE_VALUE) * (MAX_BALANCE_VALUE - adjustedY)) / DISTANCE_SQUARED;
            center = temp;
        } else {
            frontLeft = (temp * (adjustedY + MAX_BALANCE_VALUE)) / DISTANCE;
            frontRight = (temp * (adjustedX + MAX_BALANCE_VALUE) * (adjustedY + MAX_BALANCE_VALUE)) / DISTANCE_SQUARED;
            rearLeft = temp;
            rearRight = (temp * (adjustedX + MAX_BALANCE_VALUE)) / DISTANCE;
            center = (temp * (adjustedY + MAX_BALANCE_VALUE)) / DISTANCE;
        }

        // 构建平衡数据数组并调用 Native 方法设置平衡
        int[] balanceData = new int[]{frontLeft, frontRight, rearLeft, rearRight, center, 15};
//        if (EqUtils.isGB02()) {
//            nativeBalance02(balanceData);
//            return;
//        }
        NativeHelper.getEq().setEqBalance(balanceData);
        Log.d(TAG, String.format("nativeBalance data : %s", Arrays.toString(balanceData)));
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

    private void nativeBalance02(int[] data) {
        int minGain = -144;
        int unit = 5;
        data[0] = data[0] == 0 ? Ak7739Utils.cal_gain(minGain) : Ak7739Utils.cal_gain((data[0] < 7 ? data[0] - 7 : 0) * unit);
        data[1] = data[1] == 0 ? Ak7739Utils.cal_gain(minGain) : Ak7739Utils.cal_gain((data[1] < 7 ? data[1] - 7 : 0) * unit);
        data[2] = data[2] == 0 ? Ak7739Utils.cal_gain(minGain) : Ak7739Utils.cal_gain((data[2] < 7 ? data[2] - 7 : 0) * unit);
        data[3] = data[3] == 0 ? Ak7739Utils.cal_gain(minGain) : Ak7739Utils.cal_gain((data[3] < 7 ? data[3] - 7 : 0) * unit);
        data[4] = data[4] == 0 ? Ak7739Utils.cal_gain(minGain) : Ak7739Utils.cal_gain((data[4] < 7 ? data[4] - 7 : 0) * unit);
        data[5] = Ak7739Utils.cal_gain(0);
        NativeHelper.getEq().setEqBalance(data);
        Log.d(TAG, String.format("nativeBalance02 data : %s", Arrays.toString(data)));
    }
}