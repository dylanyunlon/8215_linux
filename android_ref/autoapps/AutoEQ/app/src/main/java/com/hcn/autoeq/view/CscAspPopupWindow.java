package com.hcn.autoeq.view;

import android.view.View;
import android.widget.PopupWindow;

public class CscAspPopupWindow extends PopupWindow {

    private OnCscAspopupListener onUpdateListener;

    public interface OnCscAspopupListener {
        //更新PopupWindow内容
        void UpdatePopupContent();

        //监听悬浮窗的开启和关闭
        void openOrCloseListener(boolean isOpenStatus);
    }

    public CscAspPopupWindow(View contentView, int width, int height, boolean focusable) {
        super(contentView, width, height, focusable);
    }

    public void setOnDspPopupListener(OnCscAspopupListener onItemClickListener) {
        this.onUpdateListener = onItemClickListener;
    }

    @Override
    public void showAsDropDown(View anchor, int xoff, int yoff, int gravity) {
        super.showAsDropDown(anchor, xoff, yoff, gravity);
        if (onUpdateListener != null) {
            onUpdateListener.UpdatePopupContent();
            onUpdateListener.openOrCloseListener(true);
        }
    }
    public void UpdatePopupWindow() {
        if (onUpdateListener != null) {
            onUpdateListener.UpdatePopupContent();
            onUpdateListener.openOrCloseListener(true);
        }
    }

    @Override
    public void dismiss() {
        super.dismiss();
        if (onUpdateListener != null) {
            onUpdateListener.openOrCloseListener(false);
        }
    }

    public void close(CscAspPopupWindow mPopupWindow) {
        if (mPopupWindow != null && mPopupWindow.isShowing()) {
            mPopupWindow.dismiss();
        }
    }


}
