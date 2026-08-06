package com.hcn.autoeq.fragment.ecdspt;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.fragment.app.DialogFragment;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.ExtDspBandSettings;
import com.hcn.autoeq.util.ECDConstantExtDsp;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.util.Locale;

public class ECDspQValueFragment extends DialogFragment implements View.OnClickListener, ECDConstantExtDsp {
    private static final String TAG = ECDspQValueFragment.class.getSimpleName();

    private Context context;
    private ExtDspBandSettings extDspBandSettings;
    private final int SEEK_BAR_MIN = 500; // q值调节最小为0.5
    private final int SEEK_BAR_MAX = 6000; // q值调节最大为6
    private SeekBar seekBar;
    private TextView tvQValue;
    private int position, gainValue, currentQValue;
    private Callback callback;

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        this.context = context;
        extDspBandSettings = ExtDspBandSettings.getInstance(this.context);
    }

    public ECDspQValueFragment() {
    }

    /**
     * 把当前调节的数据传入到对话框中
     *
     * @param position      第几段索引
     * @param currentQValue 当前调节的q值
     * @return
     */
    public static ECDspQValueFragment newInstance(int position, int gainValue, int currentQValue) {
        ECDspQValueFragment fragment = new ECDspQValueFragment();
        Bundle args = new Bundle();
        args.putInt("position", position);
        args.putInt("gainValue", gainValue);
        args.putInt("currentQValue", currentQValue);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getArguments() != null) {
            position = getArguments().getInt("position");
            gainValue = getArguments().getInt("gainValue");
            currentQValue = getArguments().getInt("currentQValue");
        }
    }

    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    public boolean checkLayoutExists(int layoutId) {
        boolean isLayoutIdExit = true;
        int id = SkinCompatResources.getInstance().getSkinResId(layoutId, "layout");
        if (id == 0) {
            Log.e(TAG, "getLayoutRes: layout not found.");
            isLayoutIdExit = false;
        }
        return isLayoutIdExit;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        // 设置dialog背景透明，才可以显示圆角背景图
        getDialog().getWindow().setBackgroundDrawableResource(android.R.color.transparent);
        View view;
        if (checkLayoutExists(R.layout.ext_c_dsp_fragment_qvalue)) {
            view = inflater.inflate(R.layout.ext_c_dsp_fragment_qvalue, container, false);
        } else {
            view = inflater.inflate(R.layout.extdsp_fragment_qvalue, container, false);
        }
        view.setOnClickListener(this); // 触摸任何地方都关闭窗口

        seekBar = view.findViewById(R.id.sb_qvalue);
        // FIXME, 换肤方式，在xml里设置 padding=0, offset=0，滑块依然会超出，通过代码设置后正常
        seekBar.setPadding((int) getResources().getDimension(R.dimen.extdsp_seekbar_padding), (int) getResources().getDimension(R.dimen.extdsp_seekbar_padding)
                , (int) getResources().getDimension(R.dimen.extdsp_seekbar_padding), (int) getResources().getDimension(R.dimen.extdsp_seekbar_padding));
        seekBar.setThumbOffset((int) getResources().getDimension(R.dimen.extdsp_seekbar_thumb_offset));
        seekBar.setMax(SEEK_BAR_MAX);
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // 不能小于最低的范围值
                if (progress < SEEK_BAR_MIN) {
                    progress = SEEK_BAR_MIN;
                }
                tvQValue.setText(String.format(Locale.getDefault(), "%.1f", progress / 1000f).replace(".0", ""));
                extDspBandSettings.nativeBand(position, gainValue, progress);

                if (callback != null) {
                    callback.onQValueChanged(progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                int progress = seekBar.getProgress();
                // 不能小于最低的范围值
                if (progress < SEEK_BAR_MIN) {
                    progress = SEEK_BAR_MIN;
                }

                // 拿出当前保存的数值，并修改当前索引的q值
                int[][] bandValue = extDspBandSettings.getUserBandValue(extDspBandSettings.getReverb());
                for (int i = 0; i < bandValue[1].length; i++) {
                    if (i == position) {
                        bandValue[1][position] = progress;
                        break;
                    }
                }
                // 修改完后，重新保存到文件
                extDspBandSettings.saveBandValue(bandValue);
            }
        });
        this.tvQValue = view.findViewById(R.id.tv_q_value);
        return view;
    }

    @Override
    public void onResume() {
        super.onResume();
        seekBar.setProgress(currentQValue);
        tvQValue.setText(String.format(Locale.getDefault(), "%.1f", currentQValue / 1000f).replace(".0", ""));
    }

    @Override
    public void onClick(View v) {
        dismiss();
    }

    public interface Callback {
        void onQValueChanged(int qValue);
    }
}
