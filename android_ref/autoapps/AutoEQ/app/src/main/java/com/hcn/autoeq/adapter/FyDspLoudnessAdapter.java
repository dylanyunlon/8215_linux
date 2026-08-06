package com.hcn.autoeq.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Spinner;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.FyDspHLPFFreq;
import com.hcn.autoeq.bean.FyDspHLPFSlope;
import com.hcn.autoeq.bean.FyDspLoudness;
import com.hcn.autoeq.view.MarqueeTextView;

import java.util.List;

public class FyDspLoudnessAdapter extends BaseAdapter {
    private List<FyDspLoudness> fyDspLoudnessList;
    private Context mContext;
    private Spinner spinner;

    public FyDspLoudnessAdapter(Context context, List<FyDspLoudness> fyDspLoudnessList, Spinner spinner) {
        this.mContext = context;
        this.fyDspLoudnessList = fyDspLoudnessList;
        this.spinner = spinner;
    }

    @Override
    public int getCount() {
        return fyDspLoudnessList.size();
    }

    @Override
    public Object getItem(int i) {
        return fyDspLoudnessList.get(i);
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
            textView.setText(FyDspLoudness.format(mContext, fyDspLoudnessList.get(position)));

            if (spinner.getSelectedItemPosition() == position) {
                textView.setBackgroundResource(R.drawable.fydsp_hlpf_spinner_item_f);
            }
        }
        return view;
    }
}
