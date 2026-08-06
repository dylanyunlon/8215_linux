package com.hcn.autoeq.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Spinner;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.FyDspHLPFFreq;
import com.hcn.autoeq.view.MarqueeTextView;

import java.util.List;

public class FyDspFreqAdapter extends BaseAdapter {
    private List<FyDspHLPFFreq> fyDspHLPFFreqList;
    private Context mContext;
    private Spinner spinner;

    public FyDspFreqAdapter(Context context, List<FyDspHLPFFreq> fyDspHLPFFreqList, Spinner spinner) {
        this.mContext = context;
        this.fyDspHLPFFreqList = fyDspHLPFFreqList;
        this.spinner = spinner;
    }

    @Override
    public int getCount() {
        return fyDspHLPFFreqList.size();
    }

    @Override
    public Object getItem(int i) {
        return fyDspHLPFFreqList.get(i);
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
            textView.setText(fyDspHLPFFreqList.get(position).getText());

            if (spinner.getSelectedItemPosition() == position) {
                textView.setBackgroundResource(R.drawable.fydsp_hlpf_spinner_item_f);
            }
        }
        return view;
    }
}
