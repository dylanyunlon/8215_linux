package com.hcn.autoeq.data;

import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.blankj.utilcode.util.StringUtils;

import java.util.Map;

public class FyDspBaseSettings {

    private static final String TAG = FyDspBaseSettings.class.getSimpleName();
    private static final String FY_DSP_BASE_FILE = "v2_fy_dsp_base";

    public SPUtils spUtils; // 各子类的 SP 对象
    private String file;

    private SPUtils baseSPUtils;

    public FyDspBaseSettings(String file) {
        this.spUtils = SPUtils.getInstance(file);
        this.file = file;
        baseSPUtils = SPUtils.getInstance(FY_DSP_BASE_FILE);
    }

    // 把当前模块的文件数据复制一份到对应的 USER 文件
    public void reload(String userModeFrom, String userModeTo) {
        Log.d(TAG, String.format("copy data from [%s] to [%s].", userModeFrom, userModeTo));
        final SPUtils spUtilsUserFrom;
        final SPUtils spUtilsUserTo;
        if (StringUtils.isTrimEmpty(userModeFrom)) { // 从自定义模式，复制到一个 user 模式
            spUtilsUserFrom = this.spUtils;
            spUtilsUserTo = SPUtils.getInstance(this.file + "_" + userModeTo);
        } else {
            spUtilsUserFrom = SPUtils.getInstance(this.file + "_" + userModeFrom);
            if (StringUtils.isTrimEmpty(userModeTo)) { // 从一个 user 模式复制到自定义模式
                spUtilsUserTo = this.spUtils;
            } else { // 从一个 user 模式复制到另外一个 user 模式
                spUtilsUserTo = SPUtils.getInstance(this.file + "_" + userModeTo);
            }
        }

        // 复制数据前，先把旧的数据删掉
        Map<String, ?> allTo = spUtilsUserTo.getAll();
        allTo.forEach((key, value) -> {
            if (!"key_band_mode".equals(key)) { // 这个比较特殊，不能删
                spUtilsUserTo.remove(key);
            }
        });

        Map<String, ?> all = spUtilsUserFrom.getAll();
        all.forEach((key, value) -> {
            Object v = all.get(key);
            if (v instanceof String) {
                spUtilsUserTo.put(key, (String) v);
            } else if (v instanceof Integer) {
                spUtilsUserTo.put(key, (int) v);
            } else if (v instanceof Float) {
                spUtilsUserTo.put(key, (float) v);
            } else if (v instanceof Boolean) {
                spUtilsUserTo.put(key, (boolean) v);
            }
        });
        spUtilsUserTo.put("commit_flag", "1", true);
    }

    // 根据当前模式来返回对应的 SP 对象
    public SPUtils getSpUtils() {
        String userMode = getUserMode();
        if (StringUtils.isEmpty(userMode)) {
            return spUtils;
        } else {
            return SPUtils.getInstance(file + "_" + getUserMode());
        }
    }

    public void saveUserMode(String tag) {
        Log.d(TAG, "saveUserMode user mode : [" + tag + "]");
        baseSPUtils.put("key_user_mode", tag);
    }

    public String getUserMode() {
        return baseSPUtils.getString("key_user_mode", "");
    }
}
