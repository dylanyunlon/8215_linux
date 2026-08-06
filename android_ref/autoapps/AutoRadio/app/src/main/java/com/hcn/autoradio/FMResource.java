package com.hcn.autoradio;

import com.hcn.autoradio.skin.SkinUtils;

public final class FMResource {

    public static final int REGION_USA_FM = 0;
    public static final int REGION_USA_AM = 1;

    public static final int REGION_EUROPE_FM = 2;
    public static final int REGION_EUROPE_AM = 3;

    public static final int REGION_LATIN_FM = 4;
    public static final int REGION_LATIN_AM = 5;

    public static final int REGION_OIRT_FM = 6;
    public static final int REGION_OIRT_AM = 7;

    public static final int REGION_CHINA_FM = 8;
    public static final int REGION_CHINA_AM = 9;

    public static final int REGION_JAPAN_FM = 10;
    public static final int REGION_JAPAN_AM = 11;

    public static final int REGION_LATIN2_FM = 12;
    public static final int REGION_LATIN2_AM = 13;

    public static final class styleable {

        public static int getSkinAttr(int resId){
            return SkinUtils.getId(resId);
        }

        public static final int[] EqScroll = {getSkinAttr(R.attr.center_x),
                getSkinAttr(R.attr.center_y), getSkinAttr(R.attr.circle_r),getSkinAttr(R.attr.scroll_bar),
                getSkinAttr(R.attr.scroll_item), getSkinAttr(R.attr.scroll_begin), getSkinAttr(R.attr.scroll_end),
                getSkinAttr(R.attr.scroll_uid), getSkinAttr(R.attr.thumb_offset_y)};

        public static final int center_x = 0;
        public static final int center_y = 1;
        public static final int circle_r = 2;
        public static final int scroll_bar = 3;
        public static final int scroll_item = 4;
        public static final int scroll_begin = 5;
        public static final int scroll_end = 6;
        public static final int scroll_uid = 7;
        public static final int thumb_offset_y = 8;
    }
}
