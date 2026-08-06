package com.hcn.autoradio.view;

import android.view.animation.Interpolator;

public class ScaleInterpolator implements Interpolator {

    @Override
    public float getInterpolation(float input) {
        return (float) (1.0D - Math.pow(1.0F - input, 6.0D));
    }
}