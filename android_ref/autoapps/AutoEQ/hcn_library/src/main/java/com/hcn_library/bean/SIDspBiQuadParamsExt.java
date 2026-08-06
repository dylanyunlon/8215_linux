package com.hcn_library.bean;

public class SIDspBiQuadParamsExt {
    // S.W24.F22
    double a1, a2, b0, b1, b2;
    int bShift;

    public double getA1() {
        return a1;
    }

    public void setA1(double a1) {
        this.a1 = a1;
    }

    public double getA2() {
        return a2;
    }

    public void setA2(double a2) {
        this.a2 = a2;
    }

    public double getB0() {
        return b0;
    }

    public void setB0(double b0) {
        this.b0 = b0;
    }

    public double getB1() {
        return b1;
    }

    public void setB1(double b1) {
        this.b1 = b1;
    }

    public double getB2() {
        return b2;
    }

    public void setB2(double b2) {
        this.b2 = b2;
    }

    public int getbShift() {
        return bShift;
    }

    public void setbShift(int bShift) {
        this.bShift = bShift;
    }


    @Override
    public String toString() {
        return "SIDspBiQuadParamsExt{" +
                "a1=" + a1 +
                ", a2=" + a2 +
                ", b0=" + b0 +
                ", b1=" + b1 +
                ", b2=" + b2 +
                ", bShift=" + bShift +
                '}';
    }
}
