package com.hcn.autoradio.view;

import android.content.ClipData;
import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.hcn.autoradio.R;
import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.skin.SkinUtils;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class PtyAdapter extends BaseAdapter {
    private WeakReference<Context> mContext = null;
    private List<Integer> mData = new ArrayList<Integer>();
    private FMDataControl mFMDCC= FMDataControl.getInstance();
    public PtyAdapter(Context context, List<String> data) {
        mContext = new WeakReference<>(context);
        mData.add(R.string.pty_type_none);
        mData.add(R.string.pty_type_news);
        mData.add(R.string.pty_type_affairs);
        mData.add(R.string.pty_type_info);
        mData.add(R.string.pty_type_sport);
        mData.add(R.string.pty_type_education);
        mData.add(R.string.pty_type_drama);
        mData.add(R.string.pty_type_culture);
        mData.add(R.string.pty_type_science);
        mData.add(R.string.pty_type_varied);
        mData.add(R.string.pty_type_popm);
        mData.add(R.string.pty_type_rockm);
        mData.add(R.string.pty_type_easym);
        mData.add(R.string.pty_type_lightm);
        mData.add(R.string.pty_type_classics);
        mData.add(R.string.pty_type_otherm);
        mData.add(R.string.pty_type_weather);
        mData.add(R.string.pty_type_finance);
        mData.add(R.string.pty_type_children);
        mData.add(R.string.pty_type_social);
        mData.add(R.string.pty_type_religion);
        mData.add(R.string.pty_type_phonein);
        mData.add(R.string.pty_type_travel);
        mData.add(R.string.pty_type_leisure);
        mData.add(R.string.pty_type_jazz);
        mData.add(R.string.pty_type_country);
        mData.add(R.string.pty_type_nationm);
        mData.add(R.string.pty_type_oldies);
        mData.add(R.string.pty_type_folk);
        mData.add(R.string.pty_type_document);
        mData.add(R.string.pty_type_test);
        mData.add(R.string.pty_type_alarm);
    }
    @Override
    public int getCount() {
        return mData.size();
    }

    @Override
    public Object getItem(int i) {
        return mData.get(i);
    }

    @Override
    public long getItemId(int i) {
        return i;
    }

    @Override
    public View getView(int i, View view, ViewGroup viewGroup) {
        if (i > mData.size()) {
            return null;
        }
        ListHolder holder = null;
        if (view == null) {
            view = LayoutInflater.from(mContext.get()).inflate(SkinUtils.getId(R.layout.pty_listitem),
                    viewGroup, false);
            holder = new ListHolder(view);
            view.setTag(holder);
        } else {
            holder = (ListHolder) view.getTag();
        }
        holder.radioPty.setText(SkinUtils.getString(mData.get(i)));
        if (mFMDCC.getPty() == i) {
            holder.ptyView.setVisibility(View.VISIBLE);
            holder.item_pty.setBackgroundResource(SkinUtils.getId(R.drawable.butt_ptylist_selector));
            AnimationDrawable animationDrawable = (AnimationDrawable) holder.ptyView.getBackground();
            animationDrawable.start();
        } else {
            holder.item_pty.setBackgroundResource(SkinUtils.getId(R.drawable.butt_ptylist_n_selector));
            holder.ptyView.setVisibility(View.INVISIBLE);
            AnimationDrawable animationDrawable = (AnimationDrawable) holder.ptyView.getBackground();
            animationDrawable.stop();
        }
        return view;
    }

    private static final class ListHolder {
        private View item_pty;
        private TextView radioPty;
        private View convertView;
        private ImageView ptyView;

        public ListHolder(View view) {
            convertView = view;
            if (convertView != null) {
                item_pty =  convertView.findViewById(SkinUtils.getId(R.id.item_pty));
                radioPty = (TextView) convertView.findViewById(SkinUtils.getId(R.id.radio_pty));
                ptyView = (ImageView) convertView.findViewById(SkinUtils.getId(R.id.radio_pty_play));
            }
        }
    }
}