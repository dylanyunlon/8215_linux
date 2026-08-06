package com.hcn_library.bean;

import android.content.Context;

import com.hcn_library.hcn_library.R;


/**
 * 输出模式，和 fydsp_fragment_hlpf.xml 里控件的 tag 一致
 */
public enum FyDspOutputMode {
    WAY2, WAY3, CHANNEL51, WAY6;

    public static String format(Context context, FyDspOutputMode fyDspOutputMode) {
        switch (fyDspOutputMode) {
            case WAY2:
                return context.getString(com.hcn_library.hcn_library.R.string.fydsp_hlpf_output_mode_way2);
            case WAY3:
                return context.getString(R.string.fydsp_hlpf_output_mode_way3);
            case CHANNEL51:
                return context.getString(R.string.fydsp_hlpf_output_mode_channel51);
        }
        return "";
    }
}
