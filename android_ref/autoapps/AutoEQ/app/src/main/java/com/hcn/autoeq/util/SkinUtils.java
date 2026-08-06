package com.hcn.autoeq.util;

import static com.hcn.autoeq.util.EqUtils.KEY_SKIN;

import android.app.Application;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.blankj.utilcode.util.StringUtils;
import com.hcn.skin.support.SkinCompatManager;
import com.hcn.skin.support.resources.SkinCompatResources;
import com.hcn.skin2.Skin2;
import com.hcn.skin2.enhance.IOverlayCallback;

import java.util.HashSet;
import java.util.Set;

public interface SkinUtils {
    String TAG = "SkinUtils";
    String SETTING_SKINS_PACKAGE_PREFIX = "com.hcn.autoeq.skin.";
    String SETTING_SKINS_APK_PREFIX = "HEQSkins_";
    Set<String> subClassOfTextView = new HashSet<>();
    static Context getContext() {
        return SkinCompatResources.getInstance().getContext();
    }

    /*
    当：app 里有：
        type=id,name=a,id=123
        type=string,name=a,id=456
    skin 里有：
        type=string,name=b,id=123

    SkinCompatResources.getInstance().getViewId(View view)的逻辑：
    会找到 skin 里 type=string,name=b,id=123，然后返回 app 里 type=string,name=a,id=456

    所以重新写这个方法，固定 type = "id"，皮肤包里就找不到了，就直接返回 app 的
     */
    static int getViewId(View v) {
        return SkinCompatResources.getInstance().getViewId(v);
    }

    // R.id.
    static int getId(int id) {
        return Skin2.xId(id);
    }

    // R.drawable.
    static Drawable getDrawable(int id) {
        return SkinCompatResources.getInstance().getDrawable(id);
    }

    // R.drawable.
    static Drawable getDrawable(String name) {
        return SkinCompatResources.getInstance().getDrawable(name);
    }

    // R.layout.
    static View inflate(int resId){
        return inflate(resId,null);
    }
    static View inflate(int resId,ViewGroup viewParent) {
        return inflate(resId,viewParent,false);
    }
    static View inflate(int resId, ViewGroup viewParent, boolean attachParent){
        return Skin2.inflate(resId,viewParent,attachParent);
    }
    // R.string.
    static CharSequence getText(int id) {
        return SkinCompatResources.getInstance().getText(id);
    }

    static String getString(int resId){
        return SkinCompatResources.getInstance().getString(resId);
    }

    // R.color.
    static int getColor(int resId) {
        return SkinCompatResources.getInstance().getColor(resId);
    }

    // R.dimen.
    static float getDimension(int id) {
        return SkinCompatResources.getInstance().getDimension(id);
    }

    // R.integer.
    static int getInteger(int id) {
        return SkinCompatResources.getInstance().getInteger(id);
    }

    //R.array string
    static String[] getStringArray(int resId){
        return Skin2.getStringArray(resId);
    }

    //R.array int
    static int[] getIntArray(int resId){
        return Skin2.getIntArray(resId);
    }

    static void init(Application application){
        Skin2.init(application);
        SkinCompatManager skinCompatManager = SkinCompatManager.getInstance();
        String skinName = EqUtils.getSkinName();
        Log.d(TAG, "loadSkinSync skinName : " + skinName);
        if (!StringUtils.isTrimEmpty(skinName)) {
            // 必须使用同步，如果是使用异步方式，会先显示默认ui，再同时加载皮肤，会导致界面看起来闪
            String skinNameLoaded = skinCompatManager.loadSkinSync(SETTING_SKINS_APK_PREFIX + skinName + ".apk");
            if (skinNameLoaded == null || "".equals(skinNameLoaded)) {
                String packageName = SETTING_SKINS_PACKAGE_PREFIX + skinName;
                Log.d(TAG, "loadSkinByPackage skinName : " + skinName);
                skinCompatManager.loadSkinByPackage(packageName);
            }
        }
        subClassOfTextView.add("com.hcn.autoeq.view.DrawableCenterRadioButton");
        Skin2.instance().rroCallback(new IOverlayCallback() {
            @Override
            public boolean isSupportRROView(String s) {
                return subClassOfTextView.contains(s);
            }

            @Override
            public View tryReplaceAndCreateView(String s, @NonNull Context context, @NonNull AttributeSet attributeSet) {
                return null;
            }
        });
    }
}
