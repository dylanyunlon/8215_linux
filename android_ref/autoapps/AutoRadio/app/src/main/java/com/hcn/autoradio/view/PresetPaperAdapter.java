package com.hcn.autoradio.view;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.viewpager.widget.PagerAdapter;

import com.hcn.autoradio.R;
import com.hcn.autoradio.skin.SkinUtils;

public class PresetPaperAdapter extends PagerAdapter {
    private Context mContext;
    private OnPresetPaperAdapterListener mListener = null;
    private View[] mPageViews = new View[3];

    public PresetPaperAdapter(Context context) {
        super();
        mContext = context;
    }

    @NonNull
    @Override
    public Object instantiateItem(@NonNull ViewGroup container, int position) {
        if (mPageViews[position] == null) {
            mPageViews[position] = View.inflate(mContext, SkinUtils.getId(R.layout.preset_layout), null);
            if (null != mListener) {
                mListener.onInitPresetPaper(mPageViews[position], position);
            }
        }
        container.addView(mPageViews[position]);
        return mPageViews[position];
    }

    @Override
    public void destroyItem(@NonNull ViewGroup container, int position, @NonNull Object object) {
        container.removeView((View) object);
    }

    @Override
    public int getCount() {
        return mPageViews.length;
    }

    @Override
    public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
        return view == object;
    }

    public interface OnPresetPaperAdapterListener {
        void onInitPresetPaper(View view, int position);
    }

    public void setOnPresetPaperAdapterListener(OnPresetPaperAdapterListener listener) {
        mListener = listener;
    }
}
