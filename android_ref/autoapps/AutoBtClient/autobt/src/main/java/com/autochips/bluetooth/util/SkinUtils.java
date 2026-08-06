package com.autochips.bluetooth.util;

import android.content.Context;
import android.content.res.ColorStateList;
import android.view.View;

import com.autochips.bluetooth.BaseApplication;
import com.hcn.skin.support.resources.SkinCompatResources;

public interface SkinUtils {

    // view : app view， 从皮肤包里找对应view的id，找不到就返回app view的id
    static int getViewId(View view) {
        return SkinCompatResources.getInstance().getViewId(view);
    }

    // R.id.
    static int getId(int id) {
        return SkinCompatResources.getInstance().getId(id);
    }

    static int getResId(int id) {
        if(useSkinPackage()) {
            return SkinCompatResources.getInstance().getSkinResId(id);
        }
        return id;
    }

    /**
     * 获取皮肤包里面的String
     *
     * @param resId
     * @return
     */
    public static String getString(int resId) {
        if (useSkinPackage()) {
            return SkinCompatResources.getInstance().getString(resId);
        }
        return getContext().getString(resId);
    }

    // R.layout.
    static View getLayout(int resId) {
        return SkinCompatResources.getInstance().getLayout(resId);
    }

    static int getColor(int resId){
        if(useSkinPackage()){
            return SkinCompatResources.getInstance().getColor(resId);
        }
        return getContext().getColor(resId);
    }

    static ColorStateList getColorStateList(int resId){
        if(useSkinPackage()){
            return SkinCompatResources.getInstance().getColorStateList(resId);
        }
        return getContext().getColorStateList(resId);
    }

    /**
     * 是否使用了皮肤包
     *
     * @return
     */
    public static boolean useSkinPackage() {
        return SkinCompatResources.getInstance().getSkinResources() != null;
    }

    static Context getContext(){
        return BaseApplication.getInstance().getApplicationContext();
    }
}
