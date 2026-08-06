package com.hcn.media_data;

import com.hcn.media_data.base.UserListScene;
import com.hcn.media_data.ui.MediaPageState;
import com.hcn.media_data.ui.base.PageDataKV;

public class ListSceneManager {
    private static ListSceneManager sInstance;
    private final UserListScene mUserListScene;
    private boolean mIsSupportListMemory;

    public static ListSceneManager getInstance() {
        if (sInstance == null) {
            sInstance = new ListSceneManager();
        }
        return sInstance;
    }

    private ListSceneManager(){
        mUserListScene = new UserListScene(UserListScene.SceneType.NORMAL, "");
    }

    public void setSupportListMemory(boolean isSupport) {
        mIsSupportListMemory = isSupport;
    }

    public void updateListSceneType(int type) {
        mUserListScene.mType = type;
    }

    public void updateListScenePath(String path) {
        mUserListScene.mPath = path;
    }

    public int getListSceneType() {
        return mUserListScene.mType;
    }

    public String getListScenePath() {
        return mUserListScene.mPath;
    }

    public void saveUserListScene(int actionSceneValue, String folderPath) {
        if (mIsSupportListMemory) {
            MediaPageState.instance().write(PageDataKV.Key.CURRENT_LIST_ACTION_SCENE, actionSceneValue);
            MediaPageState.instance().write(PageDataKV.Key.MUSIC_FOLDER_PATH, folderPath);
        }
    }

    public int readActionScene() {
        return MediaPageState.instance().readInt(PageDataKV.Key.CURRENT_LIST_ACTION_SCENE, PageDataKV.ActionSceneValue.NORMAL);
    }

    public String readFolderPath() {
        return MediaPageState.instance().readString(PageDataKV.Key.MUSIC_FOLDER_PATH);
    }
}
