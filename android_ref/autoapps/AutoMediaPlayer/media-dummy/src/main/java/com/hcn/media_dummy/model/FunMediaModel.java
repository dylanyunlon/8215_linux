package com.hcn.media_dummy.model;

/**
 * @author 65821
 */
public class FunMediaModel {
    private String mUrl;
    private String mTitle;

    public FunMediaModel(String url, String title) {
        mUrl = url;
        mTitle = title;
    }

    public String getUrl() {
        return mUrl;
    }

    public void setUrl(String url) {
        this.mUrl = url;
    }

    public String getTitle() {
        return mTitle;
    }

    public void setTitle(String title) {
        this.mTitle = title;
    }
}
