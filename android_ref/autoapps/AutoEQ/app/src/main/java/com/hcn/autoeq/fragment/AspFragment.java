package com.hcn.autoeq.fragment;

import static com.hcn.autoeq.util.EqUtils.KEY_SKIN;

import android.os.Bundle;
import android.util.Log;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.Spinner;

import com.hcn.autoeq.R;
import com.hcn.autoeq.adapter.AspAdapter;
import com.hcn.autoeq.adapter.ExtDspBandAdapter;
import com.hcn.autoeq.bean.AspItemBean;
import com.hcn.autoeq.bean.ExtDspBandItemBean;
import com.hcn.autoeq.data.AspSettings;
import com.hcn.autoeq.util.ConstantAsp;
import com.hcn.autoeq.util.ConstantEq;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SetupSharedData;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.util.SystemUtils;
import com.hcn.autoeq.view.AspSeekBar;
import com.hcn.autoeq.view.CustomSpinner;
import com.hcn.common.misc.LogUtils;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;

public class AspFragment extends BaseFragment
        implements AspSeekBar.OnCustomSeekBarChangeListener
        , RadioGroup.OnCheckedChangeListener
        , ConstantAsp {

    private static final String ARG_PARAM1 = "data_band";
    private static final String ARG_PARAM2 = "param2";
    private static final String TAG = AspFragment.class.getSimpleName();
    private View mAspBandView;
    private int[] mBandData;//User模式Band数值, 模式拖动时变化. Reset时还原.
    private int[] mRealData;

    private CustomSpinner spReverb;

    private ArrayList<AspItemBean> bandItemList = null;

    private CheckBox mSurroundCheck, mLoudCheck;

    private int mBassVal, mTrebleVal;
    private int mAspSeekRes[] = {SkinUtils.getId(R.id.asp_seek_0), SkinUtils.getId(R.id.asp_seek_1),
            SkinUtils.getId(R.id.asp_seek_2), SkinUtils.getId(R.id.asp_seek_3), SkinUtils.getId(R.id.asp_seek_4),
            SkinUtils.getId(R.id.asp_seek_5), SkinUtils.getId(R.id.asp_seek_6), SkinUtils.getId(R.id.asp_seek_7),
            SkinUtils.getId(R.id.asp_seek_8), SkinUtils.getId(R.id.asp_seek_9), SkinUtils.getId(R.id.asp_seek_10),
            SkinUtils.getId(R.id.asp_seek_11)};
    private RadioGroup mAspReverbGroup;
    private Button mAspReset;

    public AspFragment() {
        // Required empty public constructor
    }

    public static AspFragment newInstance(int[] param1, int[] param2) {
        AspFragment fragment = new AspFragment();
        Bundle args = new Bundle();
        args.putIntArray(ARG_PARAM1, param1);
        args.putIntArray(ARG_PARAM2, param2);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getArguments() != null) {
            mBandData = getArguments().getIntArray(ARG_PARAM1);
            mRealData = getArguments().getIntArray(ARG_PARAM2);
        }
    }

    @Override
    public int getLayoutRes() {
        return R.layout.asp_band_fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mAspBandView = super.onCreateView(inflater, container, savedInstanceState);
        return mAspBandView;
    }

    @Override
    public void initView() {
        int mAspReverb = AspSettings.getInstance(mContext).getAspReverbType();
        mAspReverbGroup = mAspBandView.findViewById(SkinUtils.getId(R.id.asp_reverb_group));
        mAspReverbGroup.setOnCheckedChangeListener(this);
        for (int child = 0; child < mAspReverbGroup.getChildCount(); child++) {
            mAspReverbGroup.getChildAt(child).setOnFocusChangeListener(mFocusChangeListener);
        }
        ((RadioButton) mAspReverbGroup.getChildAt(mAspReverb)).requestFocus();
        ((RadioButton) mAspReverbGroup.getChildAt(mAspReverb)).setChecked(true);
        mAspReset = mAspBandView.findViewById(SkinUtils.getId(R.id.asp_reset));
        if (mAspReset != null) {
            mAspReset.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    //还原mBandData,重置seekbar,ReverbGroup.
                    mBandData = Arrays.copyOf(DEF_ASP_BANDS[ConstantEq.EQ_REVERB_USER], DEF_ASP_BANDS[ConstantEq.EQ_REVERB_USER].length);

                    AspSettings.getInstance(mContext).setAspBandValue(Arrays.toString(DEF_ASP_BANDS[ConstantEq.EQ_REVERB_USER]));
                    setAspSeekBarVal(DEF_ASP_BANDS[ConstantEq.EQ_REVERB_USER]);
                    AspSettings.getInstance(mContext).setAspReverbType(ConstantEq.EQ_REVERB_USER, new int[]{7, 7});
                    ((RadioButton) mAspReverbGroup.getChildAt(ConstantEq.EQ_REVERB_USER)).requestFocus();
                    ((RadioButton) mAspReverbGroup.getChildAt(ConstantEq.EQ_REVERB_USER)).setChecked(true);
                    if (null != spReverb){
                        spReverb.setSelection(0,true);
                        spReverb.setPressed(true);

                    }
                }
            });
        }

        spReverb = mAspBandView.findViewById(SkinUtils.getId(R.id.sp_reverb));
        if (null != spReverb){
            initReverb();
        }
            try {
                mSurroundCheck = mAspBandView.findViewById(SkinUtils.getId(R.id.dsp_ck_surround));
                // mAspBandView如果为本体的view，获取的id为皮肤包的id，在本体找到的view与皮肤包的id相同，但是类型不同，所以此处会报类型转换错误异常。
                if (null != mSurroundCheck){
                    mSurroundCheck.setChecked(AspSettings.getInstance(mContext).getAspSubWoofer() == 1);
                    mSurroundCheck.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                        @Override
                        public void onCheckedChanged(CompoundButton compoundButton, boolean mCheck) {
                            AspSettings.getInstance(mContext).setAspSubWoofer(mCheck ? 1 : 0, true);
                        }
                    });
                }

                mLoudCheck = mAspBandView.findViewById(SkinUtils.getId(R.id.dsp_ck_loudness));
                if (null != mLoudCheck){
                    mLoudCheck.setChecked(AspSettings.getInstance(mContext).getAspLoudness());

                    mLoudCheck.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                        @Override
                        public void onCheckedChanged(CompoundButton compoundButton, boolean mCheck) {
                            AspSettings.getInstance(mContext).setAspLoudness(mCheck ? 1 : 0);
                        }
                    });
                }
            }catch (Exception e){
                e.printStackTrace();
            }

    }



    //添加焦点处理,用于字符串过长走马灯显示.
    View.OnFocusChangeListener mFocusChangeListener = new View.OnFocusChangeListener() {
        @Override
        public void onFocusChange(View v, boolean hasFocus) {
            if (null != mAspReverbGroup && hasFocus) {
                ((RadioButton) mAspReverbGroup.findViewById(v.getId())).setChecked(true);
            }
        }
    };

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (!hidden) {
            int mAspReverb = AspSettings.getInstance(mContext).getAspReverbType();
            ((RadioButton) mAspReverbGroup.getChildAt(mAspReverb)).requestFocus();
            ((RadioButton) mAspReverbGroup.getChildAt(mAspReverb)).setChecked(true);
        }

    }

    //TODO used to fake band
    private void setAspSeekEnable(boolean mEnable) {
        for (int bandRes : mAspSeekRes) {
            AspSeekBar mSeekBarView = mAspBandView.findViewById(bandRes);
            if (mEnable) {
                mSeekBarView.setOnSeekBarChangeListener(this);
            } else {
                mSeekBarView.setOnSeekBarChangeListener(null);
            }
            mSeekBarView.setCanSeek(mEnable);
        }
    }

    public void setAspSeekBarVal(int[] mReverb) {
        int bandIndex = 0;
        for (int bandRes : mAspSeekRes) {
            AspSeekBar mSeekBarView = mAspBandView.findViewById(bandRes);
            mSeekBarView.setProgress(mReverb[bandIndex], true);
            bandIndex++;
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            int mTag = (int) seekBar.getTag();
            mBandData[mTag] = progress;
            AspSettings.getInstance(mContext).setAspBassBoost((mBandData[0] + mBandData[1] + mBandData[2] + mBandData[3]
                    + mBandData[4] + mBandData[5]) / 6, false);
            AspSettings.getInstance(mContext).setAspTreble((mBandData[6] + mBandData[7] + mBandData[8] + mBandData[9]
                    + mBandData[10] + mBandData[11]) / 6, false);
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        mRealData[0] = (mBandData[0] + mBandData[1] + mBandData[2] + mBandData[3]
                + mBandData[4] + mBandData[5]) / 6;
        mRealData[1] = (mBandData[6] + mBandData[7] + mBandData[8] + mBandData[9]
                + mBandData[10] + mBandData[11]) / 6;
        //结束拖动需保存ASP 高低音和每个Band值.
        AspSettings.getInstance(mContext).setAspBassBoost(mRealData[0], true);
        AspSettings.getInstance(mContext).setAspTreble(mRealData[1], true);
        AspSettings.getInstance(mContext).setAspBandValue(Arrays.toString(mBandData));
    }

    @Override
    public void onCheckedChanged(RadioGroup radioGroup, int checkedId) {
        int soundMode = 0;
        if (checkedId == SkinUtils.getId(R.id.asp_reverb_user)) {
            soundMode = ConstantEq.EQ_REVERB_USER;
        } else if (checkedId == SkinUtils.getId(R.id.asp_reverb_news)) {
            soundMode = ConstantEq.EQ_REVERB_NEWS;
        } else if (checkedId == SkinUtils.getId(R.id.asp_reverb_jazz)) {
            soundMode = ConstantEq.EQ_REVERB_JAZZ;
        } else if (checkedId == SkinUtils.getId(R.id.asp_reverb_city)) {
            soundMode = ConstantEq.EQ_REVERB_CITY;
        } else if (checkedId == SkinUtils.getId(R.id.asp_reverb_pop)) {
            soundMode = ConstantEq.EQ_REVERB_POP;
        } else if (checkedId == SkinUtils.getId(R.id.asp_reverb_electronic)) {
            soundMode = ConstantEq.EQ_REVERB_ELECTRONIC;
        } else if (checkedId == SkinUtils.getId(R.id.asp_reverb_classiz)) {
            soundMode = ConstantEq.EQ_REVERB_CLASSIZ;
        } else if (checkedId == SkinUtils.getId(R.id.asp_reverb_movie)) {
            soundMode = ConstantEq.EQ_REVERB_MOVIE;
        } else if (checkedId == SkinUtils.getId(R.id.asp_reverb_rock)) {
            soundMode = ConstantEq.EQ_REVERB_ROCK;
        } else if (checkedId == SkinUtils.getId(R.id.asp_reverb_techno)) {
            soundMode = ConstantEq.EQ_REVERB_TECHNO;
        } else {
            soundMode = checkedId;
        }
        if (soundMode > ConstantEq.EQ_REVERB_USER) {
            setAspSeekEnable(false);
            setAspSeekBarVal(DEF_ASP_BANDS[soundMode]);
            mBassVal = (int) Math.floor((DEF_ASP_BANDS[soundMode][0] + DEF_ASP_BANDS[soundMode][1] + DEF_ASP_BANDS[soundMode][2] +
                    DEF_ASP_BANDS[soundMode][3] + DEF_ASP_BANDS[soundMode][4] + DEF_ASP_BANDS[soundMode][5]) / 6);
            mTrebleVal = (int) Math.floor((DEF_ASP_BANDS[soundMode][6] + DEF_ASP_BANDS[soundMode][7] + DEF_ASP_BANDS[soundMode][8] +
                    DEF_ASP_BANDS[soundMode][9] + DEF_ASP_BANDS[soundMode][10] + DEF_ASP_BANDS[soundMode][11]) / 6);
            mRealData[0] = mBassVal;
            mRealData[1] = mTrebleVal;
            AspSettings.getInstance(mContext).setAspReverbType(soundMode, mRealData);
        } else {
            setAspSeekEnable(true);
            setAspSeekBarVal(mBandData);
            mRealData[0] = (mBandData[0] + mBandData[1] + mBandData[2] + mBandData[3]
                    + mBandData[4] + mBandData[5]) / 6;
            mRealData[1] = (mBandData[6] + mBandData[7] + mBandData[8] + mBandData[9]
                    + mBandData[10] + mBandData[11]) / 6;
            AspSettings.getInstance(mContext).setAspReverbType(soundMode, mRealData);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        int mAspReverb = AspSettings.getInstance(mContext).getAspReverbType();
        if (null != spReverb){
            spReverb.setPressed(true);
            spReverb.setSelection(mAspReverb);
        }
    }

    private void initReverb() {

        try {
            Field popupField = Spinner.class.getDeclaredField("mPopup");
            popupField.setAccessible(true);
            android.widget.ListPopupWindow popupWindow = (android.widget.ListPopupWindow) popupField.get(spReverb);
            popupWindow.setHeight(SkinUtils.getInteger(R.integer.asp_reverb_height));
            // 设置垂直偏移
            String skinName = SystemUtils.getSystemProperty(KEY_SKIN, "");
            if(EqUtils.KEY_SKIN_RK02.equals(skinName)){
                int offsetY = (int) TypedValue.applyDimension(
                        TypedValue.COMPLEX_UNIT_DIP,
                        5,
                        getResources().getDisplayMetrics()
                );
                popupWindow.setVerticalOffset(offsetY);
            }
        } catch (NoSuchFieldException e) {
            throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        }

        int mAspReverb = AspSettings.getInstance(mContext).getAspReverbType();

        bandItemList = new ArrayList<>();
        String[] v = SkinUtils.getStringArray(R.array.asp_reverb);
        int count = 0;
        if(v != null){
            count = v.length;
        }else{
            LogUtils.vTag(TAG,"IR get asp_reverb fail!");
        }
        for (int i = 0; i < count; i++) {
            AspItemBean bandItemBean = (AspItemBean) spReverb.getItemAtPosition(i);
            if (i == mAspReverb) {
                AspItemBean aspItemBean =  new AspItemBean(SkinUtils.getDrawable("extdsp_band_icon_select")
                        , v[i]);
                aspItemBean.setHide(true);
                bandItemList.add(aspItemBean);

            } else {
                bandItemList.add(new AspItemBean(null
                        , v[i]));
            }
        }

        AspAdapter adapter = new AspAdapter(bandItemList, mContext);
        spReverb.setAdapter(adapter);
        spReverb.setSelection(mAspReverb,true);
        spReverb.setSpinnerEventsListener(new CustomSpinner.OnSpinnerEventsListener() {
            @Override
            public void onSpinnerOpened(int lastSelectedItemPosition, int currentSelectedItemPosition) {
                spReverb.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_band_mode_opened_selector));

            }

            @Override
            public void onSpinnerClosed(int lastSelectedItemPosition, int currentSelectedItemPosition) {
                spReverb.setBackground(SkinUtils.getDrawable(R.drawable.extdsp_band_mode_closed_selector));
                spReverb.setPressed(true);
                spReverb.setSelection(currentSelectedItemPosition);
            }
        });
        spReverb.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                Log.d("TAG", "onItemSelected position : " + position + ", spinnerFromUser : ");

                for (int i = 0; i < spReverb.getCount(); i++) {
                    AspItemBean bandItemBean = (AspItemBean) spReverb.getItemAtPosition(i);
                    if (i == position) {
                        bandItemBean.setIcon(SkinUtils.getDrawable("extdsp_band_icon_select"));
                    } else {
                        bandItemBean.setIcon(null);
                    }
                }
                ImageView imageView = view.findViewById(R.id.tv_band_mode_select);
                if (imageView != null) {
                    imageView.setVisibility(View.INVISIBLE);
                }
//                spReverb.invalidate();


                if (position > ConstantEq.EQ_REVERB_USER) {
                    setAspSeekEnable(false);
                    setAspSeekBarVal(DEF_ASP_BANDS[position]);
                    mBassVal = (int) Math.floor((DEF_ASP_BANDS[position][0] + DEF_ASP_BANDS[position][1] + DEF_ASP_BANDS[position][2] +
                            DEF_ASP_BANDS[position][3] + DEF_ASP_BANDS[position][4] + DEF_ASP_BANDS[position][5]) / 6);
                    mTrebleVal = (int) Math.floor((DEF_ASP_BANDS[position][6] + DEF_ASP_BANDS[position][7] + DEF_ASP_BANDS[position][8] +
                            DEF_ASP_BANDS[position][9] + DEF_ASP_BANDS[position][10] + DEF_ASP_BANDS[position][11]) / 6);
                    mRealData[0] = mBassVal;
                    mRealData[1] = mTrebleVal;
                    AspSettings.getInstance(mContext).setAspReverbType(position, mRealData);
                } else {
                    setAspSeekEnable(true);
                    setAspSeekBarVal(mBandData);
                    mRealData[0] = (mBandData[0] + mBandData[1] + mBandData[2] + mBandData[3]
                            + mBandData[4] + mBandData[5]) / 6;
                    mRealData[1] = (mBandData[6] + mBandData[7] + mBandData[8] + mBandData[9]
                            + mBandData[10] + mBandData[11]) / 6;
                    AspSettings.getInstance(mContext).setAspReverbType(position, mRealData);
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
        spReverb.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                return false;
            }
        });

    }


}