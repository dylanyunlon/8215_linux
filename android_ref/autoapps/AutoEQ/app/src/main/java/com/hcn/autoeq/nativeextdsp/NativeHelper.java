package com.hcn.autoeq.nativeextdsp;

import com.hcn.autoeq.util.EqUtils;

public class NativeHelper {

    private static IEq iEq = null;

    public static IEq getEq() {
        if (null == iEq) {
            String chipType = EqUtils.getEqChipType();

            if (EqUtils.DSP_CHIP_7604.equals(chipType)) {
                iEq = new Ak7604();
            } else if (EqUtils.DSP_CHIP_FY7604.equals(chipType)) {
                iEq = new FY7604();
            } else if (EqUtils.DSP_CHIP_SI47925.equals(chipType)) {
                iEq = new SI47925();
            }else {
                iEq = new Ak7604();
            }
            if (EqUtils.ASP_CHIP_CSC37534.equals(chipType) || EqUtils.ASP_CHIP_ZL3560.equals(EqUtils.getEqChipType())) {
                iEq = new CSC37534();
            }
        }
        return iEq;
    }
}
