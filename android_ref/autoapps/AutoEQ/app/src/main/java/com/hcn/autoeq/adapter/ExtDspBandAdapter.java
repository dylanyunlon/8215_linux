package com.hcn.autoeq.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.ExtDspBandItemBean;
import com.hcn.autoeq.view.MarqueeTextView;

import java.util.List;

public class ExtDspBandAdapter extends BaseAdapter {
    private List<ExtDspBandItemBean> mBandItemBean;
    private Context mContext;
    private int mSelectPosition = -1 ;

    public ExtDspBandAdapter(List<ExtDspBandItemBean> bandItemBean, Context context) {
        this.mBandItemBean = bandItemBean;
        this.mContext = context;
    }
    public void setSelectPosition(int position) {
        mSelectPosition = position;
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
        ViewHolder holder;
        if (view == null) {
            LayoutInflater layoutInflater = LayoutInflater.from(mContext);
            view = layoutInflater.inflate(R.layout.extdsp_band_item, viewGroup, false);
            holder = new ViewHolder();
            holder.textView = view.findViewById(R.id.tv_band_mode);
            holder.imageView = view.findViewById(R.id.tv_band_mode_select);
            view.setTag(holder);
        } else {
            holder = (ViewHolder) view.getTag();
        }

        if (holder != null) {
            holder.textView.setText(mBandItemBean.get(position).getName());
            if (mSelectPosition != -1) {
                if (mSelectPosition == position) {
                    holder.textView.setSelected(true);
                } else {
                    holder.textView.setSelected(false);
                }
            }
            if (holder.imageView != null) {
                int id = mBandItemBean.get(position).getIcon();
                if (id != 0) {
                    holder.imageView.setImageResource(id);
                    holder.textView.setSelected(true);
                } else {
                    holder.imageView.setImageDrawable(null);
                    holder.textView.setSelected(false);
                }
            }
        }
        return view;
    }

    private static class ViewHolder {
        MarqueeTextView textView;
        ImageView imageView;
    }

}
