package com.hcn.autoeq.bean;

import android.graphics.drawable.Drawable;

public class AspItemBean {
    private Drawable icon;
    private String name;

    private boolean hide;

    public AspItemBean(Drawable icon, String name) {
        this.icon = icon;
        this.name = name;
        this.hide = false;
    }

    public Drawable getIcon() {
        return icon;
    }

    public String getName() {
        return name;
    }

    public void setHide(boolean hide) {
        this.hide = hide;
    }

    public boolean getHide(){
        return hide;
    }

    public void setIcon(Drawable icon) {
        this.icon = icon;
    }

    public void seName(String name) {
        this.name = name;
    }
}
