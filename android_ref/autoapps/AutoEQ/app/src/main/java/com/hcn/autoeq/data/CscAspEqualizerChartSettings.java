package com.hcn.autoeq.data;

import static com.hcn.autoeq.util.ConstantCscAsp.BAND_TOTAL;
import static com.hcn.autoeq.util.ConstantCscAsp.CSC_ASP_DEFAULT_LOUDNESS;
import static com.hcn.autoeq.util.ConstantCscAsp.DEF_CSC_ASP_EQ_36_BANDS_VALUES;
import static com.hcn.autoeq.util.ConstantCscAsp.DEF_CSC_ASP_EQ_REVERB;
import static com.hcn.autoeq.util.ConstantCscAsp.EXT_CSC_ASP_REVERB_SIZE;
import static com.hcn.autoeq.util.ConstantCscAsp.EXT_CSC_ASP_REVERB_USER0;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.blankj.utilcode.util.ToastUtils;
import com.hcn.autoeq.R;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.common.misc.LogUtils;
import com.hcn.skin.support.SkinCompatManager;
import com.hcn.skin.support.resources.SkinCompatResources;


import java.util.Arrays;

public class CscAspEqualizerChartSettings {

    private static final String TAG = CscAspEqualizerChartSettings.class.getSimpleName();

    private static final String KEY_CSC_ASP_VOLUME = "key_csc_asp_volume"; // 响度的key

    private static final String EQ_SAVE_CSC_ASP = "eq_csc_band_file"; //频率文件

    private static final String KEY_CSC_ASP_REVERB_TYPE = "key_csc_asp_reverb_type"; // 音效模式的 key
    private static final String KEY_CSC_ASP_USER_BAND[] = {"csc_asp_user0_band", "csc_asp_user1_band"}; // 音效频率的 key


    //配置增益区间
    public static int MAX_GAIN_DB = 15;
    public static int MIN_GAIN_DB = -15;
    //配置中心频段个数


    private Context context;

    private SPUtils spUtils;

    /**
     * 主要用来存储band值，中心频率值；（显示值）
     */
    float[][] showBandValue;

    //用来存储band值（用来上传的）
    private int BassGain[] = {0, 0, 0};
    //用来中心频率值,固定；（用来上传的）
    private int BassCenterFre[] = {2, 1, 1};


    //段数数量
    private static int bandNumber = BAND_TOTAL;

    private static CscAspEqualizerChartSettings cscAspEqualizerChartSettings = null;

    private CscAspEqualizerChartSettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(EQ_SAVE_CSC_ASP);
    }

    public static CscAspEqualizerChartSettings getInstance(Context mContext) {
        if (null == cscAspEqualizerChartSettings) {
            cscAspEqualizerChartSettings = new CscAspEqualizerChartSettings(mContext);
            bandNumber = mContext.getApplicationContext().getResources().getStringArray(R.array.center_csc_asp_freq_36_segment).length;
        }
        return cscAspEqualizerChartSettings;
    }

    /**
     * 获取模式
     *
     * @return
     */
    public int getReverb() {
        int reverb = spUtils.getInt(KEY_CSC_ASP_REVERB_TYPE, DEF_CSC_ASP_EQ_REVERB);
        Log.d(TAG, "getReverb " + reverb);
        return reverb;
    }

    /**
     * 存储模式
     *
     * @return
     */
    public void saveReverb(int reverb) {
        String[] reverbList = SkinUtils.getStringArray(R.array.csc_asp_band_reverb);
        String[] userReverbList = SkinUtils.getStringArray(R.array.csc_asp_user_band_reverb);
        spUtils.put(KEY_CSC_ASP_REVERB_TYPE, reverb, true);
        if (reverb > EXT_CSC_ASP_REVERB_SIZE) {
            if(userReverbList != null){
                ToastUtils.showShort(userReverbList[reverb - EXT_CSC_ASP_REVERB_USER0]);
            }else{
                LogUtils.vTag(TAG,"get user revert list fail!");
            }
        } else {
            if(reverbList != null){
                ToastUtils.showShort(reverbList[reverb]);
            }else{
                LogUtils.vTag(TAG,"get revert list fail!");
            }
        }
        Log.d(TAG, "saveReverb " + reverb);
    }


    /**
     * 获取当前用户模式各Band值.
     * 需要段增益值，中心频率值；
     */
    public float[][] getUserBandValue(int reverb) {
        String[] freq = SkinUtils.getStringArray(R.array.center_csc_asp_freq_36_segment);
        if(freq == null){
            LogUtils.eTag(TAG,"get center_csc_asp_freq_36_segment fail!");
            return null;
        }
        int bandTotal = bandNumber;
        float[][] bandValue = new float[2][bandTotal];
        if (reverb > EXT_CSC_ASP_REVERB_SIZE) {
            // USER 模式获取保存值
            String userBand = spUtils.getString(KEY_CSC_ASP_USER_BAND[reverb - EXT_CSC_ASP_REVERB_USER0]);
            if (!"".equals(userBand)) {
                userBand = userBand.substring(userBand.indexOf("[") + 1, userBand.indexOf("]"));
                String[] mStr = userBand.split(",");
                for (int i = 0; i < bandTotal; i++) {
                    //获取各Band值
                    bandValue[0][i] = Float.parseFloat(mStr[i].trim());
                    bandValue[1][i] = Float.parseFloat(freq[i]);
                }
            }
        } else {
            for (int i = 0; i < bandTotal; i++) {
                //获取各Band值
                bandValue[0][i] = DEF_CSC_ASP_EQ_36_BANDS_VALUES[reverb][0][i];
                bandValue[1][i] = Float.parseFloat(freq[i]);
            }
        }
        Log.d(TAG, "reverb " + reverb + ",getUserBandValue " + Arrays.toString(bandValue[0]));
        showBandValue = bandValue;
        return bandValue;
    }

    /**
     * 存储模式的值，实际上只存储自定义用户模式
     */
    public void saveBandValue(float[][] bandValue) {
        int reverb = getReverb();
        showBandValue = Arrays.copyOf(bandValue, bandValue[0].length);
        if (reverb > EXT_CSC_ASP_REVERB_SIZE) {
            // 只保存 User 模式下的值
            spUtils.put(KEY_CSC_ASP_USER_BAND[reverb - EXT_CSC_ASP_REVERB_USER0], Arrays.toString(showBandValue[0]),true);
        }
        nativeBand(reverb);
        Log.d(TAG, "reverb " + reverb + ",saveBandValue " + Arrays.toString(bandValue[0]));
    }

    /**
     * 存储响度
     *
     * @return
     */
    public void saveCscAspLoudness(int loudness) {
        spUtils.put(KEY_CSC_ASP_VOLUME, loudness, true);
        nativeLoudness(loudness);
        Log.d(TAG, "saveCscAspLoudness " + loudness);
    }


    /**
     * 获取响度
     *
     * @return
     */
    public int getCscAspLoudness() {
        return spUtils.getInt(KEY_CSC_ASP_VOLUME, CSC_ASP_DEFAULT_LOUDNESS);
    }

    /**
     * 存储响度值到本地
     */
    public void nativeLoudness(int index) {
        int[] data = {index};
        NativeHelper.getEq().setEqLoudness(data);
        Log.d(TAG, String.format("nativeLoudness : %s", Arrays.toString(data)));
    }


    /**
     * 设置各 Band 值
     * 当均衡值发生变化时
     */
    public void nativeBand(int reverb) {
        getUserBandValue(reverb);
        dealGain();
        int qValue[] = CscAspQValueSettings.getInstance(context).getCscQValue(reverb);
        int Bass_gain = BassGain[0];
        int Bass_fo = BassCenterFre[0];
        int Bass_q = qValue[0];
        int Middle_gain = BassGain[1];
        int Middle_fo = BassCenterFre[1];
        int Middle_q = qValue[1];
        int Treble_gain = BassGain[2];
        int Treble_fo = BassCenterFre[2];
        int Treble_q = qValue[2];
        int[] data = {Bass_gain, Bass_fo, Bass_q, Middle_gain, Middle_fo, Middle_q, Treble_gain, Treble_fo, Treble_q};
        NativeHelper.getEq().setEqBand(data);
        Log.d(TAG, String.format("nativeBand data : %s", Arrays.toString(data)));
    }


    /**
     * 处理增益值
     */
    public void dealGain() {
        int paragraph = BAND_TOTAL / 3;
        int bassGain = 0;
        int middleGain = 0;
        int trebleGain = 0;
        for (int i = 0; i < BAND_TOTAL; i++) {
            if (i < paragraph) {
                bassGain += (int) showBandValue[0][i];
                BassGain[0] = bassGain / (i + 1);
            } else if (i < paragraph * 2) {
                middleGain += (int) showBandValue[0][i];
                BassGain[1] = middleGain / (i - paragraph + 1);
            } else {
                trebleGain += (int) showBandValue[0][i];
                BassGain[2] = trebleGain / (i - paragraph * 2 + 1);
            }
        }
    }


    /**
     * 设置各 Band 值
     * 这是给服务初始化使用的
     */
    public void nativeAllBand() {
        int reverb = getReverb();
        getUserBandValue(reverb);
        int qValue[] = CscAspQValueSettings.getInstance(context).getCscQValue(reverb);
        int Bass_gain = BassGain[0];
        int Bass_fo = BassCenterFre[0];
        int Bass_q = qValue[0];
        int Middle_gain = BassGain[1];
        int Middle_fo = BassCenterFre[1];
        int Middle_q = qValue[1];
        int Treble_gain = BassGain[2];
        int Treble_fo = BassCenterFre[2];
        int Treble_q = qValue[2];
        int[] data = {Bass_gain, Bass_fo, Bass_q, Middle_gain, Middle_fo, Middle_q, Treble_gain, Treble_fo, Treble_q};
        NativeHelper.getEq().setEqBand(data);
        Log.d(TAG, String.format("nativeBand data : %s", Arrays.toString(data)));
    }

}

