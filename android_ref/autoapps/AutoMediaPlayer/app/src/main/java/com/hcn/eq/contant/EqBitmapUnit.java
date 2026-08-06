package com.hcn.eq.contant;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.PixelFormat;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;

import com.hcn.media_common.debug.LogUtil;

public class EqBitmapUnit {


    private static final String TAG = "EqBitmapUnit";


    /**
     *
     */
    public static Bitmap drawableToBitmap(Drawable drawable) {

        if (null == drawable) {
            return null;
        }


        int width = drawable.getIntrinsicWidth();
        int height = drawable.getIntrinsicHeight();

        LogUtil.i(TAG, "drawableToBitmap()----->width:" + width + "\t height:" + height);

        width = Math.round(width / ScreenSpec.mScreenDensity + 0.1F);
        height = Math.round(height / ScreenSpec.mScreenDensity + 0.1F);

        LogUtil.i(TAG, "drawableToBitmap()=====>width:" + width + "\t height:" + height);

        Bitmap.Config config =
                (drawable.getOpacity() != PixelFormat.OPAQUE) ? Bitmap.Config.ARGB_8888
                        : Bitmap.Config.ARGB_8888;


        Bitmap bitmap = Bitmap.createBitmap(width, height, config);


        Canvas canvas = new Canvas(bitmap);


        drawable.setBounds(0, 0, width, height - 5);


        drawable.draw(canvas);

        return bitmap;
    }

    /**
     * Bitmap to Drawable
     */
    public static Drawable bitmapToDrawble(Bitmap bitmap, Context context) {

        if (null == bitmap || null == context) {
            return null;
        }

        return new BitmapDrawable(context.getResources(), bitmap);
    }
}
