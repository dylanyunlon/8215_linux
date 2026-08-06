package com.hcn.autoradio;

public enum EDigitFreq {
    DIGIT_FREQ(6), HUNDRED(5), DECADE(4), UNITS(3), DOT(2), TENTH(1), PERCENTILE(0);

    private int nValue;

    EDigitFreq(int nCode) {
        nValue = nCode;
    }

    public int getValue() {
        return nValue;
    }

    public void setValue(int nCode) {
        nValue = nCode;
    }

    @Override
    public String toString() {
        return String.valueOf(this.nValue);
    }
}
