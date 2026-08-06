package com.hcn.autoradio.view;

import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.hcn.autoradio.R;
import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.skin.SkinUtils;
import com.hcn.autoradio.ui.RadioHz;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

public class CollectionAdapter extends BaseAdapter {
    String mCurFreq = "87.5";
    private FMDataControl mFMDCC= FMDataControl.getInstance();
    private WeakReference<Context> mContext = null;
    private List<String> mData = new ArrayList<String>();

    public CollectionAdapter(Context context, List<String> data) {
        mContext = new WeakReference<>(context);

        if (data != null && data.size() != 0) {
            mData.addAll(data);
        }
    }

    public void updateListData(List<String> data) {
        mData.clear();
        if (data != null && data.size() != 0) {
            mData.addAll(data);
        }
        notifyDataSetChanged();
    }

    public void updateCurFreq(String freq) {
        mCurFreq = freq;
        notifyDataSetChanged();
    }
    public int getPosition(String freq) {
        String tempInfo;
        for (int i = 0; i < mData.size(); i++) {
            tempInfo=mData.get(i);
            if(tempInfo!=null && tempInfo.equals(freq)){
                return i;
            }
        }
        return -1;
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
            view = LayoutInflater.from(mContext.get()).inflate(SkinUtils.getId(R.layout.collection_listitem),
                    viewGroup, false);
            holder = new ListHolder(view);
            view.setTag(holder);
        } else {
            holder = (ListHolder) view.getTag();
        }

        String freq = mData.get(i);
        String string = "";
        /*20211222 hide no do */
        if(mFMDCC.isFMBand()){
            //string="FM "+freq;
            string="FM    " +freq+ "    MHz";
        }else{
            //string="AM "+freq;
            string= "AM    " +freq+"    KHz";
        }
        holder.freq.setText(string);
        if (freq!=null && freq.equals(mCurFreq)) {
            holder.playView.setVisibility(View.VISIBLE);
            holder.item_container.setBackgroundResource(SkinUtils.getId(R.drawable.butt_ptylist_selector));
            AnimationDrawable animationDrawable = (AnimationDrawable) holder.playView.getBackground();
            animationDrawable.start();
        } else {
            holder.item_container.setBackgroundResource(SkinUtils.getId(R.drawable.butt_ptylist_n_selector));
            holder.playView.setVisibility(View.INVISIBLE);
            AnimationDrawable animationDrawable = (AnimationDrawable) holder.playView.getBackground();
            animationDrawable.stop();
        }

        return view;
    }

    private static final class ListHolder {
        private View item_container;
        private TextView freq;
        private View convertView;
        private ImageView playView;
        private ImageView notCollection;

        public ListHolder(View view) {
            convertView = view;
            if (convertView != null) {
               item_container =  convertView.findViewById(SkinUtils.getId(R.id.item_container));
                freq = (TextView) convertView.findViewById(SkinUtils.getId(R.id.collection_freq));
                notCollection = convertView.findViewById(SkinUtils.getId(R.id.btn_not_collection));
                playView = (ImageView) convertView.findViewById(SkinUtils.getId(R.id.collection_play));
            }
        }
    }
}

