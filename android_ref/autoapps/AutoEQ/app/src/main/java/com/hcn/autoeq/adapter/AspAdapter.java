package com.hcn.autoeq.adapter;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.AspItemBean;
import com.hcn.autoeq.bean.ExtDspBandItemBean;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.MarqueeTextView;

import java.util.List;

public class AspAdapter extends BaseAdapter {
    private List<AspItemBean> mBandItemBean;
    private Context mContext;

    public AspAdapter(List<AspItemBean> bandItemBean, Context context) {
        this.mBandItemBean = bandItemBean;
        this.mContext = context;
    }

    @Override
    public int getCount() {
        return mBandItemBean.size();
    }

    @Override
    public Object getItem(int i) {
        return mBandItemBean.get(i);
    }

    @Override
    public long getItemId(int i) {
        return i;
    }

    @Override
    public View getView(int position, View view, ViewGroup viewGroup) {
        view = SkinUtils.inflate(R.layout.extdsp_band_item);
        if (view != null) {
            MarqueeTextView textView = view.findViewById(SkinUtils.getId(R.id.tv_band_mode));
            ImageView imageView = view.findViewById(SkinUtils.getId(R.id.tv_band_mode_select));
            textView.setText(mBandItemBean.get(position).getName());
            if (mBandItemBean.get(position).getHide()){
                mBandItemBean.get(position).setHide(false);
                imageView.setImageDrawable(null);
                textView.setSelected(true);
                return view;
            }
            if (imageView != null) {
                Drawable id = mBandItemBean.get(position).getIcon();
                if (id != null) {
                    imageView.setImageDrawable(id);
                    textView.setSelected(true);

                } else {
                    imageView.setImageDrawable(null);
                    textView.setSelected(false);
                }
            }
        }
        return view;
    }
}
