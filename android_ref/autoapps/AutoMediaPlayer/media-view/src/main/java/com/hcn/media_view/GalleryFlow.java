package com.hcn.media_view;

import android.content.Context;
import android.graphics.Camera;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.util.AttributeSet;
import android.view.View;
import android.widget.Gallery;

/**
 * 相册流控件
 * @author 86158
 */
public class GalleryFlow extends Gallery {

    /**
     * The camera class is used to 3D transformation matrix.
     */
    private final Camera mCamera = new Camera();
    private final Matrix mMatrix = new Matrix();

    /**
     * The max rotation angle.
     */
    private int mMaxRotationAngle = 60;

    /**
     * The max zoom value (Z axis).
     */
    private int mMaxZoom = -120;

    /**
     * The center of the gallery.
     */
    private int mCoverflowCenter = 0;

    public GalleryFlow(Context context) {
        this(context, null);
        this.setChildrenDrawingOrderEnabled(true);
    }

    public GalleryFlow(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
        this.setChildrenDrawingOrderEnabled(true);
    }

    public GalleryFlow(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.setChildrenDrawingOrderEnabled(true);
    }

    public int getMaxRotationAngle() {
        return mMaxRotationAngle;
    }

    public void setMaxRotationAngle(int maxRotationAngle) {
        mMaxRotationAngle = maxRotationAngle;
    }

    public int getMaxZoom() {
        return mMaxZoom;
    }

    public void setMaxZoom(int maxZoom) {
        mMaxZoom = maxZoom;
    }

    @Override
    protected int getChildDrawingOrder(int childCount, int i) {
        // Current selected index.
        int selectedIndex = getSelectedItemPosition() - getFirstVisiblePosition();
        if (selectedIndex < 0) {
            return i;
        }

        if (i < selectedIndex) {
            return i;
        } else {
            return childCount - 1 - i + selectedIndex;
        }
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        mCoverflowCenter = getCenterOfCoverflow();
        super.onSizeChanged(w, h, oldw, oldh);
    }

    private int getCenterOfCoverflow() {
        return ((getWidth() - getPaddingLeft() - getPaddingRight()) >> 1) + getPaddingLeft();
        // return (getWidth() - getPaddingLeft() - getPaddingRight()) / 2 + getPaddingLeft();
    }

    private int getCenterOfView(View view) {
        return view.getLeft() + view.getWidth() / 2;
    }

    protected float calculateOffsetOfCenter(View view) {
        final int pCenter = getCenterOfCoverflow();
        final int cCenter = getCenterOfView(view);

        float offset = (cCenter - pCenter) / (pCenter * 1.0f);
        offset = Math.min(offset, 1.0f);
        offset = Math.max(offset, -1.0f);

        return offset;
    }

    private void transformImageBitmap(View child, Matrix mt, int rotationAngle) {
        mCamera.save();

        final Matrix imageMatrix = mt;
        final int halfWidth = child.getLeft() + (child.getMeasuredWidth() >> 1);
        final int halfHeight = child.getMeasuredHeight() >> 1;
        final int rotation = Math.abs(rotationAngle);

        // Zoom on Z axis.
        mCamera.translate(0, 0, mMaxZoom);

        if (rotation < mMaxRotationAngle) {
            float zoomAmount = (float) (mMaxZoom + rotation * 1.5f);
            mCamera.translate(0, 0, zoomAmount);
        }

        // Rotate the camera on Y axis.
        mCamera.rotateY(rotationAngle);
        // Get the matrix from the camera, in fact, the matrix is S (scale) transformation.
        mCamera.getMatrix(imageMatrix);

        // The matrix final is T2 * S * T1, first translate the center point to (0, 0),
        // then scale, and then translate the center point to its original point.
        // T * S * T

        mMatrix.preTranslate(-halfWidth, -halfHeight - 15);
        mMatrix.postTranslate(halfWidth, halfHeight - 15);

        mCamera.restore();
    }

    private void getTransformationMatrix(View child, float offset) {
        final int halfWidth = child.getLeft() + (child.getMeasuredWidth() >> 1);
        final int halfHeight = child.getMeasuredHeight() >> 1;

        mCamera.save();
        mCamera.translate(-offset * 50, 0.0f, Math.abs(offset) * 200);

        mCamera.getMatrix(mMatrix);
        mCamera.restore();
        mMatrix.preTranslate(-halfWidth, -halfHeight);
        mMatrix.postTranslate(halfWidth, halfHeight);
    }

    @Override
    protected boolean drawChild(Canvas canvas, View child, long drawingTime) {
        boolean ret;
        // Android SDK 4.1
        if (android.os.Build.VERSION.SDK_INT > 15) {
            getChildStaticTrans(child, mMatrix);
            final int saveCount = canvas.save();
            canvas.concat(mMatrix);
            ret = super.drawChild(canvas, child, drawingTime);
            canvas.restoreToCount(saveCount);
        } else {
            ret = super.drawChild(canvas, child, drawingTime);
        }
        return ret;
    }

    protected void getChildStaticTrans(View child, Matrix mt) {
        final int childCenter = getCenterOfView(child);
        final int childWidth = child.getWidth();
        int rotationAngle = 0;

        if (childCenter == mCoverflowCenter) {
            transformImageBitmap(child, mt, 0);
        } else {
            rotationAngle = (int) (((float) (mCoverflowCenter - childCenter) / childWidth)
                    * mMaxRotationAngle);
            if (Math.abs(rotationAngle) > mMaxRotationAngle) {
                rotationAngle = (rotationAngle < 0) ? -mMaxRotationAngle : mMaxRotationAngle;
            }
            transformImageBitmap(child, mt, rotationAngle);
        }
    }
}
