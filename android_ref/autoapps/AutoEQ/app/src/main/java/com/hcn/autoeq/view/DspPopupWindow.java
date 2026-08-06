package com.hcn.autoeq.view;

import android.view.View;
import android.widget.PopupWindow;

public class DspPopupWindow extends PopupWindow {

    private OnDspPopupListener onUpdateListener;

    public interface OnDspPopupListener {
        //更新PopupWindow内容
        void UpdatePopupContent();

        //监听悬浮窗的开启和关闭
        void openOrCloseListener(boolean isOpenStatus);
    }

    public DspPopupWindow(View contentView, int width, int height, boolean focusable) {
        super(contentView, width, height, focusable);
    }

    public void setOnDspPopupListener(OnDspPopupListener onItemClickListener) {
        this.onUpdateListener = onItemClickListener;
    }

    @Override
    public void showAsDropDown(View anchor, int xoff, int yoff, int gravity) {
        super.showAsDropDown(anchor, xoff, yoff, gravity);
        onUpdateListener.UpdatePopupContent();
        onUpdateListener.openOrCloseListener(true);
    }


    @Override
    public void dismiss() {
        super.dismiss();
        onUpdateListener.openOrCloseListener(false);
    }

    public void close(DspPopupWindow mPopupWindow) {
        if (mPopupWindow != null && mPopupWindow.isShowing()) {
            mPopupWindow.dismiss();
        }
    }
}
