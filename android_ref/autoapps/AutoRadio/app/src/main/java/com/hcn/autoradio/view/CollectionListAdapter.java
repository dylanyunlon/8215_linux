package com.hcn.autoradio.view;

import android.content.Context;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.hcn.autoradio.R;
import com.hcn.autoradio.skin.SkinUtils;

import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Vector;

/**
 * 描述：收藏列表Adapter
 *
 * @author simon
 * @date 2023/6/19 17:44
 */
public class CollectionListAdapter extends BaseAdapter {

    String mCurFreq = "87.5";
    private WeakReference<Context> mContext = null;
    private List<String> mDataList = new Vector<>();

    public CollectionListAdapter(Context context, List<String> data) {
        mContext = new WeakReference(context);
        if (data != null && data.size() != 0) {
            mDataList.addAll(data);
        }
    }

    public void updateListData(List<String> data) {
        mDataList.clear();
        if (data != null && data.size() != 0) {
            mDataList.addAll(data);
        }
        notifyDataSetChanged();
    }

    public void updateCurFreq(String freq) {
        mCurFreq = freq;
        notifyDataSetChanged();
    }

    public int getPosition(String freq) {
        String tempInfo;
        for (int i = 0; i < mDataList.size(); i++) {
            tempInfo = mDataList.get(i);
            if (tempInfo != null && tempInfo.equals(freq)) {
                return i;
            }
        }
        return -1;
    }

    @Override
    public int getCount() {
        return mDataList.size();
    }

    @Override
    public Object getItem(int position) {
        return mDataList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if (position > mDataList.size()) {
            return null;
        }

        ListHolder holder = null;
        if (convertView == null) {
            convertView = LayoutInflater.from(mContext.get()).inflate(R.layout.radio_collection_list_item,
                    parent, false);
            holder = new ListHolder(convertView);
            convertView.setTag(holder);
        } else {
            holder = (ListHolder) convertView.getTag();
        }

        String freq = mDataList.get(position);
        holder.tvIndex.setText(String.valueOf(position+1));
        holder.tvFreq.setText(freq);

        if (mCurFreq.equals(freq)) {
            holder.convertView.setBackground(mContext.get().getDrawable(R.drawable.radio_freq_list_item_p));
        } else {
            holder.convertView.setBackground(mContext.get().getDrawable(R.drawable.radio_freq_list_item_n));
        }

        if (TextUtils.isEmpty(mDataList.get(position))) {
            holder.tvIndex.setVisibility(View.GONE);
            holder.tvFreq.setVisibility(View.GONE);
        } else {
            holder.tvIndex.setVisibility(View.VISIBLE);
            holder.tvFreq.setVisibility(View.VISIBLE);
        }

        return convertView;
    }

    private static final class ListHolder {
        private View convertView;
        private TextView tvIndex;
        private TextView tvFreq;

        public ListHolder(View view) {
            convertView = view;
            if (convertView != null) {
                tvIndex = convertView.findViewById(R.id.tv_collect_list_index);
                tvFreq = convertView.findViewById(R.id.tv_collect_list_freq);
            }
        }
    }
}
