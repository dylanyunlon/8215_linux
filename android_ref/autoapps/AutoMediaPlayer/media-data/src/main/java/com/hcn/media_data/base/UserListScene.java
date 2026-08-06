package com.hcn.media_data.base;

public class UserListScene {
    public int mType;
    public String mPath;

    public UserListScene(int type, String path) {
        mType = type;
        mPath = path;
    }

    public interface SceneType {
        int NORMAL = 0;
        int FOLDER = 1;
    }
}
