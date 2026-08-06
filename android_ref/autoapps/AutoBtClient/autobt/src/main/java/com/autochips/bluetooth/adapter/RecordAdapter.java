package com.autochips.bluetooth.adapter;

import static android.provider.CallLog.Calls.INCOMING_TYPE;
import static android.provider.CallLog.Calls.OUTGOING_TYPE;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.autochips.bluetooth.bean.HRecord;
import com.autochips.bluetooth.R;
import com.autochips.bluetooth.util.SkinUtils;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

public class RecordAdapter extends BaseAdapter {
    private Context mContext;
    private List<HRecord> mData = new ArrayList<>();

    public RecordAdapter(Context mContext) {
        this.mContext = mContext;
    }

    public void setData(List<HRecord> data) {
        if(data != null){
            mData.clear();
            mData.addAll(data);
            notifyDataSetChanged();
        }
    }

    public void reset(){
        if(mData != null){
            mData.clear();
        }
        notifyDataSetChanged();
    }

    @Override
    public int getCount() {
        return mData != null ? mData.size() : 0;
    }

    @Override
    public Object getItem(int i) {
        return mData != null ? mData.get(i) : null;
    }

    @Override
    public long getItemId(int i) {
        return 0;
    }

    @Override
    public View getView(int i, View view, ViewGroup viewGroup) {
        RecordHolder holder = null;
        if(view == null){
            view = View.inflate(mContext, SkinUtils.getId(R.layout.adapter_record),null);
            holder = new RecordHolder();
            holder.icon = view.findViewById(SkinUtils.getId(R.id.id_record_adapter_icon));
            holder.name = view.findViewById(SkinUtils.getId(R.id.id_record_adapter_name));
            holder.number = view.findViewById(SkinUtils.getId(R.id.id_record_adapter_number));
            holder.date = view.findViewById(SkinUtils.getId(R.id.id_record_adapter_date));
            holder.time = view.findViewById(SkinUtils.getId(R.id.id_record_adapter_time));
            view.setTag(holder);
        }else{
            holder = (RecordHolder) view.getTag();
        }
        HRecord record = mData.get(i);
        int type = record.getType();
        switch(type){
            case OUTGOING_TYPE:
                holder.icon.setImageResource(SkinUtils.getResId(R.drawable.icon_outgoing_call));
                break;
            case INCOMING_TYPE:
                holder.icon.setImageResource(SkinUtils.getResId(R.drawable.icon_incoming_call));
                break;
            default:
                holder.icon.setImageResource(SkinUtils.getResId(R.drawable.icon_missed_call));
                break;
        }
        holder.name.setText(record.getName());
        holder.number.setText(record.getPhone());
        String[] time = getTime(record.getTime());
        if(time != null && time.length > 1) {
            holder.date.setText(time[0]);
            holder.time.setText(time[1]);
        }
        return view;
    }

    class RecordHolder{
        ImageView icon;
        TextView name;
        TextView number;
        TextView date;
        TextView time;
    }

    private String[] getTime(long time){
        SimpleDateFormat sdf = new SimpleDateFormat("yy/MM/dd&HH:mm:ss");
        Date date = new Date(time);
        String strDate = sdf.format(date);

        return strDate.split("&");
    }
}
