package com.hcn.autoeq.fragment.fydsp;

import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SeekBar;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.Band;
import com.hcn.autoeq.bean.EventMessage;
import com.hcn.autoeq.data.FyDspBandSettings;
import com.hcn.autoeq.view.FyDspSeekBar;

import org.greenrobot.eventbus.EventBus;

import java.util.Locale;

public class FyDspQValueDialog extends DialogFragment {
    private static final String TAG = FyDspQValueDialog.class.getSimpleName();

    private static final int SEEK_BAR_MIN_Q_VALUE = 1; // q值调节最小为0.1
    private static final int SEEK_BAR_MAX_Q_VALUE = 100; // q值调节最大为10
    public static final float Q_VALUE_PRECISION = 10f; // q值百分比，数值范围是1~100，显示是0.1~10

    private Context context;
    private FyDspBandSettings fyDspBandSettings;
    private FyDspSeekBar fdsbQ;
    private Band band;
    private Callback callback;

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        this.context = context;
        fyDspBandSettings = FyDspBandSettings.getInstance(this.context);
    }

    public FyDspQValueDialog() {
    }

    /**
     * 把当前调节的数据传入到对话框中
     *
     * @return
     */
    public static FyDspQValueDialog newInstance(Band band) {
        FyDspQValueDialog fragment = new FyDspQValueDialog();
        Bundle args = new Bundle();
        args.putParcelable("band", band);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setStyle(DialogFragment.STYLE_NO_TITLE, android.R.style.Theme_Dialog);
        Bundle bundle = getArguments();
        if (getArguments() != null) {
            band = bundle.getParcelable("band");
        }
    }

    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        // 设置dialog背景透明，才可以显示圆角背景图
        getDialog().getWindow().setBackgroundDrawableResource(android.R.color.transparent);
        View view = inflater.inflate(R.layout.fydsp_fragment_qvalue, container, false);

        fdsbQ = view.findViewById(R.id.fdsb_q);
        fdsbQ.setTitle("Q:");
        fdsbQ.getSeekBar().setMin(SEEK_BAR_MIN_Q_VALUE);
        fdsbQ.getSeekBar().setMax(SEEK_BAR_MAX_Q_VALUE);
        fdsbQ.setMinText(String.format(Locale.getDefault(), "%.1f", SEEK_BAR_MIN_Q_VALUE / Q_VALUE_PRECISION).replace(".0", ""));
        fdsbQ.setMaxText(String.format(Locale.getDefault(), "%.1f", SEEK_BAR_MAX_Q_VALUE / Q_VALUE_PRECISION).replace(".0", ""));
        fdsbQ.getSeekBar().setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // 进度条拖动的时候，只修改底层音效和临时变量，拖动完毕后，再把临时变量值保存到 sp 文件中
                // 不能小于最低的范围值
                if (progress < SEEK_BAR_MIN_Q_VALUE) {
                    progress = SEEK_BAR_MIN_Q_VALUE;
                }
                fdsbQ.setValue(String.format(Locale.getDefault(), "%.1f", progress / Q_VALUE_PRECISION).replace(".0", ""));
                band.setQ(progress);
                fyDspBandSettings.nativeBandQ(band);

                if (callback != null) {
                    callback.onQValueChanged(progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                fyDspBandSettings.saveBandQ(band);
                EventBus.getDefault().post(new EventMessage(EventMessage.MSG_BAND_Q_VALUE_CHANGED, band));
            }
        });

        return view;
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        fdsbQ.setProgress(band.getQ());
        fdsbQ.setValue(String.format(Locale.getDefault(), "%.1f", band.getQ() / Q_VALUE_PRECISION).replace(".0", ""));
    }

    public interface Callback {
        void onQValueChanged(int qValue);
    }

    @Override
    public void onPause() {
        super.onPause();
        dismiss();
    }
}
