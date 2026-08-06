package com.hcn.autoeq.data;

import static com.hcn.autoeq.util.ConstantCscAsp.DEF_CSC_ASP_HIGH_QVALUE;
import static com.hcn.autoeq.util.ConstantCscAsp.DEF_CSC_ASP_QVALUE;
import static com.hcn.autoeq.util.ConstantCscAsp.EXT_CSC_ASP_REVERB_SIZE;
import static com.hcn.autoeq.util.ConstantCscAsp.EXT_CSC_ASP_REVERB_USER0;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;

import java.util.Arrays;

public class CscAspQValueSettings {
    private static final String TAG = CscAspQValueSettings.class.getSimpleName();
    private static final String EXT_CSC_ASP_Q_VALUE_FILE = "ext_csc_asp_q_value"; // 各模式的Q值保存的文件名
    private static final String EXT_CSC_ASP_Q_VALUE_LOW = "csc_asp_balance_Low"; // 低音
    private static final String EXT_CSC_ASP_Q_VALUE_MIDDLE = "csc_asp_balance_middle"; // 中音
    private static final String EXT_CSC_ASP_Q_VALUE_HIGH = "csc_asp_balance_high"; // 高音
    private Context context;

    private SPUtils spUtils;

    private int[] qValue = {1, 1, 1};

    private static CscAspQValueSettings cscAspQValueSettings = null;

    private CscAspQValueSettings(Context context) {
        this.context = context;
    }

    public static CscAspQValueSettings getInstance(Context mContext) {
        if (null == cscAspQValueSettings) {
            cscAspQValueSettings = new CscAspQValueSettings(mContext);

        }
        return cscAspQValueSettings;
    }

    /**
     * 仅有用户模式才能改变Q值
     */
    public void setCscQValue(int low, int middle, int high) {
        Log.i(TAG, "setCscQValue: "+context);
        int reverb = CscAspEqualizerChartSettings.getInstance(context).getReverb();
        String fileName;
        if (reverb < EXT_CSC_ASP_REVERB_SIZE) {
            Log.i(TAG, "setCscQValue: "+context);
            CscAspEqualizerChartSettings.getInstance(context).saveReverb(EXT_CSC_ASP_REVERB_USER0);
            fileName = EXT_CSC_ASP_Q_VALUE_FILE + EXT_CSC_ASP_REVERB_USER0;
        } else {
            fileName = EXT_CSC_ASP_Q_VALUE_FILE + reverb;
        }
        qValue[0] = low;
        qValue[1] = middle;
        qValue[2] = high;
        spUtils = SPUtils.getInstance(fileName);
        spUtils.put(EXT_CSC_ASP_Q_VALUE_LOW, low);
        spUtils.put(EXT_CSC_ASP_Q_VALUE_MIDDLE, middle);
        spUtils.put(EXT_CSC_ASP_Q_VALUE_HIGH, high);
        Log.d(TAG, "setCscQ:" + Arrays.toString(qValue));
        Log.i(TAG, "setCscQValue: "+context);
        CscAspEqualizerChartSettings.getInstance(context).nativeAllBand();
    }

    public int[] getCscQValue(int reverb) {
        String fileName;
        if (reverb < EXT_CSC_ASP_REVERB_SIZE) {
            qValue[0] = DEF_CSC_ASP_QVALUE;
            qValue[1] = DEF_CSC_ASP_QVALUE;
            qValue[2] = DEF_CSC_ASP_HIGH_QVALUE;
        } else {
            fileName = EXT_CSC_ASP_Q_VALUE_FILE + reverb;
            spUtils = SPUtils.getInstance(fileName);
            qValue[0] = spUtils.getInt(EXT_CSC_ASP_Q_VALUE_LOW, DEF_CSC_ASP_QVALUE);
            qValue[1] = spUtils.getInt(EXT_CSC_ASP_Q_VALUE_MIDDLE, DEF_CSC_ASP_QVALUE);
            qValue[2] = spUtils.getInt(EXT_CSC_ASP_Q_VALUE_HIGH, DEF_CSC_ASP_HIGH_QVALUE);
        }
        Log.d(TAG, "getCscQ:" + Arrays.toString(qValue));
        return qValue;
    }

}
