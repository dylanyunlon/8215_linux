package com.hcn.autoeq.util;

import android.content.Context;
import android.graphics.Point;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;

public class EqUtils {

    public static final String ASP_CHIP_2348 = "pt2348";
    public static final String ASP_CHIP_3313 = "cd3313";
    public static final String ASP_CHIP_CSC37534 = "csc37534";
    public static final String ASP_CHIP_ZL3560 = "zl3560";
    public static final String DSP_CHIP_7604 = "ak7604";
    public static final String DSP_CHIP_7604_C = "ak7604c";
    public static final String DSP_CHIP_FY7604 = "fy7604"; // 飞音客户的 7604

    public static final String DSP_CHIP_SI47925 = "si479x";
    private static final String KEY_DSP_CHIP = "ro.hw.dsp.chip";
    private static final String KEY_BAND_TOTAL = "persist.sys.band"; // 段数
    public static final String INTERNAL_DSP_DISABLE = "persist.sys.internal.eq.disable";
    public static final String HEQ_BASS_BOOST = "heq_bass_boost"; //超重低音的值以及asp的开关

    public static final String HEQ_SHOW_CENTER = "heq_show_center";
    public static final String HEQ_USE_CENTER = "heq_use_center";
    public final static String SUPPORT_NEW_EQ_SKIN = "support_new_eq_skin";

    // 2: mcx客户，内置dsp+asp，作假32段，曲线控件带触摸特效
    private static final String KEY_DSP_UITYPE = "persist.sys.dspui";

    public final static String KEY_SKIN = "persist.sys.heq.skins";

    public final static String KEY_SKIN_RK02 = "rk02";
    public final static String KEY_SKIN_GB01 = "gb01";

    //默认平衡长度15段（上传是15段）
    public static final int BALANCE_DEPTH = 15;
    //CSC_ASP平衡长度19段（实际上传是15段）
    public static final int CSC_ASP_BALANCE_DEPTH = 19;

    // 内置 dsp 段数（K981需求：MCX客户只做32段）
    public static final int DSP_BAND_DEPTH = "2".equals(getDspUI()) ? 32 : 16;
    // asp 段数
    public static final int ASP_BAND_DEPTH = 12;

    // 外置 dsp 段数（14段是芯片支持的段数，32和48是作假段数）
    public static final int BAND_TOTAL_14 = 14;
    public static final int BAND_TOTAL_16 = 16;
    public static final int BAND_TOTAL_32 = 32;
    public static final int BAND_TOTAL_36 = 36;
    public static final int BAND_TOTAL_48 = 48;

    /**
     * 均衡器曲线被任意触摸时，曲线会变化（默认）
     */
    public static final int EQUALIZER_ACTION_TOUCHING = 0;
    /**
     * 均衡器曲线在水平方向左右被滑动后，曲线会变化
     */
    public static final int EQUALIZER_ACTION_SLIDE_HORIZONTAL = 1;
    /**
     * 强制使用内置dsp进行音效调节
     */
    private static final boolean FORCE_INTERNAL_DSP = "1".equals(
            SystemUtils.getSystemProperty("ro.force.internal.dsp", "0"));
    /**
     * 8581字段配置支持隐藏csc_asp场景模式
     */
    public static final boolean HEQ_CSC_ASP_HIDE_SCENE = "1".equals(
            SystemUtils.getSystemProperty("persist.sys.heq.csc_asp_hide_scene", "0"));
    public static String getEqChipType() {
        if (FORCE_INTERNAL_DSP) {
            return "NULL";
        }
        return SystemUtils.getSystemProperty(KEY_DSP_CHIP, "NULL");
    }

    //判断是否有外挂ASP芯片
    public static boolean hasAsp() {
        String chipType = getEqChipType();
        if (ASP_CHIP_2348.equals(chipType) || ASP_CHIP_3313.equals(chipType)) {
            return true;
        }
        return false;
    }

    /**
     * @param
     * @return $boolean . internal dsp disable ?0:enable,1:disable
     * @Description: 用于返回工厂设置内内置音效使能状态.
     **/
    public static boolean disableInternalDsp() {
        return "1".equals(SystemUtils.getSystemProperty(INTERNAL_DSP_DISABLE, "0"));
    }

    public static int getBandTotal() {
        String band = SystemUtils.getSystemProperty(KEY_BAND_TOTAL, "");
        if (String.valueOf(BAND_TOTAL_32).equals(band)) {
            return BAND_TOTAL_32;
        } else if (String.valueOf(BAND_TOTAL_48).equals(band)) {
            if (DSP_CHIP_SI47925.equals(getEqChipType())) {
                return BAND_TOTAL_36;
            }
            return BAND_TOTAL_48;
        } else if (String.valueOf(BAND_TOTAL_16).equals(band)) {
            return BAND_TOTAL_16;
        } else if (String.valueOf(BAND_TOTAL_36).equals(band)) {
            return BAND_TOTAL_36;
        } else if (isRk3326() && (DSP_CHIP_7604_C.equals(getEqChipType()))) {
            return BAND_TOTAL_48;
        }
        return BAND_TOTAL_14;
    }

    // 配置 dsp band 可调节的范围值，默认14(+-7)
    public static int getDspGainMax() {
        String dspBandMax = SystemUtils.getSystemProperty("persist.sys.dsp_band_max", String.valueOf(ConstantExtDsp.EXT_DSP_BAND_GAIN_MAX));
        return Integer.parseInt(dspBandMax);
    }

    // 美灿星客户
    public static boolean isMCX() {
        return "403".equals(getEThemeGod());
    }

    // 中道客户
    public static boolean isZD() {
        return "153".equals(getEThemeGod());
    }

    // 安桥客户
    public static boolean isAQ() {
        return "405".equals(getEThemeGod());
    }

    // 驭丰客户
    public static boolean isYuFeng() {
        return "400".equals(EqUtils.getEThemeGod()) && "052".equals(EqUtils.getEThemeSub());
    }

    public static String getEThemeGod() {
        return SystemUtils.getSystemProperty("persist.sys.etheme_god", "");
    }

    public static String getEThemeSub() {
        return SystemUtils.getSystemProperty("persist.sys.etheme_sub", "");
    }

    /**
     * 是否头枕
     *
     * @return boolean
     */
    public static boolean isHeadRest() {
        return "1".equals(SystemUtils.getSystemProperty("ro.headrest.pad", ""));
    }


    /*
     0:Normal
     1:ASP UI
     2:mcx 内置dsp（K981需求：曲线控件带滑动特效）
     3:dsp 界面风格：ExtDspBandSecondFragment
     */
    public static String getDspUI() {
        return SystemUtils.getSystemProperty(KEY_DSP_UITYPE, "0");
    }

    public static boolean is8581() {
        return SystemUtils.getSystemProperty("ro.build.product", "").startsWith("uis8581");
    }

    public static boolean is6225() {
        Log.d("EqUtils", SystemUtils.getSystemProperty("ro.build.product", ""));
        return SystemUtils.getSystemProperty("ro.build.product", "").startsWith("sm6225");
    }

    public static boolean isRk3326() {
        return SystemUtils.getSystemProperty("ro.build.product", "").startsWith("rk3326");
    }

    public static boolean showCenter() {
        String value = SystemUtils.getSystemProperty(HEQ_SHOW_CENTER, "");
        if ("".equals(value)) {
            return false;
        }
        return Integer.parseInt(value) == 1;
    }

    public static boolean useCenter() {
        String value = SystemUtils.getSystemProperty(HEQ_USE_CENTER, "");
        if ("".equals(value)) {
            return false;
        }
        return Integer.parseInt(value) == 1;
    }

    /**
     * @description:判断目前是否是阿拉伯语等右至左语言
     * @author: liangxuchen
     * @date: 2024/7/1
     * @param: Context
     * @return: boolean
     **/
    public static boolean isRtL(Context context) {
//        return view.getLayoutDirection()==View.LAYOUT_DIRECTION_RTL;
//        Configuration config = SkinCompatResources.getInstance().getSkinResources().getConfiguration();
        return context.getResources().getConfiguration().getLayoutDirection() == View.LAYOUT_DIRECTION_RTL;
    }

    public static int getDefaultLoudness() {
        return Integer.parseInt(SystemUtils.getSystemProperty("persist.sys.dsp_default_loudness", EqUtils.is8581() ? "0" : "1"));
    }

    /**
     * 翻页方式
     *
     * @return 0：触摸滑动翻页 1：点击按钮翻页
     */
    public static boolean flipPageByBtn(Context context) {
        int flipPage = Settings.System.getInt(context.getContentResolver(), "dsp_flip_page", 0);

        // 飞音客户 48段情况下两个分辨率支持
        if (flipPage == 1) {
            WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            Point realSize = new Point();
            windowManager.getDefaultDisplay().getRealSize(realSize);

            if ((realSize.x == 1024 && realSize.y == 600)
                    || (realSize.x == 1280 && realSize.y == 720)) {
                if (getBandTotal() == BAND_TOTAL_48) {
                    return true;
                }
            }
        }

        return false;
    }

    /**
     * 翻页次数
     * 默认48段情况下，翻4次
     * 其它段数可以通过 控件总宽度/屏幕宽度 来计算
     *
     * @return
     */
    public static int flipPageCount(Context context) {
        return Settings.System.getInt(context.getContentResolver(), "dsp_flip_page_count", 4);
    }

    /**
     * 支持白天黑夜切换的皮肤包
     *
     * @return false：不支持 true：支持
     */
    public static boolean supportDayAndNightMode(){
        String skinName = EqUtils.getSkinName();
        if (null == skinName){
            return false;
        }
        switch (skinName){
            case KEY_SKIN_GB01:
            case KEY_SKIN_RK02:
                return true;
            default:
                break;
        }
        return false;
    }

    private static String skinNameValue = null;
    public static String getSkinName() {
        if (null == skinNameValue) {
            skinNameValue = SystemUtils.getSystemProperty(KEY_SKIN, "");
        }
        return skinNameValue;
    }

    public static int getSupportNewSkin() {
        int result = 0;
        String value = SystemUtils.getSystemProperty(SUPPORT_NEW_EQ_SKIN, "");
        if (! "".equals(value)) {
           result = Integer.parseInt(value);
        }
        if (isGB04_05()) {
            result = 1;
        }
        return result;
    }

    static String skinName = null;

    public static boolean isGB04_05() {
        if (skinName == null) {
            skinName = getSkinName();
        }
        switch (skinName) {
            case "gb04":
            case "gb05":
                return true;
            default:
                return false;
        }
    }
}
