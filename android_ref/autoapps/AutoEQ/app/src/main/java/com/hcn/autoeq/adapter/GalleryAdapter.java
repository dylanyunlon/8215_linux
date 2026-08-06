package com.hcn.autoeq.adapter;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.viewpager.widget.PagerAdapter;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.SkinUtils;

import java.util.List;

/**
 * 可无限循环的 adapter
 */
public class GalleryAdapter extends PagerAdapter {

    private static final String TAG = GalleryAdapter.class.getSimpleName();

    private Context context;
    private List<Drawable> drawableList;

    private String[] nameList;

    public GalleryAdapter(Context context, List<Drawable> drawableList, String[] nameList) {
        this.context = context;
        this.drawableList = drawableList;
        this.nameList = nameList;
    }

    /**
     * 真实数据的大小
     */
    public int getItemRawCount() {
        return nameList == null ? 0 : nameList.length;
    }

    @Override
    public int getCount() {
        return Short.MAX_VALUE;
    }

    @Override
    public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
        return view == object;
    }

    @Override
    public void destroyItem(@NonNull ViewGroup container, int position, @NonNull Object object) {
        container.removeView((View) object);
    }

    @SuppressLint("InflateParams")
    @NonNull
    @Override
    public Object instantiateItem(@NonNull ViewGroup container, final int position) {
        View view = SkinUtils.inflate(R.layout.extdsp_gallery_band_reverb_item);
        TextView btnImage = view.findViewById(SkinUtils.getId(R.id.btn_image));
        TextView btnName = view.findViewById(SkinUtils.getId(R.id.btn_name));
        btnImage.setBackground(drawableList.get(position % getItemRawCount()));
        btnName.setText(nameList[position % getItemRawCount()]);
        btnImage.setTag(position);
        view.setTag(position);
        container.addView(view);
        return view;
    }

}
