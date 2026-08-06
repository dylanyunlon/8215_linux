package com.autochips.bluetooth.view;

import android.content.Context;
import android.content.res.Configuration;
import android.content.res.TypedArray;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.ToggleButton;

import com.autochips.bluetooth.R;

public class HSettingsItemView extends RelativeLayout implements OnClickListener, OnCheckedChangeListener {
    /* <enum name="none" value="0"/>
    <enum name="choose" value="1"/>
    <enum name="goview" value="2"/>
    <enum name="expand" value="3"/>*/
    public final static int ITEM_TYPE_NORMAL = 0;
    public final static int ITEM_TYPE_CHOOSE = 1;
    public final static int ITEM_TYPE_GO_VIEW = 2;
    public final static int ITEM_TYPE_EXPAND = 3;

    private Context mContext;
    private String mTitleText = "";
    private String mStateText = "";
    private String mContentText = "";

    private View mBgContent;
    private View mExpandView;
    private ImageView mEnterIV;
    private ToggleButton mToggleButton;
    private TextView mTitleTV, mStateTV, mContentTV;

    private int mItemType;
    private boolean isChoose = false;
    private boolean isExpaned = false;
    private boolean isCanChangeColor = true;

    private OnItemViewClickListener mClickListener;
    private OnItemViewExpandChangedListener mExpandChangedListener;
    private OnItemViewCheckedChangedListener mCheckedChangedListener;

    /**
     * listener
     */
    public interface OnItemViewClickListener {
        void onItemViewClick(View v);
    }

    public interface OnItemViewCheckedChangedListener {
        void onItemViewCheckedChanged(View v, boolean isChecked);
    }

    public interface OnItemViewExpandChangedListener {
        void onItemViewExpand(View v, View expandView, boolean isExpand, boolean isFormUser);
    }


    public void setOnItemViewClickListener(OnItemViewClickListener listener) {
        mClickListener = listener;
    }

    public void setOnItemViewCheckedChangedListener(OnItemViewCheckedChangedListener listener) {
        mCheckedChangedListener = listener;
    }

    public void setOnItemViewExpandChangedListener(OnItemViewExpandChangedListener listener) {
        mExpandChangedListener = listener;
    }

    public HSettingsItemView(Context context) {
        this(context, null);
    }

    public HSettingsItemView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public HSettingsItemView(Context context, AttributeSet attrs,
                             int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        mContext = context;
        View.inflate(context, R.layout.settings_view_item, this);
        parseAttr(attrs);
        initView();
    }

    private void parseAttr(AttributeSet attrs) {
        if (attrs != null) {
            TypedArray attributes = mContext.obtainStyledAttributes(attrs, R.styleable.ItemStyle);
            mTitleText = attributes.getString(R.styleable.ItemStyle_item_title);
            mStateText = attributes.getString(R.styleable.ItemStyle_item_subtitle);
            mItemType = attributes.getInt(R.styleable.ItemStyle_item_type, 0);
            isChoose = attributes.getBoolean(R.styleable.ItemStyle_item_state, false);
            attributes.recycle();
        }
    }

    private void initView() {
        mBgContent = findViewById(R.id.rl_bg);
        mTitleTV = (TextView) findViewById(R.id.tv_item_title);
        mStateTV = (TextView) findViewById(R.id.tv_item_state);
        mContentTV = (TextView) findViewById(R.id.tv_item_content);
        mToggleButton = (ToggleButton) findViewById(R.id.tog_item);
        mEnterIV = (ImageView) findViewById(R.id.iv_item_enter);
        loadItemType();
    }

    private void loadItemType() {
        switch (mItemType) {
            case ITEM_TYPE_NORMAL:
                showView(mStateTV);
                hideView(mEnterIV);
                hideView(mToggleButton);
                break;
            case ITEM_TYPE_EXPAND:
            case ITEM_TYPE_GO_VIEW:
                showView(mEnterIV);
                showView(mStateTV);
                hideView(mToggleButton);
                this.setOnClickListener(this);
                break;
            case ITEM_TYPE_CHOOSE:
                setCanChangeColor(false);
                hideView(mStateTV);
                hideView(mEnterIV);
                showView(mToggleButton);
                setChooseState(isChoose);
                mToggleButton.setSoundEffectsEnabled(true);
                mToggleButton.setOnClickListener(this);
                mToggleButton.setOnCheckedChangeListener(this);
                break;
            default:
                break;
        }
        updateExpandViewState(false, false);
        mToggleButton.setEnabled(true);
        mStateTV.setText(mStateText);
        mTitleTV.setText(mTitleText);
        mContentTV.setText(mContentText);
    }

    private void updateExpandViewState(boolean isExpaned, boolean isFormUser) {
        if (isExpaned) {
            showView(mExpandView);
        } else {
            hideView(mExpandView);
        }
        setSelected(isExpaned);
        this.isExpaned = isExpaned;
        if (mItemType == ITEM_TYPE_EXPAND && mExpandChangedListener != null) {
            mExpandChangedListener.onItemViewExpand(this, mExpandView, isExpaned, isFormUser);
        }
    }

    public boolean isExpaned() {
        return this.isExpaned;
    }

    public void setExpandView(View v) {
        setExpandView(v, false);
    }

    /**
     * 定制需求，默认第一个item显示时，切换异常。不能正常隐藏
     *
     * @param v
     * @param expand
     */
    public void setExpandView(View v, boolean expand) {
        if (mExpandView != null && mExpandView == v) return;
        mExpandView = v;
        closeExpandView();
        //
        isExpaned = expand;
    }

    public void openExpandView() {
        if (/*mItemType==ITEM_TYPE_EXPAND*/!isExpaned) {
            updateExpandViewState(true, false);
        }
    }

    public void closeExpandView() {
        if (/*mItemType==ITEM_TYPE_EXPAND*/isExpaned) {
            updateExpandViewState(false, false);
        }
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        /*if (enabled) {
            mTitleTV.setTextColor(getResources().getColor(R.color.item_title_enable_color));
            mStateTV.setTextColor(getResources().getColor(R.color.item_state_enable_color));
            mContentTV.setTextColor(getResources().getColor(R.color.item_title_enable_color));
        } else {
            closeExpandView();
            if (isCanChangeColor) {
                mTitleTV.setTextColor(getResources().getColor(R.color.item_title_disenable_color));
                mStateTV.setTextColor(getResources().getColor(R.color.item_state_disenable_color));
                mContentTV.setTextColor(getResources().getColor(R.color.item_title_disenable_color));
            }
        }*/
    }

    @Override
    public void setSelected(boolean selected) {
        super.setSelected(selected);
        if (mItemType == ITEM_TYPE_CHOOSE) {
            mBgContent.setSelected(selected);
        }
    }

    public void showEnterView(boolean show) {
        if (show) {
            showView(mEnterIV);
        } else {
            hideView(mEnterIV);
        }
    }

    public void setTitleText(String title) {
        mTitleText = title;
        if (mTitleTV != null) {
            mTitleTV.setText(mTitleText);
        }
    }

    public void setContentText(String content) {
        mContentText = content;
        if (mContentTV != null) {
            mContentTV.setText(mContentText);
        }
    }

    public void setStateText(String state) {
        mStateText = state;
        if (mStateTV != null) {
            mStateTV.setText(mStateText);
        }
    }

    public void setStateText(int resid) {
        mStateText = mContext.getResources().getString(resid);
        if (mStateTV != null) {
            mStateTV.setText(mStateText);
        }
    }

    public void setTitleTextSize(float size) {
        if (mTitleTV != null) {
            mTitleTV.setTextSize(size);
        }
    }

    public void setContentTextSize(float size) {
        if (mContentTV != null) {
            mContentTV.setTextSize(size);
        }
    }

    public void setStateTextSize(float size) {
        if (mStateTV != null) {
            mStateTV.setTextSize(size);
        }
    }

    public void setContentVisible(int visible) {
        if (mContentTV != null) {
            mContentTV.setVisibility(visible);
        }
    }

    public void setContentFont(Typeface face) {
        if (mContentTV != null) {
            mContentTV.setTypeface(face);
        }
    }

    public void setCanChangeColor(boolean can) {
        isCanChangeColor = can;
    }

    public void setChooseState(boolean isChoose) {
        if (mItemType == ITEM_TYPE_CHOOSE) {
            mToggleButton.setChecked(isChoose);
        }
    }

    public boolean isChooseState() {
        return mToggleButton.isChecked();
    }

    public void setChooseViewEnable(final boolean isEnable) {
        if (mItemType == ITEM_TYPE_CHOOSE) {
            this.setEnabled(isEnable);
            mToggleButton.setEnabled(isEnable);
        }
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        switch (id) {
            default:
                if (mItemType == ITEM_TYPE_CHOOSE && mCheckedChangedListener != null) {
                    mCheckedChangedListener.onItemViewCheckedChanged(this, mToggleButton.isChecked());
                }
                if (mClickListener != null) {
                    if (mItemType != ITEM_TYPE_NORMAL && mItemType != ITEM_TYPE_EXPAND) {
                        mClickListener.onItemViewClick(this);
                        return;
                    }
                }

                if (/*mItemType==ITEM_TYPE_EXPAND*/!isExpaned) {
                    updateExpandViewState(!isExpaned, true);
                }
                break;
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        //if(mItemType==ITEM_TYPE_CHOOSE&&mCheckedChangedListener!=null){
        //	mCheckedChangedListener.onItemViewCheckedChanged(this, isChecked);
        //}
    }

    protected void hideView(View v) {
        if (v == null) return;
        if (v.getVisibility() != View.GONE) {
            v.setVisibility(View.GONE);
        }
    }

    protected void showView(View v) {
        if (v == null) return;
        if (v.getVisibility() != View.VISIBLE) {
            v.setVisibility(View.VISIBLE);
        }
    }

    protected void hideInView(View v) {
        if (v == null) return;
        if (v.getVisibility() != View.INVISIBLE) {
            v.setVisibility(View.INVISIBLE);
        }
    }

    @Override
    protected void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        /*if (mToggleButton != null) {
            mToggleButton.setBackground(getResources().getDrawable(R.drawable.tog_off_bg_selector));
        }
        if (mTitleTV != null) {
            mTitleTV.setTextColor(getResources().getColor(R.color.item_text_normal_color));
            mStateTV.setTextColor(getResources().getColor(R.color.item_text_normal_color));
            mContentTV.setTextColor(getResources().getColor(R.color.item_text_normal_color));
        }*/
    }

}
