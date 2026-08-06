package com.hcn.autoeq.adapter;


import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.hcn.autoeq.R;
import com.hcn_library.util.SkinUtils;

import java.util.List;

public class CircularAdapter extends RecyclerView.Adapter<CircularAdapter.ViewHolder> {
    private List<String> dataList;

    public CircularAdapter(List<String> dataList) {
        this.dataList = dataList;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(SkinUtils.getContext()).inflate(SkinUtils.getId(R.layout.nine_freq_item_layout), parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        holder.textView.setText(dataList.get(position % dataList.size()));
    }

    @Override
    public int getItemCount() {
        return dataList.size();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        TextView textView;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            textView = itemView.findViewById(SkinUtils.getId(R.id.textView));
        }
    }
}