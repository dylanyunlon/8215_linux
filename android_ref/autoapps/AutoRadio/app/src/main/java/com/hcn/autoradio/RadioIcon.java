package com.hcn.autoradio;

import android.widget.CheckBox;

import com.hcn.autoradio.skin.SkinUtils;

public class RadioIcon {

    private static int[] mPxxIconID = {R.drawable.radio_icon_p00,
            R.drawable.radio_icon_p01, R.drawable.radio_icon_p02,
            R.drawable.radio_icon_p03, R.drawable.radio_icon_p04,
            R.drawable.radio_icon_p05, R.drawable.radio_icon_p06,
            R.drawable.radio_icon_p07, R.drawable.radio_icon_p08,
            R.drawable.radio_icon_p09, R.drawable.radio_icon_p10,
            R.drawable.radio_icon_p11, R.drawable.radio_icon_p12,
            R.drawable.radio_icon_p13, R.drawable.radio_icon_p14,
            R.drawable.radio_icon_p15, R.drawable.radio_icon_p16,
            R.drawable.radio_icon_p17, R.drawable.radio_icon_p18};

    private CheckBox[] mRadioIcon = null;

    public final static int ICON_PXX = 0;
    public final static int ICON_LOCDX = 1;
    public final static int ICON_STEREO = 2;
    public final static int ICON_SCAN = 3;
    public final static int ICON_AS = 4;
    public final static int ICON_PS = 5;
    public final static int ICON_COUNT = 6;

    public RadioIcon() {
        mRadioIcon = new CheckBox[ICON_COUNT];
    }

    public void initRadioIcon(RadioMain context) {

        if (null != context) {
            mRadioIcon[ICON_PXX] = (CheckBox) context
                    .findViewById(SkinUtils.getId(R.id.checkbox_pxx));
            mRadioIcon[ICON_LOCDX] = (CheckBox) context
                    .findViewById(SkinUtils.getId(R.id.checkbox_locdx));
            mRadioIcon[ICON_STEREO] = (CheckBox) context
                    .findViewById(SkinUtils.getId(R.id.checkbox_stereo));
            mRadioIcon[ICON_SCAN] = (CheckBox) context
                    .findViewById(SkinUtils.getId(R.id.checkbox_scan));
            mRadioIcon[ICON_AS] = (CheckBox) context
                    .findViewById(SkinUtils.getId(R.id.checkbox_as));
            mRadioIcon[ICON_PS] = (CheckBox) context
                    .findViewById(SkinUtils.getId(R.id.checkbox_ps));
        }
    }

    // index p0 -- p18
    public void setCheckBoxPxx(int index) {
        try {
            int resid = mPxxIconID[index];

            if (null != mRadioIcon[ICON_PXX]) {
                mRadioIcon[ICON_PXX].setBackgroundResource(SkinUtils.getId(resid));
            }
        } catch (Exception e) {

        }
    }

    public void setCheckBox(int icon, boolean check) {

        if (icon < ICON_COUNT && ICON_LOCDX <= icon) {

            if (null != mRadioIcon[icon]) {
                mRadioIcon[icon].setChecked(check);
            }
        }
    }
}
