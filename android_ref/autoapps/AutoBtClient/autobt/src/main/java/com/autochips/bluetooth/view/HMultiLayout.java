package com.autochips.bluetooth.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.SoundEffectConstants;
import android.widget.LinearLayout;

import androidx.annotation.Nullable;

public class HMultiLayout extends LinearLayout {

    public HMultiLayout(Context context) {
        super(context);
    }

    public HMultiLayout(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public HMultiLayout(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public HMultiLayout(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }


    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        return super.onInterceptTouchEvent(ev);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if(event.getAction() == MotionEvent.ACTION_DOWN){
            setFocus(true);
            return true;
        }else if(event.getAction() == MotionEvent.ACTION_CANCEL){
            setFocus(false);
        }else if(event.getAction() == MotionEvent.ACTION_UP){
            setFocus(false);
            if(listen != null){
                listen.onClick(this);
                playSoundEffect(SoundEffectConstants.CLICK);
            }
        }

        return super.onTouchEvent(event);
    }
    private OnClickListener listen = null;
    @Override
    public void setOnClickListener(@Nullable OnClickListener l) {
        //super.setOnClickListener(l);
        listen = l;
    }

    private void setFocus(boolean focus){
        int count = getChildCount();
        this.setSelected(focus);
        if(count > 0){
            for(int i = 0;i< count ;i++){
                getChildAt(i).setFocusable(focus);

            }
        }
    }
}
