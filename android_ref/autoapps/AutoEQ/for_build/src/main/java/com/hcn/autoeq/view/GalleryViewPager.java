package com.hcn.autoeq.view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;
import android.widget.ImageView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.viewpager.widget.ViewPager;

public class GalleryViewPager extends ViewPager {

    public GalleryViewPager(@NonNull Context context) {
        super(context);
    }

    public GalleryViewPager(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }
}
