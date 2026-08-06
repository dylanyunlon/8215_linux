/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 */

package com.hcn.autoeq.view;

import android.graphics.Camera;
import android.graphics.Matrix;
import android.view.animation.Animation;
import android.view.animation.Transformation;

public class Eq3DAnimation extends Animation {

    private final float mBeginAngle;
    private final float mEndAngle;

    private final float mCenterX;
    private final float mCenterY;
    private final float mDepthZ;

    private final boolean mReverse;
    private Camera mCamera;

    public Eq3DAnimation(float fromAngle, float toAngle, float centerX,
                         float centerY, float depthZ, boolean reverse) {
        mBeginAngle = fromAngle;
        mEndAngle = toAngle;
        mCenterX = centerX;
        mCenterY = centerY;
        mDepthZ = depthZ;
        mReverse = reverse;
    }


    @Override
    public void initialize(int width, int height, int parentWidth,
                           int parentHeight) {
        super.initialize(width, height, parentWidth, parentHeight);
        mCamera = new Camera();
    }


    @Override
    protected void applyTransformation(float interpolatedTime, Transformation t) {
        final float fromAngle = mBeginAngle;

        float degrees = fromAngle
                + ((mEndAngle - fromAngle) * interpolatedTime);

        final float centerX = mCenterX;
        final float centerY = mCenterY;
        final Camera camera = mCamera;


        final Matrix matrix = t.getMatrix();

        camera.save();


        if (mReverse) {

            camera.translate(0.0f, 0.0f, mDepthZ * interpolatedTime);
        } else {

            camera.translate(0.0f, 0.0f, mDepthZ * (1.0f - interpolatedTime));
        }


        camera.rotateY(degrees);
        // camera.rotateX(degrees);

        camera.getMatrix(matrix);
        camera.restore();


        matrix.preTranslate(-centerX, -centerY);
        matrix.postTranslate(centerX, centerY);
    }
}
