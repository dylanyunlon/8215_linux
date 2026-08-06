package com.hcn.autoeq.fragment.extdsp;

import android.content.Context;
import android.graphics.Rect;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.fragment.app.DialogFragment;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.ExtDspBandSettings;
import com.hcn.autoeq.data.SIExtDspBandSettings;
import com.hcn.autoeq.util.ConstantExtDsp;
import com.hcn.autoeq.util.EqUtils;

import java.util.Locale;

public class ExtDspQValueFragment extends DialogFragment implements View.OnClickListener, ConstantExtDsp {
    private static final String TAG = ExtDspQValueFragment.class.getSimpleName();

    private Context context;
    private ExtDspBandSettings extDspBandSettings;
    private SIExtDspBandSettings mSIExtDspBandSettings;
    private final int SEEK_BAR_MIN = 500; // q值调节最小为0.5
    private final int SEEK_BAR_MAX = 6000; // q值调节最大为6
    private SeekBar seekBar;
    private TextView tvQValue;
    private int position, gainValue, currentQValue;
    private Callback callback;
    private boolean isSI47925;

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        this.context = context;
        extDspBandSettings = ExtDspBandSettings.getInstance(this.context);
        mSIExtDspBandSettings = SIExtDspBandSettings.getInstance(this.context);
        isSI47925 = EqUtils.DSP_CHIP_SI47925.equals(EqUtils.getEqChipType());
    }

    public ExtDspQValueFragment() {
    }

    /**
     * 把当前调节的数据传入到对话框中
     *
     * @param position      第几段索引
     * @param currentQValue 当前调节的q值
     * @return
     */
    public static ExtDspQValueFragment newInstance(int position, int gainValue, int currentQValue) {
        ExtDspQValueFragment fragment = new ExtDspQValueFragment();
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

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        // 设置dialog背景透明，才可以显示圆角背景图
        getDialog().getWindow().setBackgroundDrawableResource(android.R.color.transparent);
        View view = inflater.inflate(R.layout.extdsp_fragment_qvalue, container, false);
//        view.setOnClickListener(this); // 触摸任何地方都关闭窗口

        seekBar = view.findViewById(R.id.sb_qvalue);
        //防止点击父布局时，导致seekbar也响应点击事件，资源文件显示被点击状态，出现未响应的错觉
        view.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                // 获取点击位置
                int x = (int) event.getX();
                int y = (int) event.getY();

                // 检查点击是否发生在 SeekBar 内部
                Rect seekBarRect = new Rect();
                seekBar.getGlobalVisibleRect(seekBarRect);
                if (!seekBarRect.contains(x, y)) {
                    return true; // 消费点击事件，不在 SeekBar 上
                }

                return false; // 不消费点击事件，让 SeekBar 处理
            }
        });
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
                if (isSI47925) {
                    mSIExtDspBandSettings.nativeBand(position, gainValue, progress);
                } else {
                    extDspBandSettings.nativeBand(position, gainValue, progress);
                }
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
                int[][] bandValue;
                if (isSI47925) {
                    bandValue = mSIExtDspBandSettings.getUserBandValue(mSIExtDspBandSettings.getReverb());
                } else {
                    bandValue = extDspBandSettings.getUserBandValue(extDspBandSettings.getReverb());
                }
                for (int i = 0; i < bandValue[1].length; i++) {
                    if (i == position) {
                        bandValue[1][position] = progress;
                        break;
                    }
                }
                // 修改完后，重新保存到文件
                if (isSI47925) {
                    mSIExtDspBandSettings.saveBandValue(bandValue);
                } else {
                    extDspBandSettings.saveBandValue(bandValue);
                }
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
