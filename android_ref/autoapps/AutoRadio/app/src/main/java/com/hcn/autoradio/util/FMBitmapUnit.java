package com.hcn.autoradio.util;

import com.hcn.autoradio.ScreenSpec;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.PixelFormat;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;

public class FMBitmapUnit {

    public static Bitmap drawableToBitmap(Drawable drawable) {

        if (null == drawable) {
            return null;
        }


        int width = drawable.getIntrinsicWidth();
        int height = drawable.getIntrinsicHeight();

        width = Math.round(width / ScreenSpec.mScreenDensity + 0.1F);
        height = Math.round(height / ScreenSpec.mScreenDensity + 0.1F);


        Bitmap.Config config =
                (drawable.getOpacity() != PixelFormat.OPAQUE) ? Bitmap.Config.ARGB_8888
                        : Bitmap.Config.RGB_565;


        Bitmap bitmap = Bitmap.createBitmap(width, height, config);


        Canvas canvas = new Canvas(bitmap);


        drawable.setBounds(0, 0, width, height);


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
