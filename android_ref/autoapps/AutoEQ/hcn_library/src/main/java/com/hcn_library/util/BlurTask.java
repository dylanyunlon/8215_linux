package com.hcn_library.util;

import android.content.Context;
import android.graphics.drawable.BitmapDrawable;
import android.os.AsyncTask;
import android.util.Log;
import android.view.View;

 public class BlurTask extends AsyncTask<Void, Void, BitmapDrawable[]> {
    private int currentDrawableId;
    private View[] views;
    private int[] drawableIds;
    private float[] cornerRadii;
    private boolean[] ifBlur;
    private Context mContext;
    private int[] positionArr;
    private int blurWindowHeight;
    private int blurWindowWidth;

    public BlurTask(int currentDrawableId, View[] views, int[] drawableIds, float[] cornerRadii, Context context, int[] positionArr,int blurWindowHeight, int blurWindowWidth, boolean[] ifBlur) {
        this.currentDrawableId = currentDrawableId;
        this.views = views;
        this.drawableIds = drawableIds;
        this.cornerRadii = cornerRadii;
        this.mContext = context;
        this.positionArr = positionArr;
        this.blurWindowHeight = blurWindowHeight;
        this.blurWindowWidth = blurWindowWidth;
        this.ifBlur = ifBlur;
    }


    @Override
    protected BitmapDrawable[] doInBackground(Void... voids) {
        BitmapDrawable[] blurBackgrounds = new BitmapDrawable[views.length];
        for (int i = 0; i < views.length; i++) {
            Log.e("doInBackground", "doInBackground: " + i + " view height: " + views[i].getMeasuredHeight() + " view width: " + views[i].getMeasuredWidth() + " blurWindowHeight: " + blurWindowHeight + " blurWindowWidth: " + blurWindowWidth);
            if (!ifBlur[i]) {
                continue;
            }
            // 控件频谱曲线展示区，需修改模糊参数
            if (i == 7) {
                FastBlurUtils.setBlurQuality(1, 15);
            } else {
                FastBlurUtils.resetBlurQuality();
            }
            // 混响面板需要走window模糊方法
            if (i == 1) {
                blurBackgrounds[i] = FastBlurUtils.getBlurWindowBg(positionArr[0], positionArr[1], blurWindowWidth, blurWindowHeight, currentDrawableId, drawableIds[i], mContext, cornerRadii[i]);
            } else {
                blurBackgrounds[i] = FastBlurUtils.getBlurBackground(views[i].getMeasuredWidth(), views[i].getMeasuredHeight(), currentDrawableId, drawableIds[i], views[i], mContext, cornerRadii[i]);
            }
        }
        return blurBackgrounds;
    }

    @Override
    protected void onPostExecute(BitmapDrawable[] blurBackgrounds) {
        for (int i = 0; i < views.length; i++) {
            if (!ifBlur[i]) {
                continue;
            }
            if (blurBackgrounds[i] != null) {
                views[i].setBackground(blurBackgrounds[i]);
            } else {
                views[i].setBackground(SkinUtils.getDrawable(drawableIds[i]));
            }
        }
    }
}