package com.autochips.bluetooth.adapter;

import android.content.Context;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.autochips.bluetooth.bean.HContact;
import com.autochips.bluetooth.R;
import com.autochips.bluetooth.util.SkinUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class ContactsAdapter extends BaseAdapter {
    private Context mContext ;
    private List<HContact> mData = new ArrayList<>();
    private String mKey = null;

    public ContactsAdapter(Context mContext) {
        this.mContext = mContext;
    }

    public ContactsAdapter(Context mContext, List<HContact> data) {
        this.mContext = mContext;
        setData(data);
    }

    public void setData(List<HContact> data) {
        if(data != null){
            mData.clear();
            mData.addAll(data);
        }
        notifyDataSetChanged();
    }

    public void resetData(){
        if(mData != null) {
            mData.clear();
        }
        notifyDataSetChanged();
    }

    @Override
    public void notifyDataSetChanged() {
        if(mData != null){
            Collections.sort(mData);
        }
        super.notifyDataSetChanged();
    }

    @Override
    public int getCount() {
        return mData != null ? mData.size() : 0;
    }

    @Override
    public Object getItem(int position) {
        return mData != null ? mData.get(position) : null;
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ContactHolder holder = null;
        if(convertView == null){
            convertView = View.inflate(mContext,SkinUtils.getId(R.layout.adapter_contacts), null);
            holder = new ContactHolder();
            holder.name = convertView.findViewById(SkinUtils.getId(R.id.id_contact_adapter_name));
            holder.number = convertView.findViewById(SkinUtils.getId(R.id.id_contact_adapter_number));
            convertView.setTag(holder);
        }else{
            holder = (ContactHolder) convertView.getTag();
        }

        HContact contact = mData.get(position);
        holder.name.setText(contact.getName());
        holder.number.setText(contact.getPhone());
        return convertView;
    }

    class ContactHolder{
        TextView name;
        TextView number;
    }
}
