package com.hcn.autoeq.bean;

import android.graphics.Color;

import com.hcn.autoeq.R;

/**
 * 输出通道，和 fydsp_fragment_hlpf.xml 里喇叭控件的 tag 一致
 */
public enum FyDspOutputChannel {
    WAY2_F,
    WAY2_R,
    WAY2_SUBWOOFER_CENTER,

    WAY3_F,
    WAY3_R,
    WAY3_SUBWOOFER_CENTER,

    CHANNEL51_F,
    CHANNEL51_R,
    CHANNEL51_SUBWOOFER,
    CHANNEL51_CENTER,

    WAY6_F,
    WAY6_R,
    WAY6_SUBWOOFER_CENTER;

    // 控件对应线条的颜色
    public static int getLineColor(FyDspOutputChannel fyDspOutputChannel) {
        if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_F) {
            return R.color.fydsp_hlpf_color_channel51_f;
        } else if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_R) {
            return R.color.fydsp_hlpf_color_channel51_r;
        } else if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_CENTER) {
            return R.color.fydsp_hlpf_color_channel51_center;
        } else if (fyDspOutputChannel == FyDspOutputChannel.CHANNEL51_SUBWOOFER) {
            return R.color.fydsp_hlpf_color_channel51_subwoofer;
        } else if (fyDspOutputChannel == FyDspOutputChannel.WAY2_F) {
            return R.color.fydsp_hlpf_color_way2_f;
        } else if (fyDspOutputChannel == FyDspOutputChannel.WAY2_R) {
            return R.color.fydsp_hlpf_color_way2_r;
        } else if (fyDspOutputChannel == FyDspOutputChannel.WAY2_SUBWOOFER_CENTER) {
            return R.color.fydsp_hlpf_color_way2_subwoofer_center;
        } else if (fyDspOutputChannel == FyDspOutputChannel.WAY3_F) {
            return R.color.fydsp_hlpf_color_way3_f;
        } else if (fyDspOutputChannel == FyDspOutputChannel.WAY3_R) {
            return R.color.fydsp_hlpf_color_way3_r;
        } else if (fyDspOutputChannel == FyDspOutputChannel.WAY3_SUBWOOFER_CENTER) {
            return R.color.fydsp_hlpf_color_way3_subwoofer_center;
        } else if (fyDspOutputChannel == FyDspOutputChannel.WAY6_F) {
            return R.color.fydsp_hlpf_color_way6_f;
        } else if (fyDspOutputChannel == FyDspOutputChannel.WAY6_R) {
            return R.color.fydsp_hlpf_color_way6_r;
        } else if (fyDspOutputChannel == FyDspOutputChannel.WAY6_SUBWOOFER_CENTER) {
            return R.color.fydsp_hlpf_color_way6_subwoofer_center;
        }
        return Color.WHITE;
    }
}
