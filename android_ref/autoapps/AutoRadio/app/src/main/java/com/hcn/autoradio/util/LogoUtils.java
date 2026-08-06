package com.hcn.autoradio.util;

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.util.DisplayMetrics;
import android.util.Log;

import com.hcn.autoradio.R;
import com.hcn.autoradio.skin.SkinUtils;

import java.io.File;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @describe dz17飛音定制
 */
public class LogoUtils {
    private static final String TAG = LogoUtils.class.getSimpleName();
    public static final String MALAYSIA_LOGO_PATH = "apd/radio/malaysia";
    public static final String INDONESIA_LOGO_PATH = "apd/radio/indonesia";
    public static final String THE_PHILIPPINES_LOGO_PATH = "apd/radio/philippines";
    public static String COUNTRY_CODE;
    public static Map<Integer, String> logoSitesMalaysia = new HashMap<Integer, String>();
    public static Map<Integer, String> logoSitesIndonesia = new HashMap<Integer, String>();
    public static Map<Integer, String> logoSitesPhilippines = new HashMap<Integer, String>();

    /**
     * 初始化工作
     */
    public static void initialize() {
        getCountryCode();
        analysisImgPath();
    }

    /**
     * 获取国家码
     */
    public static void getCountryCode() {
        Context context = SkinUtils.getContext();
        if (context == null) {
            return;
        }
        Configuration config = context.getResources().getConfiguration();
        if (config != null) {
            Locale locale = config.locale;
            if (locale != null) {
                COUNTRY_CODE = locale.getCountry();
            } else {
                COUNTRY_CODE = "";
            }
        } else {
            COUNTRY_CODE = "";
        }
        Log.d(TAG, "COUNTRY_CODE:" + COUNTRY_CODE);
    }

    /**
     * 解析图片并存储
     */
    public static void analysisImgPath() {
        if (!isSupportArea()) {
            Log.d(TAG, "not Support Area!!!");
            return;
        }
        String path = getPath();
        Log.d(TAG, "analysisImgPath:" + path);
        if (path != null) {
            try {
                File dir = new File(path);
                File[] files = dir.listFiles();
                for (File file : files) {
                    if (checkIsImageFile(file.getPath())) {
                        String countryCode = COUNTRY_CODE;
                        if (countryCode != null) {
                            Map<Integer, String> logoSites;
                            if ("MY".equals(countryCode)) {
                                logoSites = logoSitesMalaysia;
                            } else if ("PH".equals(countryCode)) {
                                logoSites = logoSitesPhilippines;
                            } else if ("ID".equals(countryCode)) {
                                logoSites = logoSitesIndonesia;
                            } else {
                                continue;
                            }
                            String logoFreq = getStation(file.getName());
                            int logoSuffix = Integer.parseInt(logoFreq);
                            if (logoSuffix / 100000 > 1) {
                                logoSuffix = logoSuffix / 10;
                            }
                            Log.d(TAG, "logoFreq=" + logoFreq + " logoSuffix=" + logoSuffix);
                            logoSites.put(logoSuffix, file.getPath());
                        }
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 根据文件名解析出电台
     *
     * @param fileName
     * @return
     */
    public static String getStation(final String fileName) {
        String numberPart = "";
        // 使用正则表达式匹配文件名中的数字
        Pattern pattern = Pattern.compile(".*_(\\d+)\\..*");
        Matcher matcher = pattern.matcher(fileName);
        if (matcher.matches()) {
            numberPart = matcher.group(1);
        }
        return numberPart;
    }

    /*** 判断是否是图片*/
    public static boolean checkIsImageFile(String fName) {
        boolean isImageFile = false;
        //获取拓展名
        String fileExtension  = fName.substring(fName.lastIndexOf(".") + 1).toLowerCase();
        if ("jpg".equals(fileExtension ) || "png".equals(fileExtension )) {
            isImageFile = true;
        }
        return isImageFile;
    }

    /**
     * 获取特定国家的电台图片
     *
     * @param key
     * @return
     */
    public static Drawable getLogo(int key) {
        String countryCode = COUNTRY_CODE;
        if ("MY".equals(countryCode)) {
            if (!logoSitesMalaysia.isEmpty() && new File(logoSitesMalaysia.get(key)).exists()) {
                return new BitmapDrawable(BitmapFactory.decodeFile(logoSitesMalaysia.get(key)));
            }
        } else if ("PH".equals(countryCode)) {
            if (!logoSitesPhilippines.isEmpty() && new File(logoSitesPhilippines.get(key)).exists()) {
                return new BitmapDrawable(BitmapFactory.decodeFile(logoSitesPhilippines.get(key)));
            }
        } else if ("ID".equals(countryCode)) {
            if (!logoSitesIndonesia.isEmpty() && new File(logoSitesIndonesia.get(key)).exists()) {
                return new BitmapDrawable(BitmapFactory.decodeFile(logoSitesIndonesia.get(key)));
            }
        }
        return SkinUtils.getDrawable(R.drawable.preset_xml_selector);
    }

    /**
     * 获取电台图片路径
     *
     * @return
     */
    private static String getPath() {
        DisplayMetrics dm = SkinUtils.getContext().getResources().getDisplayMetrics();
        int screenWidth = dm.widthPixels;
        String path = null;
        switch (COUNTRY_CODE) {
            case "MY":
                if (screenWidth == 1280 || screenWidth == 720) {
                    path = MALAYSIA_LOGO_PATH + "_720/";
                } else if (screenWidth == 1024 || screenWidth == 600) {
                    path = MALAYSIA_LOGO_PATH + "_600/";
                }
                break;
            case "PH":
                if (screenWidth == 1280 || screenWidth == 720) {
                    path = THE_PHILIPPINES_LOGO_PATH + "_720/";
                } else if (screenWidth == 1024 || screenWidth == 600) {
                    path = THE_PHILIPPINES_LOGO_PATH + "_600/";
                }
                break;
            case "ID":
                if (screenWidth == 1280 || screenWidth == 720) {
                    path = INDONESIA_LOGO_PATH + "_720/";
                } else if (screenWidth == 1024 || screenWidth == 600) {
                    path = INDONESIA_LOGO_PATH + "_600/";
                }
                break;
            default:
                break;
        }
        return path;
    }

    public static Boolean isSupportArea() {
        if ("MY".equals(COUNTRY_CODE) || "PH".equals(COUNTRY_CODE) || "ID".equals(COUNTRY_CODE)) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * 是否支持设置电台图片的频率
     *
     * @param freq
     * @return
     */
    public static boolean isSupportAreaFreq(int freq) {
        return LogoUtils.isIndonesiaStation(freq) || LogoUtils.isMalaysiaStation(freq) || LogoUtils.isPhilippinesStation(freq);
    }

    /**
     * 马来西亚预设电台
     *
     * @param freq
     * @return
     */
    public static Boolean isMalaysiaStation(int freq) {
        return "MY".equals(COUNTRY_CODE) && logoSitesMalaysia.containsKey(freq);
    }

    /**
     * 印度尼西亚电台
     *
     * @param freq
     * @return
     */
    public static Boolean isIndonesiaStation(int freq) {
        return "PH".equals(COUNTRY_CODE) && logoSitesPhilippines.containsKey(freq);
    }

    /**
     * 菲律宾电台
     *
     * @param freq
     * @return
     */
    public static Boolean isPhilippinesStation(int freq) {
        return "ID".equals(COUNTRY_CODE) && logoSitesIndonesia.containsKey(freq);
    }


}
