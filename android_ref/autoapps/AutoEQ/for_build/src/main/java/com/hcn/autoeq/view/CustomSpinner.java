package com.hcn.autoeq.view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;
import android.widget.Spinner;

/**
 * 自定义下拉框，增加监听展开和折叠事件
 * <p>
 * 注意：如果继承 androidx 的 Spinner，去掉列表项的点击效果做法会不一样
 */
@SuppressLint("AppCompatCustomView")
public class CustomSpinner extends Spinner {

    private static final String TAG = "CustomSpinner";
    private OnSpinnerEventsListener mListener;
    private boolean mOpenInitiated = false;
    private int lastSelectedItemPosition;

    public CustomSpinner(Context context, AttributeSet attrs, int defStyleAttr, int mode) {
        super(context, attrs, defStyleAttr, mode);
    }

    public CustomSpinner(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public CustomSpinner(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public CustomSpinner(Context context, int mode) {
        super(context, mode);
    }

    public CustomSpinner(Context context) {
        super(context);
    }

    public interface OnSpinnerEventsListener {
        void onSpinnerOpened(int lastSelectedItemPosition, int currentSelectedItemPosition);

        void onSpinnerClosed(int lastSelectedItemPosition, int currentSelectedItemPosition);
    }

    @Override
    public boolean performClick() {
        // register that the Spinner was opened so we have a status
        // indicator for the activity(which may lose focus for some other
        // reasons)
        mOpenInitiated = true;
        if (mListener != null) {
            lastSelectedItemPosition = getSelectedItemPosition();
            mListener.onSpinnerOpened(lastSelectedItemPosition, getSelectedItemPosition());
        }
        return super.performClick();
    }

    public void setSpinnerEventsListener(OnSpinnerEventsListener onSpinnerEventsListener) {
        mListener = onSpinnerEventsListener;
    }

    /**
     * Propagate the closed Spinner event to the listener from outside.
     */
    public void performClosedEvent() {
        mOpenInitiated = false;
        if (mListener != null) {
            mListener.onSpinnerClosed(lastSelectedItemPosition, getSelectedItemPosition());
        }
    }

    /**
     * A boolean flag indicating that the Spinner triggered an open event.
     *
     * @return true for opened Spinner
     */
    public boolean hasBeenOpened() {
        return mOpenInitiated;
    }

    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        super.onWindowFocusChanged(hasWindowFocus);
        if (hasBeenOpened() && hasWindowFocus) {
            performClosedEvent();
        }
    }

}