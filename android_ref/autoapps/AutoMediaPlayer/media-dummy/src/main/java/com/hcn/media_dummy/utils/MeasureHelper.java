package com.hcn.media_dummy.utils;

import android.view.View;

import com.hcn.common.misc.LogUtils;
import com.hcn.media_dummy.Config;

import java.lang.ref.WeakReference;

public class MeasureHelper {
    private final WeakReference<View> mWeakView;

    private int mVideoWidth;
    private int mVideoHeight;

    private int mVideoRotationDegree;

    private int mMeasuredWidth;
    private int mMeasuredHeight;

    private int mCurrentAspectRatio = FunVideoType.SCREEN_TYPE_DEFAULT;

    private final MeasureFormVideoParamsListener mParamsListener;

    public MeasureHelper(View view, MeasureFormVideoParamsListener listener) {
        mParamsListener = listener;
        mWeakView = new WeakReference<>(view);
    }

    public View getView() {
        return mWeakView.get();
    }

    public void setVideoSize(int videoWidth, int videoHeight) {
        mVideoWidth = videoWidth;
        mVideoHeight = videoHeight;
    }

    public void setVideoRotation(int videoRotationDegree) {
        mVideoRotationDegree = videoRotationDegree;
    }

    /**
     * Must be called by View.onMeasure(int, int)
     *
     * @param widthMeasureSpec
     * @param heightMeasureSpec
     */
    public void doMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        mCurrentAspectRatio = FunVideoType.getShowType();

        if (mVideoHeight == 0 || mVideoWidth == 0) {
            mMeasuredWidth = 1;
            mMeasuredHeight = 1;
            return;
        }

        if (mVideoRotationDegree == 90 || mVideoRotationDegree == 270) {
            int tempSpec = widthMeasureSpec;
            widthMeasureSpec = heightMeasureSpec;
            heightMeasureSpec = tempSpec;
        }

        int realWidth = mVideoWidth;
        int width = View.getDefaultSize(realWidth, widthMeasureSpec);
        int height = View.getDefaultSize(mVideoHeight, heightMeasureSpec);
        if (mCurrentAspectRatio == FunVideoType.SCREEN_MATCH_FULL) {
            width = widthMeasureSpec;
            height = heightMeasureSpec;
        } else if (realWidth > 0 && mVideoHeight > 0) {
            int widthSpecMode = View.MeasureSpec.getMode(widthMeasureSpec);
            int widthSpecSize = View.MeasureSpec.getSize(widthMeasureSpec);
            int heightSpecMode = View.MeasureSpec.getMode(heightMeasureSpec);
            int heightSpecSize = View.MeasureSpec.getSize(heightMeasureSpec);

            if (widthSpecMode == View.MeasureSpec.AT_MOST && heightSpecMode == View.MeasureSpec.AT_MOST) {
                float specAspectRatio = (float) widthSpecSize / (float) heightSpecSize;
                float displayAspectRatio;
                switch (mCurrentAspectRatio) {
                    case FunVideoType.SCREEN_TYPE_16_9:
                        displayAspectRatio = 16.0f / 9.0f;
                        if (mVideoRotationDegree == 90 || mVideoRotationDegree == 270) {
                            displayAspectRatio = 1.0f / displayAspectRatio;
                        }
                        break;
                    case FunVideoType.SCREEN_TYPE_18_9:
                        displayAspectRatio = 18.0f / 9.0f;
                        if (mVideoRotationDegree == 90 || mVideoRotationDegree == 270) {
                            displayAspectRatio = 1.0f / displayAspectRatio;
                        }
                        break;
                    case FunVideoType.SCREEN_TYPE_4_3:
                        displayAspectRatio = 4.0f / 3.0f;
                        if (mVideoRotationDegree == 90 || mVideoRotationDegree == 270) {
                            displayAspectRatio = 1.0f / displayAspectRatio;
                        }
                        break;
                    case FunVideoType.SCREEN_TYPE_CUSTOM:
                        displayAspectRatio = FunVideoType.getScreenScaleRatio();
                        if (mVideoRotationDegree == 90 || mVideoRotationDegree == 270) {
                            displayAspectRatio = 1.0f / displayAspectRatio;
                        }
                        break;
                    case FunVideoType.SCREEN_TYPE_DEFAULT:
                    case FunVideoType.SCREEN_TYPE_FULL:
                    default:
                        displayAspectRatio = (float) realWidth / (float) mVideoHeight;
                        break;
                }

                boolean shouldBeWider = displayAspectRatio > specAspectRatio;
                switch (mCurrentAspectRatio) {
                    case FunVideoType.SCREEN_TYPE_DEFAULT:
                    case FunVideoType.SCREEN_TYPE_16_9:
                    case FunVideoType.SCREEN_TYPE_4_3:
                    case FunVideoType.SCREEN_TYPE_18_9:
                    case FunVideoType.SCREEN_TYPE_CUSTOM:
                        if (shouldBeWider) {
                            // too wide, fix width
                            width = widthSpecSize;
                            height = (int) (width / displayAspectRatio);
                        } else {
                            // too high, fix height
                            height = heightSpecSize;
                            width = (int) (height * displayAspectRatio);
                        }
                        break;
                    case FunVideoType.SCREEN_TYPE_FULL:
                        if (shouldBeWider) {
                            // not high enough, fix height
                            height = heightSpecSize;
                            width = (int) (height * displayAspectRatio);
                        } else {
                            // not wide enough, fix width
                            width = widthSpecSize;
                            height = (int) (width / displayAspectRatio);
                        }
                        break;
                    default:
                        if (shouldBeWider) {
                            // too wide, fix width
                            width = Math.min(realWidth, widthSpecSize);
                            height = (int) (width / displayAspectRatio);
                        } else {
                            // too high, fix height
                            height = Math.min(mVideoHeight, heightSpecSize);
                            width = (int) (height * displayAspectRatio);
                        }
                        break;
                }
            } else if (widthSpecMode == View.MeasureSpec.EXACTLY && heightSpecMode == View.MeasureSpec.EXACTLY) {
                // the size is fixed
                width = widthSpecSize;
                height = heightSpecSize;

                // for compatibility, we adjust size based on aspect ratio
                if (realWidth * height < width * mVideoHeight) {
                    //Log.i("@@@", "image too wide, correcting");
                    width = height * realWidth / mVideoHeight;
                } else if (realWidth * height > width * mVideoHeight) {
                    //Log.i("@@@", "image too tall, correcting");
                    height = width * mVideoHeight / realWidth;
                }
            } else if (widthSpecMode == View.MeasureSpec.EXACTLY) {
                // only the width is fixed, adjust the height to match aspect ratio if possible
                width = widthSpecSize;
                height = width * mVideoHeight / realWidth;
                if (heightSpecMode == View.MeasureSpec.AT_MOST && height > heightSpecSize) {
                    // couldn't match aspect ratio within the constraints
                    height = heightSpecSize;
                }
            } else if (heightSpecMode == View.MeasureSpec.EXACTLY) {
                // only the height is fixed, adjust the width to match aspect ratio if possible
                height = heightSpecSize;
                width = height * realWidth / mVideoHeight;
                if (widthSpecMode == View.MeasureSpec.AT_MOST && width > widthSpecSize) {
                    // couldn't match aspect ratio within the constraints
                    width = widthSpecSize;
                }
            } else {
                // neither the width nor the height are fixed, try to use actual video size
                width = realWidth;
                height = mVideoHeight;
                if (heightSpecMode == View.MeasureSpec.AT_MOST && height > heightSpecSize) {
                    // too tall, decrease both width and height
                    height = heightSpecSize;
                    width = height * realWidth / mVideoHeight;
                }
                if (widthSpecMode == View.MeasureSpec.AT_MOST && width > widthSpecSize) {
                    // too wide, decrease both width and height
                    width = widthSpecSize;
                    height = width * mVideoHeight / realWidth;
                }
            }
        } else {
            // no size yet, just adopt the given spec sizes
        }

        mMeasuredWidth = width;
        mMeasuredHeight = height;
    }

    public void prepareMeasure(int widthMeasureSpec, int heightMeasureSpec, int rotate) {
        if (mParamsListener != null) {
            try {
                int videoWidth = mParamsListener.getCurrentVideoWidth();
                int videoHeight = mParamsListener.getCurrentVideoHeight();
                LogUtils.vTag(Config.TAG,
                        "videoWidth: " + videoWidth + " videoHeight: " + videoHeight);

                if (videoWidth > 0 && videoHeight > 0) {
                    setVideoSize(videoWidth, videoHeight);
                }

                setVideoRotation(rotate);
                doMeasure(widthMeasureSpec, heightMeasureSpec);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public int getMeasuredWidth() {
        return mMeasuredWidth;
    }

    public int getMeasuredHeight() {
        return mMeasuredHeight;
    }

    public void setAspectRatio(int aspectRatio) {
        mCurrentAspectRatio = aspectRatio;
    }

    /**
     * 构造宽高所需要的视频相关参数
     */
    public interface MeasureFormVideoParamsListener {
        int getCurrentVideoWidth();

        int getCurrentVideoHeight();
    }
}
