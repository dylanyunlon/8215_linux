package com.hcn.autoeq.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Spinner;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.FyDspHLPFSlope;
import com.hcn.autoeq.view.MarqueeTextView;

import java.util.List;

public class FyDspSlopeAdapter extends BaseAdapter {
    private List<FyDspHLPFSlope> fyDspHLPFSlopeList;
    private Context mContext;
    private Spinner spinner;

    public FyDspSlopeAdapter(Context context, List<FyDspHLPFSlope> fyDspHLPFSlopeList, Spinner spinner) {
        this.mContext = context;
        this.fyDspHLPFSlopeList = fyDspHLPFSlopeList;
        this.spinner = spinner;
    }

    @Override
    public int getCount() {
        return fyDspHLPFSlopeList.size();
    }

    @Override
    public Object getItem(int i) {
        return fyDspHLPFSlopeList.get(i);
    }

    @Override
    public long getItemId(int i) {
        return i;
    }

    @Override
    public View getView(int position, View view, ViewGroup viewGroup) {
        view = LayoutInflater.from(mContext).inflate(R.layout.fydsp_spinner_item, null);
        if (view != null) {
            MarqueeTextView textView = view.findViewById(R.id.tv_title);
            textView.setText(FyDspHLPFSlope.format(mContext, fyDspHLPFSlopeList.get(position)));

            if (spinner.getSelectedItemPosition() == position) {
                textView.setBackgroundResource(R.drawable.fydsp_hlpf_spinner_item_f);
            }
        }
        return view;
    }
}
